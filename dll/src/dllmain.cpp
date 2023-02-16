#include "../pch.h"
#include "./headers/dllmain.h"
#include "./headers/Halo1.h"
#include "./graphics/headers/DX11Hook.h"
#include "./graphics/headers/DX11Utils.h"
#include "./graphics/headers/Renderer.h"
#include "./headers/Hook.h"
#include "./utils/headers/AllocationUtils.h"
#include "./utils/headers/MathUtils.h"
#include "./utils/headers/keypressed.h"
#include "./utils/headers/Vec.h"

using MathUtils::randf;

HMODULE hmSuperHotHack;
HMODULE hmHalo1;
UINT_PTR halo1Base;
HWND mccWindow;

Halo1::DeviceContainer* pDeviceContainer;

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    switch ( ul_reason_for_call ) {
        case DLL_PROCESS_ATTACH:
            hmSuperHotHack = hModule;
            hmHalo1 = GetModuleHandleA( "halo1.dll" );
            halo1Base = (UINT_PTR) hmHalo1;
            CreateThread( 0, 0, mainThread, 0, 0, 0 );
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

BOOL CALLBACK getMCCWindow_enumWindowsProc( HWND hwnd, LPARAM lParam ) {
    char name[255] = {};
    auto size = GetWindowTextA( hwnd, name, 255 );
    if ( size == 0 )
        return true;
    if ( strncmp( name, "Halo: The Master Chief Collection  ", size ) == 0 ) {
        HWND* pHwndResult = (HWND*) lParam;
        *pHwndResult = hwnd;
        return false;
    }
    return true;
}
HWND getMCCWindow() {
    HWND hwnd;
    EnumWindows( getMCCWindow_enumWindowsProc, (LONG_PTR) &hwnd );
    return hwnd;
}

bool printEntityData( Halo1::EntityRecord* pRecord ) {
    auto pEntity = Halo1::getEntityPointer( pRecord );
    if ( !pEntity )
        return true;
    if ( pEntity->health <= 0 )
        return true;
    std::cout << "Position: ";
    Vec::print( pEntity->pos );
    // pEntity->pos.print();
    std::cout << "\n";
    std::cout << "Type ID: " << pRecord->typeId;
    std::cout << ", Health: " << pEntity->health;
    std::cout << ", Shield: " << pEntity->shield << "\n\n";
    return true;
}

void printEntities() {
    auto pEntityList = Halo1::getEntityListPointer();
    if ( pEntityList ) {
        std::cout << "Entity list at: " << pEntityList << "\n";
        std::cout << "Entities: \n\n";
        Halo1::foreachEntityRecord( printEntityData );
    }
}

float clippingNear = 0.063f;
float clippingFar = 200.0f;
HRESULT halo1CameraMatrix( float w, float h, XMMATRIX& result ) {
    auto pCam = Halo1::getPlayerCameraPointer();
    if ( !pCam )
        return E_FAIL;
    result = cameraMatrix(
        pCam->pos, pCam->fwd,
        pCam->fov,
        clippingNear, clippingFar,
        w, h
    );
    return S_OK;
}

Renderer* renderer;

bool drawRandomTriangleOnEntity( Halo1::EntityRecord* rec ) {
    Vec4 red = { 1.0f, 0.0f, 0.0f, 0.25f };
    Vec4 green = { 0.0f, 1.0f, 0.0f, 0.25f };
    Vec4 blue = { 0.0f, 0.0f, 1.0f, 0.25f };
    Vertex verticies[3] = { {{}, red }, {{}, green}, {{}, blue} };

    auto entity = Halo1::getEntityPointer( rec );
    auto pos = entity->pos;
    for ( int i = 0; i < 3; i++ ) {
#define RAND ( randf() * 2.0f - 1.0f )
        verticies[i].pos.x = pos.x + RAND;
        verticies[i].pos.y = pos.y + RAND;
        verticies[i].pos.z = pos.z + RAND;
#undef RAND
    }

    renderer->pushVerticies( ARRAYSIZE( verticies ), verticies );

    return true;
}

bool rendererInit = false;
DWORD startTime;
void testRender( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
    if ( !rendererInit ) {
        std::cout << "Constructing new Renderer.\n";
        renderer = new Renderer( pDevice, 4096 );
        rendererInit = true;
        startTime = GetTickCount();
    }

    RECT rect;
    GetClientRect( mccWindow, &rect );
    float w = (float) ( rect.right - rect.left );
    float h = (float) ( rect.bottom - rect.top );

    // XMMATRIX transform = XMMatrixScaling( h / w * .5f, 1.0f * .5f, 1.0f );
    XMMATRIX transform;
    if ( FAILED( halo1CameraMatrix( w, h, transform ) ) )
        return;

    // system( "CLS" );
    // Vec::print( transform );

    renderer->setTransform( &transform );

    fitViewportToWindow( pCtx, mccWindow );
    renderer->setPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
    renderer->begin();

    Vec4 red = { 1.0f, 0.0f, 0.0f, 0.25f };
    Vec4 green = { 0.0f, 1.0f, 0.0f, 0.25f };
    Vec4 blue = { 0.0f, 0.0f, 1.0f, 0.25f };

    float t = ( ( GetTickCount() - startTime ) ) / 1000.0f;
    float angle = (float) M_PI * 2.0f / 3.0f;

    // std::cout << t << "\n";

    Vertex verticies[3] = { {{}, red }, {{}, green}, {{}, blue} };

    Halo1::foreachEntityRecord( drawRandomTriangleOnEntity );

    // int numTris = 5;
    // float s = sinf( t );
    // float scale = 1.0f;
    // for ( int j = 0; j < numTris; j++ ) {
    //     for ( int i = 0; i < 3; i++ )
    //         verticies[i].pos = { sinf( i * angle + t ) * s * scale, cosf( i * angle + t ) * scale, 0.99f };
    //     renderer->pushVerticies( ARRAYSIZE( verticies ), verticies );
    //     t += angle / numTris;
    // }

    // Vec4 color = { 1.0f, 1.0f, 1.0f, 0.25f };
    // renderer->drawText( { 500, 100 }, L"Hello Text!", color, NULL, 100.0f, nullptr );

    renderer->flush();
    renderer->end();

}

DWORD __stdcall mainThread( LPVOID lpParameter ) {

    const bool useConsole = true;
    const bool useStdin = false;
    const bool pressKeyToExit = false;

    const char* logFile = "C:\\Users\\Kody\\Desktop\\log.txt";

    errno_t err = 0;
    FILE* pFile_stdout, * pFile_stderr, * pFile_stdin;
    if ( useConsole ) {
        AllocConsole();
        // err = freopen_s( &pFile_stdout, logFile, "w", stdout );
        err = freopen_s( &pFile_stdout, "CONOUT$", "w", stdout );
        err |= freopen_s( &pFile_stderr, "CONOUT$", "w", stderr );
        if ( useStdin )
            err |= freopen_s( &pFile_stdin, "CONIN$", "r", stdin );
    }

    mccWindow = getMCCWindow();

    // Bring MCC to front.
    BringWindowToTop( mccWindow );
    SetForegroundWindow( mccWindow );

    std::cout << "MCC-SUPERHOT Mod Loaded\n\n";

    std::cout << "halo1.dll located at: ";
    std::cout << std::uppercase << std::hex << hmHalo1;
    std::cout << "\n\n";

    Halo1::init( halo1Base );
    pDeviceContainer = Halo1::getDeviceContainerPointer();
    if ( pDeviceContainer ) {
        std::cout << "Device container at: " << pDeviceContainer << std::endl;
        std::cout << "Device at: " << pDeviceContainer->pDevice << std::endl;
    } else {
        std::cout << "Device container not found!" << std::endl;
    }

    printEntities();

    DX11Hook::hook( mccWindow );
    DX11Hook::addOnPresentCallback( testRender );

    if ( !err ) {
        while ( !GetAsyncKeyState( VK_F9 ) ) {

            if ( keypressed( VK_F4 ) ) {
                auto pCam = Halo1::getPlayerCameraPointer();
                std::cout << "Fov: " << pCam->fov << "\n";
                std::cout << "Pos: "; Vec::print( pCam->pos ); std::cout << "\n";
                std::cout << "Fwd: "; Vec::print( pCam->fwd ); std::cout << "\n";
            }

            Sleep( 10 );
        }
    }

    std::cout << "Exiting..." << std::endl;

    DX11Hook::cleanup();
    Hook::cleanupHooks();
    if ( renderer )
        renderer->~Renderer();

    Sleep( 500 );

    if ( useConsole ) {
        if ( useStdin && pressKeyToExit ) {
            std::cout << std::endl << "Press anything to continue" << std::endl;
            getchar();
        }

        if ( pFile_stdout ) fclose( pFile_stdout );
        if ( pFile_stderr ) fclose( pFile_stderr );
        if ( useStdin && pFile_stdin ) fclose( pFile_stdin );
        FreeConsole();
    }

    FreeLibraryAndExitThread( hmSuperHotHack, 0 );

}
