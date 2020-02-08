/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Miscellaneous FIR/IIR design dialogs.
 *
 * \note     Includes raw filters (filters without a design, except \f$f_{Sample}\f$).
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef MISCFLT_DESIGNDLG_H
#define MISCFLT_DESIGNDLG_H


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "cfgSettings.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/



/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


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
                              const CFG_DESKTOP* pPrefs);


/* FUNCTION *******************************************************************/
/** Misc filter design dialog preset function. Restores all states of dialog
 *  elements from design data of a Misc filter.
 *
 *  \param topWidget    Toplevel widget.
 *  \param pDesign      Pointer to misc filter design data.
 *  \param pFilter      Pointer to filter coefficients (only member \a f0 used).
 *  \param pPrefs       Pointer to desktop preferences.
 *
 ******************************************************************************/
    void miscDesignDlgPreset (GtkWidget *topWidget,
                              const MISCFLT_DESIGN *pDesign,
                              const FLTCOEFF *pFilter,
                              const CFG_DESKTOP* pPrefs);


/* FUNCTION *******************************************************************/
/** Raw filter design dialog destroy function.
 *
 *  \note If the dialog is not active the function does nothing.
 *
 *  \param topWidget    Toplevel widget.
 *
 ******************************************************************************/
    void miscDesignDlgDestroy (GtkWidget *topWidget);


/* FUNCTION *******************************************************************/
/** Checks whether the misc filter design dialog is active or not.
 *
 *  \param topWidget    Toplevel widget.
 *
 *  \return             TRUE if the dialog is active (the main-widget of
 *                      misc dialog found), else FALSE.
 ******************************************************************************/
    BOOL miscDesignDlgActive (GtkWidget *topWidget);


/* FUNCTION *******************************************************************/
/** Miscellaneous FIR/IIR filter design dialog \e Apply function.
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
 *                        filter can't be created. You can use the
 *                        FLTERR_CRITICAL macro from filterSupport.h to check
 *                        this condition. If the value is INT_MAX then an
 *                        error message box was displayed by function
 *                        miscDesignDlgApply(), means the caller should not
 *                        popup a (second) message box.
 ******************************************************************************/
    int miscDesignDlgApply (GtkWidget *topWidget, const CFG_DESKTOP* pPrefs);


#ifdef  __cplusplus
}
#endif


#endif /* MISCFLT_DESIGNDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

