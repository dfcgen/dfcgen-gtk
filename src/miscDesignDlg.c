/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     miscDesignDlg.c
 * \brief    Miscellaneous FIR/IIR design dialogs.
 *
 * \note     Includes raw filters (filters without a design).
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dfcgen.h"
#include "projectFile.h"
#include "filterSupport.h"
#include "dialogSupport.h"
#include "miscFilter.h"
#include "miscDesignDlg.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

typedef struct
{
    int degree;                                         /**< (Default) degree */
    char *fname;                    /**< raw filter filename in GLib encoding */
    char *title;                                  /**< Title (name) of filter */
    char *desc;                         /**< Filter description (may be NULL) */
} MISCDLG_FILTER_DESC;


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define MISCDLG_WIDGET_MAIN     "miscDesignDlgMain"
#define MISCDLG_ENTRY_SAMPLE    "entrySampleF"    /**< Sample frequency entry */
#define MISCDLG_SPIN_DEGREE     "spinDegree" /**< Degree of filter entry/spin */
#define MISCDLG_COMBO_TYPE      "comboType"      /**< Type of filter combobox */
#define MISCDLG_LABEL_DESC      "labelDesc"        /**< Description of filter */
#define MISCDLG_EXPANDER_DESC   "expanderDesc"           /**< Expander widget */
#define MISCDLG_UNIT_SAMPLE     "unitSampleF" /**< Sample frequency unit label */


/* LOCAL VARIABLE DEFINITIONS *************************************************/

static MISCDLG_FILTER_DESC miscFilterList[MISCFLT_SIZE] =
{
    {                             /* MISCFLT_HILBERT (0): Hilbert transformer */
        1, NULL, N_("Hilbert transformer (FIR)"),
        N_("A <i>Hilbert</i> transformer is a 90° phase shifter with the impulse response g(t)=<sup>1</sup>/<sub>ϖt</sub>. The approximation is based on <i>Fourier</i> series expansion of the repetitive frequency response:\nH(f)=-j sgn(f).")
    },
    {                                          /* MISCFLT_INT (1): Integrator */
        1, NULL, N_("Perfect Integrator (FIR)"),
        N_("A perfect integrator has the <i>Heaviside</i> unit step function as it&apos;s impulse response. The approximation is based on <i>Fourier</i> series expansion of the repetitive frequency response:\nH(f)=½δ(f)+<sup>1</sup>/<sub>j2ϖf</sub>.")
    },
    {                                     /* MISCFLT_DIFF (2): Differentiator */
        1, NULL, N_("Perfect Differentiator (FIR)"),
        N_("A perfect differentiator has the <i>Dirac</i> impulse as it&apos;s impulse response.  The approximation is based on <i>Fourier</i> series expansion of the repetitive frequency response:\nH(f)=j2ϖf.")
    },
    {                                        /* MISCFLT_COMB (3): Comb filter */
        1, NULL, N_("Comb filter (FIR)"),
        N_("The comb filter is a computational physical model of a single discrete echo. It has the transfer function:\nH(z)=1-z<sup>-n</sup>\nand thus a linear phase.")
    },
    {            /* MISCFLT_AVGFIR (4): Moving average filter (non-recursive) */
        1, NULL, N_("Moving average (FIR)"),
        N_("The moving average filter (also called digital window integrator or sinc filter) is an approximation of the impulse response of the ideal lowpass. As a FIR implementation it&apos;s transfer function is:\nH(z)=1+z<sup>-1</sup>+z<sup>-2</sup>+...+z<sup>-n</sup>.")
    },
    {                /* MISCFLT_AVGIIR (5): Moving average filter (recursive) */
        1, NULL, N_("Moving average (IIR)"),
        N_("The moving average filter (also called digital window integrator or sinc filter) is an approximation of the impulse response of the ideal lowpass. As an IIR implementation it&apos;s transfer function is:\nH(z)=(1-z<sup>-n</sup>)/(1-z<sup>-1</sup>).")
    },
    {                       /* MISCFLT_AVGEXP (6): Exponential average filter */
        1, NULL, N_("Exponential average (IIR)"),
        N_("The exponential average lowpass approximates a first order (analog RC-) lowpass. The transfer function is:\nH(z)=1/[n-(n-1)z<sup>-1</sup>].")
    },
};


static GArray* rawFilterList = NULL;


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static gboolean miscDlgComboSeparator(GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static void miscDlgOnTypeComboChanged (GtkComboBox *combo, gpointer user_data);
static GtkWidget* createDialog (GtkWidget *topWidget, GtkWidget *boxDesignDlg,
                                const CFG_DESKTOP* pPrefs);
static void updateLayout (GtkWidget *topWidget, GtkWidget *combo, int index);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Realizes misc filter dialog layout.
 *
 *  \param topWidget    Toplevel widget.
 *  \param boxDesignDlg The box widget to be used for the filter dialog.
 *  \param pPrefs       Pointer to desktop preferences.
 *
 *  \return             The filter type combobox widget.
 ******************************************************************************/
static GtkWidget* createDialog (GtkWidget *topWidget, GtkWidget *boxDesignDlg,
                                const CFG_DESKTOP* pPrefs)
{
    GtkWidget *miscDesignDlgMain, *miscDesignDlgTable;
    GtkWidget *label, *expander, *combo, *widget;
    GtkObject *spinAdjustment;

    miscDesignDlgMain = gtk_frame_new (NULL);             /* create the frame */
    gtk_box_pack_start (GTK_BOX (boxDesignDlg), miscDesignDlgMain, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (miscDesignDlgMain), 6);
    gtk_frame_set_shadow_type (GTK_FRAME (miscDesignDlgMain), GTK_SHADOW_NONE);
    gtk_box_reorder_child (GTK_BOX (boxDesignDlg), miscDesignDlgMain, 1);
    GLADE_HOOKUP_OBJECT (topWidget, miscDesignDlgMain, MISCDLG_WIDGET_MAIN);

    widget = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (miscDesignDlgMain), widget);
    gtk_alignment_set_padding (GTK_ALIGNMENT (widget), 0, 0, 12, 0); /* indent childs */

    miscDesignDlgTable = gtk_table_new (4, 3, FALSE);
    gtk_container_add (GTK_CONTAINER (widget), miscDesignDlgTable);
    gtk_container_set_border_width (GTK_CONTAINER (miscDesignDlgTable), 6);
    gtk_table_set_row_spacings (GTK_TABLE (miscDesignDlgTable), 6);
    gtk_table_set_col_spacings (GTK_TABLE (miscDesignDlgTable), 6);

    combo = gtk_combo_box_new_text ();             /* type of filter combobox */
    gtk_table_attach (GTK_TABLE (miscDesignDlgTable), combo, 0, 3, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 6);
    g_signal_connect_after ((gpointer) combo, "changed",
                            G_CALLBACK (miscDlgOnTypeComboChanged),
                            NULL);
    GLADE_HOOKUP_OBJECT (topWidget, combo, MISCDLG_COMBO_TYPE);

    label = gtk_label_new_with_mnemonic (_("<b>_Type</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (miscDesignDlgMain), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);

    label = gtk_label_new_with_mnemonic (_("f<sub>_Sample</sub>"));
    gtk_table_attach (GTK_TABLE (miscDesignDlgTable), label, 0, 1, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 3, 0);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    widget = gtk_entry_new ();
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (miscDesignDlgTable), widget, 1, 2, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Sample frequency"));
    gtk_entry_set_width_chars (GTK_ENTRY (widget), GUI_ENTRY_WIDTH_CHARS);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
    GLADE_HOOKUP_OBJECT (topWidget, widget, MISCDLG_ENTRY_SAMPLE);

    label = gtk_label_new (pPrefs->frequUnit.name);  /* sample frequency unit */
    gtk_table_attach (GTK_TABLE (miscDesignDlgTable), label, 2, 3, 2, 3,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    GLADE_HOOKUP_OBJECT (topWidget, label, MISCDLG_UNIT_SAMPLE);

    label = gtk_label_new_with_mnemonic (_("_Degree"));
    gtk_table_attach (GTK_TABLE (miscDesignDlgTable), label, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    spinAdjustment = gtk_adjustment_new (1, FLT_DEGREE_MIN, FLT_DEGREE_MAX, 1, 10, 0);
    widget = gtk_spin_button_new (GTK_ADJUSTMENT (spinAdjustment), 1, 0);
    gtk_entry_set_activates_default (GTK_ENTRY (widget), TRUE);
    gtk_table_attach (GTK_TABLE (miscDesignDlgTable), widget, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 0);
    gtk_widget_set_tooltip_text (widget, _("Degree of system"));
    gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (widget), TRUE);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
    GLADE_HOOKUP_OBJECT (topWidget, widget, MISCDLG_SPIN_DEGREE);

    expander = gtk_expander_new (NULL);
    gtk_table_attach (GTK_TABLE (miscDesignDlgTable), expander, 0, 3, 3, 4,
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (GTK_SHRINK | GTK_FILL), 0, 12);
    gtk_expander_set_expanded (GTK_EXPANDER (expander), TRUE);
    gtk_expander_set_spacing (GTK_EXPANDER (expander), 12);
    GLADE_HOOKUP_OBJECT (topWidget, expander, MISCDLG_EXPANDER_DESC);

    label = gtk_label_new ("");                      /* filter description text */
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_selectable (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_container_add (GTK_CONTAINER (expander), label);
    GLADE_HOOKUP_OBJECT (topWidget, label, MISCDLG_LABEL_DESC);

    label = gtk_label_new (_("<i>Description</i>"));
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_expander_set_label_widget (GTK_EXPANDER (expander), label);

    gtk_widget_show_all (miscDesignDlgMain);

    return combo;
} /* createDialog() */


/* FUNCTION *******************************************************************/
/** Updates the design layout.
 *
 *  \param topWidget    Top level widget.
 *  \param combo        Combobox widget (filter type).
 *  \param index        Filter associated combobox index. Notice that index
 *                      MISCFLT_SIZE is reserved for the separator.
 *
 ******************************************************************************/
static void updateLayout (GtkWidget *topWidget, GtkWidget *combo, int index)
{
    const char *desc;
    GtkWidget *widget;
    MISCDLG_FILTER_DESC raw;

    ASSERT (topWidget != NULL);
    ASSERT (combo != NULL);

    if (index < 0)                                       /* nothing selected? */
    {
        index = 0;                               /* set default startup value */
    } /* if */

    widget = lookup_widget (topWidget, MISCDLG_SPIN_DEGREE);

    if (index < MISCFLT_SIZE)                    /* misc filters (hard coded) */
    {
        desc = miscFilterList[index].desc;
        gtk_widget_set_sensitive (widget, TRUE);
    } /* if */
    else                                           /* raw/predefined filters? */
    {
        raw = g_array_index (rawFilterList, MISCDLG_FILTER_DESC,
                             index - MISCFLT_SIZE - 1);
        desc = raw.desc;                                       /* may be NULL */
        gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), raw.degree);
        gtk_widget_set_sensitive (widget, FALSE);
    } /* else */

    widget = lookup_widget (topWidget, MISCDLG_EXPANDER_DESC);

    if ((desc != NULL) && (g_utf8_strlen (desc, -1) > 0)) /* show description if available */
    {
        gtk_label_set_markup (GTK_LABEL (lookup_widget (topWidget, MISCDLG_LABEL_DESC)),
                              gettext (desc));
        gtk_widget_show_all (widget);
    } /* if */
    else
    {
        gtk_widget_hide_all (widget);
    } /* else */

} /* updateLayout() */



/* FUNCTION *******************************************************************/
/** This function is called if the filter type changes.
 *
 *  \param combo        Combobox widget (filter type).
 *  \param user_data    User data of \e changed event (unused).
 *
 ******************************************************************************/
static void miscDlgOnTypeComboChanged (GtkComboBox *combo, gpointer user_data)
{
    updateLayout (gtk_widget_get_toplevel (GTK_WIDGET (combo)),
                  GTK_WIDGET (combo),
                  gtk_combo_box_get_active(combo));
} /* miscDlgOnTypeComboChanged */



/* FUNCTION *******************************************************************/
/** This function is called from the GtkComboBox for each path (list element
 *  inside).
 *
 *  \param model        Model of combobox widget (filter type).
 *  \param iter         The iterator associated with the current element/row.
 *  \param data         (Unused) data pointer passed to function
 *                      gtk_combo_box_set_row_separator_func().
 *
 *  \return             TRUE if an separator should be drawn after the current
 *                      element.
 *
 ******************************************************************************/
static gboolean miscDlgComboSeparator(GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
    GtkTreePath* path = gtk_tree_model_get_path (model, iter);
    gint* pIndex = gtk_tree_path_get_indices (path);      /* get path of iter */

    if (pIndex == NULL)
    {
        return FALSE;
    } /* if */

    return (*pIndex == MISCFLT_SIZE);       /* depth is 1, take first integer */
} /* miscDlgComboSeparator() */




/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Misc filter design dialog creation function.
 *
 *  \note There is no design for raw filters (except the cutoff frequency).
 *
 *  \param topWidget    Toplevel widget.
 *  \param boxDesignDlg The box widget to be used for the filter dialog.
 *  \param pPrefs       Pointer to desktop preferences.
 *
 ******************************************************************************/
void miscDesignDlgCreate (GtkWidget *topWidget, GtkWidget *boxDesignDlg,
                          const CFG_DESKTOP* pPrefs)
{
    const gchar* filename;
    gchar* path;                            /* path to filters sub-directory */
    DFCPRJ_FILTER prj;
    GDir* dir;
    GtkRequisition size;                      /* requisition size of combobox */
    MISCDLG_FILTER_DESC raw;
    int count;

    GError *err = NULL;

    /* 1st step: create dialog layout
     */
    GtkComboBox* combo = GTK_COMBO_BOX (createDialog (topWidget, boxDesignDlg, pPrefs));


    /* 2nd step: insert misc filters
     */
    for (count = 0; count < MISCFLT_SIZE; count++)
    {
        gtk_combo_box_append_text (combo, gettext (miscFilterList[count].title));
    } /* for */


    /* 3rd step: insert all predefined (raw) filters found in a sub-directory,
     *           identified by DIR_ID_FILTERS
     */
    rawFilterList = g_array_new (FALSE, TRUE, sizeof (MISCDLG_FILTER_DESC));
    path = getPackageDirectory (DIR_ID_FILTERS);
    dir = g_dir_open (path, 0, &err);

    if (err == NULL)                                             /* no error? */
    {
        gtk_combo_box_append_text (combo, "");  /* insert space for separator */

        do         /* read all predefined filters from datadir (PREFIX/share) */
        {
            filename = g_dir_read_name (dir);   /* get next file in directory */

            if (filename != NULL)                                     /* end? */
            {
                if (g_str_has_suffix (filename, PRJFILE_NAME_SUFFIX))
                {
                    raw.fname = g_build_filename (path, filename, NULL);
                    prjFileRead (raw.fname, &prj, &err);

                    if ((err == NULL) &&               /* valid project file? */
                        (prj.fltcls == FLTCLASS_MISC) &&       /* raw filter? */
                        (prj.design.miscFlt.type == MISCFLT_UNKNOWN))
                    {
                        DEBUG_LOG("Predefined filter read from '%s'", filename);

                        if (prj.info.title != NULL)
                        {
                            raw.degree = GSL_MAX (prj.filter.den.degree,
                                                  prj.filter.num.degree);
                            raw.title = g_strdup (prj.info.title);
                            raw.desc = g_strdup (prj.info.desc); /* may be NULL */

                            g_array_append_val(rawFilterList, raw);
                            gtk_combo_box_append_text (combo, prj.info.title);
                            ++count;
                        } /* if */
                        else
                        {
                            DEBUG_LOG("Predefined filter has no title (ignored)");
                        } /* else */

                        dfcPrjFree (&prj);
                    } /* if */
                    else                        /* seems to be any other file */
                    {
                        DEBUG_LOG("'%s' seems not to be a predefined filter.", filename);

                        if (err != NULL)
                        {
                            g_error_free (err);
                            err = NULL;
                        } /* if */
                    } /* else */
                } /* if */
                else
                {
                    DEBUG_LOG("'%s' is not a " PACKAGE " project file", filename);
                } /* else */
            } /* if */
        } while (filename != NULL);

        g_dir_close (dir);
    } /* if */
    else                                               /* directory not found */
    {
        g_error_free (err);

        dlgError (topWidget,
                  _("Could not load predefined filter(s) from '%s'"),
                  path);
    } /* else */


    g_free (path);                     /* free the sub-directory path string */
    gtk_combo_box_set_row_separator_func (combo, miscDlgComboSeparator, NULL, NULL);


    /* To use the wrap feature of the description label the width of lable
     * widget must must be limited. A good reference width (used here) ist the
     * combobox.
     */
    gtk_widget_size_request (GTK_WIDGET (combo), &size);
    gtk_widget_set_size_request (lookup_widget (topWidget, MISCDLG_LABEL_DESC),
                                 size.width, -1);

    gtk_combo_box_set_active (combo, 0);

} /* miscDesignDlgCreate() */


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
void miscDesignDlgPreset (GtkWidget *topWidget, const MISCFLT_DESIGN *pDesign,
                          const FLTCOEFF *pFilter, const CFG_DESKTOP* pPrefs)
{
    GtkWidget *widget = lookup_widget (topWidget, MISCDLG_SPIN_DEGREE);
    gtk_label_set_text (GTK_LABEL (lookup_widget (topWidget, MISCDLG_UNIT_SAMPLE)),
                        pPrefs->frequUnit.name);                  /* set unit */

    dlgSetDouble (topWidget, MISCDLG_ENTRY_SAMPLE,        /* sample frequency */
                  pPrefs->frequUnit.multiplier, pFilter->f0);

    if ((pDesign->type >= 0) && (pDesign->type < MISCFLT_SIZE)) /* known design? */
    {                                                          /* show degree */
        gtk_combo_box_set_active(
            GTK_COMBO_BOX (lookup_widget (topWidget, MISCDLG_COMBO_TYPE)),
            pDesign->type);

        gtk_spin_button_set_value (GTK_SPIN_BUTTON (widget), pDesign->order);
        gtk_widget_set_sensitive (widget, TRUE);
    } /* if */
    else
    {
        gtk_spin_button_set_value (
            GTK_SPIN_BUTTON (widget),
            GSL_MAX (pFilter->den.degree, pFilter->num.degree));

        widget = gtk_message_dialog_new_with_markup (
            GTK_WINDOW (topWidget), GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_CLOSE,
            _("For this digital system only a limited set of design data is"
              " available. Thus the <i>Type</i> box and <i>Description</i>"
              " field will not reflect the original design (but still the"
              " coefficients)."));

        gtk_dialog_run (GTK_DIALOG (widget));
        gtk_widget_destroy (widget);
    } /* else */

} /* miscDesignDlgPreset() */


/* FUNCTION *******************************************************************/
/** Misc filter design dialog destroy function.
 *
 *  \note If the dialog is not active the function does nothing.
 *
 *  \param topWidget    Toplevel widget.
 *
 ******************************************************************************/
void miscDesignDlgDestroy (GtkWidget *topWidget)
{
    int i;
    MISCDLG_FILTER_DESC raw;

    GtkWidget* widget = lookup_widget (topWidget, MISCDLG_WIDGET_MAIN);

    if (widget != NULL)
    {
        for (i = 0; i < rawFilterList->len; i++)
        {
            raw = g_array_index (rawFilterList, MISCDLG_FILTER_DESC, i);
            g_free (raw.fname);
            g_free (raw.title);                            /* never can be NULL */

            if (raw.desc != NULL)
            {
                g_free (raw.desc);
            } /* if */
        } /* for */

        g_array_free (rawFilterList, TRUE);

        /* remove all references
         */
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, MISCDLG_WIDGET_MAIN);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, MISCDLG_ENTRY_SAMPLE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, MISCDLG_COMBO_TYPE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, MISCDLG_SPIN_DEGREE);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, MISCDLG_EXPANDER_DESC);
        GLADE_HOOKUP_OBJECT_NO_REF (topWidget, NULL, MISCDLG_LABEL_DESC);
        gtk_widget_destroy(widget);
    } /* if */

} /* miscDesignDlgDestroy() */



/* FUNCTION *******************************************************************/
/** Checks whether the misc filter design dialog is active or not.
 *
 *  \param topWidget    Toplevel widget.
 *
 *  \return             TRUE if the dialog is active (the main-widget of
 *                      raw dialog found), else FALSE.
 ******************************************************************************/
BOOL miscDesignDlgActive (GtkWidget *topWidget)
{
    GtkWidget* widget = lookup_widget (topWidget, MISCDLG_WIDGET_MAIN);

    return (widget != NULL);
} /* miscDesignDlgActive() */


/* FUNCTION *******************************************************************/
/** Miscellaneous FIR/IIR filter design dialog \e Apply function.
 *
 *  \param topWidget    Toplevel widget.
 *  \param pPrefs       Pointer to desktop preferences.
 *
 *  \return             - 0 (or GSL_SUCCESS) if okay and no coefficients are
 *                        dropped.
 *                      - a negative number (typically GSL_CONTINUE) if a
 *                        coefficient or the degree has changed, but the filter
 *                        is valid. You can use the FLTERR_WARNING macro from
 *                        filterSupport.h to check this condition.
 *                      - a positive error number (typically from from errno.h
 *                        or gsl_errno.h) that something is wrong and the
 *                        filter can't be created. You can use the
 *                        FLTERR_CRITICAL macro from filterSupport.h to check
 *                        this condition. If the value is INT_MAX then an
 *                        error message box was displayed by function
 *                        miscDesignDlgApply(), means the caller should not
 *                        popup a (second) message box.
 ******************************************************************************/
int miscDesignDlgApply (GtkWidget *topWidget, const CFG_DESKTOP* pPrefs)
{
    int index;
    DFCPRJ_FILTER prj;                                      /* filter project */

    int err = INT_MAX;
    GtkComboBox *combo = GTK_COMBO_BOX (lookup_widget (topWidget, MISCDLG_COMBO_TYPE));

    if (combo != NULL)                        /* normally combox should exist */
    {
        index = gtk_combo_box_get_active(combo);

        if (index >= 0)                  /* anything selected (sanity check)? */
        {
            if (dlgGetDouble (topWidget, MISCDLG_ENTRY_SAMPLE, FLT_SAMPLE_MIN,
                              FLT_SAMPLE_MAX, pPrefs->frequUnit.multiplier,
                              &prj.filter.f0))
            {
                if (index < MISCFLT_SIZE)       /* misc filter (with design)? */
                {
                    if (dlgGetInt (topWidget, MISCDLG_SPIN_DEGREE, FLT_DEGREE_MIN,
                                   FLT_DEGREE_MAX, &prj.design.miscFlt.order))
                    {
                        prj.design.miscFlt.type = index;
                        err = miscFilterGen (&prj.design.miscFlt, &prj.filter);

                        if (!FLTERR_CRITICAL (err))
                        {
                            dfcPrjSetFilter (FLTCLASS_MISC, &prj.filter, &prj.design);
                        } /* if */
                    } /* if */
                } /* if */
                else                              /* predefined filters (raw) */
                {
                    GError *gerr = NULL;
                    double sample = prj.filter.f0;
                    MISCDLG_FILTER_DESC raw = g_array_index (rawFilterList,
                                                             MISCDLG_FILTER_DESC,
                                                             index - MISCFLT_SIZE - 1);
                    prjFileRead (raw.fname, &prj, &gerr);

                    if (gerr == NULL)
                    {
                        prj.filter.f0 = sample;         /* restore user input */
                        dfcPrjSetFilter (FLTCLASS_MISC, &prj.filter, &prj.design);
                        prjFileFree (&prj.info);      /* only free author,... */
                        err = 0;
                    } /* if */
                    else
                    {
                        dlgErrorFile (topWidget,
                                      _("Could not load predefined filter from '%s'"),
                                      raw.fname, gerr);
                        g_error_free (gerr);
                    } /* else */
                } /* else */
            } /* if */
        } /* if */
    } /* if */

    return err;
} /* miscDesignDlgApply() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
