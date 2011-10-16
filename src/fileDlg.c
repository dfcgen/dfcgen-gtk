/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           File menu dialogs.
 *
 * \author   Copyright (C) 2006, 2011 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Id$
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "fileDlg.h"
#include "dialogSupport.h"
#include "projectFile.h"
#include "dfcProject.h"
#include "mainDlg.h"
#if GTK_CHECK_VERSION(2, 10, 0)           /* print support requires GTK 2.10 */
#include "filterPrint.h"
#endif
#include <errno.h>


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/

static char* filename = NULL;                   /**< Current project filename */


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/


static void previewUpdate (GtkFileChooser *chooser, gpointer labelWidget);
static GtkWidget* createFileDialog (const gchar *title, GtkWindow *parent,
                                    GtkFileChooserAction action,
                                    const gchar *btn1, GtkResponseType resp1,
                                    const gchar *btn2, GtkResponseType resp2);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Callback function from GtkFileChooser dialog on event \e update-preview.
 *
 * \param chooser       GtkFileChooser widget.
 * \param labelWidget   Preview label widget.
 *
 ******************************************************************************/
static void previewUpdate (GtkFileChooser *chooser, gpointer labelWidget)
{
    GError *err;
    gchar* buf;
    char *author, *title;
    DFCPRJ_INFO info;

    char* fname = gtk_file_chooser_get_preview_filename (chooser);

    if (fname != NULL)
    {
        if (g_str_has_suffix (fname, PRJFILE_NAME_SUFFIX))
        {
            err = NULL;
            prjFileScan (fname, &info, &err);

            if (err == NULL)
            {
                title = info.title;
                author = info.author;

                if (author == NULL)
                {
                    author = _("<i>Unknown</i>");
                } /* if */

                if (title == NULL)
                {
                    title = _("<i>Unknown</i>");
                } /* if */

                buf = g_strdup_printf (_("<b>Title</b>: %s\n<b>Author</b>: %s"),
                                       title, author);
                gtk_file_chooser_set_preview_widget_active(chooser, TRUE);
                gtk_label_set_markup (GTK_LABEL (labelWidget), buf);

                prjFileFree (&info);                    /* free project info */
                FREE (buf);

                return;
            } /* if */
        } /* if */

        FREE (fname);
    } /* if */


    gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
} /* previewUpdate() */


/* FUNCTION *******************************************************************/
/** Creates the file dialog.
 *
 * \param title         Title of the dialog, or NULL.
 * \param parent        Transient parent of the dialog, or NULL.
 * \param action        Open or save mode for the dialog.
 * \param btn1          Stock ID or text to go in the first button.
 * \param resp1         Response ID to \a btn1 press.
 * \param btn2          Stock ID or text to go in the first button.
 * \param resp2         Response ID to \a btn2 press.
 *
 *  \return             A new \e GtkFileChooserDialog widget.
 ******************************************************************************/
static GtkWidget* createFileDialog (const gchar *title, GtkWindow *parent,
                                    GtkFileChooserAction action,
                                    const gchar *btn1, GtkResponseType resp1,
                                    const gchar *btn2, GtkResponseType resp2)
{
    GtkFileFilter* filter;

    GtkWidget* preview = gtk_label_new (NULL);
    GtkWidget* dialog = gtk_file_chooser_dialog_new (title, parent, action,
                                                     btn1, resp1, btn2, resp2,
                                                     NULL);

    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, gettext ("Project files (*" PRJFILE_NAME_SUFFIX ")"));
    gtk_file_filter_add_pattern (filter, "*" PRJFILE_NAME_SUFFIX);
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files (*)"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    gtk_label_set_use_markup (GTK_LABEL (preview), TRUE);
    gtk_label_set_angle (GTK_LABEL (preview), 90);
    gtk_file_chooser_set_use_preview_label (GTK_FILE_CHOOSER (dialog), FALSE);
    gtk_file_chooser_set_preview_widget (GTK_FILE_CHOOSER (dialog), preview);
    g_signal_connect ((gpointer) dialog, "update-preview",
                      G_CALLBACK (previewUpdate), preview);

    return dialog;
} /* createFileDialog() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e New menuitem is selected
 *  from \e File menu.
 *
 *  \param menuitem     The menu item object which received the signal (New).
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
void fileDlgNewActivate (GtkMenuItem* menuitem, gpointer user_data)
{
    if (filename != NULL)
    {
        FREE (filename);
        filename = NULL;
    } /* if */

    dfcPrjFree (NULL);
    mainDlgUpdateAll (NULL);
} /* fileDlgNewActivate() */



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Open menuitem is selected
 *  from \e File menu.
 *
 *  \param srcWidget    \e File \e Open widget (GtkMenuItem on event \e activate
 *                      or GtkToolButton on event \e clicked), which causes this
 *                      call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
void fileDlgOpenActivate (GtkWidget* srcWidget, gpointer user_data)
{
    GError *err = NULL;
    GtkWidget *topWidget = gtk_widget_get_toplevel (srcWidget);
    GtkWidget* dialog = createFileDialog (_("Load project file"),
                                          GTK_WINDOW (topWidget),
                                          GTK_FILE_CHOOSER_ACTION_OPEN,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        dfcPrjLoad (fname, &err);

        if (err == NULL)
        {
            if (filename != NULL)
            {
                FREE (filename);
            } /* if */

            filename = fname;        /* store as current name (for next save) */
            mainDlgUpdateAll (fname);
        } /* if */
        else
        {
            dlgErrorFile (topWidget, _("Error loading project file '%s'."), fname, err),
            g_error_free (err);
            FREE (fname);
        } /* if */
    } /* if */

    gtk_widget_destroy (dialog);

} /* fileDlgOpenActivate() */


/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Save menuitem is selected
 *  from \e File menu.
 *
 *  \param srcWidget    \e File \e Save widget (GtkMenuItem on event \e activate
 *                      or GtkToolButton on event \e clicked), which causes this
 *                      call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
void fileDlgSaveActivate (GtkWidget* srcWidget, gpointer user_data)
{
    if (filename != NULL)                       /* associated filename known? */
    {
        if (dfcPrjSave (filename) == 0)                           /* success? */
        {
            return;
        } /* if */

        dlgErrorFile (gtk_widget_get_toplevel (srcWidget),
                      _("Error saving project file '%s'."), filename, NULL);
    } /* if */


    fileDlgSaveAsActivate (srcWidget, user_data);

} /* fileDlgSaveActivate() */



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Save \e As menuitem is
 *  selected from \e File menu.
 *
 *  \param srcWidget    \e File \e Save \e As widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
void fileDlgSaveAsActivate (GtkWidget* srcWidget, gpointer user_data)
{
    gchar* fname = NULL;
    GtkWidget *topWidget = gtk_widget_get_toplevel (srcWidget);
    GtkWidget* dialog = createFileDialog (_("Save project file"),
                                          GTK_WINDOW (topWidget),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT);
    if (filename != NULL)
    {
        fname = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

        if (fname != NULL)
        {
            gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), fname);
            FREE (fname);
        } /* if */
        else
        {
            gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), _("Untitled"));
        } /* else */
    } /* if */

    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        /* Use fname directly because it is coded in filesysteme coding (must
         * not be UTF-8).
         */
        if (dfcPrjSave (fname) == 0)                              /* success? */
        {
            if (filename != NULL)
            {
                FREE (filename);
            } /* if */

            filename = fname;        /* store as current name (for next save) */
        } /* if */
        else
        {
            dlgErrorFile (topWidget, _("Error saving project file '%s'."), fname, NULL);
            FREE (fname);
        } /* else */
    } /* if */

    gtk_widget_destroy (dialog);

} /* fileDlgSaveAsActivate() */



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Print menuitem is selected
 *  from \e File menu.
 *
 *  \param[in] srcWidget \e File \e Save \e As widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param[in] ref      User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
void fileDlgPrintActivate (GtkWidget* srcWidget, gpointer ref)
{

#if GTK_CHECK_VERSION(2, 10, 0)           /* print support requires GTK 2.10 */
    static GtkPrintSettings *settings = NULL;


    GError *error;
    GtkPrintOperationResult result;

    GtkWidget *widget, *topWidget = gtk_widget_get_toplevel (srcWidget);
    GtkPrintOperation* print = gtk_print_operation_new ();

    gtk_print_operation_set_print_settings (print, settings);
    gtk_print_operation_set_default_page_setup (print, NULL);
  
    g_signal_connect (print, "begin-print", G_CALLBACK (filterPrintCoeffsInit), NULL);
    g_signal_connect (print, "draw-page", G_CALLBACK (filterPrintCoeffsDo), NULL);
 
    result = gtk_print_operation_run (print,
                                      GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                      NULL, NULL);
    switch (result)
    {
        case GTK_PRINT_OPERATION_RESULT_ERROR:
            gtk_print_operation_get_error (print, &error);
            widget = gtk_message_dialog_new (GTK_WINDOW (topWidget),
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_MESSAGE_ERROR,
                                             GTK_BUTTONS_CLOSE,
                                             _("Error printing: %s"),
                                             error->message);
            g_signal_connect (widget, "response", 
                              G_CALLBACK (gtk_widget_destroy), NULL);
            gtk_widget_show (widget);
            g_error_free (error);
            break; /* GTK_PRINT_OPERATION_RESULT_ERROR */


        case GTK_PRINT_OPERATION_RESULT_APPLY:   /* store the print settings */
            if (settings != NULL)
            {
                g_object_unref (settings);
            } /* if */

            settings = g_object_ref (gtk_print_operation_get_print_settings (print));
            break;

        default: /* FIXME: GTK_PRINT_OPERATION_RESULT_CANCEL */
            break;
    } /* switch */

    g_object_unref(print);
#endif

} /* fileDlgPrintActivate() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
