#include "movie.h"

Movie::Movie() : Video() {
    director = "";
}

Movie::Movie(uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating, std::string director) : Video(id, name, length, genre, rating) {
    this -> director = director;
}

std::string Movie::getDirector() {
    return director;
}

void Movie::setDirector(std::string director) {
    this -> director = director;
}