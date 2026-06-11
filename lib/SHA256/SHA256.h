#ifndef SHA256_H
#define SHA256_H

#include <string>
#include <unordered_map>
#include <cstdint>

// SHA-256 Engine with intuitive naming
class SHA256 {
private:
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;
    const uint32_t k[64];

    // Internal bitwise block compressor
    void process_64byte_block();

    inline uint32_t rotr(uint32_t x, uint32_t n);
    inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z);
    inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z);
    inline uint32_t ep0(uint32_t x);
    inline uint32_t ep1(uint32_t x);
    inline uint32_t sig0(uint32_t x);
    inline uint32_t sig1(uint32_t x);

public:
    SHA256();
    void add_data(const uint8_t* buf, size_t len);
    std::string emit_hash_string();

    // The main convenience method used across your project
    static std::string hash(const std::string& input);
};

// Database structure for memory management
struct UserRecord {
    std::string password_hash;
    std::string salt;
};

// Global Authentication Utilities
std::string generate_salt(size_t length = 16);
bool load_users_from_env();
bool save_users_to_env();
bool register_user(const std::string& username, const std::string& password);
bool login_user(const std::string& username, const std::string& password);
#endif