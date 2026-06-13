#include "SHA256.h"
#include <sstream>
#include <iomanip>
#include <cstring>

SHA256::SHA256() : k{0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2} {
    state[0] = 0x6a09e667; state[1] = 0xbb67ae85; state[2] = 0x3c6ef372; state[3] = 0xa54ff53a;
    state[4] = 0x510e527f; state[5] = 0x9b05688c; state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
    bitlen = 0; datalen = 0;
}

inline uint32_t SHA256::rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
inline uint32_t SHA256::ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
inline uint32_t SHA256::maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
inline uint32_t SHA256::ep0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
inline uint32_t SHA256::ep1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }
inline uint32_t SHA256::sig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
inline uint32_t SHA256::sig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }

void SHA256::process_64byte_block() {
    uint32_t m[64];
    for (int i = 0; i < 16; ++i) { m[i] = (data[i * 4] << 24) | (data[i * 4 + 1] << 16) | (data[i * 4 + 2] << 8) | (data[i * 4 + 3]); }
    for (int i = 16; i < 64; ++i) { m[i] = sig1(m[i - 2]) + m[i - 7] + sig0(m[i - 15]) + m[i - 16]; }
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3], e = state[4], f = state[5], g = state[6], h = state[7];
    for (int i = 0; i < 64; ++i) {
        uint32_t t1 = h + ep1(e) + ch(e, f, g) + k[i] + m[i];
        uint32_t t2 = ep0(a) + maj(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void SHA256::add_data(const uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        data[datalen] = buf[i]; datalen++;
        if (datalen == 64) { process_64byte_block(); bitlen += 512; datalen = 0; }
    }
}

std::string SHA256::emit_hash_string() {
    uint32_t i = datalen;
    if (datalen < 56) {
        data[i++] = 0x80; while (i < 56) data[i++] = 0x00;
    } else {
        data[i++] = 0x80; while (i < 64) data[i++] = 0x00;
        process_64byte_block(); memset(data, 0, 56);
    }
    bitlen += datalen * 8;
    data[63] = bitlen; data[62] = bitlen >> 8; data[61] = bitlen >> 16; data[60] = bitlen >> 24;
    data[59] = bitlen >> 32; data[58] = bitlen >> 40; data[57] = bitlen >> 48; data[56] = bitlen >> 56;
    process_64byte_block();

    std::stringstream ss;
    for (int j = 0; j < 8; ++j) {
        ss << std::hex << std::setw(8) << std::setfill('0') << state[j];
    }
    return ss.str();
}
/**
 * =========================================================================
 * SHA256::hash
 * =========================================================================
 * Public static helper method that manages the hashing life cycle.
 * Converts a regular string into an un-reversible 64-character hex string.
 */
std::string SHA256::hash(const std::string& input) {
    SHA256 ctx;
    ctx.add_data(reinterpret_cast<const uint8_t*>(input.c_str()), input.length());
    return ctx.emit_hash_string();
}