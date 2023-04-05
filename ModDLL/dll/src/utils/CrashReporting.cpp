#include "headers/CrashReporting.h"

#ifndef _DEBUG
#include "BugSplat.h"
#pragma comment(lib, "BugSplat64.lib")
#endif

namespace CrashReporting {

    #ifndef _DEBUG
    bool ExceptionCallback_SendFiles( UINT nCode, LPVOID lpVal1, LPVOID lpVal2 );

    const wchar_t* database = L"kody_j_king_gmail_com";
    const wchar_t* app = L"Superhot-MCC";
    const wchar_t* version = L"0.03-alpha";

    static MiniDmpSender* miniDumpSender = nullptr;
    static std::unordered_set<HANDLE> initializedThreads;
    #endif

    void initialize() {
        #ifndef _DEBUG
        miniDumpSender = new MiniDmpSender( database, app, version, NULL, MDSF_LOGFILE | MDSF_PREVENTHIJACKING );
        SetGlobalCRTExceptionBehavior();
        miniDumpSender->setCallback( ExceptionCallback_SendFiles );
        #endif
    }

    /// @brief Setup crash handling for the current thread if it hasn't already been setup.
    void initializeForCurrentThread() {
        #ifndef _DEBUG
        auto handle = GetCurrentThread();
        if ( initializedThreads.contains( handle ) )
            return;
        initializedThreads.insert( handle );
        SetPerThreadCRTExceptionBehavior();
        #endif
    }

    #ifndef _DEBUG
    bool ExceptionCallback_SendFiles( UINT nCode, LPVOID lpVal1, LPVOID lpVal2 ) {
        switch ( nCode ) {
            case MDSCB_EXCEPTIONCODE:
            {
                auto modDir = getModDirectory();
                std::wstringstream ss;
                ss << modDir.c_str();
                auto modDirW = ss.str();

                wchar_t filePath[MAX_PATH];
                wsprintf( filePath, L"%ssuperhotmcc-log.txt", modDirW.c_str() );
                miniDumpSender->sendAdditionalFile( filePath );
                wsprintf( filePath, L"%sSuperhotMCC.ini", modDirW.c_str() );
                miniDumpSender->sendAdditionalFile( filePath );
            }
            break;
        }

        return false;
    }
    #endif

}