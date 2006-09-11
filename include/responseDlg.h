/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Response settings/properties dialog.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/responseDlg.h,v 1.1.1.1 2006-09-11 15:52:20 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef RESPONSE_DLG_H
#define RESPONSE_DLG_H


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
/** Creates the properties dialog for a response window.
 *
 *  \return             Dialog widget.
 ******************************************************************************/
    GtkWidget* responseDlgCreate (void);


#ifdef  __cplusplus
}
#endif


#endif /* RESPONSE_DLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

