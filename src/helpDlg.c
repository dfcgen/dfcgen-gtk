/******************************************************************************/
/**
 * \file
 *           Dialogs from the Help menu.
 *
 * \author   Copyright (c) Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/src/helpDlg.c,v 1.1.1.1 2006-09-11 15:52:19 ralf Exp $
 *
 *
 * \see
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
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
    gtk_about_dialog_set_name (GTK_ABOUT_DIALOG (dialogAbout), PACKAGE);
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (dialogAbout), _("Copyright (c) 2006 " PACKAGE_AUTHOR));
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (dialogAbout), PACKAGE_WEBSITE);
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (dialogAbout), authors);
    gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (dialogAbout), documenters);
    gtk_about_dialog_set_translator_credits (GTK_ABOUT_DIALOG (dialogAbout), translators);
    gtk_widget_show(GTK_WIDGET(dialogAbout));   /* inserted call (glade bug?) */

    /* Store pointers to all widgets, for use by lookup_widget(). */
    GLADE_HOOKUP_OBJECT_NO_REF (dialogAbout, dialogAbout, "dialogAbout");

} /* helpDlgMenuActivate() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
