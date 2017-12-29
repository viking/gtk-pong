#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "database.h"

G_MODULE_EXPORT void
start_game_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  GtkBuilder *builder = GTK_BUILDER(data);

  gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "start-game")));
  gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "stats-grid")));
  gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(builder, "game-grid")));
  gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "main-label")), "GAME");
}

G_MODULE_EXPORT void
cancel_game_button_clicked(widget, data)
  GtkWidget *widget;
  gpointer data;
{
  GtkBuilder *builder = GTK_BUILDER(data);

  gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "game-grid")));
  gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(builder, "start-game")));
  gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(builder, "stats-grid")));
  gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "main-label")), "STATS");
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
  gtk_builder_connect_signals(builder, builder);

  /* Connect signal handlers to the constructed widgets */
  window = gtk_builder_get_object(builder, "window");
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_main();

  return 0;
}
