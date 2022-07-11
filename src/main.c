/**
 * \file        main.c
 * \brief       DFCGen main file.
 * \note        Initial main.c file generated by Glade.
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 */

#include "gui.h"             /* includes config.h (include before all others) */
#include "cfgSettings.h"
#include "dfcProject.h"
#include "mainDlg.h"

#include <gtk/gtk.h>


int main (int argc, char *argv[])
{
  GtkWidget *topWidget;

#ifdef ENABLE_NLS
  gchar* localedir = getPackageDirectory (DIR_ID_LOCALE);

#ifdef G_OS_WIN32
  /* bindtextdomain() is not UTF-8 aware
   */
  gchar* tmp = g_win32_locale_filename_from_utf8 (localedir);

  g_free (localedir);
  localedir = tmp;
#endif /* G_OS_WIN32 */

  bindtextdomain (GETTEXT_PACKAGE, localedir);
  g_free (localedir);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gtk_init (&argc, &argv);

  /*
   * The following code was added by Glade to create one of each component
   * (except popup menus), just so that you see something after building
   * the project. Delete any components that you don't want shown initially.
   */
  topWidget = mainDlgCreate ();
  cfgCacheSettings (topWidget);   /* get all settings from configuration file */
  gtk_widget_show (topWidget);
  designDlgUpdate (topWidget);           /* adopt layout to restored settings */
  gtk_main ();
  cfgFlushSettings ();

  return 0;
}
