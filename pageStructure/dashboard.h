#pragma once

#include "loginManager/loginManager.h"
#include "crypto/ECC.h"
#include <string>

class Dashboard {
public:
    Dashboard(const std::string& username);
    ~Dashboard();
    bool run();

private:
    std::string username_;
    ecc::EccCrypto crypto_;
    bool eccReady_ = false;
    bool dbDecrypted_ = false;
    std::string dbPath_;
    std::string eccPath_;

    bool initCrypto_();
    bool decryptDb_();
    void registerCrashGuard_();
    void unregisterCrashGuard_();
    bool reencryptDb();
};
