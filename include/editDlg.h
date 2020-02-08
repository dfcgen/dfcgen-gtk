/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Interface to dialog functions of \e Edit menu.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
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
 *  \param widget       \e Edit \e Settings widget (GtkMenuItem on event
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
