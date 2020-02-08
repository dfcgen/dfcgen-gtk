/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Roots plot functions.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef ROOTSPLOT_H
#define ROOTSPLOT_H


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "mathPoly.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Creates a \e GtkDrawingArea used for roots display.
 *
 *  \return             Pointer to widget (\e GtkDrawingArea, GDK drawable), which
 *                      is used in function rootsPlotUpdate() to draw polynomial
 *                      roots.
 ******************************************************************************/
    GtkWidget *rootsPlotCreate (void);


/* FUNCTION *******************************************************************/
/** Re-calculates the roots of transfer \f$H(z)\f$ of a filter.
 *
 *  \param pFilter      Pointer to filter coefficients, for which the roots
 *                      shall be calculated. Set this to NULL, if the filter
 *                      (and therefore the roots too) is invalid.
 *
 ******************************************************************************/
    void rootsPlotUpdate (FLTCOEFF *pFilter);



/* FUNCTION *******************************************************************/
/** Forces a asynchronous redraw of all transfer function roots.
 *
 ******************************************************************************/
    void rootsPlotRedraw (void);


#ifdef  __cplusplus
}
#endif


#endif /* ROOTSPLOT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

