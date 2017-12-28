#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
  sqlite3 *db;
} database;

database *database_open();
void database_close(database *db);
