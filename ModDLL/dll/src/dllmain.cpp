#include "../pch.h"
#include "headers/dllmain.h"
#include "headers/Halo1.h"
#include "headers/Halo1Mod.h"
#include "headers/HaloMCC.h"
#include "headers/Overlay.h"
#include "headers/Tracers.h"
#include "headers/Hook.h"
#include "utils/headers/common.h"
#include "utils/headers/Config.h"
#include "utils/headers/Vec.h"
#include "utils/headers/BytePattern.h"
#include "graphics/headers/DX11Hook.h"
#include "graphics/headers/Renderer.h"
#include "timehack/headers/TimeHack.h"

static HMODULE hmSuperHotHack;

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    switch ( ul_reason_for_call ) {
        case DLL_PROCESS_ATTACH:
            hmSuperHotHack = hModule;
            CreateThread( 0, 0, mainThread, 0, 0, 0 );
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

static bool exitMod = false;
bool checkExit() {
    if ( GetAsyncKeyState( VK_F9 ) )
        exitMod = true;
    return exitMod;
}

void renderLoadedText( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
    static std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>( pDevice, 1024 );

    renderer->begin();

    static uint64_t startTick = GetTickCount64();
    uint64_t tick = GetTickCount64();
    uint64_t uptime = tick - startTick;
    float fUptimeSeconds = (float) uptime / 1000.0f;

    const float flashAmplitude = 0.5f;
    const float flashFreq = 2.0f;
    float alpha = sinf( fUptimeSeconds * flashFreq ) * flashAmplitude + ( 1 - flashAmplitude );
    Vec4 color = { 1.0f, 1.0f, 1.0f, alpha };

    auto size = HaloMCC::getWindowSize();
    Vec2 pos = { size.x / 2, 0 };

    renderer->drawText( pos, "SUPERHOT MCC Loaded", color, FW1_CENTER | FW1_TOP, 16.0f, nullptr );

    renderer->end();
}

DWORD __stdcall mainThread( LPVOID lpParameter ) {

    const bool useConsole = true;
    const bool useStdin = false;
    const bool printToLog = false;
    const char* logFile = "C:\\Users\\Kody\\Desktop\\log.txt";

    errno_t err = 0;
    FILE* pFile_stdout, * pFile_stderr, * pFile_stdin;
    if ( useConsole ) {
        AllocConsole();
        if ( printToLog )
            err = freopen_s( &pFile_stdout, logFile, "w", stdout );
        else
            err = freopen_s( &pFile_stdout, "CONOUT$", "w", stdout );
        err |= freopen_s( &pFile_stderr, "CONOUT$", "w", stderr );
        if ( useStdin )
            err |= freopen_s( &pFile_stdin, "CONIN$", "r", stdin );
    }

    // Bring MCC to front.
    HWND mccWindow = HaloMCC::getWindow();
    BringWindowToTop( mccWindow );
    // SetForegroundWindow( mccWindow );
    if ( useConsole ) {
        HWND consoleWindow = GetConsoleWindow();
        BringWindowToTop( consoleWindow );
    }

    auto pSwapChain = HaloMCC::getSwapChainPointer();
    if ( !pSwapChain ) {
        std::cout << "Could not find swap chain!\n";
        err = E_POINTER;
    } else {
        auto hr = DX11Hook::hook( mccWindow, pSwapChain );
        if ( FAILED( hr ) )
            err = hr;
        else
            DX11Hook::addOnPresentCallback( renderLoadedText );
    }

    if ( !err ) {
        while ( !checkExit() ) {
            while ( !checkExit() && !HaloMCC::isInGame() )
                Sleep( 100 );

            if ( !Halo1Mod::init() )
                exitMod = true;

            while ( !checkExit() && HaloMCC::isInGame() ) {
                Halo1Mod::onDllThreadUpdate();
                Sleep( 10 );
            }
            Halo1Mod::cleanup();
        }
    }

    DX11Hook::cleanup();

    // Give any other executing hook code a moment to finish before unloading.
    // It might be smart to eventually wrap all hooks with a mutex lock / unlock.
    Sleep( 500 );

    if ( useConsole ) {
        if ( pFile_stdout ) fclose( pFile_stdout );
        if ( pFile_stderr ) fclose( pFile_stderr );
        if ( useStdin && pFile_stdin ) fclose( pFile_stdin );
        FreeConsole();
    }

    FreeLibraryAndExitThread( hmSuperHotHack, 0 );

}
