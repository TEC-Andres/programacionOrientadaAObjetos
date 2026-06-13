#include <iostream>
#include <string>
#include <limits>
#include "loginManager/loginManager.h"

int main() {
    // Instantiate our login manager.
    // This automatically triggers the .env search and loads the database.
    LoginManager auth;

    int choice = 0;
    std::string username, password;

    while (true) {
        std::cout << "\n==============================\n";
        std::cout << "       SECURE PASS GATEWAY    \n";
        std::cout << "==============================\n";
        std::cout << "1. Register New Account\n";
        std::cout << "2. User Login\n";
        std::cout << "3. Exit System\n";
        std::cout << "Choose an option (1-3): ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 3) {
            std::cout << "Exiting system. Goodbye!\n";
            return 0;
        }

        switch (choice) {
            case 1:
                std::cout << "\n--- [REGISTER ACCOUNT] ---\n" << "Enter Username: ";
                std::getline(std::cin, username);
                std::cout << "Enter Password: ";
                std::getline(std::cin, password);

                if (!username.empty() && !password.empty()) {
                    if (auth.register_user(username, password)) {
                        std::cout << "\n Registration successful for '" << username << "'!\n";
                    } else {
                        std::cout << " Internal database error during registration.\n";
                    }
                } else {
                    std::cout << " Fields cannot be blank.\n";
                }
                break;

            case 2:
                std::cout << "\n--- [USER LOGIN] ---\n" << "Enter Username: ";
                std::getline(std::cin, username);
                std::cout << "Enter Password: ";
                std::getline(std::cin, password);

                if (auth.login_user(username, password)) {
                    std::cout << "\n Session Owner: " << auth.get_current_user() << " has connected.\n";

                    // --- Start your main application logic here! ---
                }
                break;

            default:
                std::cout << " Invalid selection.\n";
                break;
        }
    }
    return 0;
}