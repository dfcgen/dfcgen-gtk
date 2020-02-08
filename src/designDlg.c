/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Design dialogs management.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "filterSupport.h"
#include "dialogSupport.h"
#include "mainDlg.h"
#include "designDlg.h"
#include "linFirDesignDlg.h"
#include "stdIirDesignDlg.h"
#include "miscDesignDlg.h"
#include "dfcProject.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

typedef void (*DESIGNDLG_CREATE_FUNC)(GtkWidget *topWidget,
                                      GtkWidget *boxWidget,
                                      const CFG_DESKTOP* pPrefs);
typedef void (*DESIGNDLG_PRESET_FUNC)(GtkWidget *topWidget,
                                      const DESIGNDLG *pDesign,
                                      const FLTCOEFF *pFilter,
                                      const CFG_DESKTOP* pPrefs);
typedef void (*DESIGNDLG_DESTROY_FUNC)(GtkWidget *topWidget);
typedef int (*DESIGNDLG_APPLY_FUNC)(GtkWidget *topWidget,
                                    const CFG_DESKTOP* pPrefs);
typedef BOOL (*DESIGNDLG_ACTIVE_FUNC)(GtkWidget *topWidget);


/** Design dialog descriptor.
 */ 
typedef struct
{
    char *name;                                     /**< Name of filter class */
    DESIGNDLG_CREATE_FUNC create;        /**< Design dialog creation function */
    DESIGNDLG_PRESET_FUNC preset;          /**< Design dialog preset function */
    DESIGNDLG_DESTROY_FUNC destroy;       /**< Design dialog destroy function */
    DESIGNDLG_APPLY_FUNC apply;             /**< Design dialog apply function */
    DESIGNDLG_ACTIVE_FUNC active;    /**< Design dialog active check function */
} DESIGNDLG_DESC;


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define DESIGNDLG_DEFAULT       FLTCLASS_DEFAULT          /**< Default design */



/* LOCAL VARIABLE DEFINITIONS *************************************************/

static DESIGNDLG_DESC dlgDesc[FLTCLASS_SIZE] =
{
    {                                                        /* FLTCLASS_MISC */
        N_("Miscellaneous"),
        miscDesignDlgCreate, (DESIGNDLG_PRESET_FUNC)miscDesignDlgPreset,
        miscDesignDlgDestroy, miscDesignDlgApply, miscDesignDlgActive
    },
    {                                                      /* FLTCLASS_LINFIR */
        N_("Linear FIR"),
        linFirDesignDlgCreate, (DESIGNDLG_PRESET_FUNC)linFirDesignDlgPreset,
        linFirDesignDlgDestroy, linFirDesignDlgApply, linFirDesignDlgActive
    },
    {                                                      /* FLTCLASS_STDIIR */
        N_("Standard IIR"),
        stdIirDesignDlgCreate, (DESIGNDLG_PRESET_FUNC)stdIirDesignDlgPreset,
        stdIirDesignDlgDestroy, stdIirDesignDlgApply, stdIirDesignDlgActive
    }
};


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void updateLayout (GtkWidget *topWidget, FLTCLASS type);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Updates the design layout.
 *
 *  \param topWidget    Top level widget.
 *  \param type         Filter class (may be FLTCLASS_NOTDEF).
 *
 ******************************************************************************/
static void updateLayout (GtkWidget *topWidget, FLTCLASS type)
{
    static FLTCLASS currentDlgType = FLTCLASS_NOTDEF;

    FLTCLASS i;

    GtkWidget* boxWidget = lookup_widget (topWidget, "boxDesignDlg");
    ASSERT (boxWidget != NULL);

    if ((type < 0) || (type >= FLTCLASS_SIZE))
    {
        if (currentDlgType == FLTCLASS_NOTDEF)             /* the first call? */
        {
            type = DESIGNDLG_DEFAULT;            /* set default startup value */
        } /* if */
        else
        {
            type = currentDlgType;                          /* change nothing */
        } /* if */
    } /* if */

    if (type != currentDlgType)
    {
        if (currentDlgType != FLTCLASS_NOTDEF)         /* not the first call? */
        {
            for (i = 0; i < FLTCLASS_SIZE; i++)
            {
                dlgDesc[i].destroy (topWidget); /* does nothing if not active */
            } /* for */
        } /* if */


        dlgDesc[type].create (topWidget, boxWidget, cfgGetDesktopPrefs ());
        gtk_window_resize (GTK_WINDOW (topWidget), 1, 1); /* resize to minimum */
        currentDlgType = type;
    } /* if */
} /* updateLayout() */




/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** This function should be called, if the design dialog (box) is realized.
 *
 *
 *  \param widget       Widget pointer to \a boxDesignDlg, a GtkVBox.
 *  \param user_data    Pointer to user data as passed to g_signal_connect()
 *                      for the \e realize event (unused).
 *
 ******************************************************************************/
void designDlgBoxRealize(GtkWidget *widget, gpointer user_data)
{
    FLTCLASS index;                     /* selection in filter class combobox */

    GtkWidget *topWidget = gtk_widget_get_toplevel (widget);
    GtkWidget* classWidget = lookup_widget (topWidget, DESIGNDLG_COMBO_CLASS);

    ASSERT (topWidget != NULL);
    ASSERT (classWidget != NULL);

    for (index = 0; index < FLTCLASS_SIZE; index++)
    {
        gtk_combo_box_append_text (GTK_COMBO_BOX (classWidget),
                                   gettext (dlgDesc[index].name));
    } /* for */

#if GTK_CHECK_VERSION (2, 18, 0)
    if (gtk_widget_is_toplevel (topWidget))                /* new since 2.18 */
#else
    if (GTK_WIDGET_TOPLEVEL (topWidget))            /* deprecated since 2.20 */
#endif
    {
        index = dfcPrjGetDesign(NULL);

        if ((index < 0) || (index >= FLTCLASS_SIZE))
        {
            index = DESIGNDLG_DEFAULT;           /* set default startup value */
        } /* if */

        /* The following call emits a "changed" event, which then forces
         * execution of updateLayout().
         */
        gtk_combo_box_set_active (GTK_COMBO_BOX (classWidget), index);
    } /* if */
} /* designDlgBoxRealize() */


/* FUNCTION *******************************************************************/
/** This function is called if the filter class changes.
 *
 *
 *  \param combobox     Filter class combobox widget.
 *  \param user_data    User data of \e changed event (unused).
 *
 ******************************************************************************/
void designDlgOnFilterComboChanged (GtkComboBox* combobox, gpointer user_data)
{
    FLTCLASS index = gtk_combo_box_get_active(combobox);
    GtkWidget* topWidget = gtk_widget_get_toplevel (GTK_WIDGET(combobox));

    ASSERT (topWidget != NULL);
    ASSERT (index < FLTCLASS_SIZE);
    updateLayout (topWidget, index);

} /* designDlgOnFilterComboChanged() */



/* FUNCTION *******************************************************************/
/** Initializes or updates the design dialog from (may be new) project data.
 *
 *  \param topWidget    Top level widget.
 *
 ******************************************************************************/
void designDlgUpdate (GtkWidget *topWidget)
{
    DESIGNDLG design;
    FLTCLASS type = dfcPrjGetDesign (&design);

    updateLayout (topWidget, type);

    if (type != FLTCLASS_NOTDEF)
    {
        gtk_combo_box_set_active (
            GTK_COMBO_BOX (
                lookup_widget (topWidget, DESIGNDLG_COMBO_CLASS)), type);

        dlgDesc[type].preset (topWidget, &design, dfcPrjGetFilter (),
                              cfgGetDesktopPrefs ());
    } /* if */
} /* designDlgUpdate() */



/* FUNCTION *******************************************************************/
/** This function is called if the \e Apply button emits the \e clicked signal.
 *
 *  \param button       \e Apply button widget handle.
 *  \param data         User data as passed to function g_signal_connect. In
 *                      that case (here) it is the filter class combobox widget
 *                      handle.
 *
 ******************************************************************************/
void designDlgApply (GtkButton *button, gpointer data)
{
    int err;

    GtkWidget *widget, *topWidget = gtk_widget_get_toplevel (GTK_WIDGET (button));
    GtkComboBox* combobox = GTK_COMBO_BOX (data);
    FLTCLASS type = gtk_combo_box_get_active(combobox);

    if (type >= 0)
    {
        gint ack = GTK_RESPONSE_YES;

        if (dfcPrjGetFlags() & DFCPRJ_FLAG_SUPERSEDED)
        {
            widget = gtk_message_dialog_new_with_markup (
                GTK_WINDOW (topWidget), GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                _("Somewhere along the way you made some direct changes at"
                  " the coefficients of current filter. Would you really"
                  " forget these changes and generate new filter coefficients?"));

            ack = gtk_dialog_run (GTK_DIALOG (widget));
            gtk_widget_destroy (widget);
        } /* if */


        if (ack == GTK_RESPONSE_YES)                         /* apply really? */
        {
            err = dlgDesc[type].apply (topWidget, cfgGetDesktopPrefs ());

            if (!mainDlgUpdateFilter (err))
            {
                if (err != INT_MAX)
                {
                    dlgError (topWidget, _("Cannot generate such a filter."
                                           " Please check sample frequency, degree"
                                           " and other design parameters."));
                } /* if */
            } /* if */
        } /* if */
    } /* if */

} /* designDlgApply() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
