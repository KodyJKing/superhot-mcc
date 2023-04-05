#include "headers/CrashReporting.h"

#include "BugSplat.h"
#pragma comment(lib, "BugSplat64.lib")

namespace CrashReporting {

    static MiniDmpSender* miniDumpSender = nullptr;
    void initialize() {
        miniDumpSender = new MiniDmpSender( L"kody_j_king_gmail_com", L"Superhot-MCC", L"0.03-alpha", NULL, MDSF_LOGFILE | MDSF_PREVENTHIJACKING );
        SetGlobalCRTExceptionBehavior();
    }

    static std::unordered_set<HANDLE> initializedThreads;
    /// @brief Setup crash handling
    void initializeForCurrentThread() {
        auto handle = GetCurrentThread();
        if ( initializedThreads.contains( handle ) )
            return;
        initializedThreads.insert( handle );
        SetPerThreadCRTExceptionBehavior();
    }

}