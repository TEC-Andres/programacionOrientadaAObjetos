#include "episodes.h"

Episodes::Episodes() : Series() {
    episodeNumber = 0;
}

Episodes::Episodes(uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating, uint32_t season, std::string title, uint32_t episodeNumber) : Series(id, name, length, genre, rating, season, title) {
    this -> episodeNumber = episodeNumber;
}

uint32_t Episodes::getEpisodeNumber(){
    return episodeNumber;
}

void Episodes::setEpisodeNumber(uint32_t episodeNumber){
    this -> episodeNumber = episodeNumber;
}