/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           2-dimensional plot functions (GDK), normally for filter responses.
 *
 * \author   Copyright (C) 2006, 2011, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef CAIROPLOT_H
#define CAIROPLOT_H


/* INCLUDE FILES **************************************************************/

#include "base.h"    /* includes config.h (include before GNU system headers) */

#include <gdk/gdk.h>
#include <cairo.h>

#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/


#define PLOT_TOLERANCE          (DBL_EPSILON * 4)         /**< Plot tolerance */
#define PLOT_AXIS_MAX           (32768 / PLOT_TOLERANCE) /**< Maximum world value */
#define PLOT_AXIS_MIN           (-PLOT_AXIS_MAX)     /**< Minimum world value */


#define PLOT_AXIS_FLAG_LOG      1                       /**< logarithmic axis */
#define PLOT_AXIS_FLAG_GRID     2                           /**< display grid */
#define PLOT_AXIS_FLAG_AUTO     4                 /**< auto-scaling of y-axis */



/* GLOBAL TYPE DECLARATIONS ***************************************************/

/** Style of plot.
 *
 * \attention   Do not change the values assigned to enumarations, because used
 *              as index into array implementations.
 */
    typedef enum
    {
        PLOT_STYLE_LINE_ONLY = 0,
        PLOT_STYLE_CIRCLE_ONLY = 1,
        PLOT_STYLE_CIRCLE_SAMPLE = 2,
        PLOT_STYLE_CROSS_ONLY = 3,
        PLOT_STYLE_BOX_ONLY = 4
    } PLOT_STYLE;


    /** Plot color identifiers.
     */
    typedef enum 
    {
        PLOT_COLOR_LABELS = 0,       /**< Axis label numbers color identifier */
        PLOT_COLOR_GRID = 1,                 /**< Grid lines color identifier */
        PLOT_COLOR_GRAPH = 2,               /**< Graph/curve color identifier */
        PLOT_COLOR_BOX = 3,                   /**< Rectangle color identifier */
        PLOT_COLOR_AXIS_NAME = 4,        /**< Axis name/unit color identifier */
        PLOT_COLOR_NOTE_TEXT = 5, /**< Notes text color identifier (future use) */
        PLOT_COLOR_NOTE_BOX = 6, /**< Notes frame color identifier (future use) */

        PLOT_COLOR_SIZE               /**< Size of plot color identifier enum */
    } PLOT_COLOR;


    /** Unit descriptor for an axis.
     */
    typedef struct
    {
        /** Unit name string (possibly with \e Pango markup), but never
            translated into a specific language. It is assumed that units are
            always based on the international system of units (SI).
            It is allowed to set \a name NULL.

            \see http://physics.nist.gov/cuu/Units.
        */
        char *name;
        double multiplier;                      /**< Multiplier for this unit */
    } PLOT_UNIT;


    /** Description of a plot axis.
     */
    typedef struct
    {
        const char *name; /**< axis name, possibly with \e Pango markup (may be NULL) */
        const PLOT_UNIT *pUnit; /**< Pointer to unit description (may be NULL) */
        double start;               /**< real-world coordinate of start-point */
        double stop;                  /**< real-world coordinate of end-point */

        /** If the exponent of a floating point number would be less than -4 or
         *  greater than or equal to the precision \a prec, then the number will
         *  be converted in the style of '%E' (exponential notation); otherwise
         *  to the '%f' style (fixed-point notation). The `%f' conversion prints
         *  its argument in fixed-point notation, producing output of the form
         *  [-]ddd.ddd, where the number of digits following the decimal point
         *  is controlled by \a prec.  The '%E' conversion prints its argument
         *  in exponential notation, producing output of the form
         *  [-]d.dddE[+|-]dd. Again, the number of digits following the decimal
         *  point is controlled by \a prec.  The exponent always contains at
         *  least two digits.
         */
        int prec;

        unsigned flags; /**< Flags (e.g. PLOT_AXIS_FLAG_LOG, PLOT_AXIS_FLAG_GRID) */
    } PLOT_AXIS;


/* FUNCTION *******************************************************************/
/** Plot initialization function pointer. This function is called at start of
 *  (re-) paint.
 *
 *  \param start        The real-world start x-coordinate.
 *  \param stop         The real-world stop x-coordinate.
 *  \param pData        User application data pointer as passed to cairoPlot2d().
 *
 *  \return  The function shall return:
 *           - the value zero, if the number of samples is determined by member
 *             \a num in PLOT_DIAG;
 *           - a value greater than zero (to overwrite \a num), which determines
 *             the number of samples in interval \a start - \a stop;
 *           - a negative number, if if there was an singularity or
 *             calculation error.
 ******************************************************************************/
    typedef int (*PLOT_FUNC_INIT)(double start, double stop, void *pData);



/* FUNCTION *******************************************************************/
/** Pointer to function which computes an y-value from a x-value in real-world
 *  units.
 *
 *  \param px           Pointer to real-world x-coordinate (input value). Notice
 *                      that the pointed value may become an output on discrete
 *                      plots.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG.
 *
 *  \return             Calculated real-world y-coordinate on success. If there
 *                      is no value at \p px (may be a singularity), then it shall
 *                      return GSL_POSINF or GSL_NEGINF.
 ******************************************************************************/
    typedef double (*PLOT_FUNC_GET)(double *px, void *pData);



/* FUNCTION *******************************************************************/
/** Pointer to function which is called at the end of plot (counterpart to
 *  PLOT_FUNC_INIT type of function).
 *
 *  \param pData        User application data pointer as stored in member
 *                      \a pData of structure PLOT_DIAG (as passed to function
 *                      cairoPlot2d().
 ******************************************************************************/
    typedef void (*PLOT_FUNC_END)(void *pData);



/* FUNCTION *******************************************************************/
/** Pointer to plot break/progress function. For each plotted coordinate value
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
 */
    typedef int (*PLOT_FUNC_PROGRESS)(void *pData, double percent);


/** Plot diagram descriptor.
 */
    typedef struct
    {
        PLOT_AXIS x;                                   /**< x-axis descriptor */
        PLOT_AXIS y; /**< y-axis descriptor (modified if PLOT_AXIS_AUTOSCALE) */
        void *pData; /**< user (application) data ptr (passed to \a initFunc) */
        double thickness;                             /**< Thickness of graph */
        PLOT_STYLE style;                                 /**< Style of graph */
        int num;        /**< Number of samples to take (0 = number of pixels) */
        PLOT_FUNC_PROGRESS progressFunc; /**< plot progress/break function (may be NULL) */
        PLOT_FUNC_INIT initFunc; /**< plot initialization function (may be NULL) */
        PLOT_FUNC_GET sampleFunc;             /**< real-world function y=f(x) */
        PLOT_FUNC_END endFunc; /**< plot de-initialization function (may be NULL) */
        GdkColor *colors;      /**< Pointer to allocated colors (may be NULL) */
        GdkRectangle area;        /**< In: drawing area, out: graph rectangle */
    } PLOT_DIAG;


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/


/** Format string for axis name and unit (if exist).
 */
#define PLOT_AXISNAME_FORMAT(name, unit) name " [" unit "]"


/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** Plots a diagram. The function plots a 2-dimensional curve diagram \p pDiag
 *  into a \e Cairo context given in \p cr. At a precondition the clip region
 *  must be set correctly,
 *
    DeltaRatio = (ScreenMax-ScreenMin)/(WorldMax-WorldMin)      (I)
    Screen     = DeltaRatio*(World-WorldMin) + ScreenMin        (II)
    World      = (Screen - ScreenMin) / DeltaRatio + WorldMin   (III)

    if LOGAXIS (logarithmic axis) then World := log(World) before using
    of equation I,II,III what means :
    DeltaRatio = (ScreenMax-ScreenMin)/(log(WorldMax)-log(WorldMin))
               = (ScreenMax-ScreenMin)/(log(WorldMax/WorldMin))
    Screen     = DeltaRatio*(log(World) - log(WorldMin)) + ScreenMin
               = DeltaRatio*log(World/WorldMin) + ScreenMin
    World      = 10^(log(WorldMin) + (Screen - ScreenMin)/DeltaRatio))
               = WorldMin*10^((Screen - ScreenMin)/DeltaRatio)
 *
 *  \param cr           \e Cairo context for drawing, which may be retrieved
 *                      by the help of following functions:
 *                      - gdk_cairo_create ()
 *                      - gtk_print_context_get_cairo_context()
 *  \param pDiag        Pointer to plot descriptor.
 *
 *  \return             The number of samples taken to draw this plot
 *                      (independent of a possible break) or a negative
 *                      number on error.
 ******************************************************************************/
    int cairoPlot2d (cairo_t* cr, PLOT_DIAG *pDiag);



/* FUNCTION *******************************************************************/
/** Checks the plot range of an axis against some predefined limits. If the
 *  range [start, stop] doesn't match these limits, it returns ERANGE and
 *  changes the range automatically.
 *
 *
 *  \param pAxis        Pointer to axis workspace.
 *
 *  \return             If range is in boundaries then it returns the value 0,
 *                      else ERANGE from errno.h.
 ******************************************************************************/
    int cairoPlotChkRange (PLOT_AXIS *pAxis);


/* FUNCTION *******************************************************************/
/** Returns a world-coordinate associated with a GDK coordinate.
 *
 *  \note  Because cairoPlotChkRange() has checked the ranges \p pAxis->start
 *         and \p pAxis->stop with respect to the operation \p pAxis->stop -
 *         \p pAxis->start (lin. case) and \p pAxis->stop / \p pAxis->start
 *         (log. case) there is no need to use math. function mathTryDiv().
 *
 *  \param pAxis        Pointer to axis description (filled from a previous
 *                      call to cairoPlot2d).
 *  \param start        Start point of plot graph (box) in GDK coordinates.
 *  \param stop         Stop point of plot graph (box) in GDK coordinates.
 *
 *  \attention          Due to internal processing optimization (associated
 *                      with the north-west orientation of y-axis) the
 *                      calculation of y-coordinates gives correct results
 *                      only when \p start and \p stop are exchanged.
 *
 *  \return             World-coordinate within this axis.
 ******************************************************************************/
    double cairoPlotCoordinate (PLOT_AXIS *pAxis, int start, int stop,
                                int coordinate);


#ifdef  __cplusplus
}
#endif


#endif /* CAIROPLOT_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/

