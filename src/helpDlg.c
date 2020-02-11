/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Dialogs from the Help menu.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe
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
    GtkWidget* dialogAbout = gtk_about_dialog_new ();

    gtk_window_set_destroy_with_parent (GTK_WINDOW (dialogAbout), TRUE);
    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialogAbout), VERSION);
    gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialogAbout), PACKAGE);
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialogAbout), PACKAGE_COPYRIGHT);
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialogAbout), PACKAGE_URL);
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialogAbout), authors);
#ifdef TODO
    gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (dialogAbout), documenters);
#endif
    gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialogAbout),
                                             _("translator-credits"));

    if (pixbuf != NULL)
    {
        gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG (dialogAbout), pixbuf);
        g_object_unref (pixbuf);
    } /* if */

    (void) gtk_dialog_run (GTK_DIALOG (dialogAbout));
    gtk_widget_destroy (dialogAbout);

} /* helpDlgMenuActivate() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
