# Elliptic Curve Cryptography (ECC)

## Module Description
Elliptic Curve Cryptography (ECC) is a public-key cryptography method based on the algebraic structure of elliptic curves over finite fields. The main reason we chosed this method rather that something like RSA is due to its efficiency and security; being able to encrypt data in O(1) space and not being attached to size limitations.

## Class structure & implemenation
The ECC module is implemented as a set of classes that provide the necessary functionality for performing elliptic curve operations, including key generation, signing, and verification. The overall architecture is designed to be modular and extensible, allowing for easy integration with other components of the application.

```
C:.
│   ECC.cpp
│   ECC.h
│   
└───ECC
        EccException.h
        EccServiceFactory.h
        EccServiceImpl.cpp
        EccServiceImpl.h
        EncryptedFile.cpp
        EncryptedFile.h
        IEccService.h
        KeyPair.h
```

The library is able to generate key pairs as files or directly into an .env file for use in the application. This allows for flexible key management and integration with the application's configuration system. 

## Implementation details behind ECC
Elliptic Curve Cryptography is based on the discrete logarithm problem on elliptic curves. The security of ECC relies on the difficulty of solving the elliptic curve discrete logarithm problem (ECDLP), which is believed to be computationally infeasible for well-chosen curves and key sizes. A lot of the math component is made by OpenSSL. However, there are still some quirky concepts used for the implementation.

- **ECIES-like hybrid encryption** — ephemeral ECDH + HKDF-SHA256 key derivation + AES-256-GCM
- **EncryptedFile binary format** — custom serialization with magic bytes (ECCF), versioning, length-prefixed sections
- **Abort-safe database encryption** — signal handlers (SIGINT/SIGTERM) that re-encrypt on crash
- **Hex-based key serialization** — storing binary keys as hex strings in .env files
- **Public key recovery from private key** — deriveFromPrivateKey() wrapping OpenSSL's key extraction

What we implemented here is a comprehensive ECC-based encryption solution that provides both the cryptographic foundation and the necessary tools for integrating this technology into a larger application.

## Motivation
The motivation for implementing ECC in this project is to leverage its superior performance and security compared to traditional RSA-based encryption. ECC offers the same level of security with much smaller key sizes, which results in faster computations and reduced storage requirements. This makes it particularly suitable for applications where performance and resource usage are critical factors.