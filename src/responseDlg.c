/**
 * \file        responseDlg.c
 * \brief       Response settings/properties dialog.
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <dfcgen@rho62.de>
 */


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dialogSupport.h"
#include "responseDlg.h"
#include "responseWin.h"

#include <errno.h>
#include <string.h> /* memcpy() */


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define RESPONSE_DLG_WIDTH_CHARS        12         /**< Width of entry fields */
#define RESPONSE_DLG_SPIN_MAX           16384 /**< Maximum number of samples */

#define RESPONSE_DLG_ENTRY_STARTX       "entryStartX" /**< Widget name x-start */
#define RESPONSE_DLG_ENTRY_STOPX        "entryStopX"  /**< Widget name x-stop */
#define RESPONSE_DLG_CHKBTN_LOGX        "checkLogX" /**< Widget name of check button for logarithmic scaling */
#define RESPONSE_DLG_CHKBTN_GRIDX       "checkGridX" /**< Widget name of check button for grid */
#define RESPONSE_DLG_SPIN_SAMPLES       "spinSamples" /**< Widget name samples */
#define RESPONSE_DLG_ENTRY_STARTY       "entryStartY" /**< Widget name y-start */
#define RESPONSE_DLG_ENTRY_STOPY        "entryStopY"  /**< Widget name y-stop */
#define RESPONSE_DLG_CHKBTN_LOGY        "checkLogY" /**< Widget name of check button for logarithmic scaling */
#define RESPONSE_DLG_CHKBTN_GRIDY       "checkGridY" /**< Widget name of check button for grid */
#define RESPONSE_DLG_CHKBTN_AUTOSCALE   "checkAutoscale" /**< Widget name of check button for autoscaling */
#define RESPONSE_DLG_COMBO_GRAPH        "comboGraphStyle"
#define RESPONSE_DLG_COMBO_COLOR        "comboColorStyle"
#define RESPONSE_DLG_COLOR_SELECT       "colorSelect"


/* LOCAL VARIABLE DEFINITIONS *************************************************/

static GdkRGBA responseDlgColorVals[PLOT_COLOR_SIZE]; /**< Colors (modified in dialog) */
static PLOT_COLOR responseDlgColorItem; /**< Curently selected color item (labels, graph, etc.) */


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void responseDlgSetEntry (GtkWidget* topWidget, const char *name,
                                 const PLOT_UNIT *pUnit, double value);
static void createLogGridButton (GtkWidget *topWidget, GtkGrid *table,
                                 char *logBtnName, char *gridBtnName,
                                 PLOT_AXIS *pAxis);
static void autoScalingChanged (GtkCheckButton* button, gpointer user_data);
static void applyStyleColor (GtkComboBox *combobox, GtkColorChooser *colorsel);
static void colorItemChanged (GtkComboBox *combobox, gpointer user_data);
static void colorChooserApply (GtkColorChooser *colorsel, gpointer user_data);
static int responseDlgGetAxis (GtkWidget* topWidget, const char *widgets[],
                                double vmin, double vmax, PLOT_AXIS *pAxis);



/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Sets a double value into a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param name         Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param pUnit        Pointer to unit descriptor (may be NULL).
 *  \param value        The double value to set.
 *
 ******************************************************************************/
static void responseDlgSetEntry (GtkWidget* topWidget, const char *name,
                                 const PLOT_UNIT *pUnit, double value)
{
    double multiplier = 1.0;

    if (pUnit != NULL)
    {
        multiplier = pUnit->multiplier;
    } /* if */

    dlgSetDouble (topWidget, name, multiplier, value);
} /* responseDlgSetEntry() */


/* FUNCTION *******************************************************************/
/** Fetches axis data from response dialog.
 *
 *  \param topWidget    Toplevel widget.
 *  \param widgets      Array of size 4 with widget names associated with:
 *                      - index 0: \e Start value entry field.
 *                      - index 1: \e Stop value entry field.
 *                      - index 2: \e Logarithmic check button.
 *                      - index 3: \e Grid check button.
 *  \param vmin         Minimum allowed value (after applying \p pAxis->pUnit->multiplier).
 *  \param vmax         Maximum allowed value (after applying \p pAxis->pUnit->multiplier).
 *  \param pAxis        Pointer to associated, updated axis data (overwritten
 *                      on error).
 *
 *  \return             0 on success, else an error code from errno.h or
 *                      gsl_error.h.
 ******************************************************************************/
static int responseDlgGetAxis (GtkWidget* topWidget, const char *widgets[],
                                double vmin, double vmax, PLOT_AXIS *pAxis)
{
    double multiplier = 1.0;
    int err = 0;

    pAxis->flags &= ~(PLOT_AXIS_FLAG_LOG | PLOT_AXIS_FLAG_GRID);

    if (gtk_toggle_button_get_active (
            GTK_TOGGLE_BUTTON (lookup_widget (topWidget, widgets[2]))))
    {
        pAxis->flags |= PLOT_AXIS_FLAG_LOG;

        if (vmin < PLOT_TOLERANCE)
        {
            vmin = PLOT_TOLERANCE;
        } /* if */
    } /* if */

    if (gtk_toggle_button_get_active (
            GTK_TOGGLE_BUTTON (lookup_widget (topWidget, widgets[3]))))
    {
        pAxis->flags |= PLOT_AXIS_FLAG_GRID;
    } /* if */


    if (!(pAxis->flags & PLOT_AXIS_FLAG_AUTO))
    {
        if (pAxis->pUnit != NULL)
        {
            multiplier = pAxis->pUnit->multiplier;
        } /* if */

        if (dlgGetDouble (topWidget, widgets[0], vmin, vmax, multiplier,
                          &pAxis->start) &&
            dlgGetDouble (topWidget, widgets[1], vmin, vmax, multiplier,
                          &pAxis->stop))
        {
            err = cairoPlotChkRange (pAxis);

            if (err != 0)                                  /* range not okay? */
            {
                dlgError (topWidget,
                          _("Range (start, stop) for axis '%s' is invalid."),
                          pAxis->name);
            } /* if */
        } /* if */
        else
        {
            err = ERANGE;
        } /* if */
    } /* if */

    return err;
} /* responseDlgGetAxis() */



/* FUNCTION *******************************************************************/
/** Creates the dialog check buttons for logarithmic scaling and grid on/off.
 *
 *  \param topWidget    Top level widget.
 *  \param table        Table, where the buttons are located.
 *  \param logBtnName   Name of button for logarithmic scaling (used in macro
 *                      GLADE_HOOKUP_OBJECT).
 *  \param gridBtnName  Name of button for grid on/off (used in macro
 *                      GLADE_HOOKUP_OBJECT).
 *  \param pAxis        Pointer to current plot configuration of axis.
 *
 ******************************************************************************/
static void createLogGridButton (GtkWidget *topWidget, GtkGrid *table,
                                 char *logBtnName, char *gridBtnName,
                                 PLOT_AXIS *pAxis)
{
    GtkWidget *checkbutton = gtk_check_button_new_with_mnemonic (_("Logarithmic"));

    gtk_grid_attach (table, checkbutton, 1, 2, 2, 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton),
                                  pAxis->flags & PLOT_AXIS_FLAG_LOG);
    GLADE_HOOKUP_OBJECT (topWidget, checkbutton, logBtnName);

    checkbutton = gtk_check_button_new_with_mnemonic (_("Show Grid"));
    gtk_grid_attach (table, checkbutton, 1, 3, 2, 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton),
                                  pAxis->flags & PLOT_AXIS_FLAG_GRID);
    GLADE_HOOKUP_OBJECT (topWidget, checkbutton, gridBtnName);
} /* createLogGridButton() */


/* FUNCTION *******************************************************************/
/** This function is called if the \e Autoscaling check button changes its
 *  state.
 *
 *  \param button       Autoscaling check button which changes the state.
 *  \param user_data    Unused.
 *
 ******************************************************************************/
static void autoScalingChanged (GtkCheckButton* button, gpointer user_data)
{
    gboolean sensitive = !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

    gtk_widget_set_sensitive (lookup_widget (GTK_WIDGET (button),
                                             RESPONSE_DLG_ENTRY_STARTY),
                              sensitive);
    gtk_widget_set_sensitive (lookup_widget (GTK_WIDGET (button),
                                             RESPONSE_DLG_ENTRY_STOPY),
                              sensitive);
} /* autoScalingChanged() */


/* FUNCTION *******************************************************************/
/** Appply currently selected color.
 *
 *  \param combobox     Combobox widget which identifies the item.
 *  \param colorsel     \e GtkColorChooser widget.
 *
 ******************************************************************************/
static void applyStyleColor (GtkComboBox *combobox, GtkColorChooser *colorsel)
{
    int idx = gtk_combo_box_get_active (combobox);

    if (idx < 0)
    {
        idx = 0;                        /* set any, in case nothing selected */
    } /* if */

    gtk_color_chooser_get_rgba (
        colorsel, &responseDlgColorVals[responseDlgColorItem]);
    responseDlgColorItem = idx;           /* remind last selected color item */
    gtk_color_chooser_set_rgba (colorsel, &responseDlgColorVals[idx]);

} /* applyStyleColor() */


/* FUNCTION *******************************************************************/
/** This function is called if the color item \e GtkComboBox selection changes.
 *
 *  \param combobox     Combobox widget which determines the color selection.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
static void colorItemChanged (GtkComboBox *combobox, gpointer user_data)
{
    applyStyleColor (combobox, GTK_COLOR_CHOOSER (
        lookup_widget (GTK_WIDGET (combobox), RESPONSE_DLG_COLOR_SELECT)));

} /* colorItemChanged() */


/* FUNCTION *******************************************************************/
/** This function is called if the selected color in \e GtkColorChooser has been applied.
 *
 *  \param colorsel     \e GtkColorChooser widget.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
static void colorChooserApply (GtkColorChooser *colorsel, gpointer user_data)
{
    applyStyleColor (GTK_COMBO_BOX (
                         lookup_widget (GTK_WIDGET (colorsel), RESPONSE_DLG_COMBO_COLOR)),
                     colorsel);
} /* colorChooserApply() */


/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Creates the properties dialog for a response plot.
 *
 *  \attention          If anyone changes the enums PLOT_STYLE or PLOT_COLOR
 *                      from cairoPlot.h then the calls into
 *                      gtk_combo_box_text_append_text() must be adopted.
 *
 *  \param topWindow    Parent window.
 *  \param pDiag        Pointer to current plot configuration (for preset).
 *
 *  \return             Dialog widget.
 ******************************************************************************/
GtkWidget* responseDlgCreate (GtkWindow *topWindow, PLOT_DIAG *pDiag)
{
    GtkWidget *widget, *label, *box, *frame, *table, *colorSel;
    GtkAdjustment *spinAdjust;
    char *axisName;

    GtkWidget *responseDlg = gtk_dialog_new ();
    gtk_window_set_transient_for (GTK_WINDOW (responseDlg), topWindow);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (responseDlg), TRUE);

    /* Some generic and global layout settings first
     */
    gtk_window_set_title (GTK_WINDOW (responseDlg), _("Response Settings"));
    gtk_window_set_resizable (GTK_WINDOW (responseDlg), FALSE);
    gtk_window_set_icon_name (GTK_WINDOW (responseDlg), GUI_ICON_IMAGE_PREFS);
    gtk_window_set_type_hint (GTK_WINDOW (responseDlg), GDK_WINDOW_TYPE_HINT_DIALOG);

    box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6); /* x- and y-side of dialog */
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (responseDlg))),
                        box, FALSE, FALSE, 6);

    /* x-axis
     */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

    /* x-axis values
     */
    axisName = g_strdup_printf ("<b>%s '%s'</b>", _("Axis"),
                                gettext (pDiag->x.name));
    label = gtk_label_new (axisName);
    g_free (axisName);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    table = gtk_grid_new ();                /* gtk_table_new (5, 3, FALSE); */
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 6);
    gtk_widget_set_margin_start (table, GUI_INDENT_CHILD_PIXEL);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);

    label = gtk_label_new (_("Start"));                            /* x-start */
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);

    widget = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), widget, 1, 0, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("Start of x-axis interval"));
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (widget), RESPONSE_DLG_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (responseDlg, widget, RESPONSE_DLG_ENTRY_STARTX);
    responseDlgSetEntry (responseDlg, RESPONSE_DLG_ENTRY_STARTX,
                         pDiag->x.pUnit, pDiag->x.start);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    gtk_widget_grab_focus (widget);

    label = gtk_label_new (_("Stop"));                              /* x-stop */
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);

    widget = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), widget, 1, 1, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("End of x-axis interval"));
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (widget), RESPONSE_DLG_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (responseDlg, widget, RESPONSE_DLG_ENTRY_STOPX);
    responseDlgSetEntry (responseDlg, RESPONSE_DLG_ENTRY_STOPX,
                         pDiag->x.pUnit, pDiag->x.stop);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    label = gtk_label_new (_("Samples"));                          /* samples */
    gtk_grid_attach (GTK_GRID (table), label, 0, 4, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);

    spinAdjust = gtk_adjustment_new (0, 0, RESPONSE_DLG_SPIN_MAX, 1, 10, 0);
    widget = gtk_spin_button_new (spinAdjust, 1, 0);
    gtk_grid_attach (GTK_GRID (table), widget, 1, 4, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("The number of samples to be used (0 = all)"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE);
    GLADE_HOOKUP_OBJECT (responseDlg, widget, RESPONSE_DLG_SPIN_SAMPLES);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    if (pDiag->initFunc == NULL)
    {
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), pDiag->num);
    } /* if */
    else
    {
        gtk_widget_set_sensitive (widget, FALSE);
    } /* else */

    label = gtk_label_new (NULL);                             /* x-start unit */
    gtk_grid_attach (GTK_GRID (table), label, 2, 0, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_START);

    widget = gtk_label_new (NULL);                             /* x-stop unit */
    gtk_grid_attach (GTK_GRID (table), widget, 2, 1, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_START);

    if (pDiag->x.pUnit != NULL)
    {
        gtk_label_set_text (GTK_LABEL (label), pDiag->x.pUnit->name);
        gtk_label_set_text (GTK_LABEL (widget), pDiag->x.pUnit->name);
    } /* if */
    else
    {
        gtk_widget_hide (label);
        gtk_widget_hide (widget);
    } /* else */

    createLogGridButton (responseDlg, GTK_GRID (table),
                         RESPONSE_DLG_CHKBTN_LOGX, RESPONSE_DLG_CHKBTN_GRIDX,
                         &pDiag->x);

    /* y-axis
     */
    frame = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

    /* y-axis values
     */
    axisName = g_strdup_printf ("<b>%s '%s'</b>", _("Axis"),
                                gettext (pDiag->y.name));
    label = gtk_label_new (axisName);
    g_free (axisName);
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    table = gtk_grid_new ();                /* gtk_table_new (5, 3, FALSE); */
    gtk_container_add (GTK_CONTAINER (frame), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 6);
    gtk_widget_set_margin_start (table, GUI_INDENT_CHILD_PIXEL);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);

    label = gtk_label_new (_("Start"));                            /* y-start */
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);

    widget = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), widget, 1, 0, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("Start of y-axis interval"));
    gtk_widget_set_sensitive (widget, FALSE);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (widget), RESPONSE_DLG_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (responseDlg, widget, RESPONSE_DLG_ENTRY_STARTY);
    responseDlgSetEntry (responseDlg, RESPONSE_DLG_ENTRY_STARTY,
                         pDiag->y.pUnit, pDiag->y.start);
    gtk_widget_set_sensitive (widget, !(pDiag->y.flags & PLOT_AXIS_FLAG_AUTO));
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    label = gtk_label_new (_("Stop"));                              /* y-stop */
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);

    widget = gtk_entry_new ();
    gtk_grid_attach (GTK_GRID (table), widget, 1, 1, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("End of y-axis interval"));
    gtk_widget_set_sensitive (widget, FALSE);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (widget), RESPONSE_DLG_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (responseDlg, widget, RESPONSE_DLG_ENTRY_STOPY);
    responseDlgSetEntry (responseDlg, RESPONSE_DLG_ENTRY_STOPY,
                         pDiag->y.pUnit, pDiag->y.stop);
    gtk_widget_set_sensitive (widget, !(pDiag->y.flags & PLOT_AXIS_FLAG_AUTO));
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    label = gtk_label_new (NULL);                             /* y-start unit */
    gtk_grid_attach (GTK_GRID (table), label, 2, 0, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_START);

    widget = gtk_label_new (NULL);                             /* y-stop unit */
    gtk_grid_attach (GTK_GRID (table), widget, 2, 1, 1, 1);
    gtk_widget_set_halign (widget, GTK_ALIGN_START);

    if (pDiag->y.pUnit != NULL)
    {
        gtk_label_set_text (GTK_LABEL (label), pDiag->y.pUnit->name);
        gtk_label_set_text (GTK_LABEL (widget), pDiag->y.pUnit->name);
    } /* if */
    else
    {
        gtk_widget_hide (label);
        gtk_widget_hide (widget);
    } /* else */


    createLogGridButton (responseDlg, GTK_GRID (table),
                         RESPONSE_DLG_CHKBTN_LOGY, RESPONSE_DLG_CHKBTN_GRIDY,
                         &pDiag->y);

    widget = gtk_check_button_new_with_mnemonic (_("Autoscaling"));
    gtk_widget_set_tooltip_text (widget, _("Autoscaling of y-axis with respect"
                                           " to minimum and maximum values in interval"));
    gtk_grid_attach (GTK_GRID (table), widget, 1, 4, 2, 1);
    gtk_container_set_border_width (GTK_CONTAINER (widget), 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget),
                                  pDiag->y.flags & PLOT_AXIS_FLAG_AUTO);
    GLADE_HOOKUP_OBJECT (responseDlg, widget, RESPONSE_DLG_CHKBTN_AUTOSCALE);
    g_signal_connect (widget, "toggled", G_CALLBACK (autoScalingChanged), NULL);


    /* Style settings
     */
    widget = gtk_expander_new (NULL);                        /* style xpander */
    gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (responseDlg))),
                        widget, FALSE, FALSE, 6);
    gtk_widget_set_margin_start (widget, GUI_INDENT_CHILD_PIXEL);
    gtk_expander_set_spacing (GTK_EXPANDER (widget), 6);

    label = gtk_label_new (_("<b>Style</b>"));
    gtk_expander_set_label_widget (GTK_EXPANDER (widget), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 24);
    gtk_container_add (GTK_CONTAINER (widget), box);

    table = gtk_grid_new ();                /* gtk_table_new (2, 3, FALSE); */
    gtk_box_pack_start (GTK_BOX (box), table, FALSE, FALSE, 6);
    gtk_grid_set_row_spacing (GTK_GRID (table), 6);
    gtk_grid_set_column_spacing (GTK_GRID (table), 6);

    label = gtk_label_new (_("Graph"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);

    label = gtk_label_new (_("Color"));
    gtk_grid_attach (GTK_GRID (table), label, 0, 1, 1, 1);
    gtk_widget_set_halign (label, GTK_ALIGN_END);

    responseDlgColorItem = 0;                         /* color item selection */
    memcpy (responseDlgColorVals, pDiag->colors, sizeof (responseDlgColorVals));

    colorSel = gtk_color_chooser_widget_new ();
    gtk_box_pack_start (GTK_BOX (box), colorSel, FALSE, FALSE, 0);
    gtk_color_chooser_set_use_alpha (GTK_COLOR_CHOOSER (colorSel), FALSE);
    gtk_color_chooser_set_rgba (GTK_COLOR_CHOOSER (colorSel), responseDlgColorVals);
    GLADE_HOOKUP_OBJECT (responseDlg, colorSel, RESPONSE_DLG_COLOR_SELECT);
    g_signal_connect (colorSel, "color-activated", G_CALLBACK (colorChooserApply), NULL);

    widget = gtk_event_box_new ();
    gtk_grid_attach (GTK_GRID (table), widget, 1, 0, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("Style of graph"));

    box = gtk_combo_box_text_new ();
    gtk_container_add (GTK_CONTAINER (widget), box);
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Line (continuous)")); /* PLOT_STYLE_LINE_ONLY */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Circle (discrete)")); /* PLOT_STYLE_CIRCLE_ONLY */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Samples (discrete)")); /* PLOT_STYLE_CIRCLE_SAMPLE */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Cross (discrete)")); /* PLOT_STYLE_CROSS_ONLY */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Box (discrete)")); /* PLOT_STYLE_BOX_ONLY */
    GLADE_HOOKUP_OBJECT (responseDlg, box, RESPONSE_DLG_COMBO_GRAPH);
    gtk_combo_box_set_active (GTK_COMBO_BOX (box), pDiag->style);

    widget = gtk_event_box_new ();
    gtk_grid_attach (GTK_GRID (table), widget, 1, 1, 1, 1);
    gtk_widget_set_tooltip_text (widget, _("Choose the color item to be changed, then modify the color"));

    box = gtk_combo_box_text_new ();
    gtk_container_add (GTK_CONTAINER (widget), box);
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Labels")); /* PLOT_COLOR_LABELS */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Grid")); /* PLOT_COLOR_GRID */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Graph")); /* PLOT_COLOR_GRAPH */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Box")); /* PLOT_COLOR_BOX */
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (box), _("Units")); /* PLOT_COLOR_AXIS_NAME */

    GLADE_HOOKUP_OBJECT (responseDlg, box, RESPONSE_DLG_COMBO_COLOR);
    g_signal_connect (box, "changed", G_CALLBACK (colorItemChanged), NULL);

    gtk_combo_box_set_active (GTK_COMBO_BOX (box), 0);


    /* Action area (buttons)
     */
    widget = createImageButton (GUI_BUTTON_LABEL_HELP, GUI_BUTTON_IMAGE_HELP);
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), widget, GTK_RESPONSE_HELP);
#ifndef TODO
    gtk_widget_set_sensitive (widget, FALSE);
#endif

    widget = createImageButton (GUI_BUTTON_LABEL_CANCEL, GUI_BUTTON_IMAGE_CANCEL);
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), widget, GTK_RESPONSE_CANCEL);
    gtk_widget_set_can_default (widget, TRUE);

    widget = createImageButton (GUI_BUTTON_LABEL_APPLY, GUI_BUTTON_IMAGE_APPLY);
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), widget, GTK_RESPONSE_APPLY);
    gtk_widget_set_can_default (widget, TRUE);

    widget = createImageButton (GUI_BUTTON_LABEL_OK, GUI_BUTTON_IMAGE_OK);
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), widget, GTK_RESPONSE_OK);
    gtk_widget_set_can_default (widget, TRUE);

    gtk_widget_grab_default (widget);
    gtk_widget_show_all (responseDlg);

    return responseDlg;
} /* responseDlgCreate() */



/* FUNCTION *******************************************************************/
/** Sets a double value into a GtkEntry dialog widget.
 *
 *  \param dialog       Dialog (top-level) widget.
 *  \param pDiag        Pointer to plot settings to be updated.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
int responseDlgApply (GtkWidget *dialog, PLOT_DIAG *pDiag)
{
    static const char *xWidgets[] =
    {
        RESPONSE_DLG_ENTRY_STARTX,
        RESPONSE_DLG_ENTRY_STOPX,
        RESPONSE_DLG_CHKBTN_LOGX,
        RESPONSE_DLG_CHKBTN_GRIDX
    };

    static const char *yWidgets[] =
    {
        RESPONSE_DLG_ENTRY_STARTY,
        RESPONSE_DLG_ENTRY_STOPY,
        RESPONSE_DLG_CHKBTN_LOGY,
        RESPONSE_DLG_CHKBTN_GRIDY
    };


    PLOT_DIAG tmpDiag = *pDiag;
    int err = responseDlgGetAxis (dialog, xWidgets, 0,
                                  PLOT_AXIS_MAX, &tmpDiag.x);
    if (err == 0)
    {
        tmpDiag.y.flags &= ~PLOT_AXIS_FLAG_AUTO;

        if (gtk_toggle_button_get_active (
                GTK_TOGGLE_BUTTON (
                    lookup_widget (dialog, RESPONSE_DLG_CHKBTN_AUTOSCALE))))
        {
            tmpDiag.y.flags |= PLOT_AXIS_FLAG_AUTO;
        } /* if */

        err = responseDlgGetAxis (dialog, yWidgets, PLOT_AXIS_MIN,
                                  PLOT_AXIS_MAX, &tmpDiag.y);
        if (err == 0)
        {
            if (dlgGetInt (dialog, RESPONSE_DLG_SPIN_SAMPLES, 0,
                           RESPONSE_DLG_SPIN_MAX, &tmpDiag.num))
            {
                colorItemChanged (                 /* save last changed color */
                    GTK_COMBO_BOX (
                        lookup_widget (dialog, RESPONSE_DLG_COMBO_COLOR)), NULL);

                tmpDiag.style = gtk_combo_box_get_active(
                    GTK_COMBO_BOX (lookup_widget (dialog, RESPONSE_DLG_COMBO_GRAPH)));

                *pDiag = tmpDiag;                        /* set return values */
                memcpy (pDiag->colors, responseDlgColorVals, sizeof (responseDlgColorVals));
            } /* if */
            else
            {
                err = ERANGE;
            } /* else */
        } /* if */
    } /* if */

    return err;
} /* responseDlgApply() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
