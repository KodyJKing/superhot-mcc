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
#include "utils/headers/MathUtils.h"
#include "utils/headers/BytePattern.h"
#include "graphics/headers/DX11Hook.h"
#include "graphics/headers/Renderer.h"
#include "timehack/headers/TimeHack.h"

static HMODULE hmSuperHotHack;

#ifdef _DEBUG
static const bool isDebug = true;
#else
static const bool isDebug = false;
#endif

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
    auto renderer = Renderer::getDefault( pDevice );

    static uint64_t startTick = GetTickCount64();
    uint64_t tick = GetTickCount64();
    float uptime = (float) ( tick - startTick ) / 1000.0f;

    static bool hide = false;
    if ( HaloMCC::isInGame() && hide )
        return;

    const float flashAmplitude = 0.5f;
    const float flashFreq = 2.0f;
    float alpha = sinf( uptime * flashFreq ) * flashAmplitude + ( 1 - flashAmplitude );

    hide = HaloMCC::isInGame() && abs( alpha ) < 0.01f;

    std::stringstream ss;
    ss << "SUPERHOT MCC Loaded";
    if ( isDebug )
        ss << " (debug build)";
    std::string text = ss.str();

    auto size = HaloMCC::getWindowSize();
    renderer->begin();
    renderer->drawText( { size.x / 2, 0 }, text.c_str(), { 1.0f, 1.0f, 1.0f, alpha }, FW1_CENTER | FW1_TOP, 16.0f, nullptr );
    renderer->end();
}

DWORD __stdcall mainThread( LPVOID lpParameter ) {

    const bool useConsole = true; // isDebug;
    const bool useStdin = false;

    auto logFile = getModDirectory() + "superhotmcc-log.txt";

    errno_t err = 0;
    FILE* pFile_stdout = NULL;
    FILE* pFile_stderr = NULL;
    FILE* pFile_stdin = NULL;
    if ( useConsole ) {
        AllocConsole();
        err = freopen_s( &pFile_stdout, "CONOUT$", "w", stdout );
        err |= freopen_s( &pFile_stderr, "CONOUT$", "w", stderr );
        if ( useStdin )
            err |= freopen_s( &pFile_stdin, "CONIN$", "r", stdin );
    } else {
        err = freopen_s( &pFile_stdout, logFile.c_str(), "w", stdout );
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

    if ( pFile_stdout ) fclose( pFile_stdout );
    if ( pFile_stderr ) fclose( pFile_stderr );
    if ( pFile_stdin ) fclose( pFile_stdin );
    if ( useConsole )
        FreeConsole();

    FreeLibraryAndExitThread( hmSuperHotHack, 0 );
}
