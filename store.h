#include <gtk/gtk.h>
#include "database.h"

GtkListStore *players_store();
int players_store_insert(GtkListStore *store, player *p);
void players_store_hide_player_with_id(GtkListStore *store, int player_id);
void players_store_show_all(GtkListStore *store);
void players_store_print(GtkListStore *store);
