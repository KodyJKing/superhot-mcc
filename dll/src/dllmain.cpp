#include "../pch.h"
#include "./headers/dllmain.h"
#include "./headers/Halo1.h"
#include "./graphics/headers/DX11Hook.h"
#include "./graphics/headers/DX11HookTest.h"
#include "./headers/Hook.h"
#include "./utils/headers/AllocationUtils.h"
#include "./utils/headers/keypressed.h"

HMODULE hmSuperHotHack;
HMODULE hmHalo1;
UINT_PTR halo1Base;

Halo1::DeviceContainer* pDeviceContainer;
// Renderer* _renderer;

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

BOOL CALLBACK bringHaloToTop__enumWindowsProc( HWND hwnd, LPARAM lParam ) {
    char name[255] = {};
    auto size = GetWindowTextA( hwnd, name, 255 );
    if ( size == 0 )
        return true;
    if ( strncmp( name, "Halo: The Master Chief Collection  ", size ) == 0 ) {
        BringWindowToTop( hwnd );
        SetForegroundWindow( hwnd );
        return false;
    }
    return true;
}
void bringHaloToTop() {
    EnumWindows( bringHaloToTop__enumWindowsProc, NULL );
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

    // bringHaloToTop();

    std::cout << "MCC-SUPERHOT Mod Loaded\n\n";

    std::cout << "halo1.dll located at: ";
    std::cout << std::uppercase << std::hex << hmHalo1;
    std::cout << "\n\n";

    Halo1::init( halo1Base );
    pDeviceContainer = Halo1::getDeviceContainerPointer();
    if ( pDeviceContainer ) {
        std::cout << "Device container at: " << pDeviceContainer << std::endl;
        std::cout << "Device at: " << pDeviceContainer->pDevice << std::endl;
    }
    else {
        std::cout << "Device container not found!" << std::endl;
    }

    addHooks();
    DX11Hook::addPresentHook();
    DX11HookTest::init();

    if ( !err ) {
        while ( !GetAsyncKeyState( VK_F9 ) ) {
            Sleep( 10 );
        }
    }

    std::cout << "Exiting..." << std::endl;

    DX11HookTest::cleanup();
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

// void onPostRenderWorld() {
//     if ( pDeviceContainer && pDeviceContainer->pDevice )
//         DX11HookTest::render( pDeviceContainer->pDevice );
// }

void addHooks() {

    // auto hook = Hook::ezCreateJumpHook(
    //     "Post render world hook",
    //     halo1Base + 0xC41E2BU, 5,
    //     (UINT_PTR) onPostRenderWorld,
    //     HK_PUSH_STATE
    // );
    // hook->fixStolenOffset( 1 );
    // hook->protectTrampoline();
    // Hook::removeBeforeClosing( hook );
    // hook->hook();

}
