#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "../lib/Security/HeapCrypto.h"

struct EnvData {
    std::string encryptedPassword = "";
};

EnvData parseEnvFile(const std::string& targetUser) {
    std::ifstream file;
    file.open(".env");
    std::string line;
    EnvData data;
    if (file.is_open()) {
        while (std::getline(file, line)) {
            size_t equalSignPos = line.find('=');
            if (equalSignPos != std::string::npos) {
                std::string key = line.substr(0, equalSignPos);
                std::string value = line.substr(equalSignPos + 1);
                if (key == targetUser) data.encryptedPassword = value;
            }
        }
    }
    return data;
}

void renderLoginCLI() {
    std::string inputUser;
    std::string inputPassword;

    std::cout << "========================================\n";
    std::cout << "          SYSTEM LOGIN PORTAL           \n";
    std::cout << "========================================\n";

    std::cout << "Username: ";
    std::cin >> inputUser;

    std::cout << "Password: ";
    std::cin >> inputPassword;
    std::cout << "\nAuthenticating...\n";

    EnvData env = parseEnvFile(inputUser);

    if (env.encryptedPassword.empty()) {
        std::cout << "\n>> Access Denied. User does not exist.\n";
        return;
    }

    try {
        // Allocate a strict limit on the heap (10 chars)
        HeapCrypto cryptoEngine(10);

        // This will throw an error if inputPassword > 10 chars
        cryptoEngine.encryptWholeString(inputPassword);
        std::string newlyEncryptedPass = cryptoEngine.getEncryptedData();

        if (newlyEncryptedPass == env.encryptedPassword) {
            std::cout << "\n>> Access Granted. Welcome back, " << inputUser << "!\n";
        } else {
            std::cout << "\n>> Access Denied. Invalid password.\n";
        }

    } catch (const std::overflow_error& error) {
        std::cerr << "\n[CRITICAL ERROR] " << error.what() << "\n";
        std::cerr << ">> Access Denied. Memory bounds exceeded.\n";
    }
}

int main() {
    renderLoginCLI();
    return 0;
}