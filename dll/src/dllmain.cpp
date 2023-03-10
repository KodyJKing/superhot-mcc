#include "../pch.h"
#include "headers/dllmain.h"
#include "headers/Halo1.h"
#include "headers/HaloMCC.h"
#include "headers/Overlay.h"
#include "headers/Tracers.h"
#include "headers/Hook.h"
#include "utils/headers/common.h"
#include "utils/headers/Vec.h"
#include "graphics/headers/DX11Hook.h"
#include "graphics/headers/Renderer.h"
#include "timehack/headers/TimeHack.h"

static HMODULE hmSuperHotHack;
static HMODULE hmHalo1;
static UINT_PTR halo1Base;
static std::mutex onRenderMutex;

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

void onPresent( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
    if ( onRenderMutex.try_lock() ) {

        static std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>( pDevice, 4096 );

        if ( Halo1::isGameLoaded() ) {
            Overlay::render( renderer.get(), pCtx, pDevice, pSwapChain );
            Tracers::render( renderer.get(), TimeHack::timeElapsed, pCtx, pDevice, pSwapChain );
            TimeHack::onGameThreadUpdate();
        }

        onRenderMutex.unlock();
    }
}

DWORD __stdcall mainThread( LPVOID lpParameter ) {

    const bool useConsole = true;
    const bool useStdin = false;
    const bool pressKeyToExit = false;

    const char* logFile = "C:\\Users\\Kody\\Desktop\\log.txt";
    const bool printToLog = false;

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

    std::cout << "MCC-SUPERHOT Mod Loaded\n\n";

    std::cout << "halo1.dll located at: ";
    std::cout << std::uppercase << std::hex << hmHalo1;
    std::cout << "\n\n";

    Halo1::init( halo1Base );

    DX11Hook::hook( mccWindow );
    DX11Hook::addOnPresentCallback( onPresent );

    TimeHack::init( halo1Base );

    if ( !err ) {
        while ( true ) {

            if ( GetAsyncKeyState( VK_F9 ) || !GetModuleHandleA( "halo1.dll" ) )
                break;

            if ( Halo1::isGameLoaded() ) {
                TimeHack::onDllThreadUpdate();
                Overlay::onDllThreadUpdate();
            }

            Sleep( 10 );

        }
    }

    std::cout << "Exiting..." << std::endl;

    DX11Hook::cleanup();
    Hook::cleanupHooks();

    // Wait until we're done rendereing before exiting and letting renderer destruct.
    onRenderMutex.lock();

    // Give any other executing hook code a moment to finish before unloading.
    // It might be smart to eventually wrap all hooks with a mutex lock / unlock.
    Sleep( 500 );

    std::cout << "Freeing console and library.\n";

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

    onRenderMutex.unlock();

    FreeLibraryAndExitThread( hmSuperHotHack, 0 );

}
