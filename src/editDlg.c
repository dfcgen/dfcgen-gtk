/**
 * \file        editDlg.c
 * \brief       Edit dialog(s).
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 */


/* INCLUDE FILES **************************************************************/

#include "cfgSettings.h"
#include "dfcProject.h"
#include "responseWin.h"
#include "rootsPlot.h"
#include "dialogSupport.h"
#include "mainDlg.h"
#include "editDlg.h"

#include <gsl/gsl_const.h>



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define EDITSETDLG_COMBO_UNIT_FREQU    "comboUnitFrequ"
#define EDITSETDLG_COMBO_UNIT_TIME     "comboUnitTime"
#define EDITSETDLG_SPIN_PREC           "spinOutputPrec"
#define EDITINFODLG_ENTRY_AUTHOR       "entryAuthor"
#define EDITINFODLG_ENTRY_TITLE        "entryTitle"
#define EDITINFODLG_TEXTVIEW_DESC      "textviewDesc"



/* LOCAL VARIABLE DEFINITIONS *************************************************/

/** Frequency units
 */
static PLOT_UNIT unitF[] =
{
    {"Hz", 1.0},
    {"kHz", GSL_CONST_NUM_KILO},
    {"MHz", GSL_CONST_NUM_MEGA},
    {"GHz", GSL_CONST_NUM_GIGA}
};


/** Time units
 */
static PLOT_UNIT unitT[] =
{
    {"s", 1.0},
    {"ms", GSL_CONST_NUM_MILLI},
    {"µs", GSL_CONST_NUM_MICRO},
    {"ns", GSL_CONST_NUM_NANO},
    {"ps", GSL_CONST_NUM_PICO}
};




/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static char *dupInfoStr (const char* info);
static GtkWidget* createInfoDlg (GtkWidget *topWidget, const DFCPRJ_INFO *pInfo);
static GtkWidget* createSettingsDlg (GtkWidget *topWidget, const CFG_DESKTOP* pPrefs);
static int searchUnit (PLOT_UNIT units[], int size, const char* unitName);
static GtkResponseType editDlgSettingsAccept (GtkWidget* dialog);



/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Application settings dialog creation function.
 *
 *  \param topWidget    Parent widget.
 *  \param pPrefs       Pointer to desktop preferences.
 *
 *  \return Settings dialog widget pointer.
 ******************************************************************************/
static GtkWidget* createSettingsDlg (GtkWidget *topWidget, const CFG_DESKTOP* pPrefs)
{
    int i;
    GtkWidget *dialog, *widget, *label, *box, *table, *notebook;
    GtkAdjustment *spinAdjust;

    dialog = gtk_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (topWidget));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

    gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
    gtk_window_set_title (GTK_WINDOW (dialog), _(PACKAGE " Settings"));
    gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
    gtk_window_set_icon_name (GTK_WINDOW (dialog), GUI_ICON_IMAGE_PREFS);
    gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

    box = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    notebook = gtk_notebook_new ();
    gtk_box_pack_start (GTK_BOX (box), notebook, TRUE, TRUE, 0);

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add (GTK_CONTAINER (notebook), box);

    table = gtk_grid_new ();                /* gtk_table_new (3, 2, FALSE); */
    gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (table), 6);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);

    spinAdjust = gtk_adjustment_new (pPrefs->outprec, 1, DBL_DIG, 1, 1, 0);
    widget = gtk_spin_button_new (spinAdjust, 1, 0);
    gtk_grid_attach (GTK_GRID (table), widget, 1, 0, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("The number of digits following the decimal point of a floating point number"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    GLADE_HOOKUP_OBJECT (dialog, widget, EDITSETDLG_SPIN_PREC);

    label = gtk_label_new_with_mnemonic (_("Output _Precision"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    box = gtk_event_box_new ();
    gtk_grid_attach (GTK_GRID (table), box, 1, 1, 1, 1);
    gtk_widget_set_tooltip_text (box, _("Frequency unit to be used for in-/output"));

    widget = gtk_combo_box_text_new ();
    gtk_container_add (GTK_CONTAINER (box), widget);
    GLADE_HOOKUP_OBJECT (dialog, widget, EDITSETDLG_COMBO_UNIT_FREQU);

    for (i = 0; i < N_ELEMENTS (unitF); i++)
    {
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), unitF[i].name);
    } /* for */

    gtk_combo_box_set_active (GTK_COMBO_BOX (widget),
                              searchUnit (unitF, N_ELEMENTS (unitF),
                                          pPrefs->frequUnit.name));
    label = gtk_label_new_with_mnemonic (_("_Frequency Unit"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    box = gtk_event_box_new ();
    gtk_grid_attach (GTK_GRID (table), box, 1, 2, 1, 1);
    gtk_widget_set_tooltip_text (box, _("Time unit to be used for in-/output"));

    widget = gtk_combo_box_text_new ();
    gtk_container_add (GTK_CONTAINER (box), widget);
    GLADE_HOOKUP_OBJECT (dialog, widget, EDITSETDLG_COMBO_UNIT_TIME);

    for (i = 0; i < N_ELEMENTS (unitT); i++)
    {
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), unitT[i].name);
    } /* for */

    gtk_combo_box_set_active (GTK_COMBO_BOX (widget), 
                              searchUnit (unitT, N_ELEMENTS (unitT),
                                          pPrefs->timeUnit.name));

    label = gtk_label_new_with_mnemonic (_("_Time Unit"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    label = gtk_label_new_with_mnemonic (_("_In-/Output"));
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook),
                                gtk_notebook_get_nth_page (
                                    GTK_NOTEBOOK (notebook), 0), label);

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (notebook), box);

    label = gtk_label_new (_("RFU"));
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook),
                                gtk_notebook_get_nth_page (
                                    GTK_NOTEBOOK (notebook), 1), label);

    widget = createImageButton (GUI_BUTTON_LABEL_HELP, GUI_BUTTON_IMAGE_HELP);
    gtk_widget_show (widget);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget,
                                  GTK_RESPONSE_HELP);
#ifndef TODO
    gtk_widget_set_sensitive (widget, FALSE);
#endif
    gtk_widget_set_can_default (widget, TRUE);

    widget = createImageButton (GUI_BUTTON_LABEL_CANCEL, GUI_BUTTON_IMAGE_CANCEL);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget,
                                  GTK_RESPONSE_CANCEL);
    gtk_widget_set_can_default (widget, TRUE);

    widget = createImageButton (GUI_BUTTON_LABEL_OK, GUI_BUTTON_IMAGE_OK);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget,
                                  GTK_RESPONSE_OK);
    gtk_widget_set_can_default (widget, TRUE);

    gtk_widget_show_all (dialog);

    return dialog;
} /* createSettingsDlg() */


/* FUNCTION *******************************************************************/
/** Reads an integer value from key file associated with \a key under \a group.
 *  If the key is not found then target of \a pResult is unchanged.
 *
 *  \param units        Array of allowed units.
 *  \param size         Size of \p units array (in elements).
 *  \param unitName     The unit to search for in \p units.
 *
 *  \return             Index of unit in array \p units. If not found then it
 *                      returns 0.
 ******************************************************************************/
static int searchUnit (PLOT_UNIT units[], int size, const char* unitName)
{
    int i;

    for (i = 0; i < size; i++)
    {
        if (g_strcmp0 (unitName, units[i].name) == 0)              /* found? */
        {
            return i;
        } /* for */
    } /* if */

    return 0;
} /* searchUnit() */



/* FUNCTION *******************************************************************/
/** Reads and possibly accepts all values from a \e Settings dialog.
 *
 *  \param dialog       Dialog widget handle.
 *
 *  \return             GTK_RESPONSE_OK on success, else GTK_RESPONSE_REJECT.
 ******************************************************************************/
static GtkResponseType editDlgSettingsAccept (GtkWidget* dialog)
{
    CFG_DESKTOP settings;

    int idxTime = gtk_combo_box_get_active(
        GTK_COMBO_BOX (lookup_widget (dialog, EDITSETDLG_COMBO_UNIT_TIME)));
    int idxFrequ = gtk_combo_box_get_active(
        GTK_COMBO_BOX (lookup_widget (dialog, EDITSETDLG_COMBO_UNIT_FREQU)));

    if (dlgGetInt (dialog, EDITSETDLG_SPIN_PREC, 1, DBL_DIG, &settings.outprec))
    {
        settings.timeUnit = unitT[idxTime];
        settings.frequUnit = unitF[idxFrequ];

        cfgSetDesktopPrefs (&settings);               /* save new preferences */
        mainDlgRedrawAll ();            /* redraw all plots and coefficients */

        return GTK_RESPONSE_OK;
    } /* if */

    return GTK_RESPONSE_REJECT;
} /* editDlgSettingsAccept() */




/* FUNCTION *******************************************************************/
/** Project info dialog creation function.
 *
 *  \param topWidget    Parent widget.
 *  \param pInfo        Pointer to current project info (for preset of GtkEntry).
 *
 *  \return             Dialog widget pointer.
 ******************************************************************************/
static GtkWidget* createInfoDlg (GtkWidget *topWidget, const DFCPRJ_INFO *pInfo)
{
    GtkWidget *dialog, *widget, *entry, *label, *table;
    GdkPixbuf *pixbuf;

    dialog = gtk_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (topWidget));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

    gtk_window_set_title (GTK_WINDOW (dialog), _("Project Info"));
    gtk_window_set_type_hint (GTK_WINDOW (dialog), GDK_WINDOW_TYPE_HINT_DIALOG);

    pixbuf = createPixbufFromFile (PACKAGE_ICON);

    if (pixbuf != NULL)
    {
        gtk_window_set_icon (GTK_WINDOW (dialog), pixbuf);
        g_object_unref (pixbuf);
    } /* if */


    table = gtk_grid_new ();                /* gtk_table_new (3, 2, FALSE); */
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))),
                        table, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (table), 6);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);

    entry = gtk_entry_new ();                                       /* author */
    gtk_grid_attach (GTK_GRID (table), entry, 1, 0, 1, 1);
    gtk_widget_set_halign (entry, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand (entry, TRUE);
    gtk_widget_set_tooltip_text (entry, _("Author of filter/system"));
    gtk_entry_set_max_length (GTK_ENTRY (entry), 40);
    gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
    gtk_entry_set_text (GTK_ENTRY (entry),
                        (pInfo->author == NULL) ? g_get_user_name () : pInfo->author);
    gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
    gtk_widget_grab_focus (entry);
    GLADE_HOOKUP_OBJECT (dialog, entry, EDITINFODLG_ENTRY_AUTHOR);

    label = gtk_label_new_with_mnemonic (_("_Author"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

    entry = gtk_entry_new ();                                        /* title */
    gtk_grid_attach (GTK_GRID (table), entry, 1, 1, 1, 1);
    gtk_widget_set_halign (entry, GTK_ALIGN_FILL);
    gtk_widget_set_hexpand (entry, TRUE);
    gtk_entry_set_max_length (GTK_ENTRY (entry), 80);
    gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);

    if (pInfo->title != NULL)
    {
        gtk_entry_set_text (GTK_ENTRY (entry), pInfo->title);
    } /* if */

    gtk_widget_set_tooltip_text (entry, _("Project title"));
    GLADE_HOOKUP_OBJECT (dialog, entry, EDITINFODLG_ENTRY_TITLE);

    label = gtk_label_new_with_mnemonic (_("_Title"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);

    widget = gtk_scrolled_window_new (NULL, NULL);             /* description */
    gtk_grid_attach (GTK_GRID (table), widget, 1, 2, 1, 1);
    gtk_widget_set_hexpand (widget, TRUE);
    gtk_widget_set_halign (widget, GTK_ALIGN_FILL);
    gtk_widget_set_vexpand (widget, TRUE);
    gtk_widget_set_valign (widget, GTK_ALIGN_FILL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (widget), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (widget), GTK_SHADOW_IN);

    entry = gtk_text_view_new ();
    gtk_container_add (GTK_CONTAINER (widget), entry);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (entry), GTK_WRAP_WORD);
    gtk_widget_set_tooltip_text (entry, _("Project description"));

    if (pInfo->desc != NULL)
    {
        gtk_text_buffer_set_text (
            gtk_text_view_get_buffer (GTK_TEXT_VIEW (entry)), pInfo->desc, -1);
    } /* if */

    GLADE_HOOKUP_OBJECT (dialog, entry, EDITINFODLG_TEXTVIEW_DESC);

    label = gtk_label_new_with_mnemonic (_("_Description"));
    gtk_widget_set_halign (label, GTK_ALIGN_END);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
    gtk_grid_attach (GTK_GRID (table), label, 0, 2, 1, 1);

    widget = createImageButton (GUI_BUTTON_LABEL_HELP, GUI_BUTTON_IMAGE_HELP);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget, GTK_RESPONSE_HELP);
#ifndef TODO
    gtk_widget_set_sensitive (widget, FALSE);
#endif
    gtk_widget_set_can_default (widget, TRUE);

    widget = createImageButton (GUI_BUTTON_LABEL_CANCEL, GUI_BUTTON_IMAGE_CANCEL);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget, GTK_RESPONSE_CANCEL);
    gtk_widget_set_can_default (widget, TRUE);

    widget = createImageButton (GUI_BUTTON_LABEL_OK, GUI_BUTTON_IMAGE_OK);
    gtk_dialog_add_action_widget (GTK_DIALOG (dialog), widget, GTK_RESPONSE_OK);
    gtk_widget_set_can_default (widget, TRUE);

    gtk_widget_show_all (dialog);

    return dialog;
} /* createInfoDlg() */



/* FUNCTION *******************************************************************/
/** Duplicates a project info string with some postprocessing:
 *  -# removes leading whitespaces
 *  -# removes trailing whitespaces
 *  -# if length of string then is zero, it sets the pointer to NULL.
 *
 *  \param info         The original string as retrieved e.g. from a GtkEntry
 *                      widget.
 *
 *  \return             The newly allocated string or NULL (if it's length was
 *                      zero).
 ******************************************************************************/
static char *dupInfoStr (const char* info)
{
    char *newinfo = g_strdup (info); /* duplicate string (needed on GtkEntry) */

    g_strstrip (newinfo);          /* remove leading and trailing whitespaces */

    if (g_utf8_strlen (newinfo, -1) == 0)
    {
        g_free (newinfo);
        newinfo = NULL;
    } /* if */

    return newinfo;
} /* dupInfoStr() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Settings menuitem is selected
 *  from \e Edit menu.
 *
 *  \param widget       \e Edit \e Settings widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
void editDlgSettingsActivate (GtkWidget* widget, gpointer user_data)
{
    gint result;

    GtkWidget *dialog = createSettingsDlg (gtk_widget_get_toplevel (widget),
                                           cfgGetDesktopPrefs ());
    do
    {
        result = gtk_dialog_run (GTK_DIALOG (dialog));

        switch (result)
        {
            case GTK_RESPONSE_OK:
                result = editDlgSettingsAccept (dialog);
                break;

            case GTK_RESPONSE_HELP:
                result = GTK_RESPONSE_REJECT;
                break;

            default:
                break;
        } /* switch */
    }
    while (result == GTK_RESPONSE_REJECT);

    gtk_widget_destroy (dialog);

} /* editDlgSettingsActivate() */



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Info menuitem is selected
 *  from \e Edit menu.
 *
 *  \param widget       \e Edit \e Info widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
void editDlgInfoActivate (GtkWidget* widget, gpointer user_data)
{
    gint result;
    char *tmp;
    GtkTextBuffer *buffer;
    GtkTextIter start, stop;

    DFCPRJ_INFO info = {NULL, NULL, NULL};
    GtkWidget *dialog = createInfoDlg (gtk_widget_get_toplevel (widget),
                                       dfcPrjGetInfo ());
    do
    {
        result = gtk_dialog_run (GTK_DIALOG (dialog));

        switch (result)
        {
            case GTK_RESPONSE_OK:
                info.author = dupInfoStr (
                    gtk_entry_get_text (                        /* get author */
                        GTK_ENTRY (
                            lookup_widget (dialog, EDITINFODLG_ENTRY_AUTHOR))));

                info.title = dupInfoStr (
                    gtk_entry_get_text (                         /* get title */
                        GTK_ENTRY (
                            lookup_widget (dialog, EDITINFODLG_ENTRY_TITLE))));

                buffer = gtk_text_view_get_buffer (
                    GTK_TEXT_VIEW (
                        lookup_widget (dialog, EDITINFODLG_TEXTVIEW_DESC)));

                gtk_text_buffer_get_start_iter (buffer, &start);
                gtk_text_buffer_get_end_iter (buffer, &stop);
                tmp = gtk_text_buffer_get_text (buffer, &start, &stop, FALSE);

                info.desc = dupInfoStr (tmp);              /* set description */
                g_free (tmp);
                dfcPrjSetInfo (&info);                            /* save all */
                mainDlgUpdatePrjInfo ();
                break;

            case GTK_RESPONSE_HELP:
                result = GTK_RESPONSE_REJECT;
                break;

            default:
                break;
        } /* switch */
    }
    while (result == GTK_RESPONSE_REJECT);

    gtk_widget_destroy (dialog);

} /* editDlgInfoActivate() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
