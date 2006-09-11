/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Miscellaneous FIR/IIR design dialogs.
 *
 * \note     Includes raw filters (filters without a design, except \f$f_{Sample}\f$).
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/miscDesignDlg.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef MISCFLT_DESIGNDLG_H
#define MISCFLT_DESIGNDLG_H


/* INCLUDE FILES **************************************************************/

#include "gui.h"


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
 *
 ******************************************************************************/
    void miscDesignDlgCreate (GtkWidget *topWidget, GtkWidget *boxDesignDlg);


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
    int miscDesignDlgApply (GtkWidget *topWidget);


#ifdef  __cplusplus
}
#endif


#endif /* MISCFLT_DESIGNDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

