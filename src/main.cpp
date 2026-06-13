#include "ascii/__mapping.h"
#include "UIControllers/mapComponent.h"
#include "UIControllers/partition.h"
#include "UIModals/locationBar.h"
#include "UIModals/button.h"
#include "UIModals/textbox.h"
#include "UIModals/objectRenderer.h"
#include "loginManager/loginManager.h"
#include <iostream>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#endif

int main(int argc, char* argv[]) {

    ui::ObjectRenderer loginImage(
        ui::ascii::loginImage_raw,
        ui::ascii::loginImage_lineCount,
        ui::component::align = ui::Align::MiddleCenter,
        ui::component::resizable = false
    );

    ui::ObjectRenderer logo(
        ui::ascii::logo_raw,
        ui::ascii::logo_lineCount,
        ui::component::align = ui::Align::TopCenter,
        ui::component::displacementY = 100,
        ui::component::resizable = true
    );

    ui::LocationBar bar(
        "#ffffff", "#000000",
        SHOW, "Login Page", ui::component::Align::Left,
        SHOW, "Use ARROW keys to navigate, ENTER to select, ESC to exit", ui::component::Align::Center,
        HIDE, "", ui::component::Align::Right,
        0
    );

    ui::MapComponent map(5);
    map.setBackground("#111111");

    // Username textbox with ghost message
    ui::TextBox txtUsername(
        ui::component::width = 30,
        ui::component::bgColor = "#2d2d2d",
        ui::component::fgColor = "#00ff00",
        ui::component::borderColor = "#555555",
        ui::component::align = ui::Align::MiddleCenter,
        ui::component::maxLength = 20,
        ui::component::ghostMessage = " Enter username... ",
        ui::component::ghostColor = "#666666"
    );

    // Password textbox with ghost message and password mode
    ui::TextBox txtPassword(
        ui::component::width = 30,
        ui::component::bgColor = "#2d2d2d",
        ui::component::fgColor = "#00ff00",
        ui::component::borderColor = "#555555",
        ui::component::align = ui::Align::MiddleCenter,
        ui::component::maxLength = 20,
        ui::component::passwordMode = true,
        ui::component::ghostMessage = " Enter password... ",
        ui::component::ghostColor = "#666666"
    );

    ui::Button btnLogin(
        ui::component::text = " Login ",
        ui::component::width = 16,
        ui::component::bgColor = "#1a73e8",
        ui::component::fgColor = "#ffffff",
        ui::component::borderColor = "#4a9eff",
        ui::component::align = ui::Align::MiddleCenter
    );

    ui::Button btnExit(
        ui::component::text = " Exit ",
        ui::component::width = 16,
        ui::component::bgColor = "#e0e0e0",
        ui::component::fgColor = "#333333",
        ui::component::borderColor = "#888888",
        ui::component::align = ui::Align::BottomCenter
    );

    // Instantiate login manager — automatically loads .env credentials
    LoginManager auth;

    // Login validation callback — checks credentials against .env
    btnLogin.setOnActivate([&]() {
        std::string user = txtUsername.text();
        std::string pass = txtPassword.text();
        std::cout << "\x1b[2J\x1b[H";
        if (!user.empty() && !pass.empty()) {
            if (auth.login_user(user, pass)) {
                std::cout << " Session Owner: " << auth.get_current_user() << " has connected.\n";
            }
        } else {
            std::cout << "\x1b[31mPlease enter both username and password.\x1b[0m\n";
        }
        std::cout << "\nPress any key to continue...\n";
        #if defined(_WIN32) || defined(_WIN64)
            _getch();
        #elif defined(__APPLE__) || defined(__MACH__)
            std::cin.get();
        #else
            std::cin.get();
        #endif
    });

    btnExit.setOnActivate([&]() {
        map.stop();
    });

    // Create a two-side-H partition with 55/45 ratio
    ui::Partition area(ui::Partition::TwoSideH, 55.0f);

    area.left().setBackground("#ee3245");
    area.right().setBackground("#333333");
    // Left region (55%): login image
    area.left().attach(&loginImage);

    // Right region (45%): login form
    area.right().attach(&logo);
    area.right().attach(&txtUsername);
    area.right().attach(&txtPassword);
    area.right().attach(&btnLogin);
    area.right().attach(&btnExit);

    map.bind(bar);
    map.setPartition(&area);

    map.run();
    return 0;
}
