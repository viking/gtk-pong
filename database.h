#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
  sqlite3 *db;
} database;

typedef struct {
  int id;
  char *rfid;
  char *name;
  char *gender;
  int elo;
  char *image;
  int play_count;
} player;

typedef struct {
  int count;
  player *arr;
} players;

database *database_open();
void database_close(database *db);

players *database_get_players(database *db);
int database_insert_player(database *db, player *p);
void player_free(player *p);
void players_free(players *p);
