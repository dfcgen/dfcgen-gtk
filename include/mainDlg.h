/******************************************************************************/
/**
 * \file
 *           Main dialog management.
 *
 * \author   Copyright (C) 2006, 2011-2013, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef MAINDLG_H
#define MAINDLG_H


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
/** dfcgen top widget creation.
 *
 *  This function is completely taken over from interface.c, where it has been
 *  generated by glade.
 *
 *
 *  \return     Top widget reference.
 ******************************************************************************/
    GtkWidget* mainDlgCreate (void);


/* FUNCTION *******************************************************************/
/** Updates the project information in statusbar.
 *
 ******************************************************************************/
    void mainDlgUpdatePrjInfo (void);


/* FUNCTION *******************************************************************/
/** Adjustment of main filter dialog from a new project (may be read from file
 *  before).
 *
 *  \param filename     Associated filename in filesystem coding, or NULL to
 *                      reset.
 *
 ******************************************************************************/
    void mainDlgUpdateAll (const char* filename);



/* FUNCTION *******************************************************************/
/** Updates the filter dialog in main widget from current project.
 *
 ******************************************************************************/
    BOOL mainDlgUpdateFilter (int err);


/* FUNCTION *******************************************************************/
/** Redraw of main filter dialog and all associated response plot (an case a
 *  project is defined, else does nothing).
 *
 ******************************************************************************/
    void mainDlgRedrawAll (void);


#ifdef  __cplusplus
}
#endif


#endif /* MAINDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

