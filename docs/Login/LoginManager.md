# Login Manager

## module description
this module handles user authentication, registration, and the synchronization of user credentials with the permanent configuration file. it acts as the primary gatekeeper for the interactive application, managing the in-memory runtime database.

## data structures
* user database - a fast in-memory map linking usernames to a `UserRecord`.
* user record - a structured object containing the user's `password_hash` and unique `salt`.

## core routines
* register_user - validates username availability, creates a unique crypto-salt, computes the SHA-256 result of the salt and password, and backs it up to disk.
* login_user - verifies the username exists, grabs their unique salt, re-hashes the input password attempt, and checks if it matches the stored hash exactly.
* load_users_from_env - reads configurations from the `.env` file line by line and parses out credentials structured as `USERNAME=HASH:SALT`.
* save_users_to_env - synchronizes the runtime RAM database out to the permanent `.env` file using the standard format.
* generate_salt - generates an unpredictable string of characters mixed into the password before hashing to protect against cracking attacks.
