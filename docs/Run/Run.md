# Run Process

## Module Description
This document outlines the launch commands and terminal configuration profiles needed to initialize the core application binary safely.

## Execution Steps
In order to execute, first you need to build the application using CMake. Once already build, you need to set an account by accesing `__release__/environments/eccDatabase`. This will create the necessary configuration for the application to run.

In order to run, you also need to ensure that the database actually exists, if it does not exist, you will need to create it before launching the application by running on `db/rawData/` the following command:

```bash
python3 create_database.py
```

Once the database is created, you can launch the application from the `__release__` directory, if you select the default environment variables. The parameters for username and password are the following:
- Username: `admin`
- Password: `1234`

If you did everything correctly, the application should launch without any issues.