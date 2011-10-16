/******************************************************************************/
/**
 * \file     filterPrint.h
 * \brief    Filter print functions.
 *
 * \author   Copyright (C) 2006, 2011 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Id$
 *
 ******************************************************************************/


#if GTK_CHECK_VERSION(2, 10, 0) && !defined(FILTER_PRINT_H)
#define FILTER_PRINT_H


/* INCLUDE FILES **************************************************************/

#include "dfcgen.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** \brief Callback function for the \e begin-print event.
 *
 *  The \e begin-print event is emitted after the user has finished changing
 *  print settings in the dialog, before the actual rendering starts. A typical
 *  use for \e begin-print is to use the parameters from the \c GtkPrintContext
 *  and paginate the document accordingly, and then set the number of pages
 *  with \c gtk_print_operation_set_n_pages().
 *
 *
 * \param[in] op        the GtkPrintOperation on which the signal was emitted
 * \param[in] ctx       the GtkPrintContext for the current operation
 * \param[in] ref       user data set when the signal handler was connected
 *
 ******************************************************************************/
    void filterPrintCoeffsInit (GtkPrintOperation *op, GtkPrintContext *ctx, gpointer ref);



/* FUNCTION *******************************************************************/
/** \brief Callback function for the \e draw-page event.
 *
 *  This function (signal handler for \e draw-page) is called for every page
 *  that is printed. It renders the page onto the cairo context of page.
 *
 * \param[in] op        the GtkPrintOperation on which the signal was emitted
 * \param[in] ctx       the GtkPrintContext for the current operation
 * \param[in] pgno      the number of the currently printed page 
 * \param[in] ref       user data set when the signal handler was connected
 *
 ******************************************************************************/
    void filterPrintCoeffsDo (GtkPrintOperation *op, GtkPrintContext *ctx, int pgno, gpointer ref);



#ifdef  __cplusplus
}
#endif


#endif /* FILTER_PRINT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

