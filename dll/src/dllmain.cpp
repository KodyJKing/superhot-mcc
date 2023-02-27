#include "../pch.h"
#include "headers/dllmain.h"
#include "headers/Halo1.h"
#include "headers/HaloMCC.h"
#include "headers/Overlay.h"
#include "headers/Hook.h"
#include "utils/headers/common.h"
#include "utils/headers/Vec.h"
#include "graphics/headers/DX11Hook.h"
#include "timehack/headers/TimeHack.h"

HMODULE hmSuperHotHack;
HMODULE hmHalo1;
UINT_PTR halo1Base;
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

void onPresent( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain ) {
    Overlay::render( pCtx, pDevice, pSwapChain );
    TimeHack::onGameThreadUpdate();
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


    // Bring MCC to front.
    HWND mccWindow = HaloMCC::getWindow();
    BringWindowToTop( mccWindow );
    SetForegroundWindow( mccWindow );

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

            bool exit = GetAsyncKeyState( VK_F9 )
                || !GetModuleHandleA( "halo1.dll" );
            if ( exit )
                break;

            TimeHack::onDllThreadUpdate();
            Overlay::onDllThreadUpdate();

            // if ( keypressed( 'H' ) )
            //     std::cout << "Player handle: " << Halo1::getPlayerHandle() << "\n";

            if ( keypressed( VK_DELETE ) )
                system( "CLS" );

            Sleep( 10 );

        }
    }

    std::cout << "Exiting..." << std::endl;

    DX11Hook::cleanup();
    Hook::cleanupHooks();
    Overlay::cleanup();

    // Give any executing hook code a moment to finish before unloading.
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
