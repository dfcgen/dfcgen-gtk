/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Digital filter response plotter.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
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
 */
typedef enum
{
    RESPONSE_TYPE_MAGNITUDE = 0,                    /**< magnitude response */
    RESPONSE_TYPE_ATTENUATION,                             /**< attenuation */
    RESPONSE_TYPE_CHAR,                        /**< characteristic function */
    RESPONSE_TYPE_PHASE,                                /**< phase response */
    RESPONSE_TYPE_DELAY,                                   /**< phase delay */
    RESPONSE_TYPE_GROUP,                                   /**< group delay */
    RESPONSE_TYPE_IMPULSE,                /**< time-domain impulse response */
    RESPONSE_TYPE_STEP,                      /**< time-domain step response */

    RESPONSE_TYPE_SIZE               /**< administrative value (array size) */
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

