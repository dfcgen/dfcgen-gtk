/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Dialog helper functions.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/dialogSupport.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef DLGSUPPORT_H
#define DLGSUPPORT_H


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
/** Error message dialog.
 *
 *  \param topWidget    Toplevel widget.
 *  \param format       Printf like format string.
 *  \param ...          Arguments associated with format string.
 *
 ******************************************************************************/
    void dlgError (GtkWidget* topWidget, char* format, ...);


/* FUNCTION *******************************************************************/
/** File error message dialog.
 *
 *  \param topWidget    Toplevel widget.
 *  \param format       Printf like format string.
 *  \param filename     Filename in filesystem coding (must not be UTF-8), which
 *                      is used as first argument into the format string (needs
 *                      %s format coding).
 *  \param err          GLib error pointer.
 *
 ******************************************************************************/
    void dlgErrorFile (GtkWidget* topWidget, char* format, char *filename, GError *err);



/* FUNCTION *******************************************************************/
/** Fetches a double value from a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param lname        Name of associated label (as set by GLADE_HOOKUP_OBJECT).
 *  \param ename        Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value.
 *  \param vmax         Maximum allowed value.
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSe is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 ******************************************************************************/
    BOOL dlgGetDouble (GtkWidget* topWidget, char *lname, char *ename,
                       double vmin, double vmax, double *pResult);


/* FUNCTION *******************************************************************/
/** Fetches an integer value from a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param lname        Name of associated label (as set by GLADE_HOOKUP_OBJECT).
 *  \param ename        Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value.
 *  \param vmax         Maximum allowed value.
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSe is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 ******************************************************************************/
    BOOL dlgGetInt (GtkWidget* topWidget, char *lname, char *ename,
                    int vmin, int vmax, int *pResult);


#ifdef  __cplusplus
}
#endif


#endif /* DLGSUPPORT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

