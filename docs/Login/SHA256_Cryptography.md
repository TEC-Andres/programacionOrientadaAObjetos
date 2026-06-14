# SHA256 Cryptography

## module description
this module provides the cryptographic foundation for the application's security, specifically managing the hashing life cycle.

## core routines
* hash - a public static helper method that converts a regular string into an un-reversible 64-character hex string.
* process_64byte_block - internal method handling the core bitwise transformations of the SHA-256 algorithm.

## security implementation
passwords are never stored in plain text. instead, the system takes the user's password, appends a dynamically generated salt, and passes it through the `SHA256::hash` function. the resulting 64-character hex string is what is safely stored in the environment file.