/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Linear FIR filter dialog functions.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dfcgen.h"
#include "cfgSettings.h"
#include "projectFile.h"
#include "linFirFilter.h"
#include "filterSupport.h"
#include "dialogSupport.h"
#include "linFirDesignDlg.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

/** Linear FIR filter characteristic (Blackman, Kaiser, etc.)
 */
typedef struct
{
    LINFIR_TYPE type;            /**< Associated characteristic (type of FIR) */
    GtkWidget* btn;                       /**< Associated radio button widget */
    char* name;                            /**< Radio button text (type name) */
} LINFIRDLG_CHAR;


/** Linear FIR filter DSP windows.
 */
typedef struct
{
    LINFIR_DSPWIN type;                       /**< Associated DSP window type */
    GtkWidget* btn;                       /**< Associated radio button widget */
    char* name;                            /**< Radio button text (type name) */
} LINFIRDLG_DSPWIN;


/* LOCAL CONSTANT DEFINITIONS *************************************************/

/** Label for center frequency
 */
#define LINFIRDLG_LABEL_CENTER_TEXT  _("f<sub>Center</sub>")

/** Label for cutoff frequency
 */
#define LINFIRDLG_LABEL_CUTOFF_TEXT  _("f<sub>Cutoff</sub>")


#define LINFIRDLG_WIDGET_MAIN   "linFirDesignDlgMain"
#define LINFIRDLG_COMBO_FTR     "comboFtrType"
#define LINFIRDLG_ENTRY_BANDW   "entryBandwidth"
#define LINFIRDLG_LABEL_CENTER  "labelCenterF"
#define LINFIRDLG_CHKBTN_GEOMETRIC "checkGeometric"
#define LINFIRDLG_ENTRY_CENTER  "entryCenterF"
#define LINFIRDLG_ENTRY_CUTOFF  "entryCutF"
#define LINFIRDLG_ENTRY_SAMPLE  "entrySampleF"
#define LINFIRDLG_SPIN_DEGREE   "spinDegree"
#define LINFIRDLG_ENTRY_KAISER  "entryKaiser"

#define LINFIRDLG_UNIT_CUTOFF   "unitCutF"
#define LINFIRDLG_UNIT_SAMPLE   "unitSampleF"
#define LINFIRDLG_UNIT_CENTER   "unitCenterF"
#define LINFIRDLG_UNIT_BANDW    "unitBandwidth"


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/** Array of all standard FIR filter types (in same order as the enums).
 */
static LINFIRDLG_CHAR linFirDlgChar[LINFIR_TYPE_SIZE] =
{   /* type, btn, name */
    [LINFIR_TYPE_RECT] = {LINFIR_TYPE_RECT,  NULL, N_("Rectangular (perfect)")},
    [LINFIR_TYPE_COS] = {LINFIR_TYPE_COS,   NULL, N_("Cosine")},
    [LINFIR_TYPE_COS2] = {LINFIR_TYPE_COS2,  NULL, N_("Squared cosine")},
    [LINFIR_TYPE_GAUSS] = {LINFIR_TYPE_GAUSS, NULL, N_("Gaussian")},
    [LINFIR_TYPE_SQR] = {LINFIR_TYPE_SQR,   NULL, N_("Squared 1st order")}
}; /* linFirDlgChar[] */


/* DSP window GtkRadioButton widgets
 */
static LINFIRDLG_DSPWIN linFirDlgWin[LINFIR_DSPWIN_SIZE] =
{ /* type, btn, name */
    [LINFIR_DSPWIN_RECT] = {LINFIR_DSPWIN_RECT, NULL, N_("None")},
    [LINFIR_DSPWIN_HAMMING] = {LINFIR_DSPWIN_HAMMING, NULL, N_("Hamming")},
    [LINFIR_DSPWIN_VANHANN] = {LINFIR_DSPWIN_VANHANN, NULL, N_("van Hann")},
    [LINFIR_DSPWIN_BLACKMAN] = {LINFIR_DSPWIN_BLACKMAN, NULL, N_("Blackman")},
    [LINFIR_DSPWIN_KAISER] = {LINFIR_DSPWIN_KAISER, NULL, N_("Kaiser")}
};


static const gboolean ftrEntry[FTR_SIZE][5] =
{                  /* entryCutF, entryCenterF, entryBandwidth, checkGeometric, evenDegree */
    [FTR_NON]      = {TRUE,      FALSE,        FALSE,          FALSE,          FALSE},
    [FTR_HIGHPASS] = {FALSE,     TRUE,         FALSE,          FALSE,          TRUE},
    [FTR_BANDPASS] = {FALSE,     TRUE,         TRUE,           TRUE,           TRUE},
    [FTR_BANDSTOP] = {FALSE,     TRUE,         TRUE,           TRUE,           TRUE},
};



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void filterWindowChanged (GtkRadioButton* radiobutton, gpointer user_data);
static void transformTypeChanged (GtkComboBox *combobox, gpointer user_data);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** This function is called if the DSP window type changes (\e toggle event
 *  from radio button).
 *
 *  \param radiobutton  Radio button which changes the state.
 *  \param user_data    User data set when the signal handler was connected. In
 *                      that case it is of type LINFIR_DSPWIN.
 *
 ******************************************************************************/
static void filterWindowChanged (GtkRadioButton* radiobutton, gpointer user_data)
{
    LINFIRDLG_DSPWIN *pDspWin = (LINFIRDLG_DSPWIN *)user_data;

    if (pDspWin->type == LINFIR_DSPWIN_KAISER)
    {
        gtk_widget_set_sensitive (
            lookup_widget (GTK_WIDGET (radiobutton), LINFIRDLG_ENTRY_KAISER),
            gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(radiobutton)));
    } /* if */
} /* filterWindowChanged() */



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

    GtkWidget *label = lookup_widget (GTK_WIDGET (combobox), LINFIRDLG_LABEL_CENTER);

    widget[0] = lookup_widget (GTK_WIDGET (combobox), LINFIRDLG_ENTRY_CUTOFF);
    widget[1] = lookup_widget (GTK_WIDGET (combobox), LINFIRDLG_ENTRY_CENTER);
    widget[2] = lookup_widget (GTK_WIDGET (combobox), LINFIRDLG_ENTRY_BANDW);
    widget[3] = lookup_widget (GTK_WIDGET (combobox), LINFIRDLG_CHKBTN_GEOMETRIC);

    index = gtk_combo_box_get_active(combobox);

    if (index < 0)
    {
        index = 0;                       /* set any, in case nothing selected */
    } /* if */


    switch (index)
    {
        case FTR_HIGHPASS:
            gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), LINFIRDLG_LABEL_CUTOFF_TEXT);
            break;

        case FTR_BANDPASS:
        case FTR_BANDSTOP:
            gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), LINFIRDLG_LABEL_CENTER_TEXT);
            break;

        case FTR_NON: 
        default:

            break;

    } /* switch */


    for (i = 0; i < 4; i++)
    {
        gtk_widget_set_sensitive (widget[i], ftrEntry[index][i]);
    } /* for */

} /* transformTypeChanged() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Linear FIR filter design dialog creation function. This function is
 *  called if a linear FIR filter shall be designed.
 *
 *  \note Originally generated by \e Glade, but heavy modified (see interface.c,
 *        which isn't part of the project).
 *
 *  \param topWidget    Toplevel widget.
 *  \param boxDesignDlg GtkVBox widget, which is the container for the dialog.
 *                      The dialog must be mapped to row 1 of it.
 *  \param pPrefs       Pointer to desktop preferences.
 *
 ******************************************************************************/
void linFirDesignDlgCreate (GtkWidget *topWidget, GtkWidget *boxDesignDlg,
                            const CFG_DESKTOP* pPrefs)
{
    LINFIR_TYPE type;
    LINFIR_DSPWIN win;

    GtkWidget *linFirDesignDlgMain, *comboFtr;
    GtkWidget *widget, *label, *frame, *box, *table;
    GtkObject *spinAdjust;

    GSList *firTypeRadioGroup = NULL;
    GSList *dspWinRadioGroup = NULL;

    linFirDesignDlgMain = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (boxDesignDlg), linFirDesignDlgMain, TRUE, TRUE, 0);
    gtk_box_reorder_child (GTK_BOX (boxDesignDlg), linFirDesignDlgMain, 1);
    GLADE_HOOKUP_OBJECT (topWidget, linFirDesignDlgMain, LINFIRDLG_WIDGET_MAIN);

    /* Characteristic
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (linFirDesignDlgMain), frame, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    label = gtk_label_new (_("<b>Characteristic</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    widget = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frame), widget);
    gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 12, 0);

    box = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (widget), box);
    gtk_container_set_border_width (GTK_CONTAINER (box), 6);

    for (type = 0; type < LINFIR_TYPE_SIZE; type++)
    {
        linFirDlgChar[type].btn = 
            gtk_radio_button_new_with_mnemonic (
                NULL, gettext (linFirDlgChar[type].name));

        gtk_box_pack_start (GTK_BOX (box), linFirDlgChar[type].btn, FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (linFirDlgChar[type].btn), 1);
        gtk_radio_button_set_group (GTK_RADIO_BUTTON (linFirDlgChar[type].btn), firTypeRadioGroup);
        firTypeRadioGroup = gtk_radio_button_get_group (GTK_RADIO_BUTTON (linFirDlgChar[type].btn));
    } /* for */


    /* Frequency transformation
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (linFirDesignDlgMain), frame, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    label = gtk_label_new (_("<b>Transform</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    widget = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frame), widget);
    gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 12, 0);

    table = gtk_table_new (4, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (widget), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 6);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);

    widget = gtk_event_box_new ();
    gtk_table_attach (GTK_TABLE (table), widget, 1, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Type of frequency transformation"));

    label = gtk_label_new_with_mnemonic (_("_Type"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    comboFtr = gtk_combo_box_new_text ();     /* frequency transform combobox */
    gtk_container_add (GTK_CONTAINER (widget), comboFtr);
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtr), _("None"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtr), _("Highpass"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtr), _("Bandpass"));
    gtk_combo_box_append_text (GTK_COMBO_BOX (comboFtr), _("Bandstop"));

    GLADE_HOOKUP_OBJECT (topWidget, comboFtr, LINFIRDLG_COMBO_FTR);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), comboFtr);

    widget = gtk_entry_new ();                                   /* bandwidth */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Bandwidth"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (topWidget, widget, LINFIRDLG_ENTRY_BANDW);

    label = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), _("f<sub>Bandw.</sub>"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    label = gtk_label_new (pPrefs->frequUnit.name);         /* bandwidth unit */
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    GLADE_HOOKUP_OBJECT (topWidget, label, LINFIRDLG_UNIT_BANDW);

    widget = gtk_entry_new ();                            /* center frequency */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Cutoff frequency (highpass) or center"
                                           " frequency (bandpass, bandstop)"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (topWidget, widget, LINFIRDLG_ENTRY_CENTER);

    label = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), LINFIRDLG_LABEL_CENTER_TEXT);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
    GLADE_HOOKUP_OBJECT (topWidget, label, LINFIRDLG_LABEL_CENTER);

    label = gtk_label_new (pPrefs->frequUnit.name);  /* center frequency unit */
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    GLADE_HOOKUP_OBJECT (topWidget, label, LINFIRDLG_UNIT_CENTER);

    widget = gtk_check_button_new_with_mnemonic (_("_Geometric"));
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget,
                                 _("Check this if the center frequency shall be the"
                                   " geometric mean between both cutoff frequencies"
                                   " (otherwise it is the arithmetic mean)."));
    GLADE_HOOKUP_OBJECT (topWidget, widget, LINFIRDLG_CHKBTN_GEOMETRIC);

    /* Reference Lowpass
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (linFirDesignDlgMain), frame, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    label = gtk_label_new (_("<b>Lowpass</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    widget = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frame), widget);
    gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 12, 0);

    table = gtk_table_new (3, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (widget), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 6);
    gtk_table_set_row_spacings (GTK_TABLE (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);

    widget = gtk_entry_new ();                            /* cutoff frequency */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    gtk_widget_set_tooltip_text (widget, _("Cutoff frequency"));
    GLADE_HOOKUP_OBJECT (topWidget, widget, LINFIRDLG_ENTRY_CUTOFF);

    label = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), LINFIRDLG_LABEL_CUTOFF_TEXT);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    label = gtk_label_new (pPrefs->frequUnit.name); /* cutoff frequency unit label */
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    GLADE_HOOKUP_OBJECT (topWidget, label, LINFIRDLG_UNIT_CUTOFF);

    widget = gtk_entry_new ();                            /* sample frequency */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Sample frequency"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (topWidget, widget, LINFIRDLG_ENTRY_SAMPLE);

    label = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), _("f<sub>_Sample</sub>"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    label = gtk_label_new (pPrefs->frequUnit.name);  /* sample frequency unit */
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    GLADE_HOOKUP_OBJECT (topWidget, label, LINFIRDLG_UNIT_SAMPLE);


    /* degree spin-button */
    spinAdjust = gtk_adjustment_new (1, FLT_DEGREE_MIN, FLT_DEGREE_MAX, 1, 10, 0);
    widget = gtk_spin_button_new (GTK_ADJUSTMENT (spinAdjust), 1, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Degree of filter"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE);
    GLADE_HOOKUP_OBJECT (topWidget, widget, LINFIRDLG_SPIN_DEGREE);

    label = gtk_label_new_with_mnemonic (_("_Degree"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);


    /* DSP window
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (linFirDesignDlgMain), frame, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    label = gtk_label_new (_("<b>Window</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frame), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    widget = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frame), widget);
    gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 12, 0);

    table = gtk_table_new (5, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (widget), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);

    for (win = 0; win < LINFIR_DSPWIN_SIZE; win++)
    {
        linFirDlgWin[win].btn = 
            gtk_radio_button_new_with_mnemonic (
                NULL, gettext (linFirDlgWin[win].name));

        gtk_table_attach (GTK_TABLE (table), linFirDlgWin[win].btn,
                          0, (win == LINFIR_DSPWIN_KAISER)? 1 : 2,
                          win,  win + 1, (GtkAttachOptions) (GTK_FILL),
                          (GtkAttachOptions) (0), 0, 0);
        gtk_container_set_border_width (GTK_CONTAINER (linFirDlgWin[win].btn), 1);
        gtk_radio_button_set_group (GTK_RADIO_BUTTON (linFirDlgWin[win].btn), dspWinRadioGroup);
        dspWinRadioGroup = gtk_radio_button_get_group (GTK_RADIO_BUTTON (linFirDlgWin[win].btn));
        g_signal_connect ((gpointer) linFirDlgWin[win].btn, "toggled",
                          G_CALLBACK (filterWindowChanged), &linFirDlgWin[win]);
    } /* for */


    widget = gtk_entry_new ();
    gtk_table_attach (GTK_TABLE (table), widget, 2, 3, 4, 5,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_sensitive (widget, FALSE);
    gtk_widget_set_tooltip_text (widget, _("Parameter of Kaiser window"));
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (topWidget, widget, LINFIRDLG_ENTRY_KAISER);

    label = gtk_label_new (_("α="));
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    /* Final
     */
    g_signal_connect ((gpointer) comboFtr, "changed",
                      G_CALLBACK (transformTypeChanged),
                      NULL);

    gtk_widget_show_all (linFirDesignDlgMain);

    gtk_combo_box_set_active(GTK_COMBO_BOX (comboFtr), FTR_NON);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (linFirDlgChar[LINFIR_TYPE_RECT].btn), TRUE);
    gtk_toggle_button_set_active (
        GTK_TOGGLE_BUTTON (linFirDlgWin[LINFIR_DSPWIN_RECT].btn), TRUE);

} /* linFirDesignDlgCreate() */


/* FUNCTION *******************************************************************/
/** Linear FIR filter design dialog preset function. Restores all states
 *  of dialog elements from design data of a linear FIR filter.
 *
 *  \param topWidget    Toplevel widget.
 *  \param pDesign      Pointer to linear FIR design data.
 *  \param pFilter      Pointer to filter coefficients (only member \a f0 used
 *                      as input).
 *  \param pPrefs       Pointer to desktop preferences.
 *
 ******************************************************************************/
void linFirDesignDlgPreset (GtkWidget *topWidget, const LINFIR_DESIGN *pDesign,
                            const FLTCOEFF *pFilter, const CFG_DESKTOP* pPrefs)
{
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, LINFIRDLG_UNIT_SAMPLE)),
                        pPrefs->frequUnit.name);                 /* set units */
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, LINFIRDLG_UNIT_CUTOFF)),
                        pPrefs->frequUnit.name);
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, LINFIRDLG_UNIT_CENTER)),
                        pPrefs->frequUnit.name);
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, LINFIRDLG_UNIT_BANDW)),
                        pPrefs->frequUnit.name);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
                                      linFirDlgChar[pDesign->type].btn), TRUE);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
                                      linFirDlgWin[pDesign->dspwin].btn), TRUE);

    if (pDesign->dspwin == LINFIR_DSPWIN_KAISER)
    {                                                         /* Kaiser alpha */
        dlgSetDouble (topWidget, LINFIRDLG_ENTRY_KAISER, 1.0, pDesign->winparm);
    } /* if */

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (lookup_widget (topWidget, LINFIRDLG_SPIN_DEGREE)),
                               pDesign->order);                     /* degree */

    dlgSetDouble (topWidget, LINFIRDLG_ENTRY_SAMPLE,      /* sample frequency */
                  pPrefs->frequUnit.multiplier, pFilter->f0);

    gtk_combo_box_set_active (GTK_COMBO_BOX (lookup_widget (topWidget, LINFIRDLG_COMBO_FTR)),
                              pDesign->ftr.type);  /* transform */

    if (ftrEntry[pDesign->ftr.type][0])
    {
        dlgSetDouble (topWidget, LINFIRDLG_ENTRY_CUTOFF,  /* cutoff frequency */
                      pPrefs->frequUnit.multiplier, pDesign->cutoff);
    } /* if */

    if (ftrEntry[pDesign->ftr.type][1])
    {
        dlgSetDouble (topWidget, LINFIRDLG_ENTRY_CENTER,  /* center frequency */
                      pPrefs->frequUnit.multiplier, pDesign->ftr.fc);
    } /* if */

    if (ftrEntry[pDesign->ftr.type][2])
    {
        dlgSetDouble (topWidget, LINFIRDLG_ENTRY_BANDW,          /* bandwidth */
                      pPrefs->frequUnit.multiplier, pDesign->ftr.bw);
    } /* if */

    if (ftrEntry[pDesign->ftr.type][3])             /* geometric check button */
    {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (
                lookup_widget (topWidget, LINFIRDLG_CHKBTN_GEOMETRIC)),
            pDesign->ftr.flags & FTRDESIGN_FLAG_CENTER_GEOMETRIC);
    } /* if */

} /* linFirDesignDlgPreset() */



/* FUNCTION *******************************************************************/
/** Linear FIR filter design dialog destroy function.
 *
 *  \note If the dialog is not active the function does nothing.
 *
 *  \param topWidget    Toplevel widget.
 *
 ******************************************************************************/
void linFirDesignDlgDestroy (GtkWidget *topWidget)
{
    GtkWidget* widget = lookup_widget (topWidget, LINFIRDLG_WIDGET_MAIN);

    if (widget != NULL)
    {                                                /* remove all references */
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_COMBO_FTR);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_CHKBTN_GEOMETRIC);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_ENTRY_BANDW);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_LABEL_CENTER);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_ENTRY_CENTER);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_ENTRY_CUTOFF);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_ENTRY_SAMPLE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_SPIN_DEGREE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, LINFIRDLG_ENTRY_KAISER);

        GLADE_HOOKUP_OBJECT_NO_REF(topWidget, NULL, LINFIRDLG_WIDGET_MAIN);

        gtk_widget_destroy(widget);
    } /* if */

} /* linFirDesignDlgDestroy() */



/* FUNCTION *******************************************************************/
/** Checks whether the linear FIR filter design dialog is active or not.
 *
 *  \param topWidget    Toplevel widget.
 *
 *  \return             TRUE if the dialog is active (the main-widget of
 *                      linear FIR dialog found), else FALSE.
 ******************************************************************************/
BOOL linFirDesignDlgActive (GtkWidget *topWidget)
{
    GtkWidget* widget = lookup_widget (topWidget, LINFIRDLG_WIDGET_MAIN);

    return (widget != NULL);
} /* linFirDesignDlgActive() */



/* FUNCTION *******************************************************************/
/** Linear FIR filter design dialog ready/apply function.
 *
 *  \param topWidget    Toplevel widget.
 *  \param pPrefs       Pointer to desktop preferences.
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
int linFirDesignDlgApply (GtkWidget *topWidget, const CFG_DESKTOP* pPrefs)
{
    GtkWidget *combo;
    FLTCOEFF filter;

    int err = INT_MAX;
    LINFIR_DESIGN design = {LINFIR_TYPE_SIZE - 1, 0};        /* preset design */

    while ((design.type > 0) &&  /* search active characteristic radio button */
           !gtk_toggle_button_get_active (
               GTK_TOGGLE_BUTTON(linFirDlgChar[design.type].btn)))
    {
        --design.type;
    } /* while */


    design.dspwin = LINFIR_DSPWIN_SIZE - 1;

    while ((design.dspwin > 0) &&    /* search active DSP window radio button */
           !gtk_toggle_button_get_active (
               GTK_TOGGLE_BUTTON(linFirDlgWin[design.dspwin].btn)))
    {
        --design.dspwin;
    } /* while */

    combo = lookup_widget (topWidget, LINFIRDLG_COMBO_FTR);

    if (dlgGetInt (topWidget, LINFIRDLG_SPIN_DEGREE, FLT_DEGREE_MIN,
                   FLT_DEGREE_MAX, &design.order) &&
        dlgGetDouble (topWidget, LINFIRDLG_ENTRY_SAMPLE, FLT_SAMPLE_MIN,
                      FLT_SAMPLE_MAX, pPrefs->frequUnit.multiplier, &filter.f0))
    {
        gint idx = gtk_combo_box_get_active (GTK_COMBO_BOX (combo));

        if (idx < 0)
        {
            design.ftr.type = FTR_NON;  /* set none, in case nothing selected */
        } /* if */
        else
        {
            design.ftr.type = idx;
        } /* else */

        if (ftrEntry[design.ftr.type][4] && GSL_IS_ODD(design.order))
        {
            dlgError(topWidget, _("Degree must be even for frequency transformation."));
            return INT_MAX;
        }

        if (ftrEntry[design.ftr.type][0]) /* lowpass cutoff frequency required? */
        {
            if (!dlgGetDouble (topWidget, LINFIRDLG_ENTRY_CUTOFF,
                               FLT_SAMPLE_MIN / 2, FLT_SAMPLE_MAX / 2,
                               pPrefs->frequUnit.multiplier, &design.cutoff))
            {
                return INT_MAX;
            } /* if */
        } /* if */

        if (ftrEntry[design.ftr.type][1])       /* center frequency required? */
        {
            if (!dlgGetDouble (topWidget, LINFIRDLG_ENTRY_CENTER,
                               FLT_SAMPLE_MIN / 2, FLT_SAMPLE_MAX / 2,
                               pPrefs->frequUnit.multiplier, &design.ftr.fc))
            {
                return INT_MAX;
            } /* if */
        } /* if */

        if (ftrEntry[design.ftr.type][2])              /* bandwidth required? */
        {
            if (!dlgGetDouble (topWidget, LINFIRDLG_ENTRY_BANDW,
                               FLT_SAMPLE_MIN / 2, FLT_SAMPLE_MAX / 2,
                               pPrefs->frequUnit.multiplier, &design.ftr.bw))
            {
                return INT_MAX;
            } /* if */
        } /* if */

        if (ftrEntry[design.ftr.type][3]) /* geometric bandwidth check button? */
        {
            if (gtk_toggle_button_get_active (
                    GTK_TOGGLE_BUTTON (
                        lookup_widget (topWidget, LINFIRDLG_CHKBTN_GEOMETRIC))))
            {
                design.ftr.flags |= FTRDESIGN_FLAG_CENTER_GEOMETRIC;
            } /* if */
        } /* if */

        if (design.dspwin == LINFIR_DSPWIN_KAISER)
        {
            if (!dlgGetDouble (topWidget, LINFIRDLG_ENTRY_KAISER,
                               2.0, 10.0, 1.0, &design.winparm))
            {
                return INT_MAX;
            } /* if */
        } /* if */

        err = linFirFilterGen (&design, &filter);  /* generate the FIR filter */

        if (!FLTERR_CRITICAL (err))
        {
            if (!ftrEntry[design.ftr.type][0])
            {        /* show (derived) lowpass cutoff frequency on HP, BP, BS */
                dlgSetDouble (topWidget, LINFIRDLG_ENTRY_CUTOFF,
                              pPrefs->frequUnit.multiplier, design.cutoff);
            } /* if */

            dfcPrjSetFilter (FLTCLASS_LINFIR, &filter, (DESIGNDLG *)&design);
        } /* if */
    } /* if */

    return err;
} /* linFirDesignDlgApply() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
