/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Interface to dialog functions of \e Help menu.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/helpDlg.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef HELPDLG_H
#define HELPDLG_H


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
/** About dialog creation callback from menu.
 *
 *  \param menuitem     \e Help \e About menu item.
 *  \param user_data    User data as passed to function g_signal_connect (unused).
 *
 *  \return     Widget pointer.
 ******************************************************************************/
    void helpDlgMenuActivate (GtkMenuItem* menuitem, gpointer user_data);



#ifdef  __cplusplus
}
#endif


#endif /* HELPDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
