#include "pages/dbGuard.h"
#include <fstream>
#include <cstdio>

static bool file_exists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

DbGuard g_dbGuard;

void reencryptNow() {
    if (g_dbGuard.eccReady && g_dbGuard.dbDecrypted) {
        if (file_exists(g_dbGuard.dbPath)) {
            try {
                g_dbGuard.crypto.encryptDatabaseFile(g_dbGuard.dbPath, g_dbGuard.eccPath);
                std::remove(g_dbGuard.dbPath.c_str());
            } catch (...) {}
        }
        g_dbGuard.dbDecrypted = false;
    }
}
