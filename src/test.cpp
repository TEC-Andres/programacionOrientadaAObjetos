#include "dataVideo/video.h"
#include "dataVideo/series.h"
#include "dataVideo/movie.h"
#include "dataVideo/episodes.h"

int main() {
    Video v1(1, "Interstellar", 169, "SciFi", 5);
    Movie m1(v1, "Christopher Nolan");
    Series s1(v1, 1, "Interstellar Series");
    Episodes e1(s1, 3);

    cout << "ID: " << v1.getId() << endl;
    cout << "Nombre: " << v1.getName() << endl;
    cout << "Duracion: " << v1.getLength() << endl;
    cout << "Genero: " << v1.getGenre() << endl;
    cout << "Rating: " << (int)v1.getRating() << endl;

    cout << "Director: " << m1.getDirector() << endl;

    cout << "Temporada: " << s1.getSeason() << endl;
    cout << "Titulo: " << s1.getTitle() << endl;

    cout << "Numero de episodio: " << e1.getEpisodeNumber() << endl;

    return 0;
}

Video::Video() {
    id = 0;
    name = "";
    length = 0;
    genre = "";
    rating = 0;
}

Video::Video (uint32_t id, string name, uint32_t length, string genre, uint8_t rating){
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

string Video::getName(){
    return name;
}

void Video::setName(string name){
    this -> name = name;
}

uint32_t Video::getLength(){
    return length;
}

void Video::setLength(uint32_t length){
    this -> length = length;
}

string Video::getGenre(){
    return genre;
}

void Video::setGenre(string genre){
    this -> genre = genre;
}

uint8_t Video::getRating(){
    return rating;
}

void Video::setRating(uint8_t rating){
    this -> rating = rating;
}