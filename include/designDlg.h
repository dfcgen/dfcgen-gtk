/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Design dialogs handler.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef DESIGNDLG_H
#define DESIGNDLG_H


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "miscFilter.h"
#include "linFirFilter.h"
#include "stdIirFilter.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** First four common elements in a designs.
 */
typedef struct
{
    int type;            /**< Filter type. \attention Must be the 1st element */
    int order;       /**< Order of filter. \attention Must be the 2nd element */
    double cutoff;  /**< Cutoff frequency. \attention Must be the 3rd element */
    FTRDESIGN ftr; /**< Frequency transformation data. \attention Must be the 4th element */
} DESIGNDLG_COMMON;


/** Dialog data of a filter.
 */
typedef union dlg
{
    MISCFLT_DESIGN miscFlt;             /**< Miscellaneous filter design data */
    LINFIR_DESIGN linFir;                           /**< Lin. FIR design data */
    STDIIR_DESIGN stdIir;                /**< Standard IIR filter design data */
    DESIGNDLG_COMMON all;     /**< For generic access (not a specific design) */
} DESIGNDLG;



/* GLOBAL CONSTANT DECLARATIONS ***********************************************/

#define DESIGNDLG_COMBO_CLASS   "comboFilterClass" /**< Filter class combobox widget name */


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** This function should be called, if the design dialog (box) is realized.
 *
 *
 *  \param widget       Widget pointer.
 *  \param user_data    Pointer to user data (unused).
 *
 *  \return             Nothing.
 ******************************************************************************/
    void designDlgBoxRealize(GtkWidget *widget, gpointer user_data);


/* FUNCTION *******************************************************************/
/** This function is called if the filter class changes.
 *
 *
 *  \param combobox     Filter class combobox widget.
 *
 *  \return             Nothing.
 ******************************************************************************/
    void designDlgOnFilterComboChanged (GtkComboBox* combobox, gpointer user_data);



/* FUNCTION *******************************************************************/
/** This function shall be called if the filter design dialog must be updated.
 *
 *  \param topWidget    Top level widget.
 *
 ******************************************************************************/
    void designDlgUpdate (GtkWidget *topWidget);


/* FUNCTION *******************************************************************/
/** This function should be called if the \e Apply button emits the \e clicked
 *  signal.
 *
 *  \param button       \e Apply button widget handle.
 *  \param data         User data as passed to function g_signal_connect. In
 *                      that case (here) it is the filter class combobox widget
 *                      handle.
 *
 ******************************************************************************/
void designDlgApply (GtkButton *button, gpointer data);


#ifdef  __cplusplus
}
#endif


#endif /* DESIGNDLG_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
