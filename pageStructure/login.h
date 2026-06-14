#pragma once

#include "loginManager/loginManager.h"
#include <string>

class Login {
public:
    Login();
    bool run();
    bool is_authenticated() const;
    std::string get_username() const;

private:
    LoginManager auth_;
};
