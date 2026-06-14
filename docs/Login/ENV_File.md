# Environment File

## module description
this document describes the structural layout of the internal settings profile used to lock down paths and keys outside of source tracking.

## variables breakdown
your local .env configuration file must include these exact keys:
* db_path - local string pointing to the encrypted data files directory.
* crypto_key - verification hash code used to initialize cipher components.
* log_level - warning mask threshold used to throttle error logging output.