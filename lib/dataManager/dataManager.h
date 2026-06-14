#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <string>
#include <vector>
#include "dataVideo/video.h"
#include "dataVideo/movie.h"
#include "dataVideo/series.h"
#include "dataVideo/episodes.h"

struct sqlite3;

class DataManager {
public:
    explicit DataManager(const std::string& dbPath);
    ~DataManager();

    void loadFromDb();
    void saveToDb();

    std::vector<Movie>& getMovies() { return movies_; }
    std::vector<Series>& getSeries() { return series_; }
    std::vector<Episodes>& getEpisodes() { return episodes_; }

    void addMovie(const Movie& m) { movies_.push_back(m); }
    void addSeries(const Series& s) { series_.push_back(s); }
    void addEpisode(const Episodes& e) { episodes_.push_back(e); }

    bool removeMovie(uint32_t id);
    bool removeSeries(uint32_t id);
    bool removeEpisode(uint32_t id);

    Movie* findMovieById(uint32_t id);
    Series* findSeriesById(uint32_t id);
    Episodes* findEpisodeById(uint32_t id);
    std::vector<Episodes> findEpisodesBySeriesName(const std::string& seriesName);

    size_t movieCount() const { return movies_.size(); }
    size_t seriesCount() const { return series_.size(); }
    size_t episodeCount() const { return episodes_.size(); }

private:
    std::string dbPath_;
    sqlite3* db_;
    std::vector<Movie> movies_;
    std::vector<Series> series_;
    std::vector<Episodes> episodes_;
};

#endif
