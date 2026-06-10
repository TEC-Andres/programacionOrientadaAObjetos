#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <unordered_map>
#include <cstdint>

// SHA-256 State and Logic
class SHA256 {
private:
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;
    const uint32_t k[64];

    void transform();
    inline uint32_t rotr(uint32_t x, uint32_t n);
    inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z);
    inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z);
    inline uint32_t ep0(uint32_t x);
    inline uint32_t ep1(uint32_t x);
    inline uint32_t sig0(uint32_t x);
    inline uint32_t sig1(uint32_t x);

public:
    SHA256();
    void update(const uint8_t* buf, size_t len);
    std::string final();
    static std::string hash_string(const std::string& input);
};

// Structures for User Database
struct UserRecord {
    std::string password_hash;
    std::string salt;
};

// Core Authentication Functions
std::string generate_salt(size_t length = 16);
bool load_users_from_env();
bool save_users_to_env();
bool register_user(const std::string& username, const std::string& password);
bool login_user(const std::string& username, const std::string& password);

#endif // AUTH_H