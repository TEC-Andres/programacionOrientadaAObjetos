#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <cstdio>
#include <csignal>
#include "crypto/ECC.h"
#include "loginManager/loginManager.h"

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif

static bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

static void ensureDir(const std::string& d) {
    if (!fileExists(d)) {
        std::string mk = "mkdir " + d;
        system(mk.c_str());
    }
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
        if (fileExists(g_dbGuard.dbPath)) {
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

static void simulateWork() {
    std::cout << "\n  [WORKING] Decrypted database is ready for queries...\n";
    std::cout << "  [WORKING] Press Enter to exit and re-encrypt.\n";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

int main() {
    const std::string dbPath     = "db/movieverse.db";
    const std::string eccPath    = dbPath + ".ecc";

    std::cout << "==============================\n";
    std::cout << "   ECC - Encrypted Database   \n";
    std::cout << "==============================\n";

    LoginManager auth;

    // -----------------------------------------------------------------------
    // Step 1 — Load / generate ECC key pair (stored in .env)
    // -----------------------------------------------------------------------
    ecc::EccCrypto crypto;
    bool eccReady = false;

    std::string envPriv = auth.load_env_value("ECC_PRIVATE_KEY");
    std::string envPub  = auth.load_env_value("ECC_PUBLIC_KEY");

    if (!envPriv.empty() && !envPub.empty()) {
        std::cout << "\n[KEY] Loading ECC key pair from .env...\n";
        try {
            crypto.setStoredKeyPair(
                ecc::EccCrypto::hexToBytes(envPub),
                ecc::EccCrypto::hexToBytes(envPriv)
            );
            eccReady = true;
        } catch (...) {
            std::cerr << "  Failed to load keys.\n";
        }
    } else {
        std::cout << "\n[KEY] Generating new ECC P-384 key pair...\n";
        try {
            ecc::KeyPair kp = crypto.generateKeyPair();
            auth.save_env_value("ECC_PUBLIC_KEY",
                ecc::EccCrypto::bytesToHex(kp.publicKey()));
            auth.save_env_value("ECC_PRIVATE_KEY",
                ecc::EccCrypto::bytesToHex(kp.privateKey()));
            eccReady = true;
            std::cout << "  Keys saved to .env.\n";
        } catch (...) {
            std::cerr << "  Failed to generate keys.\n";
        }
    }

    if (!eccReady) {
        std::cerr << "  ECC not available. Exiting.\n";
        return 1;
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

    // -----------------------------------------------------------------------
    // Step 2 — Register / Login via SHA-256
    // -----------------------------------------------------------------------
    int choice = 0;
    std::string username, password;

    std::cout << "\n--- [REGISTER / LOGIN] ---\n";
    std::cout << "1. Register\n2. Login\nChoose: ";
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        choice = 2;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice == 1) {
        std::cout << "Username: "; std::getline(std::cin, username);
        std::cout << "Password: "; std::getline(std::cin, password);

        if (!auth.register_user(username, password)) {
            std::cerr << "Registration failed.\n";
            return 1;
        }
        std::cout << "  Account created.\n";

        // --- Encrypt database immediately upon saving the new account ---
        if (!fileExists(dbPath)) {
            std::cout << "\n[INIT] Creating " << dbPath << " for new user...\n";
            ensureDir("db");
            std::ofstream f(dbPath);
            f << "MOVIEVERSE_DB\nuser=" << username << "\n";
            f.close();
        }
        std::cout << "[ENCRYPT] Encrypting " << dbPath << " -> " << eccPath << " ...\n";
        crypto.encryptDatabaseFile(dbPath, eccPath);
        std::cout << "  Database encrypted at rest as '" << eccPath << "'.\n";
    }

    // --- Login ---
    std::cout << "Username: "; std::getline(std::cin, username);
    std::cout << "Password: "; std::getline(std::cin, password);

    if (!auth.login_user(username, password)) {
        std::cerr << "  Authentication FAILED.\n";
        return 1;
    }
    std::cout << "  Authenticated as: " << auth.get_current_user() << "\n";

    // -----------------------------------------------------------------------
    // Step 3 — Decrypt database (ecc -> db) for the session
    // -----------------------------------------------------------------------
    if (!fileExists(eccPath)) {
        std::cerr << "\n[ERROR] No encrypted database found at " << eccPath << "\n";
        return 1;
    }
    std::cout << "\n[DECRYPT] Decrypting " << eccPath << " -> " << dbPath << " ...\n";
    crypto.decryptDatabaseFile(eccPath, dbPath);
    g_dbGuard.dbDecrypted = true;
    std::cout << "  Done. '" << dbPath << "' is ready.\n";

    // -----------------------------------------------------------------------
    // Step 4 — Work with the decrypted database
    // -----------------------------------------------------------------------
    std::cout << "\n--- [DECRYPTED DATABASE] ---\n";
    {
        std::ifstream f(dbPath);
        std::string line;
        while (std::getline(f, line))
            std::cout << "  " << line << "\n";
    }
    std::cout << "----------------------------\n";

    simulateWork();

    // -----------------------------------------------------------------------
    // Step 5 — Re-encrypt database on exit (db -> ecc, removing db)
    // -----------------------------------------------------------------------
    std::cout << "\n[ENCRYPT] Re-encrypting " << dbPath << " -> " << eccPath << " ...\n";
    reencryptNow();
    std::cout << "  Done.\n";

    // Verify only the .ecc file remains
    std::cout << "\n--- [VERIFICATION] ---\n";
    if (fileExists(eccPath) && !fileExists(dbPath)) {
        std::cout << "  PASS: " << eccPath << " exists, " << dbPath << " is absent.\n";
    } else {
        std::cout << "  FAIL: State not as expected.\n";
    }

    std::cout << "\n[INFO] ECC keys stored in .env file.\n";
    std::cout << "[INFO] Encrypted database: " << eccPath << "\n";
    std::cout << "==============================\n";
    return 0;
}
