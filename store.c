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

  store = gtk_list_store_new(8,
      G_TYPE_INT,      /* id */
      G_TYPE_STRING,   /* rfid */
      G_TYPE_STRING,   /* name */
      G_TYPE_STRING,   /* gender */
      G_TYPE_INT,      /* elo */
      G_TYPE_STRING,   /* image */
      G_TYPE_INT,      /* play_count */
      G_TYPE_BOOLEAN); /* visible */

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
        7, TRUE,
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
      7, TRUE,
      -1);

  return 0;
}

void
players_store_hide_player_with_id(store, player_id)
  GtkListStore *store;
  int player_id;
{
  GtkTreeIter iter;
  gboolean valid = FALSE;
  gint current_id = 0;

  valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
  while (valid) {
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, 0, &current_id, -1);
    if (current_id == player_id) {
      gtk_list_store_set(store, &iter, 7, FALSE, -1);
      break;
    }
    valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
  }
}

void
players_store_show_all(store)
  GtkListStore *store;
{
  GtkTreeIter iter;
  gboolean valid = FALSE;

  valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
  while (valid) {
    gtk_list_store_set(store, &iter, 7, TRUE, -1);
    valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
  }
}

void
players_store_print(store)
  GtkListStore *store;
{
  GtkTreeIter iter;
  gboolean valid = FALSE;
  gint id = 0, elo = 0, play_count = 0;
  gchar *rfid = NULL, *name = NULL, *gender = NULL, *image = NULL;
  gboolean visible = FALSE;

  valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
  g_print("players store\n=========\n%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
      "id", "rfid", "name", "gender", "elo", "image", "play_count", "visible");

  while (valid) {
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter,
        0, &id,
        1, &rfid,
        2, &name,
        3, &gender,
        4, &elo,
        5, &image,
        6, &play_count,
        7, &visible,
        -1);
    g_print("%d\t%s\t%s\t%s\t%d\t%s\t%d\t%s\n",
        id,
        rfid == NULL ? "NULL" : rfid,
        name == NULL ? "NULL" : name,
        gender == NULL ? "NULL" : gender,
        elo,
        image == NULL ? "NULL" : image,
        play_count,
        visible ? "TRUE" : "FALSE");
    if (rfid != NULL) g_free(rfid);
    if (name != NULL) g_free(name);
    if (gender != NULL) g_free(gender);
    if (image != NULL) g_free(image);
    valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter);
  }
}
