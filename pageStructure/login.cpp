#include "login.h"
#include "pages/dbGuard.h"
#include "UIControllers/mapComponent.h"
#include "UIControllers/partition.h"
#include "UIModals/locationBar.h"
#include "UIModals/button.h"
#include "UIModals/textbox.h"
#include "UIModals/objectRenderer.h"
#include "ascii/__mapping.h"
#include <iostream>
#include <fstream>
#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
    #include <windows.h>
    #ifdef MessageBox
        #undef MessageBox
    #endif
#endif

Login::Login() {}

bool Login::run() {
    using namespace ui;

    std::string envPriv = auth_.load_env_value("ECC_PRIVATE_KEY");
    std::string envPub  = auth_.load_env_value("ECC_PUBLIC_KEY");

    bool eccReady = false;
    const std::string dbPath  = "db/movieverse.db";
    const std::string eccPath = dbPath + ".ecc";

    if (!envPriv.empty() && !envPub.empty()) {
        try {
            g_dbGuard.crypto.setStoredKeyPair(
                ecc::EccCrypto::hexToBytes(envPub),
                ecc::EccCrypto::hexToBytes(envPriv)
            );
            eccReady = true;
        } catch (...) {
            std::cerr << "\n  [ECC] Failed to load keys from .env.\n";
        }
    } else {
        try {
            ecc::KeyPair kp = g_dbGuard.crypto.generateKeyPair();
            auth_.save_env_value("ECC_PUBLIC_KEY",
                ecc::EccCrypto::bytesToHex(kp.publicKey()));
            auth_.save_env_value("ECC_PRIVATE_KEY",
                ecc::EccCrypto::bytesToHex(kp.privateKey()));
            eccReady = true;
        } catch (...) {
            std::cerr << "\n  [ECC] Failed to generate keys.\n";
        }
    }

    g_dbGuard.dbPath = dbPath;
    g_dbGuard.eccPath = eccPath;
    g_dbGuard.eccReady = eccReady;
    g_dbGuard.dbDecrypted = false;

    ObjectRenderer loginImage(
        ui::ascii::loginImage_raw,
        ui::ascii::loginImage_lineCount,
        component::align = Align::MiddleCenter,
        component::resizable = false
    );

    ObjectRenderer logo(
        ui::ascii::logo_raw,
        ui::ascii::logo_lineCount,
        component::align = Align::TopCenter,
        component::displacementY = 100,
        component::resizable = true
    );

    LocationBar bar(
        "#ffffff", "#000000",
        SHOW, "Login Page", component::Align::Left,
        SHOW, "Use ARROW keys to navigate, ENTER to select, ESC to exit", component::Align::Center,
        HIDE, "", component::Align::Right,
        0
    );

    MapComponent map(5);
    map.setBackground("#111111");

    TextBox txtUsername(
        component::width = 30,
        component::bgColor = "#2d2d2d",
        component::fgColor = "#00ff00",
        component::borderColor = "#555555",
        component::align = Align::MiddleCenter,
        component::maxLength = 20,
        component::ghostMessage = " Enter username... ",
        component::ghostColor = "#666666"
    );

    TextBox txtPassword(
        component::width = 30,
        component::bgColor = "#2d2d2d",
        component::fgColor = "#00ff00",
        component::borderColor = "#555555",
        component::align = Align::MiddleCenter,
        component::maxLength = 20,
        component::passwordMode = true,
        component::ghostMessage = " Enter password... ",
        component::ghostColor = "#666666"
    );

    Button btnLogin(
        component::text = " Login ",
        component::width = 16,
        component::bgColor = "#1a73e8",
        component::fgColor = "#ffffff",
        component::borderColor = "#4a9eff",
        component::align = Align::MiddleCenter
    );

    Button btnExit(
        component::text = " Exit ",
        component::width = 16,
        component::bgColor = "#e0e0e0",
        component::fgColor = "#333333",
        component::borderColor = "#888888",
        component::align = Align::BottomCenter
    );

    bool loginSuccess = false;

    btnLogin.setOnActivate([&]() {
        std::string user = txtUsername.text();
        std::string pass = txtPassword.text();
        std::cout << "\x1b[2J\x1b[H";
        if (!user.empty() && !pass.empty()) {
            if (auth_.login_user(user, pass)) {
                loginSuccess = true;
                std::cout << " Session Owner: " << auth_.get_current_user() << " has connected.\n";
                if (eccReady) {
                    try {
                        auto fileExists = [](const std::string& p) -> bool {
                            std::ifstream f(p);
                            return f.good();
                        };
                        if (!fileExists(eccPath)) {
                            if (!fileExists(dbPath)) {
                                std::cout << "\x1b[33m[INIT] No database found. Creating new database...\x1b[0m\n";
                                std::ofstream f(dbPath);
                                f << "MOVIEVERSE_DB\nuser=" << user << "\n";
                                f.close();
                            }
                            g_dbGuard.crypto.encryptDatabaseFile(dbPath, eccPath);
                            std::cout << "\x1b[32m[ENCRYPT] Database encrypted at rest: " << eccPath << "\x1b[0m\n";
                        }
                        g_dbGuard.crypto.decryptDatabaseFile(eccPath, dbPath);
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
        #else
            std::cin.get();
        #endif
        if (loginSuccess) {
            map.stop();
        }
    });

    btnExit.setOnActivate([&]() {
        map.stop();
    });

    Partition area(Partition::TwoSideH, 55.0f);

    area.left().setBackground("#ee3245");
    area.right().setBackground("#333333");
    area.left().attach(&loginImage);
    area.right().attach(&logo);
    area.right().attach(&txtUsername);
    area.right().attach(&txtPassword);
    area.right().attach(&btnLogin);
    area.right().attach(&btnExit);

    map.bind(bar);
    map.setPartition(&area);

    map.run();

    return loginSuccess;
}

bool Login::is_authenticated() const {
    return auth_.is_authenticated();
}

std::string Login::get_username() const {
    return auth_.get_current_user();
}
