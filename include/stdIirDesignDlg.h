/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Standard IIR filter dialog functions.
 *
 * \author   Copyright (c) Ralf Hoppe
 * \version  $Header: /home/cvs/dfcgen-gtk/include/stdIirDesignDlg.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * \see
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef STDIIRDESIGNDLG_H
#define STDIIRDESIGNDLG_H


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
/** Standard IIR filter design dialog creation function.
 *
 *  This function has to be called if a standard IIR filter shall be designed.
 *
 *
 *  \param topWidget    Toplevel widget.
 *  \param boxDesignDlg GtkVBox widget, which is the container for the dialog.
 *                      The dialog must be mapped to row 1 of it.
 *
 ******************************************************************************/
    void stdIirDesignDlgCreate (GtkWidget *topWidget, GtkWidget *boxDesignDlg);


/* FUNCTION *******************************************************************/
/** Standard IIR filter design dialog destroy function.
 *
 *  \note If the dialog is not active the function does nothing.
 *
 *  \param topWidget    Toplevel widget.
 *
 ******************************************************************************/
    void stdIirDesignDlgDestroy (GtkWidget *topWidget);


/* FUNCTION *******************************************************************/
/** Checks whether the standard IIR filter design dialog is active or not.
 *
 *  \param topWidget    Toplevel widget.
 *
 *  \return             TRUE if the dialog is active (the main-widget of
 *                      standard IIR dialog found), else FALSE.
 ******************************************************************************/
    BOOL stdIirDesignDlgActive (GtkWidget *topWidget);


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
    int stdIirDesignDlgApply (GtkWidget *topWidget);


#ifdef  __cplusplus
}
#endif


#endif /* STDIIRDESIGNDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
