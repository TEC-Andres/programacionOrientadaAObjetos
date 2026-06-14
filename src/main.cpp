#include "pageStructure.h"
#include <iostream>

int main(int argc, char* argv[]) {
    Login loginPage;

    if (loginPage.run()) {
        Dashboard dashboard(loginPage.get_username());
        dashboard.run();
        std::cout << "\n[*] Session ended. Goodbye, " << loginPage.get_username() << ".\n";
    } else {
        std::cout << "\n[*] Login cancelled.\n";
    }

    return 0;
}
