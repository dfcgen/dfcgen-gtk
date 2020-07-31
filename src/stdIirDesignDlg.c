/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Standard IIR filter dialog functions.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dfcgen.h"
#include "cfgSettings.h"
#include "projectFile.h"
#include "stdIirFilter.h"
#include "filterSupport.h"
#include "dialogSupport.h"
#include "stdIirDesignDlg.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

/** Standard IIR filter characteristic
 */
typedef struct
{
    STDIIR_TYPE type;            /**< Associated characteristic (type of IIR) */
    GtkWidget* btn;                       /**< Associated radio button widget */
    char* name;               /**< Radio button text (name of characteristic) */
    BOOL hasRipple;     /**< TRUE if ripple attenuation is an input parameter */
    BOOL hasStopband; /**< TRUE if stopband attenuation is an input parameter */
    BOOL hasAngle;                   /**< TRUE if angle is an input parameter */
} STDIIR_DLG_CHAR;


/* LOCAL CONSTANT DEFINITIONS *************************************************/

/** Label for center frequency
 */
#define STDIIRDLG_LABEL_CENTER_TEXT  _("f<sub>Center</sub>")

/** Label for cutoff frequency
 */
#define STDIIRDLG_LABEL_CUTOFF_TEXT  _("f<sub>Cutoff</sub>")


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

#define STDIIRDLG_UNIT_CUTOFF   "unitCutF"
#define STDIIRDLG_UNIT_SAMPLE   "unitSampleF"
#define STDIIRDLG_UNIT_CENTER   "unitCenterF"
#define STDIIRDLG_UNIT_BANDW    "unitBandwidth"


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/** Array of all standard IIR filter types (in same order as the enums).
 */
static STDIIR_DLG_CHAR stdIirDlgChar[STDIIR_TYPE_SIZE] =
{   /* type,                  btn,  name,                  hasRipple, hasStopband, hasAngle */
    {STDIIR_TYPE_BUTTERWORTH, NULL, N_("Butterworth"),     FALSE, FALSE, FALSE},
    {STDIIR_TYPE_CHEBY,       NULL, N_("Chebyshev"),       TRUE, FALSE, FALSE},
    {STDIIR_TYPE_CHEBYINV,    NULL, N_("Chebyshev Inv."),  FALSE, TRUE, FALSE},
    {STDIIR_TYPE_CAUER1,      NULL, N_("Cauer (Case I)"),  TRUE, FALSE, TRUE},
    {STDIIR_TYPE_CAUER2,      NULL, N_("Cauer (Case II)"), FALSE, TRUE, TRUE},
    {STDIIR_TYPE_BESSEL,      NULL, N_("Bessel"),          FALSE, FALSE, FALSE}
}; /* stdIirDlgChar[] */


static const gboolean ftrEntry[FTR_SIZE][5] =
{                  /* entryCutF, entryCenterF, entryBandwidth, checkGeometric, evenDegree */
    [FTR_NON]      = {TRUE,      FALSE,        FALSE,          FALSE,          FALSE},
    [FTR_HIGHPASS] = {FALSE,     TRUE,         FALSE,          FALSE,          FALSE},
    [FTR_BANDPASS] = {FALSE,     TRUE,         TRUE,           TRUE,           TRUE},
    [FTR_BANDSTOP] = {FALSE,     TRUE,         TRUE,           TRUE,           TRUE},
};



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
 *                      that case it points to the associated STDIIR_DLG_CHAR.
 *
 ******************************************************************************/
static void filterTypeChanged (GtkRadioButton* radiobutton, gpointer user_data)
{
    STDIIR_DLG_CHAR *pDesc = (STDIIR_DLG_CHAR*) user_data;

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
        gtk_widget_set_sensitive (widget[i], ftrEntry[index][i]);
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
 *  \param boxDesignDlg GtkVBox widget, which is the container for the dialog.
 *                      The dialog must be mapped to row 1 of it.
 *  \param pPrefs       Pointer to desktop preferences.
 *
 ******************************************************************************/
void stdIirDesignDlgCreate (GtkWidget *topWidget, GtkWidget *boxDesignDlg,
                            const CFG_DESKTOP* pPrefs)
{
    STDIIR_TYPE type;

    GtkWidget *stdIirDesignDlgMain, *comboFtr;
    GtkWidget *widget, *label, *frame, *box, *table;
    GtkObject *spinAdjust;

    GSList *iirTypeRadioGroup = NULL;

    stdIirDesignDlgMain = gtk_table_new (2, 2, FALSE);
    gtk_box_pack_start (GTK_BOX (boxDesignDlg), stdIirDesignDlgMain, TRUE, TRUE, 0);
    gtk_box_reorder_child (GTK_BOX (boxDesignDlg), stdIirDesignDlgMain, 1);
    GLADE_HOOKUP_OBJECT (topWidget, stdIirDesignDlgMain, STDIIRDLG_WIDGET_MAIN);

    /* Characteristic
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frame, 0, 1, 1, 2,
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

    for (type = 0; type < STDIIR_TYPE_SIZE; type++)
    {
        stdIirDlgChar[type].btn = 
            gtk_radio_button_new_with_mnemonic (
                NULL, gettext (stdIirDlgChar[type].name));

        gtk_box_pack_start (GTK_BOX (box), stdIirDlgChar[type].btn, FALSE, FALSE, 0);
        gtk_container_set_border_width (GTK_CONTAINER (stdIirDlgChar[type].btn), 1);
        gtk_radio_button_set_group (GTK_RADIO_BUTTON (stdIirDlgChar[type].btn), iirTypeRadioGroup);
        iirTypeRadioGroup = gtk_radio_button_get_group (GTK_RADIO_BUTTON (stdIirDlgChar[type].btn));

        g_signal_connect ((gpointer) stdIirDlgChar[type].btn, "toggled",
                          G_CALLBACK (filterTypeChanged), &stdIirDlgChar[type]);
    } /* for */


    /* Frequency transformation
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frame, 1, 2, 0, 1,
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

    GLADE_HOOKUP_OBJECT (topWidget, comboFtr, STDIIRDLG_COMBO_FTR);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), comboFtr);

    widget = gtk_entry_new ();                                   /* bandwidth */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Bandwidth"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_ENTRY_BANDW);

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
    GLADE_HOOKUP_OBJECT (topWidget, label, STDIIRDLG_UNIT_BANDW);

    widget = gtk_entry_new ();                            /* center frequency */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Cutoff frequency (highpass) or center"
                                           " frequency (bandpass, bandstop)"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_ENTRY_CENTER);

    label = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), STDIIRDLG_LABEL_CENTER_TEXT);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
    GLADE_HOOKUP_OBJECT (topWidget, label, STDIIRDLG_LABEL_CENTER);

    label = gtk_label_new (pPrefs->frequUnit.name);  /* center frequency unit */
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    GLADE_HOOKUP_OBJECT (topWidget, label, STDIIRDLG_UNIT_CENTER);

    widget = gtk_check_button_new_with_mnemonic (_("_Geometric"));
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 3, 4,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget,
                                 _("Check this if the center frequency shall be the"
                                   " geometric mean between both cutoff frequencies"
                                   " (otherwise it is the arithmetic mean)."));
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_CHKBTN_GEOMETRIC);

    /* Reference Lowpass
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frame, 0, 1, 0, 1,
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
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_ENTRY_CUTOFF);

    label = gtk_label_new (NULL);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL(label), STDIIRDLG_LABEL_CUTOFF_TEXT);
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
    GLADE_HOOKUP_OBJECT (topWidget, label, STDIIRDLG_UNIT_CUTOFF);

    widget = gtk_entry_new ();                            /* sample frequency */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Sample frequency"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_ENTRY_SAMPLE);

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
    GLADE_HOOKUP_OBJECT (topWidget, label, STDIIRDLG_UNIT_SAMPLE);


    /* degree spin-button */
    spinAdjust = gtk_adjustment_new (1, FLT_DEGREE_MIN, FLT_DEGREE_MAX, 1, 10, 0);
    widget = gtk_spin_button_new (GTK_ADJUSTMENT (spinAdjust), 1, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Degree of filter"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE);
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_SPIN_DEGREE);

    label = gtk_label_new_with_mnemonic (_("_Degree"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    /* Parameters
     */
    frame = gtk_frame_new (NULL);
    gtk_table_attach (GTK_TABLE (stdIirDesignDlgMain), frame, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                      (GtkAttachOptions) (GTK_FILL), 0, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);

    label = gtk_label_new (_("<b>Parameters</b>"));
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

    label = gtk_label_new ("dB");
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    label = gtk_label_new ("dB");
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    label = gtk_label_new ("°");       /* angle unit (degree, coded as UTF-8) */
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    widget = gtk_entry_new ();                 /* passband ripple attenuation */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Passband ripple in dB"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    gtk_widget_set_sensitive (widget, FALSE);
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_ENTRY_RIPPLE);

    label = gtk_label_new_with_mnemonic (_("Ripple"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    widget = gtk_entry_new ();                /* minimum stopband attenuation */
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Stopband attenuation in dB"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    gtk_widget_set_sensitive (widget, FALSE);
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_ENTRY_MINATT);

    label = gtk_label_new_with_mnemonic (_("Stop"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    spinAdjust = gtk_adjustment_new (45, 1, 89, 1, 10, 0); /* elliptic angle */
    widget = gtk_spin_button_new (GTK_ADJUSTMENT (spinAdjust), 1, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (table), widget, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Modular angle in degree"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE);
    gtk_widget_set_sensitive (widget, FALSE);
    GLADE_HOOKUP_OBJECT (topWidget, widget, STDIIRDLG_SPIN_ANGLE);

    label = gtk_label_new_with_mnemonic (_("Angle"));
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);

    g_signal_connect ((gpointer) comboFtr, "changed",
                      G_CALLBACK (transformTypeChanged),
                      NULL);

    gtk_widget_show_all (stdIirDesignDlgMain);

    gtk_combo_box_set_active(GTK_COMBO_BOX (comboFtr), FTR_NON);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(stdIirDlgChar[STDIIR_TYPE_BUTTERWORTH].btn), TRUE);

} /* stdIirDesignDlgCreate() */


/* FUNCTION *******************************************************************/
/** Standard IIR filter design dialog preset function. Restores all states
 *  of dialog elements from design data of a standard IIR filter.
 *
 *  \param topWidget    Toplevel widget.
 *  \param pDesign      Pointer to standard IIR design data.
 *  \param pFilter      Pointer to filter coefficients (only member \a f0 used).
 *  \param pPrefs       Pointer to desktop preferences.
 *
 ******************************************************************************/
void stdIirDesignDlgPreset (GtkWidget *topWidget, const STDIIR_DESIGN *pDesign,
                            const FLTCOEFF *pFilter, const CFG_DESKTOP* pPrefs)
{
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, STDIIRDLG_UNIT_SAMPLE)),
                        pPrefs->frequUnit.name);                 /* set units */
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, STDIIRDLG_UNIT_CUTOFF)),
                        pPrefs->frequUnit.name);
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, STDIIRDLG_UNIT_CENTER)),
                        pPrefs->frequUnit.name);
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, STDIIRDLG_UNIT_BANDW)),
                        pPrefs->frequUnit.name);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (
                                      stdIirDlgChar[pDesign->type].btn), TRUE);

    if (stdIirDlgChar[pDesign->type].hasRipple)            /* passband ripple */
    {
        dlgSetDouble (topWidget, STDIIRDLG_ENTRY_RIPPLE, 1.0, pDesign->ripple);
    } /* if */

    if (stdIirDlgChar[pDesign->type].hasStopband)     /* stopband attenuation */
    {
        dlgSetDouble (topWidget, STDIIRDLG_ENTRY_MINATT, 1.0, pDesign->minatt);
    } /* if */

    if (stdIirDlgChar[pDesign->type].hasAngle)    /* modular angle (elliptic) */
    {
        dlgSetDouble (topWidget, STDIIRDLG_SPIN_ANGLE, 1.0, pDesign->angle);
    } /* if */

    gtk_spin_button_set_value (GTK_SPIN_BUTTON (lookup_widget (topWidget, STDIIRDLG_SPIN_DEGREE)),
                               pDesign->order);                     /* degree */

    dlgSetDouble (topWidget, STDIIRDLG_ENTRY_SAMPLE,      /* sample frequency */
                  pPrefs->frequUnit.multiplier, pFilter->f0);

    gtk_combo_box_set_active(GTK_COMBO_BOX (lookup_widget (topWidget, STDIIRDLG_COMBO_FTR)),
                                            pDesign->ftr.type);  /* transform */

    if (ftrEntry[pDesign->ftr.type][0])
    {
        dlgSetDouble (topWidget, STDIIRDLG_ENTRY_CUTOFF,  /* cutoff frequency */
                      pPrefs->frequUnit.multiplier, pDesign->cutoff);
    } /* if */

    if (ftrEntry[pDesign->ftr.type][1])
    {
        dlgSetDouble (topWidget, STDIIRDLG_ENTRY_CENTER,  /* center frequency */
                      pPrefs->frequUnit.multiplier, pDesign->ftr.fc);
    } /* if */

    if (ftrEntry[pDesign->ftr.type][2])
    {
        dlgSetDouble (topWidget, STDIIRDLG_ENTRY_BANDW,          /* bandwidth */
                      pPrefs->frequUnit.multiplier, pDesign->ftr.bw);
    } /* if */

    if (ftrEntry[pDesign->ftr.type][3])             /* geometric check button */
    {
        gtk_toggle_button_set_active (
            GTK_TOGGLE_BUTTON (
                lookup_widget (topWidget, STDIIRDLG_CHKBTN_GEOMETRIC)),
            pDesign->ftr.flags & FTRDESIGN_FLAG_CENTER_GEOMETRIC);
    } /* if */

} /* stdIirDesignDlgPreset() */



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
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_CHKBTN_GEOMETRIC);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_ENTRY_BANDW);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, STDIIRDLG_LABEL_CENTER);
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
int stdIirDesignDlgApply (GtkWidget *topWidget, const CFG_DESKTOP* pPrefs)
{
    GtkWidget *combo;
    FLTCOEFF filter;

    int err = INT_MAX;
    STDIIR_DESIGN design = {STDIIR_TYPE_SIZE - 1, 0};        /* preset design */

    while ((design.type > 0) &&               /* look for active radio button */
           !gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(stdIirDlgChar[design.type].btn)))
    {
        --design.type;
    } /* while */


    combo = lookup_widget (topWidget, STDIIRDLG_COMBO_FTR);

    if (dlgGetInt (topWidget, STDIIRDLG_SPIN_DEGREE, FLT_DEGREE_MIN,
                   FLT_DEGREE_MAX, &design.order) &&
        dlgGetDouble (topWidget, STDIIRDLG_ENTRY_SAMPLE, FLT_SAMPLE_MIN,
                      FLT_SAMPLE_MAX, pPrefs->frequUnit.multiplier, &filter.f0))
    {
        gint idx;

        if (stdIirDlgChar[design.type].hasRipple)
        {                                        /* get passband ripple in dB */
            if (!dlgGetDouble (topWidget, STDIIRDLG_ENTRY_RIPPLE,
                               STDIIR_RIPPLE_MIN, STDIIR_RIPPLE_MAX, 1.0,
                               &design.ripple))
            {
                return INT_MAX;
            } /* if */
        } /* if */


        if (stdIirDlgChar[design.type].hasStopband)
        {                                   /* get stopband attenuation in dB */
            if (!dlgGetDouble (topWidget, STDIIRDLG_ENTRY_MINATT,
                               STDIIR_STOPATT_MIN, STDIIR_STOPATT_MAX, 1.0,
                               &design.minatt))
            {
                return INT_MAX;
            } /* if */
        } /* if */

        if (stdIirDlgChar[design.type].hasAngle)
        {                                                /* get modular angle */
            if (!dlgGetDouble (topWidget, STDIIRDLG_SPIN_ANGLE,
                               STDIIR_ANGLE_MIN, STDIIR_ANGLE_MAX, 1.0,
                               &design.angle))
            {
                return INT_MAX;
            } /* if */
        } /* if */


        idx = gtk_combo_box_get_active(GTK_COMBO_BOX (combo));

        if (idx < 0)
        {
            design.ftr.type = FTR_NON; /* set none, in case nothing selected */
        } /* if */
        else
        {
            design.ftr.type = idx;
        } /* else */

        if (ftrEntry[design.ftr.type][4] && GSL_IS_ODD(design.order))
        {
            dlgError(topWidget, _("Degree must be even for bandpass/bandstop."));
            return INT_MAX;
        }

        if (ftrEntry[design.ftr.type][0]) /* lowpass cutoff frequency required? */
        {
            if (!dlgGetDouble (topWidget, STDIIRDLG_ENTRY_CUTOFF,
                               FLT_SAMPLE_MIN / 2, FLT_SAMPLE_MAX / 2,
                               pPrefs->frequUnit.multiplier, &design.cutoff))
            {
                return INT_MAX;
            } /* if */
        } /* if */

        if (ftrEntry[design.ftr.type][1])       /* center frequency required? */
        {
            if (!dlgGetDouble (topWidget, STDIIRDLG_ENTRY_CENTER,
                               FLT_SAMPLE_MIN / 2, FLT_SAMPLE_MAX / 2,
                               pPrefs->frequUnit.multiplier, &design.ftr.fc))
            {
                return INT_MAX;
            } /* if */
        } /* if */

        if (ftrEntry[design.ftr.type][2])              /* bandwidth required? */
        {
            if (!dlgGetDouble (topWidget, STDIIRDLG_ENTRY_BANDW,
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
                        lookup_widget (topWidget, STDIIRDLG_CHKBTN_GEOMETRIC))))
            {
                design.ftr.flags |= FTRDESIGN_FLAG_CENTER_GEOMETRIC;
            } /* if */
        } /* if */

        design.zAlgo = ZTR_BILINEAR;                                   /* fix */
        err = stdIirFilterGen (&design, &filter);  /* generate the IIR filter */

        if (!FLTERR_CRITICAL (err))
        {
            if (!ftrEntry[design.ftr.type][0])
            {        /* show (derived) lowpass cutoff frequency on HP, BP, BS */
                dlgSetDouble (topWidget, STDIIRDLG_ENTRY_CUTOFF,
                              pPrefs->frequUnit.multiplier, design.cutoff);
            } /* if */

            dfcPrjSetFilter (FLTCLASS_STDIIR, &filter, (DESIGNDLG *)&design);
        } /* if */
    } /* if */

    return err;
} /* stdIirDesignDlgApply() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
