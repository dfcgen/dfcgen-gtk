/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     fileDlg.h
 *           Interface to dialog functions of \e File menu.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef FILEDLG_H
#define FILEDLG_H


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
/** \e Activate event callback emitted when the \e New menuitem is selected
 *  from \e File menu.
 *
 *  \param menuitem     The menu item object which received the signal (New).
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
    void fileDlgNewActivate (GtkMenuItem* menuitem, gpointer user_data);


/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Open menuitem is selected
 *  from \e File menu.
 *
 *  \param srcWidget    \e File \e Open widget (GtkMenuItem on event \e activate
 *                      or GtkToolButton on event \e clicked), which causes this
 *                      call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
    void fileDlgOpenActivate (GtkWidget* srcWidget, gpointer user_data);


/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Save menuitem is selected
 *  from \e File menu.
 *
 *  \param srcWidget    \e File \e Save widget (GtkMenuItem on event \e activate
 *                      or GtkToolButton on event \e clicked), which causes this
 *                      call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
    void fileDlgSaveActivate (GtkWidget* srcWidget, gpointer user_data);



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Save \e As menuitem is
 *  selected from \e File menu.
 *
 *  \param srcWidget    \e File \e Save \e As widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
    void fileDlgSaveAsActivate (GtkWidget* srcWidget, gpointer user_data);



/* FUNCTION *******************************************************************/
/** \e Activate event callback emitted when the \e Export menuitem is
 *  selected from \e File menu.
 *
 *  \param srcWidget    \e File \e Export widget (GtkMenuItem on event
 *                      \e activate or GtkToolButton on event \e clicked),
 *                      which causes this call.
 *  \param user_data    User data set when the signal handler was connected (unused).
 *
 ******************************************************************************/
    void fileDlgExportActivate (GtkWidget* srcWidget, gpointer user_data);



#ifdef  __cplusplus
}
#endif


#endif /* FILEDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
