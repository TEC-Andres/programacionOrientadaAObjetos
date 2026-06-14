#include "ascii/__mapping.h"
#include "UIControllers/mapComponent.h"
#include "UIControllers/partition.h"
#include "UIModals/locationBar.h"
#include "UIModals/button.h"
#include "UIModals/textbox.h"
#include "UIModals/objectRenderer.h"
#include "loginManager/loginManager.h"
#include "crypto/ECC.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <csignal>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
    #include <windows.h>
#endif

static bool file_exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

// ---------------------------------------------------------------------------
// Abort-safe re-encryption guard
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
        if (file_exists(g_dbGuard.dbPath)) {
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

int main(int argc, char* argv[]) {

    ui::ObjectRenderer loginImage(
        ui::ascii::loginImage_raw,
        ui::ascii::loginImage_lineCount,
        ui::component::align = ui::Align::MiddleCenter,
        ui::component::resizable = false
    );

    ui::ObjectRenderer logo(
        ui::ascii::logo_raw,
        ui::ascii::logo_lineCount,
        ui::component::align = ui::Align::TopCenter,
        ui::component::displacementY = 100,
        ui::component::resizable = true
    );

    ui::LocationBar bar(
        "#ffffff", "#000000",
        SHOW, "Login Page", ui::component::Align::Left,
        SHOW, "Use ARROW keys to navigate, ENTER to select, ESC to exit", ui::component::Align::Center,
        HIDE, "", ui::component::Align::Right,
        0
    );

    ui::MapComponent map(5);
    map.setBackground("#111111");

    // Username textbox with ghost message
    ui::TextBox txtUsername(
        ui::component::width = 30,
        ui::component::bgColor = "#2d2d2d",
        ui::component::fgColor = "#00ff00",
        ui::component::borderColor = "#555555",
        ui::component::align = ui::Align::MiddleCenter,
        ui::component::maxLength = 20,
        ui::component::ghostMessage = " Enter username... ",
        ui::component::ghostColor = "#666666"
    );

    // Password textbox with ghost message and password mode
    ui::TextBox txtPassword(
        ui::component::width = 30,
        ui::component::bgColor = "#2d2d2d",
        ui::component::fgColor = "#00ff00",
        ui::component::borderColor = "#555555",
        ui::component::align = ui::Align::MiddleCenter,
        ui::component::maxLength = 20,
        ui::component::passwordMode = true,
        ui::component::ghostMessage = " Enter password... ",
        ui::component::ghostColor = "#666666"
    );

    ui::Button btnLogin(
        ui::component::text = " Login ",
        ui::component::width = 16,
        ui::component::bgColor = "#1a73e8",
        ui::component::fgColor = "#ffffff",
        ui::component::borderColor = "#4a9eff",
        ui::component::align = ui::Align::MiddleCenter
    );

    ui::Button btnExit(
        ui::component::text = " Exit ",
        ui::component::width = 16,
        ui::component::bgColor = "#e0e0e0",
        ui::component::fgColor = "#333333",
        ui::component::borderColor = "#888888",
        ui::component::align = ui::Align::BottomCenter
    );

    // Instantiate login manager — automatically loads .env credentials
    LoginManager auth;

    // -----------------------------------------------------------------------
    // ECC key setup — keys stored in .env alongside user credentials
    // -----------------------------------------------------------------------
    const std::string dbPath     = "db/movieverse.db";
    const std::string eccPath    = dbPath + ".ecc";

    ecc::EccCrypto crypto;
    bool eccReady = false;

    std::string envPriv = auth.load_env_value("ECC_PRIVATE_KEY");
    std::string envPub  = auth.load_env_value("ECC_PUBLIC_KEY");

    if (!envPriv.empty() && !envPub.empty()) {
        try {
            crypto.setStoredKeyPair(
                ecc::EccCrypto::hexToBytes(envPub),
                ecc::EccCrypto::hexToBytes(envPriv)
            );
            eccReady = true;
        } catch (...) {
            std::cerr << "\n  [ECC] Failed to load keys from .env.\n";
        }
    } else {
        // First run — generate keys and store in .env
        try {
            ecc::KeyPair kp = crypto.generateKeyPair();
            auth.save_env_value("ECC_PUBLIC_KEY",
                ecc::EccCrypto::bytesToHex(kp.publicKey()));
            auth.save_env_value("ECC_PRIVATE_KEY",
                ecc::EccCrypto::bytesToHex(kp.privateKey()));
            eccReady = true;
            std::cout << "\n  [ECC] Generated new key pair and saved to .env.\n";
        } catch (...) {
            std::cerr << "\n  [ECC] Failed to generate keys.\n";
        }
    }

    // Populate abort-safe guard and register termination handlers
    g_dbGuard.crypto = &crypto;
    g_dbGuard.dbPath = dbPath;
    g_dbGuard.eccPath = eccPath;
    g_dbGuard.eccReady = eccReady;
    g_dbGuard.dbDecrypted = false;

    #if defined(_WIN32) || defined(_WIN64)
        SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
    #endif
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // Login validation callback — checks credentials against .env, then decrypts db
    btnLogin.setOnActivate([&]() {
        std::string user = txtUsername.text();
        std::string pass = txtPassword.text();
        std::cout << "\x1b[2J\x1b[H";
        if (!user.empty() && !pass.empty()) {
            if (auth.login_user(user, pass)) {
                std::cout << " Session Owner: " << auth.get_current_user() << " has connected.\n";
                // Ensure database is encrypted at rest, then decrypt for the session
                if (eccReady) {
                    try {
                        if (!file_exists(eccPath)) {
                            if (!file_exists(dbPath)) {
                                // No database at all — create a fresh one
                                std::cout << "\x1b[33m[INIT] No database found. Creating new database...\x1b[0m\n";
                                std::ofstream f(dbPath);
                                f << "MOVIEVERSE_DB\nuser=" << user << "\n";
                                f.close();
                            }
                            // Encrypt plaintext db -> .ecc (removes .db)
                            crypto.encryptDatabaseFile(dbPath, eccPath);
                            std::cout << "\x1b[32m[ENCRYPT] Database encrypted at rest: " << eccPath << "\x1b[0m\n";
                        }
                        // Decrypt .ecc -> .db for the session
                        crypto.decryptDatabaseFile(eccPath, dbPath);
                        g_dbGuard.dbDecrypted = true;
                        std::cout << "\x1b[32m[DECRYPT] Database decrypted: " << eccPath << " -> " << dbPath << "\x1b[0m\n";
                    } catch (const std::exception& e) {
                        std::cout << "\x1b[31m[DB] Failed: " << e.what() << "\x1b[0m\n";
                    }
                }
            }
        } else {
            std::cout << "\x1b[31mPlease enter both username and password.\x1b[0m\n";
        }
        std::cout << "\nPress any key to continue...\n";
        #if defined(_WIN32) || defined(_WIN64)
            _getch();
        #elif defined(__APPLE__) || defined(__MACH__)
            std::cin.get();
        #else
            std::cin.get();
        #endif
    });

    btnExit.setOnActivate([&]() {
        map.stop();
    });

    // Create a two-side-H partition with 55/45 ratio
    ui::Partition area(ui::Partition::TwoSideH, 55.0f);

    area.left().setBackground("#ee3245");
    area.right().setBackground("#333333");
    // Left region (55%): login image
    area.left().attach(&loginImage);

    // Right region (45%): login form
    area.right().attach(&logo);
    area.right().attach(&txtUsername);
    area.right().attach(&txtPassword);
    area.right().attach(&btnLogin);
    area.right().attach(&btnExit);

    map.bind(bar);
    map.setPartition(&area);

    map.run();

    // -----------------------------------------------------------------------
    // Re-encrypt database on exit (also handled by signal/console handlers)
    // -----------------------------------------------------------------------
    std::cout << "\x1b[33m[ENCRYPT] Re-encrypting database...\x1b[0m\n";
    reencryptNow();
    std::cout << "\x1b[32m[ENCRYPT] Database encrypted.\x1b[0m\n";

    return 0;
}
