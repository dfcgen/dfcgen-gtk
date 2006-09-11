/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Response settings/properties dialog.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/src/responseDlg.c,v 1.1.1.1 2006-09-11 15:52:18 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define RESPONSE_WIN_SPIN_MAX   16384 /**< Maximum number of points (spin button) */



/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void responseWinSpinChanged (GtkEditable* editable, gpointer user_data);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Callback function for \e editing_done event (spin button).
 *  destroyed.
 *
 *  \param editable     Spin button widget which has sent the event.
 *  \param user_data    Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect().
 *
 ******************************************************************************/
static void responseWinSpinChanged (GtkEditable* editable, gpointer user_data)
{
#if 0
    long int value;
    const char *string;

    RESPONSE_WIN* pResponse = user_data;

    ASSERT(pResponse != NULL);

    string = gtk_entry_get_text (GTK_ENTRY(editable));
    errno = 0;
    value = strtol (string, NULL, 10);

    if ((value >= 0) && (value <= RESPONSE_WIN_SPIN_MAX) && (errno == 0))
    {
        if (responseWinInvalidateIfExist(pResponse))
        {
            pResponse->diag.num = value;
        } /* if */
    } /* if */

#endif
} /* responseWinSpinChanged() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Creates the properties dialog for a response window.
 *
 *  \return             Dialog widget.
 *  \todo               Use gtk_window_resize () if expander is closed
 ******************************************************************************/
GtkWidget* responseDlgCreate (void)
{
    GtkWidget *responseDlg;
    GtkWidget *dialog_vbox1;
    GtkWidget *hbox16;
    GtkWidget *frame2;
    GtkWidget *alignment14;
    GtkWidget *table5;
    GtkWidget *label;
    GtkWidget *entryStartX;
    GtkObject *spinSamples_adj;
    GtkWidget *spinSamples;
    GtkWidget *entryStopX;
    GtkWidget *labelUnitStartX;
    GtkWidget *labelUnitStopX;
    GtkWidget *checkbutton3;
    GtkWidget *checkbutton2;
    GtkWidget *label27;
    GtkWidget *frame3;
    GtkWidget *alignment15;
    GtkWidget *table7;
    GtkWidget *entryStopY;
    GtkWidget *entryStartY;
    GtkWidget *labelUnitStartY;
    GtkWidget *labelUnitStopY;
    GtkWidget *checkbutton9;
    GtkWidget *checkbutton10;
    GtkWidget *checkbutton8;
    GtkWidget *label28;
    GtkWidget *expander1;
    GtkWidget *vbox13;
    GtkWidget *table8;
    GtkWidget *combobox2;
    GtkWidget *combobox3;
    GtkWidget *colorselection1;
    GtkWidget *label64;
    GtkWidget *dialog_action_area1;
    GtkWidget *helpbutton1;
    GtkWidget *cancelbutton1;
    GtkWidget *button1;
    GtkWidget *okbutton1;
    GtkTooltips *tooltips;

    tooltips = gtk_tooltips_new ();

    responseDlg = gtk_dialog_new ();
    gtk_window_set_title (GTK_WINDOW (responseDlg), _("Response Settings"));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (responseDlg), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (responseDlg), "gtk-preferences");
    gtk_window_set_type_hint (GTK_WINDOW (responseDlg), GDK_WINDOW_TYPE_HINT_DIALOG);

    dialog_vbox1 = GTK_DIALOG (responseDlg)->vbox;

    hbox16 = gtk_hbox_new (FALSE, 24);
    gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox16, FALSE, FALSE, 0);

    frame2 = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (hbox16), frame2, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame2), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frame2), GTK_SHADOW_NONE);

    alignment14 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frame2), alignment14);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment14), 0, 0, 12, 0);

    table5 = gtk_table_new (5, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (alignment14), table5);
    gtk_table_set_row_spacings (GTK_TABLE (table5), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table5), 12);

    label = gtk_label_new (_("Start"));
    gtk_table_attach (GTK_TABLE (table5), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    label = gtk_label_new (_("Stop"));
    gtk_table_attach (GTK_TABLE (table5), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    label = gtk_label_new (_("Samples"));
    gtk_table_attach (GTK_TABLE (table5), label, 0, 1, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);

    entryStartX = gtk_entry_new ();
    gtk_table_attach (GTK_TABLE (table5), entryStartX, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (entryStartX), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (entryStartX), 10);

    spinSamples_adj = gtk_adjustment_new (0, 0, 32768, 1, 10, 10);
    spinSamples = gtk_spin_button_new (GTK_ADJUSTMENT (spinSamples_adj), 1, 0);
    gtk_table_attach (GTK_TABLE (table5), spinSamples, 1, 2, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, spinSamples, _("The number of samples to be used (0 = all)"), NULL);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinSamples), TRUE);

    entryStopX = gtk_entry_new ();
    gtk_table_attach (GTK_TABLE (table5), entryStopX, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (entryStopX), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (entryStopX), 10);

    labelUnitStartX = gtk_label_new (_("[kHz]"));
    gtk_table_attach (GTK_TABLE (table5), labelUnitStartX, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (labelUnitStartX), 0, 0.5);

    labelUnitStopX = gtk_label_new (_("[kHz]"));
    gtk_table_attach (GTK_TABLE (table5), labelUnitStopX, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (labelUnitStopX), 0, 0.5);

    checkbutton3 = gtk_check_button_new_with_mnemonic (_("Logarithmic"));
    gtk_table_attach (GTK_TABLE (table5), checkbutton3, 1, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (checkbutton3), 1);

    checkbutton2 = gtk_check_button_new_with_mnemonic (_("Show Grid"));
    gtk_table_attach (GTK_TABLE (table5), checkbutton2, 1, 3, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (checkbutton2), 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton2), TRUE);

    label27 = gtk_label_new (_("<b>x-Axis</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frame2), label27);
    gtk_label_set_use_markup (GTK_LABEL (label27), TRUE);

    frame3 = gtk_frame_new (NULL);
    gtk_box_pack_start (GTK_BOX (hbox16), frame3, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame3), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frame3), GTK_SHADOW_NONE);

    alignment15 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frame3), alignment15);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment15), 0, 0, 12, 0);

    table7 = gtk_table_new (5, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (alignment15), table7);
    gtk_table_set_row_spacings (GTK_TABLE (table7), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table7), 12);

    label = gtk_label_new (_("Start"));
    gtk_table_attach (GTK_TABLE (table7), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    label = gtk_label_new (_("Stop"));
    gtk_table_attach (GTK_TABLE (table7), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    entryStopY = gtk_entry_new ();
    gtk_table_attach (GTK_TABLE (table7), entryStopY, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_sensitive (entryStopY, FALSE);
    gtk_entry_set_activates_default (GTK_ENTRY (entryStopY), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (entryStopY), 10);

    entryStartY = gtk_entry_new ();
    gtk_table_attach (GTK_TABLE (table7), entryStartY, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_sensitive (entryStartY, FALSE);
    gtk_entry_set_activates_default (GTK_ENTRY (entryStartY), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (entryStartY), 10);

    labelUnitStartY = gtk_label_new (_("[dB]"));
    gtk_table_attach (GTK_TABLE (table7), labelUnitStartY, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (labelUnitStartY), 0, 0.5);

    labelUnitStopY = gtk_label_new (_("[dB]"));
    gtk_table_attach (GTK_TABLE (table7), labelUnitStopY, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (labelUnitStopY), 0, 0.5);

    checkbutton9 = gtk_check_button_new_with_mnemonic (_("Logarithmic"));
    gtk_table_attach (GTK_TABLE (table7), checkbutton9, 1, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (checkbutton9), 1);

    checkbutton10 = gtk_check_button_new_with_mnemonic (_("Show Grid"));
    gtk_table_attach (GTK_TABLE (table7), checkbutton10, 1, 3, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (checkbutton10), 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton10), TRUE);

    checkbutton8 = gtk_check_button_new_with_mnemonic (_("Autoscaling"));
    gtk_table_attach (GTK_TABLE (table7), checkbutton8, 1, 3, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (checkbutton8), 1);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton8), TRUE);

    label28 = gtk_label_new (_("<b>y-Axis</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frame3), label28);
    gtk_label_set_use_markup (GTK_LABEL (label28), TRUE);

    expander1 = gtk_expander_new (NULL);
    gtk_box_pack_start (GTK_BOX (dialog_vbox1), expander1, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (expander1), 12);
    gtk_expander_set_spacing (GTK_EXPANDER (expander1), 6);

    vbox13 = gtk_vbox_new (FALSE, 24);
    gtk_container_add (GTK_CONTAINER (expander1), vbox13);

    table8 = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (vbox13), table8, FALSE, FALSE, 6);
    gtk_table_set_row_spacings (GTK_TABLE (table8), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table8), 12);

    label = gtk_label_new (_("Graph"));
    gtk_table_attach (GTK_TABLE (table8), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    label = gtk_label_new (_("Color"));
    gtk_table_attach (GTK_TABLE (table8), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    combobox2 = gtk_combo_box_new_text ();
    gtk_table_attach (GTK_TABLE (table8), combobox2, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox2), _("Graph"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox2), _("Box"));

    combobox3 = gtk_combo_box_new_text ();
    gtk_table_attach (GTK_TABLE (table8), combobox3, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox3), _("Line (continuous)"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox3), _("Circle (discrete)"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox3), _("Samples (discrete)"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox3), _("Cross (discrete)"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (combobox3), _("Box (discrete)"));

    colorselection1 = gtk_color_selection_new ();
    gtk_box_pack_start (GTK_BOX (vbox13), colorselection1, FALSE, FALSE, 0);
    gtk_color_selection_set_has_opacity_control (GTK_COLOR_SELECTION (colorselection1), FALSE);
    gtk_color_selection_set_has_palette (GTK_COLOR_SELECTION (colorselection1), TRUE);

    label64 = gtk_label_new (_("<b>Style</b>"));
    gtk_expander_set_label_widget (GTK_EXPANDER (expander1), label64);
    gtk_label_set_use_markup (GTK_LABEL (label64), TRUE);

    dialog_action_area1 = GTK_DIALOG (responseDlg)->action_area;
    gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

    helpbutton1 = gtk_button_new_from_stock ("gtk-help");
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), helpbutton1, GTK_RESPONSE_HELP);
    GTK_WIDGET_SET_FLAGS (helpbutton1, GTK_CAN_DEFAULT);

    cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), cancelbutton1, GTK_RESPONSE_CANCEL);
    GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

    button1 = gtk_button_new_from_stock ("gtk-apply");
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), button1, GTK_RESPONSE_APPLY);
    GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);

    okbutton1 = gtk_button_new_from_stock ("gtk-ok");
    gtk_dialog_add_action_widget (GTK_DIALOG (responseDlg), okbutton1, GTK_RESPONSE_OK);
    GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

    gtk_widget_show_all (responseDlg);

    /* Store pointers to all widgets, for use by lookup_widget(). */
    GLADE_HOOKUP_OBJECT_NO_REF (responseDlg, responseDlg, "responseDlg");
    GLADE_HOOKUP_OBJECT_NO_REF (responseDlg, dialog_vbox1, "dialog_vbox1");
    GLADE_HOOKUP_OBJECT (responseDlg, hbox16, "hbox16");
    GLADE_HOOKUP_OBJECT (responseDlg, frame2, "frame2");
    GLADE_HOOKUP_OBJECT (responseDlg, alignment14, "alignment14");
    GLADE_HOOKUP_OBJECT (responseDlg, table5, "table5");
    GLADE_HOOKUP_OBJECT (responseDlg, label, "label");
    GLADE_HOOKUP_OBJECT (responseDlg, label, "label");
    GLADE_HOOKUP_OBJECT (responseDlg, label, "label");
    GLADE_HOOKUP_OBJECT (responseDlg, entryStartX, "entryStartX");
    GLADE_HOOKUP_OBJECT (responseDlg, spinSamples, "spinSamples");
    GLADE_HOOKUP_OBJECT (responseDlg, entryStopX, "entryStopX");
    GLADE_HOOKUP_OBJECT (responseDlg, labelUnitStartX, "labelUnitStartX");
    GLADE_HOOKUP_OBJECT (responseDlg, labelUnitStopX, "labelUnitStopX");
    GLADE_HOOKUP_OBJECT (responseDlg, checkbutton3, "checkbutton3");
    GLADE_HOOKUP_OBJECT (responseDlg, checkbutton2, "checkbutton2");
    GLADE_HOOKUP_OBJECT (responseDlg, label27, "label27");
    GLADE_HOOKUP_OBJECT (responseDlg, frame3, "frame3");
    GLADE_HOOKUP_OBJECT (responseDlg, alignment15, "alignment15");
    GLADE_HOOKUP_OBJECT (responseDlg, table7, "table7");
    GLADE_HOOKUP_OBJECT (responseDlg, label, "label");
    GLADE_HOOKUP_OBJECT (responseDlg, label, "label");
    GLADE_HOOKUP_OBJECT (responseDlg, entryStopY, "entryStopY");
    GLADE_HOOKUP_OBJECT (responseDlg, entryStartY, "entryStartY");
    GLADE_HOOKUP_OBJECT (responseDlg, labelUnitStartY, "labelUnitStartY");
    GLADE_HOOKUP_OBJECT (responseDlg, labelUnitStopY, "labelUnitStopY");
    GLADE_HOOKUP_OBJECT (responseDlg, checkbutton9, "checkbutton9");
    GLADE_HOOKUP_OBJECT (responseDlg, checkbutton10, "checkbutton10");
    GLADE_HOOKUP_OBJECT (responseDlg, checkbutton8, "checkbutton8");
    GLADE_HOOKUP_OBJECT (responseDlg, label28, "label28");
    GLADE_HOOKUP_OBJECT (responseDlg, expander1, "expander1");
    GLADE_HOOKUP_OBJECT (responseDlg, vbox13, "vbox13");
    GLADE_HOOKUP_OBJECT (responseDlg, table8, "table8");
    GLADE_HOOKUP_OBJECT (responseDlg, label, "label");
    GLADE_HOOKUP_OBJECT (responseDlg, label, "label");
    GLADE_HOOKUP_OBJECT (responseDlg, combobox2, "combobox2");
    GLADE_HOOKUP_OBJECT (responseDlg, combobox3, "combobox3");
    GLADE_HOOKUP_OBJECT (responseDlg, colorselection1, "colorselection1");
    GLADE_HOOKUP_OBJECT (responseDlg, label64, "label64");
    GLADE_HOOKUP_OBJECT_NO_REF (responseDlg, dialog_action_area1, "dialog_action_area1");
    GLADE_HOOKUP_OBJECT (responseDlg, helpbutton1, "helpbutton1");
    GLADE_HOOKUP_OBJECT (responseDlg, cancelbutton1, "cancelbutton1");
    GLADE_HOOKUP_OBJECT (responseDlg, button1, "button1");
    GLADE_HOOKUP_OBJECT (responseDlg, okbutton1, "okbutton1");
    GLADE_HOOKUP_OBJECT_NO_REF (responseDlg, tooltips, "tooltips");

    gtk_widget_grab_focus (entryStartX);
    return responseDlg;
}


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

