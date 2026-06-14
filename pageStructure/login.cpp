#include "login.h"
#include "UIControllers/mapComponent.h"
#include "UIControllers/partition.h"
#include "UIModals/locationBar.h"
#include "UIModals/button.h"
#include "UIModals/textbox.h"
#include "UIModals/objectRenderer.h"
#include "ascii/__mapping.h"
#include <iostream>
#include <cstdio>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
    #include <windows.h>
    #ifdef MessageBox
        #undef MessageBox
    #endif
#endif

Login::Login() {}

bool Login::run() {
    using namespace ui;

    ObjectRenderer loginImage(
        ui::ascii::loginImage_raw,
        ui::ascii::loginImage_lineCount,
        component::align = Align::MiddleCenter,
        component::resizable = false
    );

    ObjectRenderer logo(
        ui::ascii::logo_raw,
        ui::ascii::logo_lineCount,
        component::align = Align::TopCenter,
        component::displacementY = 100,
        component::resizable = true
    );

    LocationBar bar(
        "#ffffff", "#000000",
        SHOW, "Login Page", component::Align::Left,
        SHOW, "Use ARROW keys to navigate, ENTER to select, ESC to exit", component::Align::Center,
        HIDE, "", component::Align::Right,
        0
    );

    MapComponent map(5);
    map.setBackground("#111111");

    TextBox txtUsername(
        component::width = 30,
        component::bgColor = "#2d2d2d",
        component::fgColor = "#00ff00",
        component::borderColor = "#555555",
        component::align = Align::MiddleCenter,
        component::maxLength = 20,
        component::ghostMessage = " Enter username... ",
        component::ghostColor = "#666666"
    );

    TextBox txtPassword(
        component::width = 30,
        component::bgColor = "#2d2d2d",
        component::fgColor = "#00ff00",
        component::borderColor = "#555555",
        component::align = Align::MiddleCenter,
        component::maxLength = 20,
        component::passwordMode = true,
        component::ghostMessage = " Enter password... ",
        component::ghostColor = "#666666"
    );

    Button btnLogin(
        component::text = " Login ",
        component::width = 16,
        component::bgColor = "#1a73e8",
        component::fgColor = "#ffffff",
        component::borderColor = "#4a9eff",
        component::align = Align::MiddleCenter
    );

    Button btnExit(
        component::text = " Exit ",
        component::width = 16,
        component::bgColor = "#e0e0e0",
        component::fgColor = "#333333",
        component::borderColor = "#888888",
        component::align = Align::BottomCenter
    );

    bool authenticated = false;

    btnLogin.setOnActivate([&]() {
        std::string user = txtUsername.text();
        std::string pass = txtPassword.text();
        if (!user.empty() && !pass.empty()) {
            if (auth_.login_user(user, pass)) {
                authenticated = true;
                map.stop();
            }
        }
    });

    btnExit.setOnActivate([&]() {
        map.stop();
    });

    Partition area(Partition::TwoSideH, 55.0f);

    area.left().setBackground("#ee3245");
    area.right().setBackground("#333333");
    area.left().attach(&loginImage);
    area.right().attach(&logo);
    area.right().attach(&txtUsername);
    area.right().attach(&txtPassword);
    area.right().attach(&btnLogin);
    area.right().attach(&btnExit);

    map.bind(bar);
    map.setPartition(&area);

    map.run();

    return authenticated;
}

bool Login::is_authenticated() const {
    return auth_.is_authenticated();
}

std::string Login::get_username() const {
    return auth_.get_current_user();
}
