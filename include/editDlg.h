/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Interface to dialog functions of \e Edit menu.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/editDlg.h,v 1.1 2006-11-04 18:28:48 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef EDITDLG_H
#define EDITDLG_H


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
/** \e Activate event callback emitted when the \e Settings menuitem is selected
 *  from \e Edit menu.
 *
 *  \param srcWidget    \e Edit \e Settings widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
    void editDlgSettingsActivate (GtkWidget* widget, gpointer user_data);



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Info menuitem is selected
 *  from \e Edit menu.
 *
 *  \param widget       \e Edit \e Info widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
    void editDlgInfoActivate (GtkWidget* widget, gpointer user_data);



#ifdef  __cplusplus
}
#endif


#endif /* EDITDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
