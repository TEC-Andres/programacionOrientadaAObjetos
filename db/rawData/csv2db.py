import csv
import sqlite3
import os

RAW_DIR = os.path.dirname(__file__)
DB_PATH = os.path.join(RAW_DIR, '..', 'movieverse.db')

SERIES_CSV = os.path.join(RAW_DIR, 'MovieVerse Database - Series.csv')
EPISODES_CSV = os.path.join(RAW_DIR, 'MovieVerse Database - Episodes.csv')
MOVIE_CSV = os.path.join(RAW_DIR, 'MovieVerse Database - Movie.csv')


def clean(val):
    val = val.strip()
    if len(val) >= 2 and val.startswith('"') and val.endswith('"'):
        val = val[1:-1].strip()
    return val


def main():
    if os.path.exists(DB_PATH):
        os.remove(DB_PATH)

    conn = sqlite3.connect(DB_PATH)
    conn.execute("PRAGMA foreign_keys = ON")
    conn.execute("PRAGMA journal_mode = OFF")
    c = conn.cursor()

    c.execute("""
        CREATE TABLE series (
            id    INTEGER PRIMARY KEY AUTOINCREMENT,
            name  TEXT    NOT NULL UNIQUE,
            genre TEXT    NOT NULL,
            rating REAL
        )
    """)

    c.execute("""
        CREATE TABLE episodes (
            id             INTEGER PRIMARY KEY AUTOINCREMENT,
            series_id      INTEGER NOT NULL,
            season         INTEGER NOT NULL,
            episode_number INTEGER NOT NULL,
            title          TEXT    NOT NULL,
            length         INTEGER NOT NULL,
            rating         REAL,
            FOREIGN KEY (series_id) REFERENCES series(id)
        )
    """)

    c.execute("""
        CREATE TABLE movies (
            id       INTEGER PRIMARY KEY AUTOINCREMENT,
            name     TEXT    NOT NULL,
            length   INTEGER NOT NULL,
            genre    TEXT    NOT NULL,
            rating   REAL,
            director TEXT    NOT NULL
        )
    """)

    # --- Series ---
    with open(SERIES_CSV, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            name = clean(row['Name'])
            genre = clean(row['Genre'])
            rating = float(clean(row['Rating']))
            c.execute(
                "INSERT INTO series (name, genre, rating) VALUES (?, ?, ?)",
                (name, genre, rating)
            )

    # --- Episodes (link to series by name, case-insensitive) ---
    with open(EPISODES_CSV, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            series_name = clean(row['Name'])
            length = int(clean(row['Length']))
            rating = float(clean(row['Rating']))
            season = int(clean(row['Season']))
            title = clean(row['Title'])
            episode_number = int(clean(row['EpisodeNumber']))
            c.execute(
                "INSERT INTO episodes (series_id, season, episode_number, title, length, rating) "
                "VALUES ((SELECT id FROM series WHERE LOWER(name) = LOWER(?)), ?, ?, ?, ?, ?)",
                (series_name, season, episode_number, title, length, rating)
            )

    # --- Movies ---
    with open(MOVIE_CSV, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            name = clean(row['Name'])
            length = int(clean(row['Length']))
            genre = clean(row['Genre'])
            rating = float(clean(row['Rating']))
            director = clean(row['Director'])
            c.execute(
                "INSERT INTO movies (name, length, genre, rating, director) "
                "VALUES (?, ?, ?, ?, ?)",
                (name, length, genre, rating, director)
            )

    conn.commit()
    conn.close()
    print(f"Done. Database created at {DB_PATH}")


if __name__ == '__main__':
    main()
