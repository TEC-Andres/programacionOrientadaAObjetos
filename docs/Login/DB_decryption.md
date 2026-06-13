# Database Decryption

## module description
this document provides the security roadmap and background information regarding how the data system handles local binary encryption schemas.

## decryption process flow
the system processes protected text records using this simple four-step routine:
* step 1: reads the raw encoded storage file directly from the local disk path.
* step 2: pipes the data payload into the backend openssl cryptographic engine.
* step 3: executes integrity verification hashes to ensure the file was not altered.
* step 4: instantiates the data models directly into runtime memory storage matrices.

## system dependencies
* openssl::crypto library flags are mandatory to compile these routines.