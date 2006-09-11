/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Standard IIR filter dialog functions.
 *
 * \author   Copyright (c) Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/src/stdIirDesignDlg.c,v 1.1.1.1 2006-09-11 15:52:20 ralf Exp $
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
#include "dfcgen.h"
#include "stdIirFilter.h"
#include "stdIirDesignDlg.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

typedef struct
{
    STDIIR_TYPE type;            /**< Associated characteristic (type of IIR) */
    GtkWidget* btn;                     /**< Associated radio button widget */
    char* name;               /**< Radio button text (name of characteristic) */
    BOOL hasRipple;     /**< TRUE if ripple attenuation is an input parameter */
    BOOL hasStopband; /**< TRUE if stopband attenuation is an input parameter */
    BOOL hasAngle;                   /**< TRUE if angle is an input parameter */
} STDIIR_DLG_DESCRIPTOR;


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define STDIIRDLG_LABEL_CENTER_TEXT  _("f<sub>_Center</sub>") /**< Label for center frequency */
#define STDIIRDLG_LABEL_CUTOFF_TEXT  _("f<sub>_Cutoff</sub>") /**< Label for cutoff frequency */

#define STDIIRDLG_WIDGET_MAIN   "stdIirDesignDlgMain"
#define STDIIRDLG_COMBO_FTR     "comboFtrType"
#define STDIIRDLG_ENTRY_BANDW   "entryBandwidth"
#define STDIIRDLG_LABEL_CENTER  "labelCenterF"
#define STDIIRDLG_CHKBTN_GEOMETRIC "checkGeometric"
#define STDIIRDLG_ENTRY_CENTER  "entryCenterF"
#define STDIIRDLG_ENTRY_CUTOFF  "entryCutF"
#define STDIIRDLG_ENTRY_SAMPLE  "entrySampleF"
#define STDIIRDLG_SPIN_DEGREE   "spinDegree"
#define STDIIRDLG_ENTRY_RIPPLE  "entryRipple"
#define STDIIRDLG_ENTRY_MINATT  "entryMinAtt"
#define STDIIRDLG_SPIN_ANGLE    "spinAngle"


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/** Array of all standard IIR filter types (in same order as the enums).
 */
static STDIIR_DLG_DESCRIPTOR stdIirDlgDesc[STDIIR_TYPE_SIZE] =
{   /* type, hasRipple, hasStopband, hasAngle */
    {STDIIR_TYPE_BUTTERWORTH, NULL, N_("Butterworth"), FALSE, FALSE, FALSE},
    {STDIIR_TYPE_CHEBY, NULL, N_("Chebyshev"), TRUE, FALSE, FALSE},
    {STDIIR_TYPE_CHEBYINV, NULL, N_("Chebyshev Inv."), FALSE, TRUE, FALSE},
    {STDIIR_TYPE_CAUER1, NULL, N_("Cauer (Case I)"), TRUE, FALSE, TRUE},
    {STDIIR_TYPE_CAUER2, NULL, N_("Cauer (Case II)"), FALSE, TRUE, TRUE},
    {STDIIR_TYPE_BESSEL, NULL, N_("Bessel"), FALSE, FALSE, FALSE}
}; /* stdIirDlgDesc[] */


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void filterTypeChanged (GtkRadioButton* radiobutton, gpointer user_data);
static void transformTypeChanged (GtkComboBox *combobox, gpointer user_data);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** This function is called if the IIR filter type changes (\e toggle event
 *  from radio button).
 *
 *  \param radiobutton  Radio button which changes the state.
 *  \param user_data    User data set when the signal handler was connected. In
 *                      that case it points to the associated STDIIRDLG_DESCRIPTOR.
 *
 ******************************************************************************/
static void filterTypeChanged (GtkRadioButton* radiobutton, gpointer user_data)
{
    STDIIR_DLG_DESCRIPTOR* pDesc = (STDIIR_DLG_DESCRIPTOR*) user_data;

    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(radiobutton)))
    {                                              /* really not de-activated */
        gtk_widget_set_sensitive (lookup_widget (GTK_WIDGET (radiobutton),
                                                 STDIIRDLG_ENTRY_RIPPLE),
                                  pDesc->hasRipple);

        gtk_widget_set_sensitive (lookup_widget (GTK_WIDGET (radiobutton),
                                                 STDIIRDLG_ENTRY_MINATT),
                                  pDesc->hasStopband);

        gtk_widget_set_sensitive (lookup_widget (GTK_WIDGET (radiobutton),
                                                 STDIIRDLG_SPIN_ANGLE),
                                  pDesc->hasAngle);
    } /* if */
} /* filterTypeChanged() */



/* FUNCTION *******************************************************************/
/** This function is called if the frequency transform selection changes.
 *
 *  \param combobox     Combobox widget which determines the frequency
 *                      transformation.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
static void transformTypeChanged (GtkComboBox *combobox, gpointer user_data)
{
    int i, index;
    GtkWidget *widget[4];

    const gboolean enabled[FTR_SIZE][4] =
    { /* entryCutF, entryCenterF, entryBandwidth, checkGeometric */
        {TRUE,      FALSE,        FALSE,          FALSE},          /* FTR_NON */
        {FALSE,     TRUE,         FALSE,          FALSE},     /* FTR_HIGHPASS */
        {FALSE,     TRUE,         TRUE,           TRUE},      /* FTR_BANDPASS */
        {FALSE,     TRUE,         TRUE,           TRUE},      /* FTR_BANDSTOP */
    };

    GtkWidget *label = lookup_widget (GTK_WIDGET (combobox), STDIIRDLG_LABEL_CENTER);

    widget[0] = lookup_widget (GTK_WIDGET (combobox), STDIIRDLG_ENTRY_CUTOFF);
    widget[1] = lookup_widget (GTK_WIDGET (combobox), STDIIRDLG_ENTRY_CENTER);
    widget[2] = lookup_widget (GTK_WIDGET (combobox), STDIIRDLG_ENTRY_BANDW);
    widget[3] = lookup_widget (GTK_WIDGET (combobox), STDIIRDLG_CHKBTN_GEOMETRIC);

    index = gtk_combo_box_get_active(combobox);

    if (index < 0)
    {
        index = 0;                       /* set any, in case nothing selected */
    } /* if */


    switch (index)
    {
        case FTR_HIGHPASS:
            gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), STDIIRDLG_LABEL_CUTOFF_TEXT);
            break;

        case FTR_BANDPASS:
        case FTR_BANDSTOP:
            gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), STDIIRDLG_LABEL_CENTER_TEXT);
            break;

        case FTR_NON: 
        default:

            break;

    } /* switch */


    for (i = 0; i < 4; i++)
    {
        gtk_widget_set_sensitive (widget[i], enabled[index][i]);
    } /* for */

} /* transformTypeChanged() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Standard IIR filter design dialog creation function. This function is
 *  called if a standard IIR filter shall be designed.
 *
 *  \note Originally generated by \e Glade, but heavy modified (see interface.c,
 *        which isn't part of the project).
 *
 *  \param topWidget    Toplevel widget.
 *  \param boxDesignDlg The box widget to be used for the filter dialog.
 *
 ******************************************************************************/
void stdIirDesignDlgCreate (GtkWidget *topWidget, GtkWidget *boxDesignDlg)
{
    STDIIR_TYPE type;

    GtkWidget *stdIirDesignDlgMain;
    GtkWidget *frameChar;
    GtkWidget *alignment7;
    GtkWidget *vbox8;
    GSList *iirTypeRadioGroup = NULL;
    GtkWidget *label8;
    GtkWidget *frameTransform;
    GtkWidget *alignment9;
    GtkWidget *table4;
    GtkWidget *label17;
    GtkWidget *eventbox1;
    GtkWidget *comboFtrType;
    GtkWidget *entryBandwidth;
    GtkWidget *label20;
    GtkWidget *labelCenterF;
    GtkWidget *label22;
    GtkWidget *checkGeometric;
    GtkWidget *entryCenterF;
    GtkWidget *label19;
    GtkWidget *frameLowpass;
    GtkWidget *alignment10;
    GtkWidget *table2;
    GtkWidget *entryCutF;
    GtkWidget *entrySampleF;
    GtkObject *spinDegree_adj;
    GtkWidget *spinDegree;
    GtkWidget *label9;
    GtkWidget *label10;
    GtkWidget *label11;
    GtkWidget *label21;
    GtkWidget *frameParams;
    GtkWidget *alignment8;
    GtkWidget *table3;
    GtkWidget *label13;
    GtkWidget *label14;
    GtkWidget *label15;
    GtkWidget *entryRipple;
    GtkWidget *entryMinAtt;
    GtkObject *spinAngle_adj;
    GtkWidget *spinAngle;
    GtkWidget *label12;

    GtkTooltips *tooltips = GTK_TOOLTIPS(g_object_get_data(G_OBJECT(topWidget), "tooltips"));

    stdIirDesignDlgMain = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (boxDesignDlg), stdIirDesignDlgMain, TRUE, TRUE, 0);
    gtk_table_set_row_spacings (GTK_TABLE (stdIirDesignDlgMain), 6);
    gtk_table_set_col_spacings (GTK_TABLE (stdIirDesignDlgMain), 12);

    /* added: */ gtk_box_reorder_child (GTK_BOX (boxDesignDlg), stdIirDesignDlgMain, 1);

    frameChar = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frameChar, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frameChar), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frameChar), GTK_SHADOW_NONE);

    alignment7 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frameChar), alignment7);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment7), 0, 0, 12, 0);

    vbox8 = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (alignment7), vbox8);
    gtk_container_set_border_width (GTK_CONTAINER (vbox8), 6);

    for (type = 0; type < STDIIR_TYPE_SIZE; type++)
    {
        stdIirDlgDesc[type].btn = 
            gtk_radio_button_new_with_mnemonic (NULL, gettext (stdIirDlgDesc[type].name));

        gtk_box_pack_start (GTK_BOX (vbox8), stdIirDlgDesc[type].btn, FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (stdIirDlgDesc[type].btn), 1);
        gtk_radio_button_set_group (GTK_RADIO_BUTTON (stdIirDlgDesc[type].btn), iirTypeRadioGroup);
        iirTypeRadioGroup = gtk_radio_button_get_group (GTK_RADIO_BUTTON (stdIirDlgDesc[type].btn));

        g_signal_connect ((gpointer) stdIirDlgDesc[type].btn, "toggled",
                          G_CALLBACK (filterTypeChanged), &stdIirDlgDesc[type]);
    } /* for */


    label8 = gtk_label_new (_("<b>Characteristic</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frameChar), label8);
    gtk_label_set_use_markup (GTK_LABEL (label8), TRUE);

    frameTransform = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frameTransform, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frameTransform), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frameTransform), GTK_SHADOW_NONE);

    alignment9 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frameTransform), alignment9);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment9), 0, 0, 12, 0);

    table4 = gtk_table_new (4, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (alignment9), table4);
    gtk_container_set_border_width (GTK_CONTAINER (table4), 6);
    gtk_table_set_row_spacings (GTK_TABLE (table4), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table4), 12);

    label17 = gtk_label_new_with_mnemonic ("");
    gtk_table_attach (GTK_TABLE (table4), label17, 0, 1, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label17), 0, 0.5);
    gtk_label_set_single_line_mode (GTK_LABEL (label17), TRUE);

    eventbox1 = gtk_event_box_new ();
    gtk_table_attach (GTK_TABLE (table4), eventbox1, 1, 2, 0, 1,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, eventbox1, _("Type of frequency transformation"), NULL);

    comboFtrType = gtk_combo_box_new_text ();
    gtk_container_add (GTK_CONTAINER (eventbox1), comboFtrType);
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtrType), _("None"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtrType), _("Highpass"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtrType), _("Bandpass"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtrType), _("Bandstop"));

    entryBandwidth = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entryBandwidth), TRUE);
    gtk_table_attach (GTK_TABLE (table4), entryBandwidth, 1, 2, 2, 3,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, entryBandwidth, _("Bandwidth"), NULL);
    gtk_entry_set_width_chars (GTK_ENTRY (entryBandwidth), 10);

    label20 = gtk_label_new_with_mnemonic (_("_Type"));
    gtk_table_attach (GTK_TABLE (table4), label20, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label20), 0, 0.5);
    gtk_label_set_single_line_mode (GTK_LABEL (label20), TRUE);

    labelCenterF = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(labelCenterF), STDIIRDLG_LABEL_CENTER_TEXT);
    gtk_table_attach (GTK_TABLE (table4), labelCenterF, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (labelCenterF), 0, 0.5);
    gtk_label_set_single_line_mode (GTK_LABEL (labelCenterF), TRUE);

    label22 = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(label22), _("f<sub>_Bandwidth</sub>"));
    gtk_table_attach (GTK_TABLE (table4), label22, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label22), 0, 0.5);

    checkGeometric = gtk_check_button_new_with_mnemonic (_("_Geometric"));
    gtk_table_attach (GTK_TABLE (table4), checkGeometric, 1, 2, 3, 4,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, checkGeometric, _("Input center frequency is not symmetric (geometric mean between cutoff frequencies)"), NULL);

    entryCenterF = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entryCenterF), TRUE);
    gtk_table_attach (GTK_TABLE (table4), entryCenterF, 1, 2, 1, 2,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, entryCenterF, _("Cutoff frequency (highpass) or center frequency (bandpass, bandstop)"), NULL);
    gtk_entry_set_width_chars (GTK_ENTRY (entryCenterF), 10);

    label19 = gtk_label_new (_("<b>Transform</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frameTransform), label19);
    gtk_label_set_use_markup (GTK_LABEL (label19), TRUE);

    frameLowpass = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frameLowpass, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frameLowpass), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frameLowpass), GTK_SHADOW_NONE);

    alignment10 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frameLowpass), alignment10);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment10), 0, 0, 12, 0);

    table2 = gtk_table_new (3, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (alignment10), table2);
    gtk_container_set_border_width (GTK_CONTAINER (table2), 6);
    gtk_table_set_row_spacings (GTK_TABLE (table2), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table2), 12);

    entryCutF = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entryCutF), TRUE);
    gtk_table_attach (GTK_TABLE (table2), entryCutF, 1, 2, 2, 3,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, entryCutF, _("Cutoff frequency"), NULL);
    gtk_entry_set_width_chars (GTK_ENTRY (entryCutF), 10);

    entrySampleF = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entrySampleF), TRUE);
    gtk_table_attach (GTK_TABLE (table2), entrySampleF, 1, 2, 1, 2,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, entrySampleF, _("Sample frequency"), NULL);
    gtk_entry_set_width_chars (GTK_ENTRY (entrySampleF), 10);

    spinDegree_adj = gtk_adjustment_new (1, 0, FLT_DEGREE_MAX, 1, 10, 10);
    spinDegree = gtk_spin_button_new (GTK_ADJUSTMENT (spinDegree_adj), 1, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (spinDegree), TRUE);
    gtk_table_attach (GTK_TABLE (table2), spinDegree, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, spinDegree, _("Degree of reference lowpass"), NULL);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinDegree), TRUE);

    label9 = gtk_label_new_with_mnemonic (_("_Degree"));
    gtk_table_attach (GTK_TABLE (table2), label9, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);
    gtk_label_set_single_line_mode (GTK_LABEL (label9), TRUE);

    label10 = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL (label10), _("f<sub>_Sample</sub>"));
    gtk_table_attach (GTK_TABLE (table2), label10, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);
    gtk_label_set_single_line_mode (GTK_LABEL (label10), TRUE);

    label11 = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(label11), STDIIRDLG_LABEL_CUTOFF_TEXT);
    gtk_table_attach (GTK_TABLE (table2), label11, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label11), 0, 0.5);

    label21 = gtk_label_new (_("<b>Lowpass</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frameLowpass), label21);
    gtk_label_set_use_markup (GTK_LABEL (label21), TRUE);

    frameParams = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frameParams, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frameParams), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frameParams), GTK_SHADOW_NONE);

    alignment8 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frameParams), alignment8);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment8), 0, 0, 12, 0);

    table3 = gtk_table_new (3, 2, FALSE);
    gtk_container_add (GTK_CONTAINER (alignment8), table3);
    gtk_container_set_border_width (GTK_CONTAINER (table3), 6);
    gtk_table_set_row_spacings (GTK_TABLE (table3), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table3), 12);

    label13 = gtk_label_new_with_mnemonic (_("_Ripple"));
    gtk_table_attach (GTK_TABLE (table3), label13, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);
    gtk_label_set_single_line_mode (GTK_LABEL (label13), TRUE);

    label14 = gtk_label_new_with_mnemonic (_("Stop-Att."));
    gtk_table_attach (GTK_TABLE (table3), label14, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label14), 0, 0.5);
    gtk_label_set_single_line_mode (GTK_LABEL (label14), TRUE);

    label15 = gtk_label_new_with_mnemonic (_("_Angle"));
    gtk_table_attach (GTK_TABLE (table3), label15, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label15), 0, 0.5);

    entryRipple = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entryRipple), TRUE);
    gtk_table_attach (GTK_TABLE (table3), entryRipple, 1, 2, 0, 1,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, entryRipple, _("Passband ripple in dB"), NULL);
    gtk_entry_set_width_chars (GTK_ENTRY (entryRipple), 10);

    entryMinAtt = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (entryMinAtt), TRUE);
    gtk_table_attach (GTK_TABLE (table3), entryMinAtt, 1, 2, 1, 2,
                      (GtkAttachOptions) (0),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, entryMinAtt, _("Stopband attenuation in dB"), NULL);
    gtk_entry_set_width_chars (GTK_ENTRY (entryMinAtt), 10);

    spinAngle_adj = gtk_adjustment_new (60, 1, 89, 1, 10, 10);
    spinAngle = gtk_spin_button_new (GTK_ADJUSTMENT (spinAngle_adj), 1, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (spinAngle), TRUE);
    gtk_table_attach (GTK_TABLE (table3), spinAngle, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_tooltips_set_tip (tooltips, spinAngle, _("Modular angle in degree"), NULL);
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (spinAngle), TRUE);

    label12 = gtk_label_new (_("<b>Parameters</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frameParams), label12);
    gtk_label_set_use_markup (GTK_LABEL (label12), TRUE);

    g_signal_connect ((gpointer) comboFtrType, "changed",
                      G_CALLBACK (transformTypeChanged),
                      NULL);

    gtk_label_set_mnemonic_widget (GTK_LABEL (label17), entrySampleF);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label20), comboFtrType);
    gtk_label_set_mnemonic_widget (GTK_LABEL (labelCenterF), entryCenterF);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label22), entryBandwidth);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label9), spinDegree);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label10), entrySampleF);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label11), entryCutF);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label13), entryRipple);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label14), entryMinAtt);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label15), spinAngle);

    /* Store pointers to some widgets, for use by lookup_widget(). */
    GLADE_HOOKUP_OBJECT (topWidget, stdIirDesignDlgMain, STDIIRDLG_WIDGET_MAIN);
    GLADE_HOOKUP_OBJECT (topWidget, comboFtrType, STDIIRDLG_COMBO_FTR);
    GLADE_HOOKUP_OBJECT (topWidget, entryBandwidth, STDIIRDLG_ENTRY_BANDW);
    GLADE_HOOKUP_OBJECT (topWidget, labelCenterF, STDIIRDLG_LABEL_CENTER);
    GLADE_HOOKUP_OBJECT (topWidget, checkGeometric, STDIIRDLG_CHKBTN_GEOMETRIC);
    GLADE_HOOKUP_OBJECT (topWidget, entryCenterF, STDIIRDLG_ENTRY_CENTER);
    GLADE_HOOKUP_OBJECT (topWidget, entryCutF, STDIIRDLG_ENTRY_CUTOFF);
    GLADE_HOOKUP_OBJECT (topWidget, entrySampleF, STDIIRDLG_ENTRY_SAMPLE);
    GLADE_HOOKUP_OBJECT (topWidget, spinDegree, STDIIRDLG_SPIN_DEGREE);
    GLADE_HOOKUP_OBJECT (topWidget, entryRipple, STDIIRDLG_ENTRY_RIPPLE);
    GLADE_HOOKUP_OBJECT (topWidget, entryMinAtt, STDIIRDLG_ENTRY_MINATT);
    GLADE_HOOKUP_OBJECT (topWidget, spinAngle, STDIIRDLG_SPIN_ANGLE);


    gtk_widget_show_all (stdIirDesignDlgMain);

    /* The following is added wrt. glade
     */
    gtk_combo_box_set_active(GTK_COMBO_BOX (comboFtrType), FTR_NON);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(stdIirDlgDesc[STDIIR_TYPE_BUTTERWORTH].btn), TRUE);

    gtk_widget_set_sensitive (entryRipple, FALSE);
    gtk_widget_set_sensitive (entryMinAtt, FALSE);
    gtk_widget_set_sensitive (spinAngle, FALSE);

} /* stdIirDlgCreate() */



/* FUNCTION *******************************************************************/
/** Standard IIR filter design dialog destroy function.
 *
 *  \note If the dialog is not active the function does nothing.
 *
 *  \param topWidget    Toplevel widget.
 *
 ******************************************************************************/
void stdIirDesignDlgDestroy (GtkWidget *topWidget)
{
    GtkWidget* widget = lookup_widget (topWidget, STDIIRDLG_WIDGET_MAIN);

    if (widget != NULL)
    {                                                /* remove all references */
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_COMBO_FTR);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_ENTRY_BANDW);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_LABEL_CENTER);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_CHKBTN_GEOMETRIC);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_ENTRY_CENTER);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_ENTRY_CUTOFF);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_ENTRY_SAMPLE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_SPIN_DEGREE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_ENTRY_RIPPLE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_ENTRY_MINATT);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_SPIN_ANGLE);

        GLADE_HOOKUP_OBJECT_NO_REF(topWidget, NULL, STDIIRDLG_WIDGET_MAIN);

        gtk_widget_destroy(widget);
    } /* if */

} /* stdIirDesignDlgDestroy() */



/* FUNCTION *******************************************************************/
/** Checks whether the standard IIR filter design dialog is active or not.
 *
 *  \param topWidget    Toplevel widget.
 *
 *  \return             TRUE if the dialog is active (the main-widget of
 *                      standard IIR dialog found), else FALSE.
 ******************************************************************************/
BOOL stdIirDesignDlgActive (GtkWidget *topWidget)
{
    GtkWidget* widget = lookup_widget (topWidget, STDIIRDLG_WIDGET_MAIN);

    return (widget != NULL);
} /* stdIirDesignDlgActive() */



/* FUNCTION *******************************************************************/
/** Standard IIR filter design dialog ready/apply function.
 *
 *  \param topWidget    Toplevel widget.
 *
 *  \return             - 0 (or GSL_SUCCESS) if okay and nothing has changed.
 *                      - a negative number (typically GSL_CONTINUE) if a
 *                        coefficient or the degree has changed, but the filter
 *                        is valid. You can use the FLTERR_WARNING macro from
 *                        filterSupport.h to check this condition.
 *                      - a positive error number (typically from from errno.h
 *                        or gsl_errno.h) that something is wrong and the
 *                        filter must be seen as invalid. You can use the
 *                        FLTERR_CRITICAL macro from filterSupport.h to check
 *                        this condition.
 ******************************************************************************/
int stdIirDesignDlgApply (GtkWidget *topWidget)
{
    STDIIR_DESIGN design =                       /* set (some) default values */
    { /* type, degree, cutoff, ... */
        STDIIR_TYPE_SIZE - 1, 0,
    };


    while ((design.type > 0) &&               /* look for active radio button */
           !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(stdIirDlgDesc[design.type].btn)))
    {
        --design.type;
    } /* while */

    if (stdIirDlgDesc[design.type].hasRipple)
    {
    } /* if */

    if (stdIirDlgDesc[design.type].hasStopband)
    {
    } /* if */

    if (stdIirDlgDesc[design.type].hasAngle)
    {
    } /* if */


    return 0;
} /* stdIirDesignDlgApply() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
