#include <iostream>
#include "dataVideo/video.h"
#include "dataVideo/series.h"
#include "dataVideo/movie.h"
#include "dataVideo/episodes.h"

int main() {
    Video v1(1, "Interstellar", 169, "SciFi", 5);
    Movie m1(1, "Interstellar", 169, "SciFi", 5, "Christopher Nolan");
    Series s1(1, "Interstellar", 169, "SciFi", 5, 1, "Interstellar Series");
    Episodes e1(1, "Interstellar", 169, "SciFi", 5, 1, "Interstellar Series", 3);

    std::cout << "ID: " << v1.getId() << std::endl;
    std::cout << "Nombre: " << v1.getName() << std::endl;
    std::cout << "Duracion: " << v1.getLength() << std::endl;
    std::cout << "Genero: " << v1.getGenre() << std::endl;
    std::cout << "Rating: " << (int)v1.getRating() << std::endl;

    std::cout << "\nDirector: " << m1.getDirector() << std::endl;

    std::cout << "\nTemporada: " << s1.getSeason() << std::endl;
    std::cout << "Titulo: " << s1.getTitle() << std::endl;

    std::cout << "\nNumero de episodio: " << e1.getEpisodeNumber() << std::endl;

    return 0;
}