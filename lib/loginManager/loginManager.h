#ifndef LOGIN_MANAGER_H
#define LOGIN_MANAGER_H

#include <string>
#include <unordered_map>

struct UserRecord {
    std::string password_hash;
    std::string salt;
};

class LoginManager {
private:
    std::unordered_map<std::string, UserRecord> user_database;
    std::string current_logged_in_user;
    std::string resolved_env_path;

    // Internal File Operations and Helpers
    bool file_exists(const std::string& path);
    std::string get_env_path();
    bool load_users_from_env();
    bool save_users_to_env();
    std::string generate_salt(size_t length = 16);

public:
    LoginManager();

    bool register_user(const std::string& username, const std::string& password);
    bool login_user(const std::string& username, const std::string& password);
    void logout();

    bool is_authenticated() const;
    std::string get_current_user() const;

    // Auxiliary key-value storage in .env (co-exists with user records)
    std::string load_env_value(const std::string& key);
    bool save_env_value(const std::string& key, const std::string& value);
};

#endif // LOGIN_MANAGER_H