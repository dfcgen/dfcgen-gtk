/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Dialogs from the Help menu.
 *
 * \author   Copyright (C) 2006, 2011, 2012 Ralf Hoppe
 * \version  $Id$
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
    GtkWidget *dialogAbout;

    const gchar *authors[] =
    {
        PACKAGE_AUTHOR,
        NULL
    };

    const gchar *documenters[] =
    {
        PACKAGE_AUTHOR,
        NULL
    };

    /* TRANSLATORS: Replace this string with your names, one name per line. */
    gchar *translators = PACKAGE_AUTHOR;

    dialogAbout = gtk_about_dialog_new ();
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dialogAbout), TRUE);
    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG (dialogAbout), VERSION);
    gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (dialogAbout), PACKAGE);
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialogAbout), _("Copyright (C) 2006, 2011, 2012 " PACKAGE_AUTHOR));
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialogAbout), PACKAGE_URL);
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialogAbout), authors);
    gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (dialogAbout), documenters);
    gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialogAbout), translators);
    gtk_widget_show(GTK_WIDGET(dialogAbout));

    /* Store pointers to all widgets, for use by lookup_widget(). */
    GLADE_HOOKUP_OBJECT_NO_REF (dialogAbout, dialogAbout, "dialogAbout");

} /* helpDlgMenuActivate() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
