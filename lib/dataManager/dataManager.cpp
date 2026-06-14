#include "dataManager.h"
#include <sqlite3.h>
#include <algorithm>
#include <cstring>

DataManager::DataManager(const std::string& dbPath)
    : dbPath_(dbPath), db_(nullptr)
{
}

DataManager::~DataManager()
{
}

void DataManager::loadFromDb()
{
    movies_.clear();
    series_.clear();
    episodes_.clear();

    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        sqlite3_close(db_);
        db_ = nullptr;
        return;
    }

    const char* sql;
    char* errMsg = nullptr;

    // --- Load Movies ---
    sql = "SELECT id, name, length, genre, rating, director FROM movies ORDER BY id";
    if (sqlite3_exec(db_, "PRAGMA foreign_keys = OFF", nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
    }
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            uint32_t id = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            uint32_t length = static_cast<uint32_t>(sqlite3_column_int(stmt, 2));
            const char* genre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            double ratingDouble = sqlite3_column_double(stmt, 4);
            uint8_t rating = static_cast<uint8_t>(ratingDouble);
            const char* director = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

            movies_.emplace_back(
                id,
                name ? name : "",
                length,
                genre ? genre : "",
                rating,
                director ? director : ""
            );
        }
        sqlite3_finalize(stmt);
    }

    // --- Load Series ---
    sql = "SELECT id, name, length, genre, rating, season FROM series ORDER BY id";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            uint32_t id = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            uint32_t length = static_cast<uint32_t>(sqlite3_column_int(stmt, 2));
            const char* genre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            double ratingDouble = sqlite3_column_double(stmt, 4);
            uint8_t rating = static_cast<uint8_t>(ratingDouble);
            uint32_t season = static_cast<uint32_t>(sqlite3_column_int(stmt, 5));

            series_.emplace_back(
                id,
                name ? name : "",
                length,
                genre ? genre : "",
                rating,
                season,
                name ? name : ""
            );
        }
        sqlite3_finalize(stmt);
    }

    // --- Load Episodes ---
    sql = "SELECT e.id, s.name, e.length, s.genre, e.rating, e.season, e.title, e.episode_number "
          "FROM episodes e JOIN series s ON e.series_id = s.id ORDER BY e.id";
    if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            uint32_t id = static_cast<uint32_t>(sqlite3_column_int(stmt, 0));
            const char* seriesName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            uint32_t length = static_cast<uint32_t>(sqlite3_column_int(stmt, 2));
            const char* genre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            double ratingDouble = sqlite3_column_double(stmt, 4);
            uint8_t rating = static_cast<uint8_t>(ratingDouble);
            uint32_t season = static_cast<uint32_t>(sqlite3_column_int(stmt, 5));
            const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            uint32_t episodeNumber = static_cast<uint32_t>(sqlite3_column_int(stmt, 7));

            episodes_.emplace_back(
                id,
                seriesName ? seriesName : "",
                length,
                genre ? genre : "",
                rating,
                season,
                title ? title : "",
                episodeNumber
            );
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db_);
    db_ = nullptr;
}

void DataManager::saveToDb()
{
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        sqlite3_close(db_);
        db_ = nullptr;
        return;
    }

    char* errMsg = nullptr;
    sqlite3_exec(db_, "PRAGMA foreign_keys = OFF", nullptr, nullptr, &errMsg);
    sqlite3_free(errMsg);
    errMsg = nullptr;

    sqlite3_exec(db_, "BEGIN TRANSACTION", nullptr, nullptr, &errMsg);
    sqlite3_free(errMsg);
    errMsg = nullptr;

    // Delete existing data (episodes first due to FK)
    sqlite3_exec(db_, "DELETE FROM episodes", nullptr, nullptr, &errMsg);
    sqlite3_free(errMsg);
    errMsg = nullptr;
    sqlite3_exec(db_, "DELETE FROM series", nullptr, nullptr, &errMsg);
    sqlite3_free(errMsg);
    errMsg = nullptr;
    sqlite3_exec(db_, "DELETE FROM movies", nullptr, nullptr, &errMsg);
    sqlite3_free(errMsg);
    errMsg = nullptr;

    sqlite3_stmt* stmt = nullptr;

    // --- Save Series ---
    const char* insertSeries = "INSERT INTO series (id, name, length, genre, rating, season) VALUES (?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db_, insertSeries, -1, &stmt, nullptr) == SQLITE_OK) {
        for (auto& s : series_) {
            sqlite3_bind_int(stmt, 1, static_cast<int>(s.getId()));
            sqlite3_bind_text(stmt, 2, s.getName().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, static_cast<int>(s.getLength()));
            sqlite3_bind_text(stmt, 4, s.getGenre().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 5, static_cast<double>(s.getRating()));
            sqlite3_bind_int(stmt, 6, static_cast<int>(s.getSeason()));

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_step(stmt);
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }

    // --- Save Episodes ---
    const char* insertEpisode = "INSERT INTO episodes (id, series_id, season, episode_number, title, length, rating) "
                                "VALUES (?, (SELECT id FROM series WHERE name = ?), ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db_, insertEpisode, -1, &stmt, nullptr) == SQLITE_OK) {
        for (auto& e : episodes_) {
            sqlite3_bind_int(stmt, 1, static_cast<int>(e.getId()));
            sqlite3_bind_text(stmt, 2, e.getName().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, static_cast<int>(e.getSeason()));
            sqlite3_bind_int(stmt, 4, static_cast<int>(e.getEpisodeNumber()));
            sqlite3_bind_text(stmt, 5, e.getTitle().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 6, static_cast<int>(e.getLength()));
            sqlite3_bind_double(stmt, 7, static_cast<double>(e.getRating()));

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_step(stmt);
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }

    // --- Save Movies ---
    const char* insertMovie = "INSERT INTO movies (id, name, length, genre, rating, director) "
                              "VALUES (?, ?, ?, ?, ?, ?)";
    if (sqlite3_prepare_v2(db_, insertMovie, -1, &stmt, nullptr) == SQLITE_OK) {
        for (auto& m : movies_) {
            sqlite3_bind_int(stmt, 1, static_cast<int>(m.getId()));
            sqlite3_bind_text(stmt, 2, m.getName().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt, 3, static_cast<int>(m.getLength()));
            sqlite3_bind_text(stmt, 4, m.getGenre().c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 5, static_cast<double>(m.getRating()));
            sqlite3_bind_text(stmt, 6, m.getDirector().c_str(), -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_step(stmt);
            }
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_exec(db_, "COMMIT", nullptr, nullptr, &errMsg);
    sqlite3_free(errMsg);

    sqlite3_close(db_);
    db_ = nullptr;
}

bool DataManager::removeMovie(uint32_t id)
{
    auto it = std::find_if(movies_.begin(), movies_.end(),
        [id](Movie& m) { return m.getId() == id; });
    if (it != movies_.end()) {
        movies_.erase(it);
        return true;
    }
    return false;
}

bool DataManager::removeSeries(uint32_t id)
{
    auto it = std::find_if(series_.begin(), series_.end(),
        [id](Series& s) { return s.getId() == id; });
    if (it != series_.end()) {
        std::string name = it->getName();
        episodes_.erase(std::remove_if(episodes_.begin(), episodes_.end(),
            [&name](Episodes& e) { return e.getName() == name; }),
            episodes_.end());
        series_.erase(it);
        return true;
    }
    return false;
}

bool DataManager::removeEpisode(uint32_t id)
{
    auto it = std::find_if(episodes_.begin(), episodes_.end(),
        [id](Episodes& e) { return e.getId() == id; });
    if (it != episodes_.end()) {
        episodes_.erase(it);
        return true;
    }
    return false;
}

Movie* DataManager::findMovieById(uint32_t id)
{
    auto it = std::find_if(movies_.begin(), movies_.end(),
        [id](Movie& m) { return m.getId() == id; });
    return it != movies_.end() ? &(*it) : nullptr;
}

Series* DataManager::findSeriesById(uint32_t id)
{
    auto it = std::find_if(series_.begin(), series_.end(),
        [id](Series& s) { return s.getId() == id; });
    return it != series_.end() ? &(*it) : nullptr;
}

Episodes* DataManager::findEpisodeById(uint32_t id)
{
    auto it = std::find_if(episodes_.begin(), episodes_.end(),
        [id](Episodes& e) { return e.getId() == id; });
    return it != episodes_.end() ? &(*it) : nullptr;
}

std::vector<Episodes> DataManager::findEpisodesBySeriesName(const std::string& seriesName)
{
    std::vector<Episodes> result;
    std::string lowerName = seriesName;
    for (auto& c : lowerName) c = std::tolower(c);
    for (auto& e : episodes_) {
        std::string en = e.getName();
        for (auto& c : en) c = std::tolower(c);
        if (en == lowerName) {
            result.push_back(e);
        }
    }
    return result;
}
