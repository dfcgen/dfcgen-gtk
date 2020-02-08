/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     fileDlg.c
 * \brief    File menu dialogs.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "fileDlg.h"
#include "dialogSupport.h"
#include "projectFile.h"
#include "dfcProject.h"
#include "mainDlg.h"
#include "filterPrint.h"

#include <errno.h>


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/

static char* dfcPrjFileName = NULL;            /**< Current project filename */
static char* dfcExportFileName = NULL;             /**< Last export filename */


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
                g_free (buf);

                return;
            } /* if */
        } /* if */

        g_free (fname);
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
    gchar* str;

    GtkWidget* preview = gtk_label_new (NULL);
    GtkWidget* dialog = gtk_file_chooser_dialog_new (title, parent, action,
                                                     btn1, resp1, btn2, resp2,
                                                     NULL);

    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);

    filter = gtk_file_filter_new ();
    str = g_strdup_printf (_("Project files (*%s)"), PRJFILE_NAME_SUFFIX);
    gtk_file_filter_set_name (filter, str);
    g_free (str);
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
    if (dfcPrjFileName != NULL)
    {
        g_free (dfcPrjFileName);
        dfcPrjFileName = NULL;
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

    gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        dfcPrjLoad (fname, &err);

        if (err == NULL)
        {
            if (dfcPrjFileName != NULL)
            {
                g_free (dfcPrjFileName);
            } /* if */

            dfcPrjFileName = fname; /* store as current name (for next save) */
            mainDlgUpdateAll (fname);
        } /* if */
        else
        {
            dlgErrorFile (topWidget, _("Error loading project file '%s'."), fname, err),
            g_error_free (err);
            g_free (fname);
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
    if (dfcPrjFileName != NULL)                /* associated filename known? */
    {
        if (dfcPrjSave (dfcPrjFileName) == 0)                    /* success? */
        {
            return;
        } /* if */

        dlgErrorFile (gtk_widget_get_toplevel (srcWidget),
                      _("Error saving project file '%s'."), dfcPrjFileName, NULL);
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
    gchar* fname;

    GtkWidget *topWidget = gtk_widget_get_toplevel (srcWidget);
    GtkWidget* dialog = createFileDialog (_("Save project file"),
                                          GTK_WINDOW (topWidget),
                                          GTK_FILE_CHOOSER_ACTION_SAVE,
                                          GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                          GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT);
    if (dfcPrjFileName == NULL)
    {
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
                                           gettext ("untitled" PRJFILE_NAME_SUFFIX));
    } /* if */
    else
    {
        fname = g_filename_to_utf8 (dfcPrjFileName, -1, NULL, NULL, NULL);

        if (fname != NULL)
        {
            gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), fname);
            g_free (fname);
        } /* if */
    } /* else */

    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        /* Use fname directly because it is coded in filesysteme coding (must
         * not be UTF-8).
         */
        if (dfcPrjSave (fname) == 0)                              /* success? */
        {
            if (dfcPrjFileName != NULL)
            {
                g_free (dfcPrjFileName);
            } /* if */

            dfcPrjFileName = fname;        /* store as current name (for next save) */
        } /* if */
        else
        {
            dlgErrorFile (topWidget, _("Error saving project file '%s'."), fname, NULL);
            g_free (fname);
        } /* else */
    } /* if */

    gtk_widget_destroy (dialog);

} /* fileDlgSaveAsActivate() */




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
void fileDlgExportActivate (GtkWidget* srcWidget, gpointer user_data)
{
    gchar* fname;
    GtkWidget *widget;
    GtkFileFilter* filter;

    GtkWidget* dialog =
        gtk_file_chooser_dialog_new (_("Export coefficients"),
                                     GTK_WINDOW (gtk_widget_get_toplevel (srcWidget)),
                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

    gtk_file_chooser_set_local_only (GTK_FILE_CHOOSER (dialog), TRUE);

    widget = gtk_label_new (_("<b>Choose the file extension according to your preferred format:</b>\n\n"
                              "<tt>\t*.txt\t->\t</tt>plain text\n"
                              "<tt>\t*.c\t\t->\t</tt>\"C\" language\n"
                              "<tt>\t*.m\t\t->\t</tt>MATLAB script"));

    gtk_label_set_use_markup (GTK_LABEL (widget), TRUE);
    gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (dialog), widget);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("All files (*)"));
    gtk_file_filter_add_pattern (filter, "*");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("Plain (*.txt)"));
    gtk_file_filter_add_pattern (filter, "*.txt");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("MATLAB (*.m)"));
    gtk_file_filter_add_pattern (filter, "*.m");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

    filter = gtk_file_filter_new ();
    gtk_file_filter_set_name (filter, _("C (*.c)"));
    gtk_file_filter_add_pattern (filter, "*.c");
    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);


    if (dfcExportFileName == NULL)
    {
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), _("untitled.txt"));
    } /* if */
    else
    {
        fname = g_filename_to_utf8 (dfcExportFileName, -1, NULL, NULL, NULL);

        if (fname != NULL)
        {
            gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), fname);
            g_free (fname);
        } /* if */
    } /* if */

    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    gtk_file_chooser_set_show_hidden (GTK_FILE_CHOOSER (dialog), FALSE);

    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        fname = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        /* Use fname directly because it is coded in filesystem coding (must
         * not be UTF-8).
         */
        if (dfcPrjExport (fname) == 0)                           /* success? */
        {
            if (dfcExportFileName != NULL)
            {
                g_free (dfcExportFileName);
            } /* if */

            dfcExportFileName = fname;   /* store as current (for next save) */
        } /* if */
        else
        {
            dlgErrorFile (gtk_widget_get_toplevel (srcWidget),
                          _("Error exporting to file '%s'."),
                          fname, NULL);
            g_free (fname);
        } /* else */
    } /* if */

    gtk_widget_destroy (dialog);

} /* fileDlgExportActivate() */




/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
