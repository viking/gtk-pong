#include "store.h"

GtkListStore *
players_store()
{
  database *db = NULL;
  players *p = NULL;
  GtkListStore *store = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  int i = 0;

  db = database_open();
  p = database_get_players(db);
  database_close(db);
  if (p == NULL) {
    g_print("couldn't get players!");
    return NULL;
  }

  store = gtk_list_store_new(7,
      G_TYPE_INT,    /* id */
      G_TYPE_STRING, /* rfid */
      G_TYPE_STRING, /* name */
      G_TYPE_STRING, /* gender */
      G_TYPE_INT,    /* elo */
      G_TYPE_STRING, /* image */
      G_TYPE_INT);   /* play_count */

  for (i = 0; i < p->count; i++) {
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
        0, p->arr[i].id,
        1, p->arr[i].rfid,
        2, p->arr[i].name,
        3, p->arr[i].gender,
        4, p->arr[i].elo,
        5, p->arr[i].image,
        6, p->arr[i].play_count,
        -1);
  }
  players_free(p);
  return store;
}

int
players_store_insert(store, p)
  GtkListStore *store;
  player *p;
{
  database *db = NULL;
  GtkTreeIter iter;
  int result = 0;

  db = database_open();
  result = database_insert_player(db, p);
  database_close(db);
  if (result != 0) {
    return 1;
  }

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter,
      0, p->id,
      1, p->rfid,
      2, p->name,
      3, p->gender,
      4, p->elo,
      5, p->image,
      6, p->play_count,
      -1);

  return 0;
}
