#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "store.h"

/* states */
enum {
  PONG_INIT,
  PONG_START,
  PONG_SETUP_1,
  PONG_CREATE_PLAYER_1,
  PONG_CREATE_PLAYER_1_SAVE,
  PONG_SETUP_2,
  PONG_CREATE_PLAYER_2,
  PONG_CREATE_PLAYER_2_SAVE,
  PONG_PLAY
};

/* application */
typedef struct {
  int state;
  GtkBuilder *builder;
  GtkLabel *main_label;
  GtkGrid *stats_grid;
  GtkGrid *game_setup_grid;
  GtkGrid *create_player_grid;
  GtkListStore *player_store;
  GtkLabel *select_player_label;
  GtkTreeView *select_player_view;
  GtkEntry *create_player_name_entry;
  GtkComboBoxText *create_player_gender_entry;
} application;

void
application_transition(app, new_state)
  application *app;
  int new_state;
{
  player *p = NULL;
  const gchar *name = NULL, *gender = NULL;
  int ok = 0, result = 0;

  if (app->state == PONG_INIT) {
    if (new_state == PONG_START) {
      /* PONG_INIT -> PONG_START */
      gtk_tree_view_set_model(app->select_player_view, GTK_TREE_MODEL(app->player_store));
      ok = 1;
    }
  }
  else if (app->state == PONG_START) {
    if (new_state == PONG_SETUP_1) {
      /* PONG_START -> PONG_SETUP_1 */
      gtk_label_set_text(app->main_label, "GAME SETUP");
      gtk_label_set_text(app->select_player_label, "Select Player 1");
      gtk_widget_hide(GTK_WIDGET(app->stats_grid));
      gtk_widget_show(GTK_WIDGET(app->game_setup_grid));
      ok = 1;
    }
  }
  else if (app->state == PONG_SETUP_1) {
    if (new_state == PONG_CREATE_PLAYER_1) {
      /* PONG_SETUP_1 -> PONG_CREATE_PLAYER_1 */
      gtk_label_set_text(app->main_label, "CREATE PLAYER 1");
      gtk_widget_hide(GTK_WIDGET(app->game_setup_grid));
      gtk_widget_show(GTK_WIDGET(app->create_player_grid));
      ok = 1;
    }
    else if (new_state == PONG_START) {
      /* PONG_SETUP_1 -> PONG_START */
      gtk_label_set_text(app->main_label, "STATS");
      gtk_widget_hide(GTK_WIDGET(app->game_setup_grid));
      gtk_widget_show(GTK_WIDGET(app->stats_grid));
      ok = 1;
    }
  } else if (app->state == PONG_CREATE_PLAYER_1) {
    if (new_state == PONG_CREATE_PLAYER_1_SAVE) {
      /* PONG_CREATE_PLAYER_1 -> PONG_CREATE_PLAYER_1_SAVE */
      if (gtk_entry_get_text_length(app->create_player_name_entry) == 0) {
        g_print("create player name entry is empty.\n");
        return;
      }
      name = gtk_entry_get_text(app->create_player_name_entry);

      gender = gtk_combo_box_text_get_active_text(app->create_player_gender_entry);
      if (gender == NULL) {
        g_print("create player gender entry is empty.\n");
        return;
      }

      p = (player *)malloc(sizeof(player));
      p->id = -1;
      p->rfid = NULL;
      p->name = strdup((char *) name);
      p->gender = strdup((char *) gender);
      p->elo = 1000;
      p->image = NULL;
      p->play_count = 0;

      result = players_store_insert(app->player_store, p);
      player_free(p);
      if (result != 0) {
        g_print("failed to insert new player\n");
        return;
      } else {
        new_state = PONG_SETUP_1;
      }
    }

    if (new_state == PONG_SETUP_1) {
      /* PONG_CREATE_PLAYER_1 -> PONG_SETUP_1 */
      gtk_label_set_text(app->main_label, "GAME SETUP");
      gtk_widget_hide(GTK_WIDGET(app->create_player_grid));
      gtk_widget_show(GTK_WIDGET(app->game_setup_grid));
      ok = 1;
    }
  }

  if (ok) {
    app->state = new_state;
  } else {
    g_print("invalid state transition, %d to %d\n", app->state, new_state);
  }
}

G_MODULE_EXPORT void
start_game_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application_transition((application *)data, PONG_SETUP_1);
}

G_MODULE_EXPORT void
create_player_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application_transition((application *)data, PONG_CREATE_PLAYER_1);
}

G_MODULE_EXPORT void
create_player_save_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application_transition((application *)data, PONG_CREATE_PLAYER_1_SAVE);
}

G_MODULE_EXPORT void
cancel_player_create_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application *app = (application *) data;
  if (app->state == PONG_CREATE_PLAYER_1) {
    application_transition(app, PONG_SETUP_1);
  } else if (app->state == PONG_CREATE_PLAYER_2) {
    application_transition(app, PONG_SETUP_2);
  }
}

G_MODULE_EXPORT void
cancel_game_setup_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application_transition((application *)data, PONG_START);
}

G_MODULE_EXPORT void
cancel_game_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  /*
  GtkBuilder *builder = GTK_BUILDER(data);
  cancel_game(builder);
  */
}

int
main(argc, argv)
  int argc;
  char *argv[];
{
  GdkScreen *screen = NULL;
  GtkCssProvider *css_provider = NULL;
  GtkBuilder *builder = NULL;
  GObject *window = NULL;
  GError *err = NULL;
  application *app = NULL;

  gtk_init(&argc, &argv);

  /* Setup CSS */
  screen = gdk_screen_get_default();
  css_provider = gtk_css_provider_new();
  if (!gtk_css_provider_load_from_path(css_provider, "style.css", &err)) {
    g_error("invalid style: %s\n", err->message);
    return 1;
  }
  gtk_style_context_add_provider_for_screen(screen,
      GTK_STYLE_PROVIDER(css_provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  /* Construct a GtkBuilder instance and load the UI description */
  builder = gtk_builder_new();
  if (!gtk_builder_add_from_file(builder, "builder.ui", &err)) {
    g_error("invalid builder UI: %s\n", err->message);
    return 1;
  }

  /* Setup application struct */
  app = (application *)malloc(sizeof(application));
  app->state = PONG_INIT;
  app->builder = builder;
  app->main_label = GTK_LABEL(gtk_builder_get_object(builder, "main-label"));
  app->stats_grid = GTK_GRID(gtk_builder_get_object(builder, "stats-grid"));
  app->game_setup_grid = GTK_GRID(gtk_builder_get_object(builder, "game-setup-grid"));
  app->create_player_grid = GTK_GRID(gtk_builder_get_object(builder, "create-player-grid"));
  app->player_store = players_store();
  app->select_player_label = GTK_LABEL(gtk_builder_get_object(builder, "select-player-label"));
  app->select_player_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "select-player-view"));
  app->create_player_name_entry = GTK_ENTRY(gtk_builder_get_object(builder, "create-player-name-entry"));
  app->create_player_gender_entry = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "create-player-gender-entry"));
  gtk_builder_connect_signals(builder, app);

  application_transition(app, PONG_START);

  gtk_main();

  return 0;
}
