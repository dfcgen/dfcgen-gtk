/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Dialogs from the Help menu.
 *
 * \author   Copyright (C) 2006-2021 Ralf Hoppe
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "helpDlg.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** About dialog creation callback from menu.
 *
 *  \param menuitem     \e Help \e About menu item.
 *  \param user_data    User data as passed to function g_signal_connect (unused).
 *
 *  \return     Widget pointer.
 ******************************************************************************/
void helpDlgMenuActivate (GtkMenuItem* menuitem, gpointer user_data)
{
    static const gchar *authors[] = {PACKAGE_AUTHOR, NULL};

#ifdef TODO
    static const gchar *documenters[] = {PACKAGE_AUTHOR, NULL};
#endif

    GdkPixbuf* pixbuf = createPixbufFromFile (PACKAGE_ICON);
    gchar* version = g_strdup_printf (_("Version %s"), VERSION);
    GtkWidget* dialog = gtk_about_dialog_new ();
    gtk_window_set_transient_for (
        GTK_WINDOW (dialog),
        GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (menuitem))));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialog), version);
    g_free (version);

    gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialog), PACKAGE);
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialog), PACKAGE_COPYRIGHT);
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialog), PACKAGE_URL);
    gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (dialog), PACKAGE_URL);
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialog), authors);
#ifdef TODO
    gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (dialog), documenters);
#endif
    gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialog),
                                             _("translator-credits"));

    if (pixbuf != NULL)
    {
        gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (dialog), pixbuf);
        g_object_unref (pixbuf);
    } /* if */

    (void) gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

} /* helpDlgMenuActivate() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
