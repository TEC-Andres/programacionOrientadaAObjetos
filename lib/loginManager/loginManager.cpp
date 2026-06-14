#include "loginManager.h"
#include "crypto/SHA256.h"
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

LoginManager::LoginManager() : current_logged_in_user(""), resolved_env_path("") {
    load_users_from_env();
}

bool LoginManager::file_exists(const std::string& path) {
    std::ifstream f(path.c_str());
    return f.good();
}

std::string LoginManager::get_env_path() {
    if (!resolved_env_path.empty()) return resolved_env_path;

    std::string root_path = ".env";
    std::string release_fallback_path = "../.env";

    if (file_exists(root_path)) {
        resolved_env_path = root_path;
    } else if (file_exists(release_fallback_path)) {
        resolved_env_path = release_fallback_path;
    } else {
        resolved_env_path = root_path;
    }
    return resolved_env_path;
}
/**
 * =========================================================================
 * generate_salt
 * =========================================================================
 * Generates an unpredictable string of characters. This gets mixed into the
 * password before hashing to protect against Rainbow Table cracking attacks.
 */
std::string LoginManager::generate_salt(size_t length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);

    std::string salt;
    salt.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        salt += charset[dis(gen)];
    }
    return salt;
}
/**
 * =========================================================================
 * load_users_from_env
 * =========================================================================
 * Reads the configurations from the `.env` file on disk line by line,
 * parses out credentials structured as: USERNAME=HASH:SALT
 * and stores them in our fast in-memory runtime `user_database` map.
 */
bool LoginManager::load_users_from_env() {
    std::string active_path = get_env_path();
    std::ifstream file(active_path);
    if (!file.is_open()) return false;

    user_database.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t delim = line.find('=');
        if (delim == std::string::npos) continue;

        std::string username = line.substr(0, delim);
        std::string payload = line.substr(delim + 1);

        size_t colon = payload.find(':');
        if (colon == std::string::npos) continue;

        std::string hash_val = payload.substr(0, colon);
        std::string salt_val = payload.substr(colon + 1);

        user_database[username] = { hash_val, salt_val };
    }
    file.close();
    return true;
}
/**
 * =========================================================================
 * save_users_to_env
 * =========================================================================
 * Synchronizes the runtime RAM database out to the permanent `.env` file.
 * Loops through all users, writing them to disk using the standard format.
 */
bool LoginManager::save_users_to_env() {
    // First, read existing file and collect non-user lines (comments,
    // blank lines, and auxiliary KEY=VALUE entries without a colon)
    std::string active_path = get_env_path();
    std::vector<std::string> preamble;
    {
        std::ifstream inFile(active_path);
        if (inFile.is_open()) {
            std::string line;
            while (std::getline(inFile, line)) {
                bool isUserLine = false;
                std::string::size_type delim = line.find('=');
                if (delim != std::string::npos) {
                    std::string payload = line.substr(delim + 1);
                    if (payload.find(':') != std::string::npos) {
                        isUserLine = true;
                    }
                }
                if (!isUserLine) {
                    preamble.push_back(line);
                }
            }
            inFile.close();
        }
    }

    // Rewrite the file: preamble first, then current user records
    std::ofstream file(active_path);
    if (!file.is_open()) return false;

    for (const auto& line : preamble) {
        file << line << "\n";
    }

    for (const auto& pair : user_database) {
        file << pair.first << "=" << pair.second.password_hash << ":" << pair.second.salt << "\n";
    }
    file.close();
    return true;
}
/**
 * =========================================================================
 * register_user
 * =========================================================================
 * Validates availability, creates a unique crypto-salt, combines it with the
 * password, computes the SHA-256 result, adds it to RAM, and backs it up to disk.
 */
bool LoginManager::register_user(const std::string& username, const std::string& password) {
    if (user_database.find(username) != user_database.end()) {
        std::cout << "\n Registration Error: Username already exists.\n";
        return false;
    }

    std::string salt = generate_salt();
    std::string password_hash = SHA256::hash(salt + password);

    user_database[username] = { password_hash, salt };
    return save_users_to_env();
}
/**
 * =========================================================================
 * login_user
 * =========================================================================
 * Verifies username existence, grabs their unique salt, re-hashes the input password
 * attempt, and checks if it matches the stored hash exactly.
 */
bool LoginManager::login_user(const std::string& username, const std::string& password) {
    auto it = user_database.find(username);
    if (it == user_database.end()) {
        std::cout << "\n Login Failed: Incorrect username or password.\n";
        return false;
    }

    const UserRecord& record = it->second;
    std::string attempt_hash = SHA256::hash(record.salt + password);

    if (attempt_hash == record.password_hash) {
        current_logged_in_user = username;
        std::cout << "\n Login Success! Welcome back, " << username << ".\n";
        return true;
    } else {
        std::cout << "\n Login Failed: Incorrect username or password.\n";
        return false;
    }
}

void LoginManager::logout() {
    current_logged_in_user = "";
}

bool LoginManager::is_authenticated() const {
    return !current_logged_in_user.empty();
}

std::string LoginManager::get_current_user() const {
    return current_logged_in_user;
}

std::string LoginManager::load_env_value(const std::string& key) {
    std::string active_path = get_env_path();
    std::ifstream file(active_path);
    if (!file.is_open()) return "";

    std::string prefix = key + "=";
    std::string line;
    while (std::getline(file, line)) {
        if (line.compare(0, prefix.size(), prefix) == 0) {
            file.close();
            return line.substr(prefix.size());
        }
    }
    file.close();
    return "";
}

bool LoginManager::save_env_value(const std::string& key, const std::string& value) {
    std::string active_path = get_env_path();
    std::string prefix = key + "=";
    std::vector<std::string> lines;
    bool found = false;

    {
        std::ifstream file(active_path);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                if (!found && line.compare(0, prefix.size(), prefix) == 0) {
                    lines.push_back(prefix + value);
                    found = true;
                } else {
                    lines.push_back(line);
                }
            }
            file.close();
        }
    }

    if (!found) {
        lines.push_back(prefix + value);
    }

    std::ofstream file(active_path);
    if (!file.is_open()) return false;
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
    return true;
}