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
  sqlite3_finalize(stmt);

  if (result == 1) {
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

players *
database_get_players(db)
  database *db;
{
  sqlite3_stmt *stmt = NULL;
  players *p = NULL;
  int result = 0, count = 0, i = 0;
  const char *str = NULL;

  result = sqlite3_prepare_v2(db->db,
      "SELECT COUNT(*) FROM players", -1,
      &stmt, NULL);
  if (result != SQLITE_OK) {
    return NULL;
  }
  result = sqlite3_step(stmt);
  if (result != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    return NULL;
  }
  count = sqlite3_column_int(stmt, 0);
  sqlite3_finalize(stmt);

  if (count == 0) {
    p = (players *)malloc(sizeof(players));
    p->count = count;
    p->arr = NULL;
    return p;
  }

  result = sqlite3_prepare_v2(db->db,
      "SELECT id, rfid, name, gender, elo, image, play_count FROM players", -1,
      &stmt, NULL);
  if (result != SQLITE_OK) {
    return NULL;
  }

  /* initialize players */
  p = (players *)malloc(sizeof(players));
  p->count = count;
  p->arr = (player *)malloc(sizeof(player) * count);
  for (i = 0; i < count; i++) {
    p->arr[i].rfid = NULL;
    p->arr[i].name = NULL;
    p->arr[i].gender = NULL;
    p->arr[i].image = NULL;
  }

  /* populate players */
  for (i = 0; i < count; i++) {
    result = sqlite3_step(stmt);
    if (result != SQLITE_ROW) {
      players_free(p);
      sqlite3_finalize(stmt);
      return NULL;
    }
    p->arr[i].id = sqlite3_column_int(stmt, 0);
    p->arr[i].elo = sqlite3_column_int(stmt, 4);
    p->arr[i].play_count = sqlite3_column_int(stmt, 6);

    str = sqlite3_column_text(stmt, 1);
    p->arr[i].rfid = str == NULL ? NULL : strdup(str);

    str = sqlite3_column_text(stmt, 2);
    p->arr[i].name = str == NULL ? NULL : strdup(str);

    str = sqlite3_column_text(stmt, 3);
    p->arr[i].gender = str == NULL ? NULL : strdup(str);

    str = sqlite3_column_text(stmt, 5);
    p->arr[i].image = str == NULL ? NULL : strdup(str);
  }

  return p;
}

int
database_insert_player(db, p)
  database *db;
  player *p;
{
  sqlite3_stmt *stmt = NULL;
  int result = 0;

  result = sqlite3_prepare_v2(db->db,
      "INSERT INTO players (rfid, name, gender, elo, image, play_count)"
      "VALUES (?, ?, ?, ?, ?, ?)",
      -1, &stmt, NULL);
  if (result != SQLITE_OK) {
    return 1;
  }

  result = sqlite3_bind_text(stmt, 0, p->rfid, -1, SQLITE_TRANSIENT);
  if (result == SQLITE_OK) {
    result = sqlite3_bind_text(stmt, 1, p->name, -1, SQLITE_TRANSIENT);
  }
  if (result == SQLITE_OK) {
    result = sqlite3_bind_text(stmt, 2, p->gender, -1, SQLITE_TRANSIENT);
  }
  if (result == SQLITE_OK) {
    result = sqlite3_bind_int(stmt, 3, p->elo);
  }
  if (result == SQLITE_OK) {
    result = sqlite3_bind_text(stmt, 4, p->image, -1, SQLITE_TRANSIENT);
  }
  if (result == SQLITE_OK) {
    result = sqlite3_bind_int(stmt, 5, p->play_count);
  }
  if (result != SQLITE_OK) {
    sqlite3_finalize(stmt);
    return 1;
  }

  result = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  if (result != SQLITE_DONE) {
    return 1;
  }

  p->id = (int)sqlite3_last_insert_rowid(db->db);
  return 0;
}

void player_free(p)
  player *p;
{
  if (p->rfid != NULL) {
    free(p->rfid);
  }
  if (p->name != NULL) {
    free(p->name);
  }
  if (p->gender != NULL) {
    free(p->gender);
  }
  if (p->image != NULL) {
    free(p->image);
  }
  free(p);
}

void
players_free(p)
  players *p;
{
  int i = 0;

  for (i = 0; i < p->count; i++) {
    if (p->arr[i].rfid != NULL) {
      free(p->arr[i].rfid);
    }
    if (p->arr[i].name != NULL) {
      free(p->arr[i].name);
    }
    if (p->arr[i].gender != NULL) {
      free(p->arr[i].gender);
    }
    if (p->arr[i].image != NULL) {
      free(p->arr[i].image);
    }
  }
  if (p->arr != NULL) {
    free(p->arr);
  }
  free(p);
}
