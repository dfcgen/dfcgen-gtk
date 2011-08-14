/******************************************************************************/
/**
 * \file
 *           Digital filter response window creation and callbacks.
 *
 * \author   Copyright (C) 2006, 2011 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/responseWin.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 ******************************************************************************/


#ifndef RESPONSE_WIN_H
#define RESPONSE_WIN_H


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "responsePlot.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Toggles visibility of a filter response widget/window. This function
 *  should be called if a \e GtkCheckMenuItem from the \e View menu receives
 *  an \e activate event.
 *
 *  \param menuitem     Menu item which has received the \e activate event.
 *  \param user_data    Pointer to response type (RESPONSE_TYPE).
 *
 ******************************************************************************/
    void responseWinMenuActivate (GtkMenuItem* menuitem, gpointer user_data);



/* FUNCTION *******************************************************************/
/** Invalidates one or all response windows for redrawing.
 *
 *  \param type         The response window which shall be redrawn. Set this
 *                      parameter to RESPONSE_TYPE_SIZE to redraw all.
 *
 ******************************************************************************/
    void responseWinRedraw (RESPONSE_TYPE type);


#ifdef  __cplusplus
}
#endif


#endif /* RESPONSE_WIN_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
