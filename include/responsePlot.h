/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Digital filter response plotter.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/include/responsePlot.h,v 1.1.1.1 2006-09-11 15:52:21 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


#ifndef RESPONSE_PLOT_H
#define RESPONSE_PLOT_H


/* INCLUDE FILES **************************************************************/

#include "cairoPlot.h"


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** Predefined (supported) digital filter/system responses.
 *
 *  \attention The enums defined here are used as index (somewhere), means do
 *             not change without think about it.
 */
typedef enum
{
    RESPONSE_TYPE_AMPLITUDE = 0,                      /**< amplitude response */
    RESPONSE_TYPE_ATTENUATION = 1,                           /**< attenuation */
    RESPONSE_TYPE_CHAR = 2,                      /**< characteristic function */
    RESPONSE_TYPE_PHASE = 3,                              /**< phase response */
    RESPONSE_TYPE_DELAY = 4,                                 /**< phase delay */
    RESPONSE_TYPE_GROUP = 5,                                 /**< group delay */
    RESPONSE_TYPE_IMPULSE = 6,              /**< time-domain impulse response */
    RESPONSE_TYPE_STEP = 7,                    /**< time-domain step response */

    RESPONSE_TYPE_SIZE                 /**< administrative value (array size) */
} RESPONSE_TYPE;


/* FUNCTION *******************************************************************/
/** Pointer to response plot progress function. For each plotted coordinate
 *  value this function is called back (if not NULL) for user break checking
 *  and progress indication.
 *
 *  \param type         Type of response.
 *  \param percent      A value between 0.0 and 1.0 which indicates the
 *                      percentage of completion.
 *
 *  \return             The function shall return an value unequal to 0, if the
 *                      plot has to be cancelled.
 */
    typedef int (*RESPONSE_PLOT_CALLBACK)(RESPONSE_TYPE type, double percent);


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Response plot drawing function.
 *
 *  \param cr           \e Cairo context for drawing, which may be retrieved
 *                      e.g. by the help of following functions:
 *                      - gdk_cairo_create ()
 *                      - gtk_print_context_get_cairo_context()
 *  \param type         Type of response plot.
 *  \param pDiag        Pointer to plot data. Notice that the callbacks
 *                      \a initFunc, \a endFunc and \a sampleFunc will be
 *                      overwritten (from callbacks associated with \p type).
 *
 *  \return             The number of samples taken to draw this response
 *                      (independent of a possible break) or a negative
 *                      number on error.
 ******************************************************************************/
    int responsePlotDraw (cairo_t* cr, RESPONSE_TYPE type, PLOT_DIAG *pDiag);



#ifdef  __cplusplus
}
#endif


#endif /* RESPONSE_PLOT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

