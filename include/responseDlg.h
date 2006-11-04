/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Response settings/properties dialog.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/responseDlg.h,v 1.2 2006-11-04 18:28:27 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2006/09/11 15:52:20  ralf
 * Initial CVS import
 *
 *
 *
 ******************************************************************************/


#ifndef RESPONSE_DLG_H
#define RESPONSE_DLG_H


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "cairoPlot.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Creates the properties dialog for a response plot.
 *
 *  \param pDiag        Pointer to current plot configuration (for preset).
 *
 *  \return             Dialog widget.
 ******************************************************************************/
    GtkWidget* responseDlgCreate (PLOT_DIAG *pDiag);


/* FUNCTION *******************************************************************/
/** Sets a double value into a GtkEntry dialog widget.
 *
 *  \param dialog       Dialog (top-level) widget.
 *  \param pDiag        Pointer to plot settings to be updated.
 *
 *  \return             Zero on success, else an error number (see errno.h or
 *                      gsl_errno.h for predefined codes).
 ******************************************************************************/
    int responseDlgApply (GtkWidget *dialog, PLOT_DIAG *pDiag);


#ifdef  __cplusplus
}
#endif


#endif /* RESPONSE_DLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

