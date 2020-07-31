/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Digital filter response plotter.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "dfcProject.h"
#include "cairoPlot.h"
#include "responsePlot.h"
#include "filterResponse.h"
#include "cfgSettings.h"

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
    void *pData;                     /**< Original data pointer (may be NULL) */
    FLTCOEFF *pFilter;                    /**< Pointer to filter coefficients */
    FLTRESP_TIME_WORKSPACE *pWorkspace;  /**< Time response workspace pointer */
} RESPONSE_PLOT;


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static double plotMagnitude (double *px, void *pData);
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
    [RESPONSE_TYPE_MAGNITUDE] =
    {
        .type = RESPONSE_TYPE_MAGNITUDE,
        .sampleFunc = plotMagnitude,
    },
    [RESPONSE_TYPE_ATTENUATION] =
    {
        .type = RESPONSE_TYPE_ATTENUATION,
        .sampleFunc = plotAttenuation,
    },
    [RESPONSE_TYPE_CHAR] =
    {
        .type = RESPONSE_TYPE_CHAR,
        .sampleFunc = plotChar,
    },
    [RESPONSE_TYPE_PHASE] =
    {
        .type = RESPONSE_TYPE_PHASE,
        .sampleFunc = plotPhase,
    },
    [RESPONSE_TYPE_DELAY] =
    {
        .type = RESPONSE_TYPE_DELAY,
        .sampleFunc = plotPhaseDelay,
    },
    [RESPONSE_TYPE_GROUP] =
    {
        .type = RESPONSE_TYPE_GROUP,
        .sampleFunc = plotGroupDelay,
    },
    [RESPONSE_TYPE_IMPULSE] =
    {
        .type = RESPONSE_TYPE_IMPULSE,
        .sampleFunc = timeResponse,
        .initFunc = plotImpulseInit,
        .endFunc = timeResponseEnd,
    },
    [RESPONSE_TYPE_STEP] =
    {
        .type = RESPONSE_TYPE_STEP,
        .sampleFunc = timeResponse,
        .initFunc = plotStepInit,
        .endFunc = timeResponseEnd
    }
}; /* responsePlot[] */



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Computes the magnitude response of a filter (for usage on a \e Cairo plot).
 *
 *  \param f            Pointer to real-world x-coordinate (input value), means
 *                      the frequency here.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. In that
 *                      special case here it is a pointer to
 *                      responsePlot[RESPONSE_TYPE_MAGNITUDE].
 *
 *  \return             Calculated real-world y-coordinate on success. If there
 *                      is no value at \p x resp. frequency (may be a singularity),
 *                      then it returns GSL_POSINF or GSL_NEGINF.
 ******************************************************************************/
static double plotMagnitude (double *f, void *pData)
{
    return filterResponseMagnitude (*f, ((RESPONSE_PLOT *)pData)->pFilter);
} /* plotMagnitude() */


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
 *  \todo               Check phase response of linear FIR filters and improve the graph layout.
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
        if (delay < -DBL_EPSILON)        /* time delay should not be negative */
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
    const CFG_DESKTOP* pPrefs = cfgGetDesktopPrefs ();

    ASSERT (type < RESPONSE_TYPE_SIZE);
    pResponse->pFilter = dfcPrjGetFilter ();

    switch (type)
    {
        case RESPONSE_TYPE_IMPULSE:
        case RESPONSE_TYPE_STEP:
            pDiag->x.pUnit = &pPrefs->timeUnit;
            break;

        case RESPONSE_TYPE_DELAY:
        case RESPONSE_TYPE_GROUP:
            pDiag->y.pUnit = &pPrefs->timeUnit;
            /* fall through here */

        case RESPONSE_TYPE_MAGNITUDE:
        case RESPONSE_TYPE_ATTENUATION:
        case RESPONSE_TYPE_CHAR:
        case RESPONSE_TYPE_PHASE:
            pDiag->x.pUnit = &pPrefs->frequUnit;
            break;

        default:
            ASSERT (0);
    } /* switch */


    if (pResponse->pFilter != NULL)
    {
        pResponse->pData = pDiag->pData;        /* save original data pointer */
        pDiag->pData = pResponse;                 /* set private data pointer */
        pDiag->initFunc = pResponse->initFunc;
        pDiag->sampleFunc = pResponse->sampleFunc;
        pDiag->endFunc = pResponse->endFunc;

        pDiag->x.prec = pDiag->y.prec = pPrefs->outprec;
        points = cairoPlot2d (cr, pDiag);
        pDiag->pData = pResponse->pData;     /* restore original data pointer */

    } /* if */

    return points;
} /* responsePlotDraw() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
