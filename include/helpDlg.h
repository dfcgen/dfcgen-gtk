/**
 * \file        helpDlg.h
 * \brief       Interface to dialog functions of \e Help menu.
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <dfcgen@rho62.de>
 */

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

