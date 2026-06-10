#include <iostream>
#include <string>
#include <limits>
#include "../lib/auth/auth.h"

void display_menu() {
    std::cout << "\n==============================\n";
    std::cout << "       SECURE PASS GATEWAY    \n";
    std::cout << "==============================\n";
    std::cout << "1. Register New Account\n";
    std::cout << "2. User Login\n";
    std::cout << "3. Exit Application\n";
    std::cout << "Choose an option (1-3): ";
}

int main() {
    // Read the database at startup
    load_users_from_env();

    int choice = 0;
    std::string username, password;

    while (true) {
        display_menu();
        if (!(std::cin >> choice)) {
            std::cout << "Invalid entry. Please enter a valid number.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        // Discard trailing newline leftover from cin >> choice
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 3) {
            std::cout << "Goodbye!\n";
            break;
        }

        switch (choice) {
            case 1: // Registration flow
                std::cout << "\n--- [REGISTER ACCOUNT] ---\n";
                std::cout << "Enter Username: ";
                std::getline(std::cin, username);
                std::cout << "Enter Password: ";
                std::getline(std::cin, password);

                if (!username.empty() && !password.empty()) {
                    register_user(username, password);
                } else {
                    std::cout << "Error: Username or password cannot be blank.\n";
                }
                break;

            case 2: // Login flow
                std::cout << "\n--- [USER LOGIN] ---\n";
                std::cout << "Enter Username: ";
                std::getline(std::cin, username);
                std::cout << "Enter Password: ";
                std::getline(std::cin, password);

                login_user(username, password);
                break;

            default:
                std::cout << " Invalid selection. Choose 1, 2, or 3.\n";
                break;
        }
    }

    return 0;
}