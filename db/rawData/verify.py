import sqlite3
import os

DB_PATH = os.path.join(os.path.dirname(__file__), '..', 'movieverse.db')
conn = sqlite3.connect(DB_PATH)
c = conn.cursor()

print("=== Tables ===")
c.execute("SELECT name FROM sqlite_master WHERE type='table'")
for t in c.fetchall():
    print(f"  {t[0]}")

print()
print("=== Series ===")
c.execute("SELECT id, name, genre, rating FROM series ORDER BY id")
for r in c.fetchall():
    print(f"  {r}")

print()
print("=== Episode count per series ===")
c.execute("""SELECT s.name, COUNT(e.id) as eps, COUNT(DISTINCT e.season) as seasons
             FROM series s LEFT JOIN episodes e ON s.id = e.series_id
             GROUP BY s.id ORDER BY s.id""")
for r in c.fetchall():
    print(f"  {r}")

print()
print("=== First 5 episodes ===")
c.execute("""SELECT e.id, s.name, e.season, e.episode_number, e.title, e.length, e.rating
             FROM episodes e JOIN series s ON e.series_id = s.id
             ORDER BY e.id LIMIT 5""")
for r in c.fetchall():
    print(f"  {r}")

print()
print("=== Emily in Paris case check ===")
c.execute("""SELECT s.name, COUNT(e.id)
             FROM series s JOIN episodes e ON s.id = e.series_id
             WHERE s.name LIKE '%Emily%'
             GROUP BY s.id""")
for r in c.fetchall():
    print(f"  {r}")

print()
print("=== Movies ===")
c.execute("SELECT id, name, length, genre, rating, director FROM movies ORDER BY id")
for r in c.fetchall():
    print(f"  {r}")

conn.close()
