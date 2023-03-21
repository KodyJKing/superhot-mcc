#include <iostream>
#include <windows.h>

#define MCC_WINDOW_NAME "Halo: The Master Chief Collection  "
#define THREAD_TIMEOUT 10000

// Thanks to 247CTF for their LoadLibrary DLL injection example.
HRESULT injectDll( DWORD pid, std::string path ) {
    HRESULT result = S_OK;

    DWORD access = PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ;
    HANDLE hProc = OpenProcess( access, false, pid );
    if ( !hProc ) {

        std::cerr << "Could not open MCC process.\n";
        result = E_FAIL;

    } else {

        LPVOID path_remotePtr = VirtualAllocEx( hProc, NULL, path.size(), MEM_COMMIT, PAGE_READWRITE );
        if ( !path_remotePtr ) {

            std::cerr << "Could not allocate DLL path in MCC process.\n";
            result = E_FAIL;

        } else {

            if ( !WriteProcessMemory( hProc, path_remotePtr, path.c_str(), path.size(), NULL ) ) {

                std::cerr << "Could not write DLL path in MCC process.\n";
                result = E_FAIL;

            } else {

                HANDLE hThread = CreateRemoteThread( hProc, NULL, NULL, (LPTHREAD_START_ROUTINE) LoadLibraryA, path_remotePtr, NULL, NULL );
                if ( !hThread ) {

                    std::cerr << "Could not create thread in MCC Process.\n";
                    result = E_FAIL;

                } else {

                    WaitForSingleObject( hThread, THREAD_TIMEOUT );
                    CloseHandle( hThread );

                }

            }

            if ( !VirtualFreeEx( hProc, path_remotePtr, NULL, MEM_RELEASE ) )
                std::cerr << "Could not free DLL path in MCC process!\n";

        }

        CloseHandle( hProc );

    }

    return result;
}

std::string getLauncherDirectory() {
    char path[MAX_PATH];
    GetModuleFileNameA( NULL, path, MAX_PATH );
    std::string pathStr = path;
    auto lastSlash = pathStr.find_last_of( '\\' );
    return pathStr.substr( 0, lastSlash + 1 );
}

void exitWithErrorMessage( std::string message ) {
    std::cerr << "Error: " << message << "\n\n";
    std::cout << "Press any key to exit.\n";
    getchar();
    exit( 0 );
}

int main() {
    auto hwnd = FindWindowA( NULL, MCC_WINDOW_NAME );
    if ( !hwnd ) exitWithErrorMessage( "Could not find MCC window." );

    DWORD pid;
    GetWindowThreadProcessId( hwnd, &pid );
    if ( !pid ) exitWithErrorMessage( "Could not get MCC process ID." );

    auto dllPath = getLauncherDirectory() + "SUPERHOTMCC.dll";
    if ( FAILED( injectDll( pid, dllPath ) ) )
        exitWithErrorMessage( "Could not inject mod dll." );
}
