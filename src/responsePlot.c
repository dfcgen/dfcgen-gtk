/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Digital filter response plotter.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/src/responsePlot.c,v 1.1.1.1 2006-09-11 15:52:19 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "dfcProject.h"
#include "cairoPlot.h"
#include "responsePlot.h"
#include "filterResponse.h"

#include <stdlib.h>


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

/** Response plot description
 */
typedef struct
{
    RESPONSE_TYPE type;                     /**< Type of response plot/window */
    PLOT_FUNC_GET sampleFunc;                 /**< real-world function y=f(x) */
    PLOT_FUNC_INIT initFunc;  /**< plot initialization function (may be NULL) */
    PLOT_FUNC_END endFunc; /**< plot de-initialization function (may be NULL) */
    PLOT_FUNC_PROGRESS progressFunc; /**< Original plot progress callback (may be NULL) */
    void *pData;                     /**< Original data pointer (may be NULL) */
    FLTCOEFF *pFilter;                    /**< Pointer to filter coefficients */
    FLTRESP_TIME_WORKSPACE *pWorkspace;  /**< Time response workspace pointer */
} RESPONSE_PLOT;


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static int plotProgressCallback (void *pData, double percent);
static double plotAmplitude (double *px, void *pData);
static double plotAttenuation (double *f, void *pData);
static double plotChar (double *f, void *pData);
static double plotPhase (double *f, void *pData);
static double plotPhaseDelay (double *f, void *pData);
static double plotGroupDelay (double *f, void *pData);
static int plotImpulseInit (double start, double stop, void *pData);
static int plotStepInit (double start, double stop, void *pData);
static double timeResponse (double *t, void *pData);
static void timeResponseEnd (void *pData);


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/** All predefined response plots (partially intialized).
 */
static RESPONSE_PLOT responsePlot[RESPONSE_TYPE_SIZE] =
{
    {
        RESPONSE_TYPE_AMPLITUDE,                                      /* type */
        plotAmplitude,                                          /* sampleFunc */
    },
    {
        RESPONSE_TYPE_ATTENUATION,
        plotAttenuation,
    },
    {
        RESPONSE_TYPE_CHAR,
        plotChar,
    },
    {
        RESPONSE_TYPE_PHASE,
        plotPhase,
    },
    {
        RESPONSE_TYPE_DELAY,
        plotPhaseDelay,
    },
    {
        RESPONSE_TYPE_GROUP,
        plotGroupDelay,
    },
    {
        RESPONSE_TYPE_IMPULSE,
        timeResponse,                                           /* sampleFunc */
        plotImpulseInit,                                          /* initFunc */
        timeResponseEnd,                                           /* endFunc */
    },
    {
        RESPONSE_TYPE_STEP,
        timeResponse,                                           /* sampleFunc */
        plotStepInit,                                             /* initFunc */
        timeResponseEnd,                                           /* endFunc */
    }
}; /* responsePlot[] */



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Private plot break/progress callback. For each plotted coordinate value
 *  this function is called back (if not NULL) for user break checking and
 *  progress indication.
 *
 *  \param pData        User application data pointer as stored in member
 *                      \a pData of structure PLOT_DIAG (as passed to function
 *                      cairoPlot2d().
 *  \param percent      A value between 0.0 and 1.0 which indicates the
 *                      percentage of completion.
 *
 *  \return             The function shall return an value unequal to 0, if the
 *                      plot has to be cancelled. In that case the associated
 *                      function of type PLOT_FUNC_END is called and the plot
 *                      in progress will be canceled.
 ******************************************************************************/
static int plotProgressCallback (void *pData, double percent)
{
    RESPONSE_PLOT *pResponse = pData;

    return pResponse->progressFunc (pResponse->pData, percent);
} /* plotProgressCallback() */



/* FUNCTION *******************************************************************/
/** Computes the amplitude response of a filter (for usage on a \e Cairo plot).
 *
 *  \param f            Pointer to real-world x-coordinate (input value), means
 *                      the frequency here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_AMPLITUDE].
 *
 *  \return             Calculated real-world y-coordinate on success. If there
 *                      is no value at \p x resp. frequency (may be a singularity),
 *                      then it returns GSL_POSINF or GSL_NEGINF.
 ******************************************************************************/
static double plotAmplitude (double *f, void *pData)
{
    return filterResponseAmplitude (*f, ((RESPONSE_PLOT *)pData)->pFilter);
} /* plotAmplitude() */


/* FUNCTION *******************************************************************/
/** Computes the attenuation response of a filter (for usage on a \e Cairo plot).
 *
 *  \param f            Pointer to real-world x-coordinate (input value), means
 *                      the frequency here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_ATTENUATION].
 *
 *  \return             Calculated real-world y-coordinate on success. If there
 *                      is no value at \p x resp. frequency (may be a singularity),
 *                      then it returns GSL_POSINF or GSL_NEGINF.
 ******************************************************************************/
static double plotAttenuation (double *f, void *pData)
{
    return filterResponseAttenuation (*f, ((RESPONSE_PLOT *)pData)->pFilter);
} /* plotAttenuation() */


/* FUNCTION *******************************************************************/
/** Computes the phase response of a filter (for usage on a \e Cairo plot).
 *
 *  \param f            Pointer to real-world x-coordinate (input value), means
 *                      the frequency here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_PHASE].
 *
 *  \return             Calculated phase (real-world y-coordinate) on success. If
 *                      there is no value at \p x resp. frequency (may be a
 *                      singularity), then it returns GSL_POSINF or GSL_NEGINF.
 ******************************************************************************/
static double plotPhase (double *f, void *pData)
{
    double phase = filterResponsePhase (*f, ((RESPONSE_PLOT *)pData)->pFilter);

    if (gsl_finite (phase))
    {
        phase = phase / M_PI * 180;
    } /* if */

    return phase;
} /* plotPhase() */


/* FUNCTION *******************************************************************/
/** Computes the phase delay response of a filter (for usage on a \e Cairo plot).
 *
 *  \param f            Pointer to real-world x-coordinate (input value), means
 *                      the frequency here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_DELAY].
 *
 *  \return             Calculated phase delay (real-world y-coordinate) on
 *                      success. If there is no value at \p x resp. frequency
 *                      (may be a singularity), then it returns GSL_POSINF or
 *                      GSL_NEGINF.
 ******************************************************************************/
static double plotPhaseDelay (double *f, void *pData)
{
    double delay = filterResponsePhaseDelay (*f, ((RESPONSE_PLOT *)pData)->pFilter);

    if (gsl_finite (delay))
    {
        if (delay < 0.0)               /* a time delay should not be negative */
        {                                   /* transform into positive values */
            delay -= (2.0 * M_PI) * floor(delay / (2.0 * M_PI));
        } /* if */
    } /* if */

    return delay;
} /* plotPhaseDelay() */


/* FUNCTION *******************************************************************/
/** Computes the group delay response of a filter (for usage on a \e Cairo plot).
 *
 *  \param f            Pointer to real-world x-coordinate (input value), means
 *                      the frequency here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_GROUP].
 *
 *  \return             Calculated group delay (real-world y-coordinate) on
 *                      success. If there is no value at \p x resp. frequency
 *                      (may be a singularity), then it returns GSL_POSINF or
 *                      GSL_NEGINF.
 ******************************************************************************/
static double plotGroupDelay (double *f, void *pData)
{
    return filterResponseGroupDelay (*f, ((RESPONSE_PLOT *)pData)->pFilter);
} /* plotGroupDelay() */



/* FUNCTION *******************************************************************/
/** Computes the characteristic function of a filter (for usage on a \e Cairo
 *  plot).
 *
 *  \param f            Pointer to real-world x-coordinate (input value), means
 *                      the frequency here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_CHAR].
 *
 *  \return             Calculated phase (real-world y-coordinate) on success. If
 *                      there is no value at \p x resp. frequency (may be a
 *                      singularity), then it returns GSL_POSINF or GSL_NEGINF.
 ******************************************************************************/
static double plotChar (double *f, void *pData)
{
    return filterResponseChar (*f, ((RESPONSE_PLOT *)pData)->pFilter);
} /* plotChar() */



/* FUNCTION *******************************************************************/
/** Initializes the impulse response of a filter (for usage on a \e Cairo plot).
 *
 *  \param start        Time to start (must be positive).
 *  \param stop         Time to stop (must be positive).
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_IMPULSE].
 *
 *  \return  The function returns:
 *           - the value zero, if the number of samples is determined by member
 *             \a num in PLOT_DIAG;
 *           - a value greater than zero (to overwrite \a num), which determines
 *             the number of samples in interval \a start - \a stop;
 *           - a negative number, if if there was a singularity or any error.
 ******************************************************************************/
static int plotImpulseInit (double start, double stop, void *pData)
{
    RESPONSE_PLOT *pResponse = pData;

    pResponse->pWorkspace = filterResponseTimeNew (start, stop, FLTSIGNAL_DIRAC,
                                                   pResponse->pFilter);
    if (pResponse->pWorkspace == NULL)
    {
        return -1;
    } /* if */

    return pResponse->pWorkspace->samples;
} /* plotImpulseInit() */


/* FUNCTION *******************************************************************/
/** Initializes the step response of a filter (for usage on a \e Cairo plot).
 *
 *  \param start        Time to start (must be positive).
 *  \param stop         Time to stop (must be positive).
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_STEP].
 *
 *  \return  The function returns:
 *           - the value zero, if the number of samples is determined by member
 *             \a num in PLOT_DIAG;
 *           - a value greater than zero (to overwrite \a num), which determines
 *             the number of samples in interval \a start - \a stop;
 *           - a negative number, if if there was a singularity or any error.
 ******************************************************************************/
static int plotStepInit (double start, double stop, void *pData)
{
    RESPONSE_PLOT *pResponse = pData;

    pResponse->pWorkspace = filterResponseTimeNew (start, stop, FLTSIGNAL_HEAVISIDE,
                                                   pResponse->pFilter);
    if (pResponse->pWorkspace == NULL)
    {
        return -1;
    } /* if */

    return pResponse->pWorkspace->samples;
} /* plotStepInit() */


/* FUNCTION *******************************************************************/
/** Computes the time domain response of a filter (for usage on a \e Cairo plot).
 *
 *  \param t            Pointer to real-world x-coordinate (output value),
 *                      means the time here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_IMPULSE] or
 *                      responsePlot[RESPONSE_TYPE_STEP].
 *
 *  \return             Calculated phase (real-world y-coordinate) on success. If
 *                      there is no value at \p x resp. frequency (may be a
 *                      singularity), then it returns GSL_POSINF or GSL_NEGINF.
 ******************************************************************************/
static double timeResponse (double *t, void *pData)
{
    return filterResponseTimeNext (((RESPONSE_PLOT *)pData)->pWorkspace, t);
} /* timeResponse() */


/* FUNCTION *******************************************************************/
/** End of time response plot function.
 *
 *  \param pData        User application data pointer as stored in member
 *                      \a pData of structure PLOT_DIAG (as passed to function
 *                      cairoPlot2d().
 ******************************************************************************/
static void timeResponseEnd (void *pData)
{
    ASSERT (pData != NULL);
    filterResponseTimeFree (((RESPONSE_PLOT *)pData)->pWorkspace);
} /* timeResponseEnd() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


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
int responsePlotDraw (cairo_t* cr, RESPONSE_TYPE type, PLOT_DIAG *pDiag)
{
    int points = 0;
    RESPONSE_PLOT *pResponse = &responsePlot[type];

    ASSERT (type < RESPONSE_TYPE_SIZE);

    pResponse->pFilter = dfcPrjGetFilter ();

    if (pResponse->pFilter != NULL)
    {
        pResponse->pData = pDiag->pData;        /* save original data pointer */
        pDiag->pData = pResponse;                 /* set private data pointer */
        pResponse->progressFunc = pDiag->progressFunc; /* save original callback */

        if (pDiag->progressFunc != NULL)
        {
            pDiag->progressFunc = plotProgressCallback; /* set private callback  */
        } /* if */

        pDiag->initFunc = pResponse->initFunc;
        pDiag->sampleFunc = pResponse->sampleFunc;
        pDiag->endFunc = pResponse->endFunc;

        points = cairoPlot2d (cr, pDiag);

        pDiag->pData = pResponse->pData;      /* restore original data and... */
        pDiag->progressFunc = pResponse->progressFunc; /* ...callback pointer */

    } /* if */

    return points;
} /* responsePlotDraw() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
