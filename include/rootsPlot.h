/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Description.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/rootsPlot.h,v 1.1 2006-11-04 18:28:48 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
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


    GtkWidget *rootsPlotCreate (void);
    void rootsPlotUpdate (FLTCOEFF *pFilter);
    void rootsPlotRedraw (void);


#ifdef  __cplusplus
}
#endif


#endif /* FILENAME_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

