#include "../pch.h"
#include "./headers/dllmain.h"
#include "./headers/Halo1.h"
#include "./graphics/headers/DX11Hook.h"
#include "./graphics/headers/DX11HookTest.h"
#include "./graphics/headers/DX11Utils.h"
#include "./headers/Hook.h"
#include "./utils/headers/AllocationUtils.h"
#include "./utils/headers/keypressed.h"

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
    std::cout << "Position: ";
    pEntity->pos.print();
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

// bool rendererInit = false;
// void testRender( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
//     if ( !rendererInit ) {
//         rendererInit = true;
//     }
//     fitViewportToWindow( pCtx, mccWindow );
// }

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

    // printEntities();

    DX11Hook::hook( mccWindow );
    DX11HookTest::init();
    // DX11Hook::addOnPresentCallback( testRender );

    if ( !err ) {
        while ( !GetAsyncKeyState( VK_F9 ) ) {

            if ( keypressed( VK_F4 ) ) {
                auto pCam = Halo1::getPlayerCameraPointer();
                std::cout << "Fov: " << pCam->fov << "\n";
            }

            Sleep( 10 );
        }
    }

    std::cout << "Exiting..." << std::endl;

    DX11HookTest::cleanup();
    DX11Hook::cleanup();
    Hook::cleanupHooks();

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
