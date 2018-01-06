#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "store.h"

/* states */
enum {
  PONG_INIT,
  PONG_START,
  PONG_SELECT_PLAYER_1,
  PONG_CREATE_PLAYER_1,
  PONG_SELECT_PLAYER_2,
  PONG_CREATE_PLAYER_2,
  PONG_GAME_START
};

/* application */
typedef struct {
  /* state machine state */
  int state;

  /* game state */
  int player_1_id;
  const gchar *player_1_name;
  int player_1_score;
  int player_2_id;
  const gchar *player_2_name;
  int player_2_score;

  /* gtk pointers */
  GtkBuilder *builder;
  GtkLabel *main_label;
  GtkButton *start_game_button;
  GtkGrid *stats_grid;
  GtkGrid *game_setup_grid;
  GtkGrid *create_player_grid;
  GtkGrid *game_grid;
  GtkListStore *players_store;
  GtkTreeView *select_player_view;
  GtkEntry *create_player_name_entry;
  GtkComboBoxText *create_player_gender_entry;
  GtkLabel *game_player_1_name_label;
  GtkLabel *game_player_1_score_label;
  GtkLabel *game_player_2_name_label;
  GtkLabel *game_player_2_score_label;
} application;

static int
application_create_player(app, result_id, result_name)
  application *app;
  gint *result_id;
  const gchar **result_name;
{
  player *p = NULL;
  const gchar *name = NULL, *gender = NULL;
  int result = 0;

  if (gtk_entry_get_text_length(app->create_player_name_entry) == 0) {
    g_print("create player name entry is empty.\n");
    return -1;
  }
  name = gtk_entry_get_text(app->create_player_name_entry);

  gender = gtk_combo_box_text_get_active_text(app->create_player_gender_entry);
  if (gender == NULL) {
    g_print("create player gender entry is empty.\n");
    return -1;
  }

  p = (player *)malloc(sizeof(player));
  p->id = -1;
  p->rfid = NULL;
  p->name = strdup((char *) name);
  p->gender = strdup((char *) gender);
  p->elo = 1000;
  p->image = NULL;
  p->play_count = 0;

  result = players_store_insert(app->players_store, p);
  if (result != 0) {
    player_free(p);
    g_print("failed to insert new player\n");
    return 1;
  }

  *result_id = p->id;
  *result_name = g_strdup(name);
  player_free(p);
  return 0;
}

static void
application_update_scores(app)
  application *app;
{
  gchar *score = NULL;

  score = g_strdup_printf("%d", app->player_1_score);
  gtk_label_set_text(app->game_player_1_score_label, score);
  g_free(score);

  score = g_strdup_printf("%d", app->player_2_score);
  gtk_label_set_text(app->game_player_2_score_label, score);
  g_free(score);
}

static void
application_start_game(app)
  application *app;
{
  app->player_1_score = 0;
  app->player_2_score = 0;
  gtk_label_set_text(app->main_label, "GAME");
  gtk_label_set_text(app->game_player_1_name_label, app->player_1_name);
  gtk_label_set_text(app->game_player_2_name_label, app->player_2_name);
  application_update_scores(app);
  gtk_widget_show(GTK_WIDGET(app->game_grid));
}

static void
application_transition(app, new_state)
  application *app;
  int new_state;
{
  GtkTreeModel *filter_model = NULL;
  player *p = NULL;
  int ok = 0, result = 0;
  gint player_id = 0;
  const gchar *player_name = NULL;

  if (app->state == PONG_INIT) {
    if (new_state == PONG_START) {
      /* PONG_INIT -> PONG_START */
      database_init();
      filter_model = gtk_tree_model_filter_new(GTK_TREE_MODEL(app->players_store), NULL);
      gtk_tree_model_filter_set_visible_column(GTK_TREE_MODEL_FILTER(filter_model), 7);
      gtk_tree_view_set_model(app->select_player_view, filter_model);
      ok = 1;
    }
  }
  else if (app->state == PONG_START) {
    if (new_state == PONG_SELECT_PLAYER_1) {
      /* PONG_START -> PONG_SELECT_PLAYER_1 */
      players_store_show_all(app->players_store);
      gtk_label_set_text(app->main_label, "SELECT PLAYER 1");
      gtk_widget_hide(GTK_WIDGET(app->stats_grid));
      gtk_widget_hide(GTK_WIDGET(app->start_game_button));
      gtk_widget_show(GTK_WIDGET(app->game_setup_grid));
      ok = 1;
    }
  }
  else if (app->state == PONG_SELECT_PLAYER_1) {
    if (new_state == PONG_CREATE_PLAYER_1) {
      /* PONG_SELECT_PLAYER_1 -> PONG_CREATE_PLAYER_1 */
      gtk_label_set_text(app->main_label, "CREATE PLAYER 1");
      gtk_widget_hide(GTK_WIDGET(app->game_setup_grid));
      gtk_widget_show(GTK_WIDGET(app->create_player_grid));
      ok = 1;
    }
    else if (new_state == PONG_SELECT_PLAYER_2) {
      /* PONG_SELECT_PLAYER_1 -> PONG_SELECT_PLAYER_2 */
      players_store_hide_player_with_id(app->players_store, app->player_1_id);
      gtk_label_set_text(app->main_label, "SELECT PLAYER 2");
      ok = 1;
    }
    else if (new_state == PONG_START) {
      /* PONG_SELECT_PLAYER_1 -> PONG_START */
      gtk_label_set_text(app->main_label, "STATS");
      gtk_widget_hide(GTK_WIDGET(app->game_setup_grid));
      gtk_widget_show(GTK_WIDGET(app->stats_grid));
      gtk_widget_show(GTK_WIDGET(app->start_game_button));
      ok = 1;
    }
  }
  else if (app->state == PONG_CREATE_PLAYER_1) {
    if (new_state == PONG_SELECT_PLAYER_2) {
      /* PONG_CREATE_PLAYER_1 -> PONG_SELECT_PLAYER_2 */
      result = application_create_player(app, &player_id, &player_name);
      if (result != 0) {
        return;
      }
      app->player_1_id = player_id;
      app->player_1_name = player_name;

      players_store_hide_player_with_id(app->players_store, result);
      gtk_label_set_text(app->main_label, "SELECT PLAYER 2");
      gtk_widget_hide(GTK_WIDGET(app->create_player_grid));
      gtk_widget_show(GTK_WIDGET(app->game_setup_grid));
      ok = 1;
    }
    else if (new_state == PONG_SELECT_PLAYER_1) {
      /* PONG_CREATE_PLAYER_1 -> PONG_SELECT_PLAYER_1 */
      gtk_label_set_text(app->main_label, "SELECT PLAYER 1");
      gtk_widget_hide(GTK_WIDGET(app->create_player_grid));
      gtk_widget_show(GTK_WIDGET(app->game_setup_grid));
      ok = 1;
    }
  }
  else if (app->state == PONG_SELECT_PLAYER_2) {
    if (new_state == PONG_CREATE_PLAYER_2) {
      /* PONG_SELECT_PLAYER_2 -> PONG_CREATE_PLAYER_2 */
      gtk_label_set_text(app->main_label, "CREATE PLAYER 2");
      gtk_widget_hide(GTK_WIDGET(app->game_setup_grid));
      gtk_widget_show(GTK_WIDGET(app->create_player_grid));
      ok = 1;
    }
    else if (new_state == PONG_GAME_START) {
      /* PONG_SELECT_PLAYER_2 -> PONG_GAME_START */
      gtk_widget_hide(GTK_WIDGET(app->game_setup_grid));
      application_start_game(app);
      ok = 1;
    }
    else if (new_state == PONG_START) {
      /* PONG_SELECT_PLAYER_2 -> PONG_START */
      gtk_label_set_text(app->main_label, "STATS");
      gtk_widget_hide(GTK_WIDGET(app->game_setup_grid));
      gtk_widget_show(GTK_WIDGET(app->stats_grid));
      gtk_widget_show(GTK_WIDGET(app->start_game_button));
      ok = 1;
    }
  }
  else if (app->state == PONG_CREATE_PLAYER_2) {
    if (new_state == PONG_GAME_START) {
      /* PONG_CREATE_PLAYER_2 -> PONG_GAME_START */
      result = application_create_player(app, &player_id, &player_name);
      if (result != 0) {
        return;
      }
      app->player_2_id = player_id;
      app->player_2_name = player_name;

      gtk_widget_hide(GTK_WIDGET(app->create_player_grid));
      application_start_game(app);
      ok = 1;
    }
  }

  if (ok) {
    app->state = new_state;
  }
  else {
    g_print("invalid state transition, %d to %d\n", app->state, new_state);
  }
}

G_MODULE_EXPORT void
start_game_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application_transition((application *)data, PONG_SELECT_PLAYER_1);
}

G_MODULE_EXPORT void
select_player_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application *app = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  gint id = 0;
  const gchar *name = NULL;

  app = (application *)data;
  selection = gtk_tree_view_get_selection(app->select_player_view);
  if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gtk_tree_model_get(model, &iter, 0, &id, 2, &name, -1);
    if (app->state == PONG_SELECT_PLAYER_1) {
      app->player_1_id = id;
      app->player_1_name = name;
      application_transition(app, PONG_SELECT_PLAYER_2);
    }
    else if (app->state == PONG_SELECT_PLAYER_2) {
      app->player_2_id = id;
      app->player_2_name = name;
      application_transition(app, PONG_GAME_START);
    }
    gtk_tree_selection_unselect_all(selection);
  }
}

G_MODULE_EXPORT void
create_player_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application *app = (application *)data;
  if (app->state == PONG_SELECT_PLAYER_1) {
    application_transition((application *)data, PONG_CREATE_PLAYER_1);
  }
  else if (app->state == PONG_SELECT_PLAYER_2) {
    application_transition((application *)data, PONG_CREATE_PLAYER_2);
  }
}

G_MODULE_EXPORT void
create_player_save_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application *app = (application *)data;
  if (app->state == PONG_CREATE_PLAYER_1) {
    application_transition((application *)data, PONG_SELECT_PLAYER_2);
  }
  else if (app->state == PONG_CREATE_PLAYER_2) {
    application_transition((application *)data, PONG_GAME_START);
  }
}

G_MODULE_EXPORT void
cancel_player_create_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  application *app = (application *) data;
  if (app->state == PONG_CREATE_PLAYER_1) {
    application_transition(app, PONG_SELECT_PLAYER_1);
  }
  else if (app->state == PONG_CREATE_PLAYER_2) {
    application_transition(app, PONG_SELECT_PLAYER_2);
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
  app->player_1_id = 0;
  app->player_1_score = 0;
  app->player_2_id = 0;
  app->player_2_score = 0;

  app->builder = builder;
  app->main_label = GTK_LABEL(gtk_builder_get_object(builder, "main-label"));
  app->start_game_button = GTK_BUTTON(gtk_builder_get_object(builder, "start-game-button"));
  app->stats_grid = GTK_GRID(gtk_builder_get_object(builder, "stats-grid"));
  app->game_setup_grid = GTK_GRID(gtk_builder_get_object(builder, "game-setup-grid"));
  app->create_player_grid = GTK_GRID(gtk_builder_get_object(builder, "create-player-grid"));
  app->game_grid = GTK_GRID(gtk_builder_get_object(builder, "game-grid"));

  app->players_store = players_store();
  app->select_player_view = GTK_TREE_VIEW(gtk_builder_get_object(builder, "select-player-view"));
  app->create_player_name_entry = GTK_ENTRY(gtk_builder_get_object(builder, "create-player-name-entry"));
  app->create_player_gender_entry = GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "create-player-gender-entry"));
  app->game_player_1_name_label = GTK_LABEL(gtk_builder_get_object(builder, "game-player-1-name-label"));
  app->game_player_1_score_label = GTK_LABEL(gtk_builder_get_object(builder, "game-player-1-score-label"));
  app->game_player_2_name_label = GTK_LABEL(gtk_builder_get_object(builder, "game-player-2-name-label"));
  app->game_player_2_score_label = GTK_LABEL(gtk_builder_get_object(builder, "game-player-2-score-label"));

  gtk_builder_connect_signals(builder, app);

  application_transition(app, PONG_START);

  gtk_main();

  return 0;
}
