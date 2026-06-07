#include "UIControllers/mapComponent.h"
#include "UIModals/locationBar.h"
#include "UIModals/messagebox.h"
#include "UIModals/button.h"
#include "UIModals/objectRenderer.h"
#include "ascii/__mapping.h"
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#endif

int main(int argc, char* argv[]) {
    
    ui::ObjectRenderer logo(
        ui::ascii::logo_raw, 
        ui::ascii::logo_lineCount, 
        ui::component::align=ui::Align::TopRight, 
        ui::component::resizable=true, 
        ui::component::displacementX=10.0f, 
        ui::component::displacementY=0.0f
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
    ui::Button btnSettings2(
        ui::component::text = " Settings2 ",
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
    btnSettings.setOnActivate([]() {
        std::cout << "\x1b[2J\x1b[H\x1b[33mSettings activated!\x1b[0m\n";
        std::cout << "Press any key to return...\n";
        #if defined(_WIN32) || defined(_WIN64)
            _getch();
        #elif defined(__APPLE__) || defined(__MACH__)
            std::cin.get();
        #else
            std::cin.get(); // Fallback for Linux or other systems
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
            std::cin.get(); // Fallback for Linux or other systems
        #endif
    });

    btnExit.setOnActivate([&]() {
        map.stop();
    });

    map.bind(bar);
    // map.bind(msgBox);
    map.bind(logo);

    map.attach(&btnSettings);
    map.attach(&btnSettings2);
    map.attach(&btnHelp);
    map.attach(&btnExit);

    map.run();
    return 0;
}
