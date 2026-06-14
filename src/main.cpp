#include "pageStructure.h"
#include "pages/dbGuard.h"
#include <iostream>
#include <csignal>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

static void signalHandler(int) {
    reencryptNow();
    _exit(1);
}

#if defined(_WIN32) || defined(_WIN64)
static BOOL WINAPI consoleCtrlHandler(DWORD dwEvent) {
    if (dwEvent == CTRL_C_EVENT || dwEvent == CTRL_BREAK_EVENT ||
        dwEvent == CTRL_CLOSE_EVENT || dwEvent == CTRL_LOGOFF_EVENT ||
        dwEvent == CTRL_SHUTDOWN_EVENT) {
        reencryptNow();
        return FALSE;
    }
    return FALSE;
}
#endif

int main(int argc, char* argv[]) {
    #if defined(_WIN32) || defined(_WIN64)
        SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
    #endif
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Login login;
    if (login.run()) {
        Dashboard dashboard(login.get_username());
        dashboard.run();
    }

    std::cout << "\x1b[33m[ENCRYPT] Re-encrypting database...\x1b[0m\n";
    reencryptNow();
    std::cout << "\x1b[32m[ENCRYPT] Database encrypted.\x1b[0m\n";

    return 0;
}
