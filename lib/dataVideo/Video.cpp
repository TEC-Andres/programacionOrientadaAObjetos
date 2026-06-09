#include "video.h"

Video::Video() {
    id = 0;
    name = "";
    length = 0;
    genre = "";
    rating = 0;
}

Video::Video (uint32_t id, std::string name, uint32_t length, std::string genre, uint8_t rating){
    this -> id = id;
    this -> name = name;
    this -> length = length;
    this -> genre = genre;
    this -> rating = rating;
}

uint32_t Video::getId(){
    return id;
}

void Video::setId(uint32_t id){
    this -> id = id;
}

std::string Video::getName(){
    return name;
}

void Video::setName(std::string name){
    this -> name = name;
}

uint32_t Video::getLength(){
    return length;
}

void Video::setLength(uint32_t length){
    this -> length = length;
}

std::string Video::getGenre(){
    return genre;
}

void Video::setGenre(std::string genre){
    this -> genre = genre;
}

uint8_t Video::getRating(){
    return rating;
}

void Video::setRating(uint8_t rating){
    this -> rating = rating;
}