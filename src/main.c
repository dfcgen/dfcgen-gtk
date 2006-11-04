/*
 * Initial main.c file generated by Glade. Edit as required.
 * Glade will not overwrite this file.
 */

#include "gui.h"             /* includes config.h (include before all others) */
#include "support.h"
#include "cfgSettings.h"
#include "dfcProject.h"
#include "mainDlg.h"

#include <gtk/gtk.h>

#if DEBUG
#define PIXMAP_DIR "../pixmaps"
#else
#define PIXMAP_DIR PACKAGE_PIXMAPS_DIR
#endif

int
main (int argc, char *argv[])
{
  GtkWidget *topWidget;

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_set_locale ();
  gtk_init (&argc, &argv);

  add_pixmap_directory (PIXMAP_DIR);

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  topWidget = mainDlgCreate ();
  cfgCacheSettings (topWidget);   /* get all settings from configuration file */
  gtk_widget_show (topWidget);
  gtk_main ();
  cfgFlushSettings ();

  return 0;
}