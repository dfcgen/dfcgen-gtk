/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Dialog helper functions.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
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
 *  \param ename        Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value (after applying \p multiplier).
 *  \param vmax         Maximum allowed value (after applying \p multiplier).
 *  \param multiplier   Multiplier for scaling the result (wrt. units).
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSe is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 ******************************************************************************/
    BOOL dlgGetDouble (GtkWidget* topWidget, const char *ename,
                       double vmin, double vmax, double multiplier, double *pResult);


/* FUNCTION *******************************************************************/
/** Fetches an integer value from a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param ename        Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param vmin         Minimum allowed value.
 *  \param vmax         Maximum allowed value.
 *  \param pResult      Pointer to result buffer.
 *
 *  \return             TRUE on success, else FALSE. If FALSe is returned then
 *                      a message was given to the user indicating the wrong
 *                      input.
 ******************************************************************************/
    BOOL dlgGetInt (GtkWidget* topWidget, const char *ename,
                    int vmin, int vmax, int *pResult);


/* FUNCTION *******************************************************************/
/** Sets a double value into a GtkEntry dialog widget.
 *
 *  \param topWidget    Toplevel widget.
 *  \param name         Name of text entry widget (as set by GLADE_HOOKUP_OBJECT).
 *  \param multiplier   Multiplier for scaling the result (wrt. units).
 *  \param value        The double value to set.
 *
 ******************************************************************************/
    void dlgSetDouble (GtkWidget* topWidget, const char *name,
                       double multiplier, double value);



/* FUNCTION *******************************************************************/
/** A little popup dialog, where the user shall enter a double value.
 *
 *  \param title        Title of window.
 *  \param label        Label to put before the GtkEntry text field.
 *  \param comment      An introduction displayed at top of the dialog (may be
 *                      NULL, then nothing is displayed at top position).
 *  \param pResult      Pointer to result buffer, which must be initialized with
 *                      a default value.
 *
 *  \return             TRUE on success, FALSE if the user has canceled.
 ******************************************************************************/
    BOOL dlgPopupDouble (char *title, char *label, char *comment, double *pResult);


#ifdef  __cplusplus
}
#endif


#endif /* DLGSUPPORT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

