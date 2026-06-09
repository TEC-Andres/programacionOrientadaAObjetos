#include "UIControllers/mapComponent.h"
#include "UIControllers/partition.h"
#include "UIModals/locationBar.h"
#include "UIModals/button.h"
#include "UIModals/textbox.h"
#include "UIModals/objectRenderer.h"
#include "ascii/__mapping.h"
#include <iostream>
#include <sstream>
#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#endif

int main(int argc, char* argv[]) {
    
    ui::ObjectRenderer loginImage(
        ui::ascii::loginImage_raw, 
        ui::ascii::loginImage_lineCount, 
        ui::component::align=ui::Align::MiddleCenter, 
        ui::component::resizable=true
    );

    ui::ObjectRenderer logo(
        ui::ascii::logo_raw, 
        ui::ascii::logo_lineCount, 
        ui::component::align=ui::Align::TopCenter, 
        ui::component::resizable=true
    );
    
    ui::LocationBar bar(
        "#ffffff", "#000000",
        SHOW, "Interactive Demo", ui::component::Align::Left,
        SHOW, "Use ARROW keys to navigate, ENTER to select, ESC to exit", ui::component::Align::Center,
        HIDE, "", ui::component::Align::Right,
        0
    );

    ui::MapComponent map(30, 3);
    map.setBackground("#111111");

    ui::TextBox txtBox(
        ui::component::width = 30,
        ui::component::bgColor = "#2d2d2d",
        ui::component::fgColor = "#00ff00",
        ui::component::borderColor = "#555555",
        ui::component::align = ui::Align::MiddleCenter,
        ui::component::maxLength = 20
    );

    ui::Button btnExit(
        ui::component::text = " Exit ",
        ui::component::width = 16,
        ui::component::bgColor = "#e0e0e0",
        ui::component::fgColor = "#333333",
        ui::component::borderColor = "#888888",
        ui::component::align = ui::Align::BottomLeft
    );
    ui::Button btnSettings(
        ui::component::text = " Settings ",
        ui::component::width = 20,
        ui::component::bgColor = "#fbbc04",
        ui::component::fgColor = "#000000",
        ui::component::borderColor = "#fdd663",
        ui::component::align = ui::Align::BottomCenter
    );

    ui::Button btnHelp(
        ui::component::text = " Help ",
        ui::component::width = 16,
        ui::component::bgColor = "#9c27b0",
        ui::component::fgColor = "#ffffff",
        ui::component::borderColor = "#ce93d8",
        ui::component::align = ui::Align::BottomRight
    );

    // Activation callbacks for buttons
    btnSettings.setOnActivate([&]() {
        std::cout << "\x1b[2J\x1b[H";
        if (txtBox.text() == "ping") {
            std::cout << "\x1b[32mpong!\x1b[0m\n";
        } else {
            std::cout << "\x1b[31mit failed\x1b[0m\n";
        }
        std::cout << "Press any key to return...\n";
        #if defined(_WIN32) || defined(_WIN64)
            _getch();
        #elif defined(__APPLE__) || defined(__MACH__)
            std::cin.get();
        #else
            std::cin.get();
        #endif
    });

    btnHelp.setOnActivate([]() {
        std::cout << "\x1b[2J\x1b[H\x1b[35mHelp activated!\x1b[0m\n";
        std::cout << "Press any key to return...\n";
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

    area.left().setBackground("#2d5ce6");
    area.right().setBackground("#333333");
    // Left region (55%): logo image
    area.left().attach(&loginImage);

    // Right region (45%): components
    area.right().attach(&logo);
    area.right().attach(&btnExit);
    area.right().attach(&btnSettings);
    area.right().attach(&btnHelp);
    area.right().attach(&txtBox);

    map.bind(bar);
    map.setPartition(&area);

    map.run();
    return 0;
}
