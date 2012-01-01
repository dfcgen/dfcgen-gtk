/******************************************************************************/
/**
 * \file
 * \brief    Filter print functions.
 *
 * \author   Copyright (C) 2006, 2011, 2012 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Id$
 *
 ******************************************************************************/


#ifndef FILTER_PRINT_H
#define FILTER_PRINT_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"
#include "cairoPlot.h"    /* PLOT_DIAG */
#include "responsePlot.h" /* RESPONSE_TYPE */


#ifdef  __cplusplus
extern "C" {
#endif


#if GTK_CHECK_VERSION(2, 10, 0)


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** \brief Response plot print function.
 *
 *  \param[in] topWidget Top widget associated with the \c GtkToolButton widget
 *                       which has caused the \e clicked event.
 *  \param[in] pDiag     Pointer to plot diag of associated response \p type.
 *  \param[in] type     ::RESPONSE_TYPE which identifies what response to print.
 *
 ******************************************************************************/
    void filterPrintResponse (GtkWidget* topWidget, const PLOT_DIAG* pDiag,
                              RESPONSE_TYPE type);



/* FUNCTION *******************************************************************/
/** \brief Coefficients print function.
 *
 *  \note  The signature of this function was chosen in a way that it is usual
 *         as \e clicked event callback (when a print menuitem/button is pressed).
 *
 *  \param[in] srcWidget \c GtkMenuItem on event \e activate or \c GtkToolButton
 *                       on event \e clicked, which causes this call.
 *  \param[in] user_data Pointer to user data (unused).
 *
 ******************************************************************************/
    void filterPrintCoeffs (GtkWidget* srcWidget, gpointer user_data);


#endif /* GTK_CHECK_VERSION() */


#ifdef  __cplusplus
}
#endif


#endif /* FILTER_PRINT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

