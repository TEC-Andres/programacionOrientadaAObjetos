#include "series.h"

Series::Series() : Video() {
    season = 0;
    title = "";
}

Series::Series(uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating, uint32_t season, std::string title) : Video(id, name, length, genre, rating) {
    this -> season = season;
    this -> title = title;
}

uint32_t Series::getSeason(){
    return season;
}

void Series::setSeason(uint32_t season){
    this -> season = season;
}

std::string Series::getTitle() {
    return title;
}

void Series::setTitle(std::string title){
    this -> title = title;
}