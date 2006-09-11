/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           \e dfcgen configuration settings.
 *
 * \note     All double values written to the configuration file are formated
 *           in the "C" locale for LC_NUMERIC.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/cfgSettings.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef CFGSETTINGS_H
#define CFGSETTINGS_H


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
/** Reads the \e dfcgen configuration from XDG_CONFIG_DIR, the XDG user
 *  configuration directory (see XDG specification at
 *  http://www.freedesktop.org/Standards/basedir-spec).
 *
 *  \param widget       Top level widget used for default colors assignment.
 *
 ******************************************************************************/
    void cfgCacheSettings (GtkWidget *widget);


/* FUNCTION *******************************************************************/
/** Writes the \e dfcgen configuration to XDG_CONFIG_DIR, the XDG user
 *  configuration directory (see XDG specification at
 *  http://www.freedesktop.org/Standards/basedir-spec).
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
    int cfgFlushSettings (void);


/* FUNCTION *******************************************************************/
/** Saves the response window configuration settings. It is assumed that the
 *  response window is closed when calling this function.
 *
 *  \param type         Type of response plot/window.
 *  \param pDiag        Pointer to plot which holds the current settings.
 *
 ******************************************************************************/
    void cfgSaveResponseSettings(RESPONSE_TYPE type, PLOT_DIAG* pDiag);



/* FUNCTION *******************************************************************/
/** Restores the response window configuration settings. It is assumed that the
 *  response window is visible when calling this function with a valid \a pDiag
 *  pointer.
 *
 *  \param type         Type of response plot/window.
 *  \param pDiag        Pointer to plot which gets the settings from last
 *                      session. If \a pDiag is NULL, then the last
 *                      \e Visibility state is returned.
 *
 *  \return             TRUE if the response window should be visible (on),
 *                      FALSE if not (off).
 *
 ******************************************************************************/
    BOOL cfgRestoreResponseSettings(RESPONSE_TYPE type, PLOT_DIAG* pDiag);



#ifdef  __cplusplus
}
#endif


#endif /* CFGSETTINGS_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

