#pragma once
#include <string>

class DataManager;

class Dashboard {
public:
    explicit Dashboard(const std::string& username);
    bool run();

private:
    std::string username_;
    DataManager* dm_ = nullptr;
};
