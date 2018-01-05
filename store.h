#include <gtk/gtk.h>
#include "database.h"

GtkListStore *players_store();
int players_store_insert(GtkListStore *store, player *p);
