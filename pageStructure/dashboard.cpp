#include "dashboard.h"
#include "UIControllers/mapComponent.h"
#include "UIControllers/partition.h"
#include "UIModals/locationBar.h"
#include <iostream>
#include <fstream>
#include <csignal>
#include <cstdio>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
    #include <windows.h>
    #ifdef MessageBox
        #undef MessageBox
    #endif
#endif

// ---------------------------------------------------------------------------
// Crash-safe re-encryption guard
// ---------------------------------------------------------------------------
static struct {
    ecc::EccCrypto* crypto = nullptr;
    std::string dbPath;
    std::string eccPath;
    bool eccReady = false;
    volatile bool dbDecrypted = false;
} g_dbGuard;

static void reencryptNow() {
    if (g_dbGuard.eccReady && g_dbGuard.crypto && g_dbGuard.dbDecrypted) {
        std::ifstream f(g_dbGuard.dbPath);
        if (f.good()) {
            f.close();
            try {
                g_dbGuard.crypto->encryptDatabaseFile(g_dbGuard.dbPath, g_dbGuard.eccPath);
            } catch (...) {}
        }
        g_dbGuard.dbDecrypted = false;
    }
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

static void signalHandler(int) {
    reencryptNow();
    _exit(1);
}
// ---------------------------------------------------------------------------

Dashboard::Dashboard(const std::string& username) : username_(username) {}
Dashboard::~Dashboard() { unregisterCrashGuard_(); }

static std::string resolve_path(const std::string& relative) {
    std::ifstream f(relative);
    if (f.good()) return relative;
    std::string fallback = "../" + relative;
    std::ifstream f2(fallback);
    if (f2.good()) return fallback;
    return relative;
}

bool Dashboard::initCrypto_() {
    LoginManager auth;
    std::string envPriv = auth.load_env_value("ECC_PRIVATE_KEY");
    std::string envPub  = auth.load_env_value("ECC_PUBLIC_KEY");

    if (envPriv.empty() || envPub.empty()) {
        std::cout << "  ECC keys not found in .env\n";
        return false;
    }
    try {
        crypto_.setStoredKeyPair(
            ecc::EccCrypto::hexToBytes(envPub),
            ecc::EccCrypto::hexToBytes(envPriv)
        );
        eccReady_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cout << "  ECC key load error: " << e.what() << "\n";
        return false;
    }
}

bool Dashboard::decryptDb_() {
    if (!eccReady_) return false;

    eccPath_ = resolve_path("db/movieverse.db.ecc");
    dbPath_  = resolve_path("db/movieverse.db");

    std::ifstream f(eccPath_);
    if (!f.good()) {
        std::cout << "  Encrypted db not found at: " << eccPath_ << "\n";
        return false;
    }
    f.close();

    try {
        crypto_.decryptDatabaseFile(eccPath_, dbPath_);
        std::cout << "  Decrypted -> " << dbPath_ << "\n";
        dbDecrypted_ = true;
        return true;
    } catch (const std::exception& e) {
        std::cout << "  Decryption error: " << e.what() << "\n";
        return false;
    }
}

bool Dashboard::reencryptDb() {
    if (!eccReady_) return false;

    std::ifstream f(dbPath_);
    if (!f.good()) {
        std::cout << "  Decrypted db not found at: " << dbPath_ << "\n";
        return false;
    }
    f.close();

    reencryptNow();
    unregisterCrashGuard_();
    std::cout << "  Re-encrypted -> " << eccPath_ << "\n";
    return true;
}

void Dashboard::registerCrashGuard_() {
    g_dbGuard.crypto = &crypto_;
    g_dbGuard.dbPath = dbPath_;
    g_dbGuard.eccPath = eccPath_;
    g_dbGuard.eccReady = eccReady_;
    g_dbGuard.dbDecrypted = true;

    #if defined(_WIN32) || defined(_WIN64)
        SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
    #endif
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
}

void Dashboard::unregisterCrashGuard_() {
    g_dbGuard.dbDecrypted = false;
    g_dbGuard.crypto = nullptr;
}

bool Dashboard::run() {
    std::cout << "\n--- Initializing main console ---\n";
    initCrypto_();
    decryptDb_();
    if (!dbDecrypted_) {
        std::cout << "  WARNING: Database could not be decrypted.\n";
        std::cout << "---------------------------\n";
        return false;
    }
    registerCrashGuard_();
    std::cout << "---------------------------\n";

    using namespace ui;

    LocationBar bar(
        "#ffffff", "#000000",
        SHOW, "MovieVerse - " + username_, Align::Left,
        SHOW, "Use ARROW keys to navigate, ENTER to select, ESC to exit", Align::Center,
        HIDE, "", Align::Right,
        0
    );

    MapComponent map(4);
    map.setBackground("#1a1a1a");

    Partition area(Partition::FourSide, 50.0f, 50.0f);

    area.topLeft().setBackground("#ff4444");
    area.topRight().setBackground("#44ff44");
    area.bottomLeft().setBackground("#4444ff");
    area.bottomRight().setBackground("#ffff44");

    map.bind(bar);
    map.setPartition(&area);

    map.run();

    if (dbDecrypted_) {
        reencryptDb();
    }
    return true;
}
