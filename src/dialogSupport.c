/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     dialogSupport.c
 * \brief    Dialog helper functions.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dialogSupport.h"
#include "cfgSettings.h"

#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <glib.h>


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void dlgEntryNumericError (GtkWidget* entry, double vmin, double vmax);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Error dialog for GTkEntry dialog elements.
 *
 *  \param entry        GtkEntry widget which has generated the error.
 *  \param vmin         Minimum allowed value (after applying \p multiplier).
 *  \param vmax         Maximum allowed value (after applying \p multiplier).
 *
 ******************************************************************************/
static void dlgEntryNumericError (GtkWidget* entry, double vmin, double vmax)
{
    GList* list;

    GtkWidget* topWidget = gtk_widget_get_toplevel (entry);

    gtk_widget_grab_focus (entry);

    /* Get list of mnemonic labels for GtkEntry
     */
    list = gtk_widget_list_mnemonic_labels (entry);
    ASSERT (g_list_first (list)->data != NULL);     /* required by GUI design */

    dlgError (topWidget,
              _("Input \"<b>%s</b>\" is invalid for <b>%s</b> (min. %G, max. %G)."),
              gtk_entry_get_text (GTK_ENTRY (entry)),
              gtk_label_get_label (GTK_LABEL (g_list_first (list)->data)),
              vmin, vmax);
    g_list_free(list);
} /* dlgEntryNumericError() */



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
                                                 NULL);
    gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), msg);
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    va_end(args);
    g_free (msg);
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

    g_free (utf8name);
} /* dlgErrorFile() */



/* FUNCTION *******************************************************************/
/** Fetches a double value from a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param ename        Name of text entry widget (as set with GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value (after applying \p multiplier).
 *  \param vmax         Maximum allowed value (after applying \p multiplier).
 *  \param multiplier   Multiplier for scaling the result (wrt. units).
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSE is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 ******************************************************************************/
BOOL dlgGetDouble (GtkWidget* topWidget, const char *ename,
                   double vmin, double vmax, double multiplier, double *pResult)
{
    const char *str;
    char *tail;
    double result;

    GtkWidget *entry = lookup_widget (topWidget, ename);
    ASSERT (entry != NULL);

    str = gtk_entry_get_text (GTK_ENTRY (entry));
    errno = 0;
    result = g_strtod (str, &tail);

    if ((errno == 0) && (tail != str) && (*tail == '\0'))
    {
        result *= multiplier;

        if ((result >= vmin) && (result <= vmax))
        {
            *pResult = result;
            return TRUE;
        } /* if */
    } /* if */


    dlgEntryNumericError (entry, vmin / multiplier, vmax / multiplier);

    return FALSE;
} /* dlgGetDouble() */



/* FUNCTION *******************************************************************/
/** Fetches an integer value from a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param ename        Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value.
 *  \param vmax         Maximum allowed value.
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSe is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 ******************************************************************************/
BOOL dlgGetInt (GtkWidget* topWidget, const char *ename,
                int vmin, int vmax, int *pResult)
{
    const char *str;
    char *tail;
    long int result;

    GtkWidget *entry = lookup_widget (topWidget, ename);
    ASSERT (entry != NULL);

    str = gtk_entry_get_text (GTK_ENTRY (entry));
    errno = 0;
    result = strtol (str, &tail, 10);

    if ((errno == 0) && (tail != str) && (*tail == '\0'))
    {
        if ((result >= vmin) && (result <= vmax))
        {
            *pResult = result;
            return TRUE;
        } /* if */
    } /* if */


    dlgEntryNumericError (entry, vmin, vmax);

    return FALSE;
} /* dlgGetInt() */



/* FUNCTION *******************************************************************/
/** Sets a double value into a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param name         Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param multiplier   Multiplier for scaling the result (wrt. units).
 *  \param value        The double value to set.
 *
 ******************************************************************************/
void dlgSetDouble (GtkWidget* topWidget, const char *name,
                   double multiplier, double value)
{
    char text[128];

    g_snprintf (text, sizeof(text), "%.*G", cfgGetDesktopPrefs()->outprec,
                value / multiplier);
    gtk_entry_set_text (GTK_ENTRY (lookup_widget (topWidget, name)), text);
} /* dlgSetDouble() */



/* FUNCTION *******************************************************************/
/** A little popup dialog, where the user shall enter a double value.
 *
 *  \param title        Title of window.
 *  \param label        Label to put before the GtkEntry text field.
 *  \param comment      An introduction displayed at top of the dialog (may be
 *                      NULL, then nothing is displayed at top position).
 *  \param pResult      Pointer to result buffer, which must be initialized with
 *                      a default value.
 *
 *  \return             TRUE on success, FALSE if the user has canceled.
 ******************************************************************************/
BOOL dlgPopupDouble (char *title, char *label, char *comment, double *pResult)
{
    BOOL ret;
    char text[128];
    GtkWidget *widget, *entry, *vbox, *hbox;

    GtkWidget *dialog = gtk_dialog_new ();

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

    vbox = GTK_DIALOG (dialog)->vbox;

    if (comment != NULL)
    {
        widget = gtk_label_new (comment);
        gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE);
        gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
        gtk_misc_set_padding (GTK_MISC (widget), 12, 0);
        gtk_box_pack_start (GTK_BOX (vbox), widget, FALSE, FALSE, 12);
    } /* if */

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 6);

    widget = gtk_label_new_with_mnemonic (label);
    gtk_box_pack_start (GTK_BOX (hbox), widget, FALSE, FALSE, 0);
    gtk_misc_set_padding (GTK_MISC (widget), 12, 0);

    entry = gtk_entry_new ();
    gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, TRUE, 0);
    gtk_entry_set_width_chars (GTK_ENTRY (entry), DBL_DIG);
    gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
    g_snprintf (text, sizeof(text), "%.*G", cfgGetDesktopPrefs()->outprec, *pResult);
    gtk_entry_set_text (GTK_ENTRY (entry), text);
    GLADE_HOOKUP_OBJECT (dialog, entry, "entry");
    gtk_label_set_mnemonic_widget (GTK_LABEL (widget), entry);

    vbox = GTK_DIALOG (dialog)->action_area;
    gtk_button_box_set_layout (GTK_BUTTON_BOX (vbox), GTK_BUTTONBOX_END);

    widget = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget, GTK_RESPONSE_CANCEL);
    GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);

    widget = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget, GTK_RESPONSE_OK);
    GTK_WIDGET_SET_FLAGS (widget, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (widget);

    gtk_widget_show_all (dialog);

    do
    {
        if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
        {
            GLADE_HOOKUP_OBJECT_NO_REF (dialog, NULL, "entry");
            gtk_widget_destroy (dialog);
            return FALSE;
        } /* if */

        ret = dlgGetDouble (dialog, "entry", -DBL_MAX, DBL_MAX, 1.0, pResult);

    } while (!ret);


    GLADE_HOOKUP_OBJECT_NO_REF (dialog, NULL, "entry");
    gtk_widget_destroy (dialog);

    return ret;
} /* dlgPopupDouble() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

