#include "dashboard.h"
#include "pages/dbGuard.h"
#include "dataManager/dataManager.h"
#include "UIControllers/mapComponent.h"
#include "UIControllers/partition.h"
#include "UIModals/locationBar.h"
#include "UIModals/button.h"
#include "UIModals/textbox.h"
#include "UIModals/histogram.h"
#include "UIModals/dataTable.h"
#include "UIModals/terminalArea.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <cctype>
#include <string>
#include <algorithm>
#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#endif

static std::vector<std::string> splitQuoted(const std::string& input) {
    std::vector<std::string> tokens;
    std::string current;
    bool inQuotes = false;
    for (size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

Dashboard::Dashboard(const std::string& username) : username_(username) {}

bool Dashboard::run() {
    using namespace ui;

    DataManager dm("db/movieverse.db");
    dm.loadFromDb();
    dm_ = &dm;

    MapComponent dashMap(3);
    dashMap.setBackground("#0d1117");

    LocationBar dashBar(
        "#161b22", "#c9d1d9",
        SHOW, " Dashboard ", Align::Left,
        SHOW, "ARROWS to navigate | ENTER to activate | ESC to exit", Align::Center,
        HIDE, "", Align::Right,
        0
    );

    DataTable dataTable(
        component::width = 44,
        component::height = 14,
        component::bgColor = "#0d1117",
        component::fgColor = "#c9d1d9",
        component::borderColor = "#30363d",
        component::titleBgColor = "#161b22",
        component::titleFgColor = "#58a6ff",
        component::bodyBgColor = "#161b22",
        component::align = Align::TopLeft
    );
    dataTable.setFillRegion(true);
    dataTable.setMaxRows(15);

    Button btnPrev(
        component::text = " < Prev ",
        component::width = 12,
        component::bgColor = "#21262d",
        component::fgColor = "#c9d1d9",
        component::borderColor = "#30363d",
        component::align = Align::BottomLeft
    );
    Button btnNext(
        component::text = " Next > ",
        component::width = 12,
        component::bgColor = "#21262d",
        component::fgColor = "#c9d1d9",
        component::borderColor = "#30363d",
        component::align = Align::BottomRight
    );

    btnPrev.setOnActivate([&]() { dataTable.prevPage(); });
    btnNext.setOnActivate([&]() { dataTable.nextPage(); });

    Histogram hist(
        component::title = " Data View ",
        component::width = 36,
        component::height = 14,
        component::bgColor = "#0d1117",
        component::fgColor = "#c9d1d9",
        component::borderColor = "#30363d",
        component::titleBgColor = "#161b22",
        component::titleFgColor = "#fbbc04",
        component::align = Align::TopLeft
    );
    hist.setFillRegion(true);

    TerminalArea terminal(
        component::width = 44,
        component::height = 8,
        component::bgColor = "#0d1117",
        component::fgColor = "#00ff00",
        component::borderColor = "#30363d",
        component::titleFgColor = "#00ff00",
        component::bodyFgColor = "#8b949e",
        component::borderFgColor = "#ff6b6b",
        component::align = Align::BottomLeft
    );
    terminal.setFillRegion(true);

    auto populateDataTable = [&](const std::string& type) {
        if (type == "movies") {
            std::vector<TableRow> rows;
            for (auto& m : dm.getMovies()) {
                TableRow r;
                r.columns.push_back(std::to_string(m.getId()));
                r.columns.push_back(m.getName());
                r.columns.push_back(std::to_string(m.getLength()));
                r.columns.push_back(m.getGenre());
                r.columns.push_back(std::to_string(static_cast<int>(m.getRating())));
                r.columns.push_back(m.getDirector());
                rows.push_back(r);
            }
            dataTable.setHeaders({"ID", "Name", "Length", "Genre", "Rating", "Director"});
            dataTable.setData(rows);
            dataTable.resetPage();
            terminal.addOutput("Showing " + std::to_string(dm.movieCount()) + " movies.");
        } else if (type == "series") {
            std::vector<TableRow> rows;
            for (auto& s : dm.getSeries()) {
                TableRow r;
                r.columns.push_back(std::to_string(s.getId()));
                r.columns.push_back(s.getName());
                r.columns.push_back(std::to_string(s.getLength()));
                r.columns.push_back(s.getGenre());
                r.columns.push_back(std::to_string(static_cast<int>(s.getRating())));
                r.columns.push_back(std::to_string(s.getSeason()));
                rows.push_back(r);
            }
            dataTable.setHeaders({"ID", "Name", "Length", "Genre", "Rating", "Season"});
            dataTable.setData(rows);
            dataTable.resetPage();
            terminal.addOutput("Showing " + std::to_string(dm.seriesCount()) + " series.");
        } else if (type == "episodes") {
            std::vector<TableRow> rows;
            for (auto& e : dm.getEpisodes()) {
                TableRow r;
                r.columns.push_back(std::to_string(e.getId()));
                r.columns.push_back(e.getName());
                r.columns.push_back(std::to_string(e.getLength()));
                r.columns.push_back(e.getGenre());
                r.columns.push_back(std::to_string(static_cast<int>(e.getRating())));
                r.columns.push_back(std::to_string(e.getSeason()));
                r.columns.push_back(std::to_string(e.getEpisodeNumber()));
                r.columns.push_back(e.getTitle());
                rows.push_back(r);
            }
            dataTable.setHeaders({"ID", "Series", "Length", "Genre", "Rating", "Season", "Ep#", "Title"});
            dataTable.setData(rows);
            dataTable.resetPage();
            terminal.addOutput("Showing " + std::to_string(dm.episodeCount()) + " episodes.");
        } else {
            terminal.addOutput("Unknown table: " + type, true);
        }
    };

    terminal.setOnCommand([&](const std::string &cmd) {
        terminal.addCommand(cmd);
        try {
        auto args = splitQuoted(cmd);
        if (args.empty()) {
            terminal.addOutput("Type :help for available commands.");
            return;
        }

        // lowercase first arg (command) for case-insensitive dispatch
        std::string cmdLower = args[0];
        for (auto& c : cmdLower) c = std::tolower(c);
        auto lower = [](const std::string& s) -> std::string {
            std::string r = s;
            for (auto& c : r) c = std::tolower(c);
            return r;
        };


        auto printTableCLI = [&](const std::string& type) {
            if (type == "movies") {
                for (auto& m : dm.getMovies()) {
                    terminal.addOutput("  #" + std::to_string(m.getId()) + " " + m.getName()
                        + " (" + std::to_string(m.getLength()) + "min, " + m.getGenre()
                        + ", " + std::to_string(static_cast<int>(m.getRating())) + "/10)");
                }
                terminal.addOutput("Total: " + std::to_string(dm.movieCount()) + " movies.");
            } else if (type == "series") {
                for (auto& s : dm.getSeries()) {
                    terminal.addOutput("  #" + std::to_string(s.getId()) + " " + s.getName()
                        + " (S" + std::to_string(s.getSeason()) + ", "
                        + std::to_string(static_cast<int>(s.getRating())) + "/10)");
                }
                terminal.addOutput("Total: " + std::to_string(dm.seriesCount()) + " series.");
            } else if (type == "episodes") {
                for (auto& e : dm.getEpisodes()) {
                    terminal.addOutput("  #" + std::to_string(e.getId()) + " " + e.getName()
                        + " S" + std::to_string(e.getSeason())
                        + "E" + std::to_string(e.getEpisodeNumber())
                        + " \"" + e.getTitle() + "\" ("
                        + std::to_string(static_cast<int>(e.getRating())) + "/10)");
                }
                terminal.addOutput("Total: " + std::to_string(dm.episodeCount()) + " episodes.");
            } else {
                terminal.addOutput("Unknown type: " + type, true);
            }
        };

        auto currentTableName = [&]() -> std::string {
            auto h = dataTable.headers();
            if (h.empty()) return "";
            if (h.size() >= 6 && h[5] == "Director") return "movies";
            if (h.size() >= 6 && h[5] == "Season") return "series";
            if (h.size() >= 8) return "episodes";
            return "";
        };

        if (cmdLower == ":help") {
            terminal.addOutput("Available commands:");
            terminal.addOutput("  :table <movies|series|episodes>");
            terminal.addOutput("  :tableCLI <movies|series|episodes>");
            terminal.addOutput("  :show <type> <id>");
            terminal.addOutput("  :graph histogram <rating|length> <N|\"title1\" \"title2\">");
            terminal.addOutput("  :graph pieChart <rating|length> <N|\"title1\" \"title2\">");
            terminal.addOutput("  :graph gradsquares <seriesName>");
            terminal.addOutput("  :cleargraph");
            terminal.addOutput("  :get <name|rating|length|size> [type] [id]");
            terminal.addOutput("  :set <name|rating|length> <type> <id> <value>");
            terminal.addOutput("  :add <movie|series|episode> <id> <args...>");
            terminal.addOutput("  :remove <movie|series|episode> <id>");
            terminal.addOutput("  :showobjectsinuse");
            terminal.addOutput("  :save");
            terminal.addOutput("  :clear");
        } else if (cmdLower == ":clear") {
            terminal.clear();
            terminal.addOutput("Terminal cleared.");
        } else if (cmdLower == ":cleargraph") {
            hist.clearBars();
            hist.setChartMode(ui::Histogram::Bar);
            hist.setTitle(" Data View ");
            hist.setMaxValue(0);
            terminal.addOutput("Graph cleared.");
        } else if (cmdLower == ":save") {
            dm.saveToDb();
            terminal.addOutput("Data saved to database.");
        } else if (cmdLower == ":table") {
            if (args.size() < 2) {
                terminal.addOutput("Usage: :table <movies|series|episodes>", true);
                return;
            }
            populateDataTable(lower(args[1]));
        } else if (cmdLower == ":tablecli") {
            if (args.size() < 2) {
                terminal.addOutput("Usage: :tableCLI <movies|series|episodes>", true);
                return;
            }
            printTableCLI(lower(args[1]));
        } else if (cmdLower == ":show") {
            if (args.size() < 3) {
                terminal.addOutput("Usage: :show <type> <id>", true);
                return;
            }
            std::string type = lower(args[1]);
            uint32_t id = static_cast<uint32_t>(std::stoi(args[2]));
            if (type == "series") {
                Series* s = dm.findSeriesById(id);
                if (!s) {
                    terminal.addOutput("Series #" + std::to_string(id) + " not found.", true);
                    return;
                }
                terminal.addOutput("Series: #" + std::to_string(s->getId()) + " " + s->getName()
                    + " (S" + std::to_string(s->getSeason()) + ", "
                    + std::to_string(static_cast<int>(s->getRating())) + "/10)");
                auto episodes = dm.findEpisodesBySeriesName(s->getName());
                if (episodes.empty()) {
                    terminal.addOutput("  No episodes found.");
                } else {
                    for (auto& e : episodes) {
                        terminal.addOutput("  S" + std::to_string(e.getSeason())
                            + "E" + std::to_string(e.getEpisodeNumber())
                            + " \"" + e.getTitle() + "\" ("
                            + std::to_string(static_cast<int>(e.getRating())) + "/10, "
                            + std::to_string(e.getLength()) + "min)");
                    }
                }
            } else if (type == "movie") {
                Movie* m = dm.findMovieById(id);
                if (!m) {
                    terminal.addOutput("Movie #" + std::to_string(id) + " not found.", true);
                    return;
                }
                terminal.addOutput("Movie: #" + std::to_string(m->getId()) + " " + m->getName()
                    + " (" + std::to_string(m->getLength()) + "min, " + m->getGenre()
                    + ", " + std::to_string(static_cast<int>(m->getRating())) + "/10)");
                terminal.addOutput("  Director: " + m->getDirector());
            } else if (type == "episode") {
                Episodes* e = dm.findEpisodeById(id);
                if (!e) {
                    terminal.addOutput("Episode #" + std::to_string(id) + " not found.", true);
                    return;
                }
                terminal.addOutput("Episode: #" + std::to_string(e->getId()) + " " + e->getName()
                    + " S" + std::to_string(e->getSeason())
                    + "E" + std::to_string(e->getEpisodeNumber())
                    + " \"" + e->getTitle() + "\" ("
                    + std::to_string(static_cast<int>(e->getRating())) + "/10, "
                    + std::to_string(e->getLength()) + "min)");
            } else {
                terminal.addOutput("Unknown type: " + type, true);
            }
        } else if (cmdLower == ":graph") {
            if (args.size() < 2) {
                terminal.addOutput("Usage: :graph histogram <rating|length> <N|\"title1\" \"title2\">", true);
                terminal.addOutput("       :graph pieChart <rating|length> <N|\"title1\" \"title2\">", true);
                terminal.addOutput("       :graph gradsquares <seriesName>", true);
                return;
            }
            std::string sub = lower(args[1]);

            std::string colors[] = {"#e6194b", "#3cb44b", "#ffe119", "#4363d8", "#f58231",
                                    "#911eb4", "#42d4f4", "#f032e6", "#bfef45", "#fabed4"};

            auto ratingColor = [](float val) -> std::string {
                if (val <= 2) return "#9c27b0";
                if (val <= 4) return "#f44336";
                if (val <= 6) return "#ffeb3b";
                if (val <= 8) return "#4caf50";
                return "#2196f3";
            };

            auto curTable = currentTableName();
            auto getNameForTable = [&](int i) -> std::string {
                if (curTable == "episodes") {
                    auto& vec = dm.getEpisodes();
                    if (i < (int)vec.size())
                        return vec[i].getName() + " S" + std::to_string(vec[i].getSeason())
                            + "E" + std::to_string(vec[i].getEpisodeNumber());
                    return "";
                }
                return "";
            };
            auto addTopBars = [&](auto &vec, int count, const std::string &metric, auto getName) {
                std::sort(vec.begin(), vec.end(),
                    [&](auto &a, auto &b) {
                        return metric == "rating"
                            ? a.getRating() > b.getRating()
                            : a.getLength() > b.getLength();
                    });
                count = std::min(count, static_cast<int>(vec.size()));
                for (int i = 0; i < count; ++i) {
                    float val = metric == "rating"
                        ? static_cast<float>(vec[i].getRating())
                        : static_cast<float>(vec[i].getLength());
                    hist.addBar(getName(vec[i]), val, colors[i % 10]);
                }
                return count;
            };
            auto findAndAddBar = [&](const std::string &name, int idx, const std::string &metric) -> bool {
                auto tryAdd = [&](auto &vec, auto getName, auto getVal) -> bool {
                    for (auto &item : vec) {
                        std::string itemName = getName(item);
                        std::string in = itemName;
                        for (auto &c : in) c = std::tolower(c);
                        std::string nl = name;
                        for (auto &c : nl) c = std::tolower(c);
                        if (in == nl || in.find(nl) != std::string::npos || nl.find(in) != std::string::npos) {
                            float val = static_cast<float>(getVal(item));
                            hist.addBar(itemName, val, colors[idx]);
                            return true;
                        }
                    }
                    return false;
                };
                if (curTable == "movies") return tryAdd(dm.getMovies(),
                    [](Movie &m) -> std::string { return m.getName(); },
                    [&](Movie &m) -> float { return metric == "rating" ? m.getRating() : m.getLength(); });
                if (curTable == "series") return tryAdd(dm.getSeries(),
                    [](Series &s) -> std::string { return s.getName(); },
                    [&](Series &s) -> float { return metric == "rating" ? s.getRating() : s.getLength(); });
                if (curTable == "episodes") return tryAdd(dm.getEpisodes(),
                    [](Episodes &e) -> std::string { return e.getName(); },
                    [&](Episodes &e) -> float { return metric == "rating" ? e.getRating() : e.getLength(); });
                return false;
            };

            // --- histogram mode ---
            if (sub == "histogram") {
                if (args.size() == 2) {
                    hist.setChartMode(ui::Histogram::Bar);
                    hist.setTitle(" Histogram Mode ");
                    if (hist.barCount() == 0)
                        terminal.addOutput("No data graphed yet. Use :graph histogram <rating|length> <N> first.");
                    else
                        terminal.addOutput("Switched to histogram view.");
                    return;
                }
                // :graph histogram <metric> <N|"title1" "title2">
                if (args.size() < 4) {
                    terminal.addOutput("Usage: :graph histogram <rating|length> <N|\"title1\" \"title2\">", true);
                    return;
                }
                std::string metric = lower(args[2]);
                hist.clearBars();
                hist.setChartMode(ui::Histogram::Bar);
                hist.setMaxValue(metric == "rating" ? 10.0f : 100.0f);

                bool isNumeric = true;
                try { size_t pos = 0; std::stoi(args[3], &pos); if (pos != args[3].size()) isNumeric = false; }
                catch (...) { isNumeric = false; }

                if (isNumeric) {
                    int n = std::stoi(args[3]);
                    hist.setTitle("Top " + std::to_string(n) + " by " + metric);
                    int count = 0;
                    if (curTable == "movies")
                        count = addTopBars(dm.getMovies(), n, metric,
                            [](Movie &m) -> std::string { return m.getName(); });
                    else if (curTable == "series")
                        count = addTopBars(dm.getSeries(), n, metric,
                            [](Series &s) -> std::string { return s.getName(); });
                    else if (curTable == "episodes")
                        count = addTopBars(dm.getEpisodes(), n, metric,
                            [](Episodes &e) -> std::string {
                                return e.getName() + " S" + std::to_string(e.getSeason())
                                    + "E" + std::to_string(e.getEpisodeNumber()); });
                    else {
                        terminal.addOutput("No table loaded. Use :table first.", true);
                        hist.setTitle(" Data View ");
                        return;
                    }
                    terminal.addOutput("Graphed top " + std::to_string(count) + " " + curTable + " by " + metric + ".");
                } else if (args.size() >= 5) {
                    std::string name1 = args[3];
                    std::string name2 = args[4];
                    hist.setTitle("Comparing: " + name1 + " vs " + name2);
                    bool found1 = findAndAddBar(name1, 0, metric);
                    bool found2 = findAndAddBar(name2, 1, metric);
                    if (!found1) terminal.addOutput("Could not find: " + name1, true);
                    if (!found2) terminal.addOutput("Could not find: " + name2, true);
                    if (found1 || found2) terminal.addOutput("Comparison graphed.");
                } else {
                    terminal.addOutput("Usage: :graph histogram <rating|length> <N|\"title1\" \"title2\">", true);
                }
                return;
            }

            // --- pie chart mode ---
            if (sub == "piechart") {
                if (args.size() < 4) {
                    terminal.addOutput("Usage: :graph pieChart <rating|length> <N|\"title1\" \"title2\">", true);
                    return;
                }
                std::string metric = lower(args[2]);
                hist.clearBars();
                hist.setChartMode(ui::Histogram::Pie);
                hist.setMaxValue(metric == "rating" ? 10.0f : 100.0f);

                bool isNumeric = true;
                try { size_t pos = 0; std::stoi(args[3], &pos); if (pos != args[3].size()) isNumeric = false; }
                catch (...) { isNumeric = false; }

                if (isNumeric) {
                    int n = std::stoi(args[3]);
                    int count = 0;
                    if (curTable == "movies")
                        count = addTopBars(dm.getMovies(), n, metric,
                            [](Movie &m) -> std::string { return m.getName(); });
                    else if (curTable == "series")
                        count = addTopBars(dm.getSeries(), n, metric,
                            [](Series &s) -> std::string { return s.getName(); });
                    else if (curTable == "episodes")
                        count = addTopBars(dm.getEpisodes(), n, metric,
                            [](Episodes &e) -> std::string {
                                return e.getName() + " S" + std::to_string(e.getSeason())
                                    + "E" + std::to_string(e.getEpisodeNumber()); });
                    else {
                        terminal.addOutput("No table loaded. Use :table first.", true);
                        hist.setTitle(" Data View ");
                        return;
                    }
                    hist.setTitle("Pie: Top " + std::to_string(count) + " by " + metric);
                    terminal.addOutput("Pie chart of top " + std::to_string(count) + " " + curTable + " by " + metric + ".");
                } else if (args.size() >= 5) {
                    std::string name1 = args[3];
                    std::string name2 = args[4];
                    hist.setTitle("Pie: " + name1 + " vs " + name2);
                    bool found1 = findAndAddBar(name1, 0, metric);
                    bool found2 = findAndAddBar(name2, 1, metric);
                    if (!found1) terminal.addOutput("Could not find: " + name1, true);
                    if (!found2) terminal.addOutput("Could not find: " + name2, true);
                    if (found1 || found2) terminal.addOutput("Pie comparison graphed.");
                } else {
                    terminal.addOutput("Usage: :graph pieChart <rating|length> <N|\"title1\" \"title2\">", true);
                }
                return;
            }

            // --- gradsquares mode ---
            if (sub == "gradsquares") {
                if (args.size() < 3) {
                    terminal.addOutput("Usage: :graph gradsquares <seriesName>", true);
                    return;
                }
                std::string seriesName = args[2];
                auto episodes = dm.findEpisodesBySeriesName(seriesName);
                if (episodes.empty()) {
                    terminal.addOutput("No episodes found for series: " + seriesName, true);
                    return;
                }
                std::vector<ui::GridRow> gridRows;
                std::map<uint32_t, std::vector<ui::GridCell>> seasonMap;
                for (auto& e : episodes) {
                    float rating = static_cast<float>(e.getRating());
                    seasonMap[e.getSeason()].push_back(
                        ui::GridCell(rating, ratingColor(rating), "S" + std::to_string(e.getSeason())
                            + "E" + std::to_string(e.getEpisodeNumber()) + ": "
                            + std::to_string(static_cast<int>(rating)) + "/10"));
                }
                for (auto &kv : seasonMap) {
                    ui::GridRow row("Season " + std::to_string(kv.first));
                    row.cells = std::move(kv.second);
                    gridRows.push_back(std::move(row));
                }
                hist.setChartMode(ui::Histogram::Grid);
                hist.setGridData(gridRows);
                hist.setMaxValue(10);
                hist.setTitle(" " + seriesName + " by Rating ");
                terminal.addOutput("Graphed " + std::to_string(episodes.size())
                    + " episodes from " + seriesName + " as grid.");
                return;
            }

            terminal.addOutput("Unknown graph type: " + sub, true);
        } else if (cmdLower == ":get") {
            if (args.size() < 2) {
                terminal.addOutput("Usage: :get <name|rating|length|size> [type] [id]", true);
                return;
            }
            if (lower(args[1]) == "size") {
                if (args.size() < 3) {
                    terminal.addOutput("Usage: :get size <movies|series|episodes>", true);
                    return;
                }
                std::string getType = lower(args[2]);
                if (getType == "movies") terminal.addOutput("Movie count: " + std::to_string(dm.movieCount()));
                else if (getType == "series") terminal.addOutput("Series count: " + std::to_string(dm.seriesCount()));
                else if (getType == "episodes") terminal.addOutput("Episode count: " + std::to_string(dm.episodeCount()));
                else terminal.addOutput("Unknown type: " + args[2], true);
            } else if (lower(args[1]) == "name" || lower(args[1]) == "rating" || lower(args[1]) == "length") {
                if (args.size() < 4) {
                    terminal.addOutput("Usage: :get <name|rating|length> <type> <id>", true);
                    return;
                }
                std::string prop = lower(args[1]);
                std::string type = lower(args[2]);
                uint32_t id = static_cast<uint32_t>(std::stoi(args[3]));
                if (type == "movie") {
                    Movie* m = dm.findMovieById(id);
                    if (!m) { terminal.addOutput("Movie not found.", true); return; }
                    if (prop == "name") terminal.addOutput(m->getName());
                    else if (prop == "rating") terminal.addOutput(std::to_string(static_cast<int>(m->getRating())));
                    else if (prop == "length") terminal.addOutput(std::to_string(m->getLength()));
                } else if (type == "series") {
                    Series* s = dm.findSeriesById(id);
                    if (!s) { terminal.addOutput("Series not found.", true); return; }
                    if (prop == "name") terminal.addOutput(s->getName());
                    else if (prop == "rating") terminal.addOutput(std::to_string(static_cast<int>(s->getRating())));
                    else if (prop == "length") terminal.addOutput(std::to_string(s->getLength()));
                } else if (type == "episode") {
                    Episodes* e = dm.findEpisodeById(id);
                    if (!e) { terminal.addOutput("Episode not found.", true); return; }
                    if (prop == "name") terminal.addOutput(e->getName());
                    else if (prop == "rating") terminal.addOutput(std::to_string(static_cast<int>(e->getRating())));
                    else if (prop == "length") terminal.addOutput(std::to_string(e->getLength()));
                } else {
                    terminal.addOutput("Unknown type: " + type, true);
                }
            }
        } else if (cmdLower == ":set") {
            if (args.size() < 5) {
                terminal.addOutput("Usage: :set <name|rating|length> <type> <id> <value>", true);
                return;
            }
            std::string prop = lower(args[1]);
            std::string type = lower(args[2]);
            uint32_t id = static_cast<uint32_t>(std::stoi(args[3]));
            std::string value = args[4];

            if (type == "movie") {
                Movie* m = dm.findMovieById(id);
                if (!m) { terminal.addOutput("Movie not found.", true); return; }
                if (prop == "name") { m->setName(value); terminal.addOutput("Name updated."); }
                else if (prop == "rating") { m->setRating(static_cast<uint8_t>(std::stoi(value))); terminal.addOutput("Rating updated."); }
                else if (prop == "length") { m->setLength(static_cast<uint32_t>(std::stoi(value))); terminal.addOutput("Length updated."); }
            } else if (type == "series") {
                Series* s = dm.findSeriesById(id);
                if (!s) { terminal.addOutput("Series not found.", true); return; }
                if (prop == "name") { s->setName(value); terminal.addOutput("Name updated."); }
                else if (prop == "rating") { s->setRating(static_cast<uint8_t>(std::stoi(value))); terminal.addOutput("Rating updated."); }
                else if (prop == "length") { s->setLength(static_cast<uint32_t>(std::stoi(value))); terminal.addOutput("Length updated."); }
            } else if (type == "episode") {
                Episodes* e = dm.findEpisodeById(id);
                if (!e) { terminal.addOutput("Episode not found.", true); return; }
                if (prop == "name") { e->setName(value); terminal.addOutput("Name updated."); }
                else if (prop == "rating") { e->setRating(static_cast<uint8_t>(std::stoi(value))); terminal.addOutput("Rating updated."); }
                else if (prop == "length") { e->setLength(static_cast<uint32_t>(std::stoi(value))); terminal.addOutput("Length updated."); }
            } else {
                terminal.addOutput("Unknown type: " + type, true);
            }
        } else if (cmdLower == ":add") {
            if (args.size() < 2) {
                terminal.addOutput("Usage: :add <movie|series|episode> <id> <args...>", true);
                return;
            }
            std::string subtype = lower(args[1]);
            auto parseRating = [](const std::string& s) -> uint8_t {
                return static_cast<uint8_t>(std::stoul(s));
            };
            if (subtype == "movie") {
                if (args.size() < 8) {
                    terminal.addOutput("Usage: :add movie <id> \"<name>\" <length> \"<genre>\" <rating> \"<director>\"", true);
                    return;
                }
                uint32_t newId = static_cast<uint32_t>(std::stoul(args[2]));
                std::string name = args[3];
                uint32_t length = static_cast<uint32_t>(std::stoul(args[4]));
                std::string genre = args[5];
                uint8_t rating = parseRating(args[6]);
                std::string director = args[7];
                dm.addMovie(Movie(newId, name, length, genre, rating, director));
                terminal.addOutput("Movie #" + std::to_string(newId) + " added.");
                populateDataTable("movies");
            } else if (subtype == "series") {
                if (args.size() < 7) {
                    terminal.addOutput("Usage: :add series <id> \"<name>\" <length> \"<genre>\" <rating> <season>", true);
                    return;
                }
                uint32_t newId = static_cast<uint32_t>(std::stoul(args[2]));
                std::string name = args[3];
                uint32_t length = static_cast<uint32_t>(std::stoul(args[4]));
                std::string genre = args[5];
                uint8_t rating = parseRating(args[6]);
                uint32_t season = args.size() > 7 ? static_cast<uint32_t>(std::stoul(args[7])) : 1;
                dm.addSeries(Series(newId, name, length, genre, rating, season, name));
                terminal.addOutput("Series #" + std::to_string(newId) + " added.");
                populateDataTable("series");
            } else if (subtype == "episode") {
                if (args.size() < 9) {
                    terminal.addOutput("Usage: :add episode <id> \"<seriesName>\" <length> \"<genre>\" <rating> <season> \"<title>\" [epNumber]", true);
                    return;
                }
                uint32_t newId = static_cast<uint32_t>(std::stoul(args[2]));
                std::string seriesName = args[3];
                uint32_t length = static_cast<uint32_t>(std::stoul(args[4]));
                std::string genre = args[5];
                uint8_t rating = parseRating(args[6]);
                uint32_t season = static_cast<uint32_t>(std::stoul(args[7]));
                std::string title = args[8];
                uint32_t epNumber = args.size() > 9 ? static_cast<uint32_t>(std::stoul(args[9])) : 1;
                dm.addEpisode(Episodes(newId, seriesName, length, genre, rating, season, title, epNumber));
                terminal.addOutput("Episode #" + std::to_string(newId) + " added.");
                populateDataTable("episodes");
            } else {
                terminal.addOutput("Unknown type: " + subtype, true);
            }
        } else if (cmdLower == ":remove") {
            if (args.size() < 3) {
                terminal.addOutput("Usage: :remove <movie|series|episode> <id>", true);
                return;
            }
            std::string subtype = lower(args[1]);
            uint32_t id = static_cast<uint32_t>(std::stoi(args[2]));
            if (subtype == "movie") {
                if (dm.removeMovie(id)) {
                    terminal.addOutput("Movie #" + std::to_string(id) + " removed.");
                    populateDataTable("movies");
                } else {
                    terminal.addOutput("Movie not found.", true);
                }
            } else if (subtype == "series") {
                if (dm.removeSeries(id)) {
                    terminal.addOutput("Series #" + std::to_string(id) + " and its episodes removed.");
                    populateDataTable("series");
                } else {
                    terminal.addOutput("Series not found.", true);
                }
            } else if (subtype == "episode") {
                if (dm.removeEpisode(id)) {
                    terminal.addOutput("Episode #" + std::to_string(id) + " removed.");
                    populateDataTable("episodes");
                } else {
                    terminal.addOutput("Episode not found.", true);
                }
            } else {
                terminal.addOutput("Unknown type: " + subtype, true);
            }
        } else if (cmdLower == ":showobjectsinuse") {
            std::vector<ui::TableRow> rows;
            for (auto& m : dm.getMovies()) {
                ui::TableRow r;
                r.columns.push_back(std::to_string(m.getId()));
                r.columns.push_back("Movie");
                r.columns.push_back(m.getName());
                r.columns.push_back(std::to_string(m.getLength()));
                r.columns.push_back(m.getGenre());
                r.columns.push_back(std::to_string(static_cast<int>(m.getRating())));
                r.columns.push_back(m.getDirector());
                rows.push_back(r);
            }
            for (auto& s : dm.getSeries()) {
                ui::TableRow r;
                r.columns.push_back(std::to_string(s.getId()));
                r.columns.push_back("Series");
                r.columns.push_back(s.getName());
                r.columns.push_back(std::to_string(s.getLength()));
                r.columns.push_back(s.getGenre());
                r.columns.push_back(std::to_string(static_cast<int>(s.getRating())));
                r.columns.push_back("S" + std::to_string(s.getSeason()));
                rows.push_back(r);
            }
            for (auto& e : dm.getEpisodes()) {
                ui::TableRow r;
                r.columns.push_back(std::to_string(e.getId()));
                r.columns.push_back("Episode");
                r.columns.push_back(e.getName());
                r.columns.push_back(std::to_string(e.getLength()));
                r.columns.push_back(e.getGenre());
                r.columns.push_back(std::to_string(static_cast<int>(e.getRating())));
                r.columns.push_back("S" + std::to_string(e.getSeason()) + "E" + std::to_string(e.getEpisodeNumber()));
                rows.push_back(r);
            }
            dataTable.setHeaders({"ID", "Type", "Name", "Length", "Genre", "Rating", "Extra"});
            dataTable.setData(rows);
            dataTable.resetPage();
            size_t total = dm.movieCount() + dm.seriesCount() + dm.episodeCount();
            terminal.addOutput("Showing " + std::to_string(total) + " objects (M:" + std::to_string(dm.movieCount())
                + " S:" + std::to_string(dm.seriesCount())
                + " E:" + std::to_string(dm.episodeCount()) + "). Use Prev/Next to navigate.");
        } else {
            terminal.addOutput("Unknown command: " + cmd, true);
        }
        } catch (const std::exception &e) {
            terminal.addOutput(std::string("Error: ") + e.what(), true);
        } catch (...) {
            terminal.addOutput("Unknown error occurred.", true);
        }
    });
    terminal.addOutput("Type :help for available commands.");

    TextBox txtAccount(
        component::width = 40,
        component::bgColor = "#161b22",
        component::fgColor = "#58a6ff",
        component::borderColor = "#30363d",
        component::align = Align::TopCenter,
        component::maxLength = 60
    );
    txtAccount.setText(" User: " + username_ + " ");

    TextBox txtSessionInfo(
        component::width = 40,
        component::bgColor = "#0d1117",
        component::fgColor = "#8b949e",
        component::borderColor = "#30363d",
        component::align = Align::MiddleCenter,
        component::maxLength = 60
    );

    Button btnLogout(
        component::text = " Terminate Session ",
        component::width = 40,
        component::bgColor = "#da3633",
        component::fgColor = "#ffffff",
        component::borderColor = "#ff6b6b",
        component::align = Align::BottomCenter
    );

    btnLogout.setOnActivate([&]() {
        terminal.addOutput("Saving data and terminating session...");
        dm.saveToDb();
        terminal.addOutput("Goodbye, " + username_ + "!");
        reencryptNow();
        dashMap.stop();
    });

    terminal.addOutput("Movies: " + std::to_string(dm.movieCount())
        + " | Series: " + std::to_string(dm.seriesCount())
        + " | Episodes: " + std::to_string(dm.episodeCount()));
    populateDataTable("movies");

    Partition dashArea(Partition::FourSide, 50.0f, 55.0f);
    dashArea.topLeft().setBackground("#0d1117");
    dashArea.topRight().setBackground("#0d1117");
    dashArea.bottomLeft().setBackground("#0d1117");
    dashArea.bottomRight().setBackground("#0d1117");

    dashArea.topLeft().attach(&dataTable);
    dashArea.topLeft().attach(&btnPrev);
    dashArea.topLeft().attach(&btnNext);
    dashArea.topRight().attach(&hist);
    dashArea.bottomLeft().attach(&terminal);
    dashArea.bottomRight().attach(&txtAccount);
    dashArea.bottomRight().attach(&txtSessionInfo);
    dashArea.bottomRight().attach(&btnLogout);

    dashMap.bind(dashBar);
    dashMap.setPartition(&dashArea);

    dashMap.run();

    dm.saveToDb();

    return true;
}
