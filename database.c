#include "database.h"

database *
database_open()
{
  database *db = NULL;
  sqlite3_stmt *stmt = NULL;
  int result = 0, version = 0;

  /* open database */
  db = (database *)malloc(sizeof(database));
  result = sqlite3_open_v2("pong.sqlite3", &db->db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
  if (result != SQLITE_OK) {
    goto fail;
  }

  /* find version number */
  result = sqlite3_prepare_v2(db->db,
      "SELECT COUNT(*) FROM sqlite_master WHERE name = 'schema_info'", -1,
      &stmt, NULL);
  if (result != SQLITE_OK) {
    goto fail;
  }
  result = sqlite3_step(stmt);
  if (result != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    goto fail;
  }
  result = sqlite3_column_int(stmt, 0);
  if (result == 1) {
    sqlite3_finalize(stmt);

    result = sqlite3_prepare_v2(db->db, "SELECT version FROM schema_info", -1, &stmt, NULL);
    if (result != SQLITE_OK) {
      goto fail;
    }
    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
      sqlite3_finalize(stmt);
      goto fail;
    }
    version = sqlite3_column_int(stmt, 0);
  }

  if (version == 0) {
    result = sqlite3_exec(db->db,
        "CREATE TABLE players ("
        "  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        "  rfid TEXT,"
        "  name TEXT,"
        "  gender TEXT,"
        "  elo INTEGER,"
        "  image TEXT,"
        "  play_count INTEGER"
        ");"
        "CREATE TABLE games ("
        "  id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,"
        "  player1_id INTEGER,"
        "  player2_id INTEGER,"
        "  game_type TEXT,"
        "  started_at TEXT,"
        "  ended_at TEXT,"
        "  player1_score INTEGER,"
        "  player2_score INTEGER,"
        "  score_delta INTEGER,"
        "  winner_id INTEGER"
        ");"
        "CREATE TABLE schema_info (version INTEGER);"
        "INSERT INTO schema_info (version) VALUES (1);",
        NULL, NULL, NULL);
    if (result != SQLITE_OK) {
      goto fail;
    }
  }

  return db;

fail:
  database_close(db);
  return NULL;
}

void
database_close(db)
  database *db;
{
  sqlite3_close(db->db);
  free(db);
}
