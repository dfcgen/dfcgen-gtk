/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Dialog helper functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/src/dialogSupport.c,v 1.1.1.1 2006-09-11 15:52:20 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dialogSupport.h"

#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>


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
/** Generic error message dialog.
 *
 *  \param topWidget    Toplevel widget.
 *  \param format       Printf like format string.
 *  \param ...          Arguments associated with format string.
 *
 ******************************************************************************/
void dlgError (GtkWidget* topWidget, char* format, ...)
{
    va_list args;
    gchar *msg;
    GtkWidget *dialog;

    va_start(args, format);
    msg = g_strdup_vprintf (format, args);

    dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (topWidget),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_CLOSE,
                                                 msg);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    va_end(args);
    FREE (msg);
} /* dlgError() */



/* FUNCTION *******************************************************************/
/** File error message dialog.
 *
 *  \param topWidget    Toplevel widget.
 *  \param format       Printf like format string.
 *  \param filename     Filename in filesystem coding (must not be UTF-8), which
 *                      is used as first argument into the format string (needs
 *                      %s format coding).
 *  \param err          GLib error pointer.
 *
 ******************************************************************************/
void dlgErrorFile (GtkWidget* topWidget, char* format, char *filename, GError *err)
{
    GtkWidget *dialog;

    char *utf8name = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

    if (utf8name == NULL)                 /* may be an UTF-8 conversion error */
    {
        utf8name = g_strdup (_("<i>Unknown</i>"));
    } /* if */


    dialog = gtk_message_dialog_new_with_markup (GTK_WINDOW (topWidget),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_CLOSE,
                                                 format, filename);
    if (err != NULL)
    {
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                  "%s", err->message);
    } /* if */

    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    FREE (utf8name);
} /* dlgErrorFile() */



/* FUNCTION *******************************************************************/
/** Fetches a double value from a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param lname        Name of associated label (as set by GLADE_HOOKUP_OBJECT).
 *  \param ename        Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value.
 *  \param vmax         Maximum allowed value.
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSe is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 *  \todo               Check for additional characters after end of converted number.
 ******************************************************************************/
BOOL dlgGetDouble (GtkWidget* topWidget, char *lname, char *ename,
                   double vmin, double vmax, double *pResult)
{
    const char *str;
    double result;

    GtkWidget *entry = lookup_widget (topWidget, ename);
    GtkWidget *label = lookup_widget (topWidget, lname);

    ASSERT (entry != NULL);
    ASSERT (label != NULL);

    str = gtk_entry_get_text (GTK_ENTRY (entry));
    errno = 0;
    result = strtod (str, NULL);

    if (errno == 0)
    {
        if ((result >= vmin) && (result <= vmax))
        {
            *pResult = result;
            return TRUE;
        } /* if */
    } /* if */


    dlgError (topWidget,
              _("Input \"<b>%s</b>\" is invalid for <b>%s</b> (min. %G, max. %G)."),
              str, gtk_label_get_label (GTK_LABEL (label)), vmin, vmax);

    return FALSE;
} /* dlgGetDouble() */



/* FUNCTION *******************************************************************/
/** Fetches an integer value from a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param lname        Name of associated label (as set by GLADE_HOOKUP_OBJECT).
 *  \param ename        Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value.
 *  \param vmax         Maximum allowed value.
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSe is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 ******************************************************************************/
BOOL dlgGetInt (GtkWidget* topWidget, char *lname, char *ename,
                int vmin, int vmax, int *pResult)
{
    const char *str;
    long int result;

    GtkWidget *entry = lookup_widget (topWidget, ename);
    GtkWidget *label = lookup_widget (topWidget, lname);

    ASSERT (entry != NULL);
    ASSERT (label != NULL);

    str = gtk_entry_get_text (GTK_ENTRY (entry));
    errno = 0;
    result = strtol (str, NULL, 10);

    if (errno == 0)
    {
        if ((result >= vmin) && (result <= vmax))
        {
            *pResult = result;
            return TRUE;
        } /* if */
    } /* if */


    dlgError (topWidget,
              _("Input \"<b>%s</b>\" is invalid for <b>%s</b> (min. %d, max. %d)."),
              str, gtk_label_get_label (GTK_LABEL (label)), vmin, vmax);

    return FALSE;
} /* dlgGetInt() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

