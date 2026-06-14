#pragma once
#include <string>
#include "crypto/ECC.h"

struct DbGuard {
    ecc::EccCrypto crypto;
    std::string dbPath;
    std::string eccPath;
    bool eccReady = false;
    volatile bool dbDecrypted = false;
};

extern DbGuard g_dbGuard;
void reencryptNow();
