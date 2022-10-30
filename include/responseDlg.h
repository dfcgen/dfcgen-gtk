/**
 * \file        responseDlg.h
 * \brief       Response settings/properties dialog.
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <dfcgen@rho62.de>
 */

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
 *  \param topWindow    Parent window.
 *  \param pDiag        Pointer to current plot configuration (for preset).
 *
 *  \return             Dialog widget.
 ******************************************************************************/
    GtkWidget* responseDlgCreate (GtkWindow *topWindow, PLOT_DIAG *pDiag);


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

