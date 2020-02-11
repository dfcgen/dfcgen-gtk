/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     cairoPlot.c
 * \brief    2-dimensional plot on a \e Cairo graphic context.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"            /* includes config.h (include before GNU headers) */
#include "mathFuncs.h"      /* POW10() */
#include "mathMisc.h"
#include "mathFuncs.h"
#include "cairoPlot.h"

#include <stdlib.h>                                         /* declares abs() */



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define PLOT_FLAG_INVALID       1      /**< Invalid plot value (singularity?) */
#define PLOT_FLAG_OUTSIDE       2      /**< Coordinate outside the plot range */

#define PLOT_LABELS_MAX         21  /**< Maximum number of labels at any axis */

#define PLOT_LABEL_MARKER_LEN   (5.0) /**< Length of label marker (stub) line */
#define PLOT_LABEL_MARKER_WIDTH (2.0)  /**< Width of label marker (stub) line */
#define PLOT_GRID_LINE_WIDTH    (1.0)                    /**< Grid line width */
#define PLOT_GRID_DASH_LEN      (1.0)                   /**< Grid dash length */
#define PLOT_BOX_LINE_WIDTH     (1.0)                /**< Plot box line width */



/* LOCAL TYPE DECLARATIONS ****************************************************/


/** Description of a single plot scale label.
 */
typedef struct
{
    PangoLayout* layout;           /**< pango layout representation of string */
    double world;                   /**< grid coordinate in world-coordinates */
    double grid;        /**< logical grid coordinate/position in drawing area */
    int pos;              /**< start position of label string in drawing area */
} PLOT_LABEL;


/** Internal axis (working) structure.
 */
typedef struct
{
    PLOT_AXIS *pAxis;                   /**< Pointer to plot axis description */
    int start, stop;                        /**< start and end-point of graph */
    double ratio;                         /**< GDK to world-coordinates ratio */
    double delta;   /**< world-coordinate step (depends on number of samples) */
    PangoLayout *layout; /**< Layout of axis name (NULL if axis without name) */
    int width;               /**< Width of axis name (0 if axis without name) */
    int pos;                                  /**< Position of labels at axis */
    int maxw;            /**< Maximum width of all label strings at this axis */
    PLOT_LABEL labels[PLOT_LABELS_MAX];              /**< List of plot labels */
} PLOT_AXIS_WORKSPACE;


/** Plot style draw function.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param pY           Pointer to y-axis workspace.
 *  \param lastFlags    Properties of last sample point.
 *  \param curFlags     Properties of current sample point.
 *  \param x            x coordinate.
 *  \param y            y coordinate.
 *  \param size         x/y extent of the point pixmap (e.g. radius).
 *
 */
typedef void (*PLOT_FUNC_DRAW)(cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                               unsigned lastFlags, unsigned curFlags,
                               int x, int y, int size);



/* LOCAL VARIABLE DEFINITIONS *************************************************/


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* MACRO **********************************************************************/
/** Sets a color from \p colors array into \e Cairo context.
 *
 *  \note               Do not use the function gdk_cairo_set_source_color(),
 *                      because it does'nt work.
 *
 *  \param cr           \e Cairo drawing context.
 *  \param colors       Pointer to array of GDK colors indexed by PLOT_COLOR.
 *                      If NULL, the color is unchanged.
 *  \param index        Identifies the color (PLOT_COLOR) to be changed.
 *
 ******************************************************************************/
#define PLOT_COLOR_SET(cr, colors, index)                               \
    if ((colors) != NULL)                                               \
    {                                                                   \
        cairo_set_source_rgb ((cr), (colors)[(index)].red / 65535.0,    \
                              (colors)[(index)].green / 65535.0,        \
                              (colors)[(index)].blue / 65535.0);        \
    }


/* LOCAL FUNCTION DECLARATIONS ************************************************/


static int drawErrorMsg (cairo_t* cr, PLOT_DIAG *pDiag, int errcode);
static int callInitFunc (PLOT_DIAG *pDiag, PLOT_AXIS_WORKSPACE *pX);
static void callEndFunc (PLOT_DIAG *pDiag);
static int callProgressFunc (PLOT_DIAG *pDiag, int cnt, int num);
static double getUnitFactor (PLOT_AXIS *pAxis);
static double w2cRatio(PLOT_AXIS *pAxis, int start, int stop);
static int searchMinMaxY (PLOT_DIAG* pDiag, PLOT_AXIS_WORKSPACE *pX);
static double w2c (PLOT_AXIS_WORKSPACE *p, double coordinate);
static double searchNearestLin (double mantissa);
static PangoRectangle createAxisLabel (cairo_t *cr, int precision,
                                       double divider, PLOT_LABEL *label);
static int scaleLin (cairo_t *cr, PLOT_AXIS_WORKSPACE *p, int margin, BOOL vertical);
static int scaleLog(cairo_t *cr, PLOT_AXIS_WORKSPACE *p, int margin, BOOL vertical);
static void drawLayout(cairo_t *cr, PangoLayout* layout, int x, int y);
static void createAxisNameLayout(cairo_t *cr, PLOT_AXIS_WORKSPACE *p);
static BOOL insertLabel(cairo_t *cr, PLOT_AXIS_WORKSPACE *p, int margin,
                        BOOL vertical, double coordinate, int idx);
static void drawGridLabels (cairo_t *cr, GdkColor *colors,
                            int numx, PLOT_AXIS_WORKSPACE* pX,
                            int numy, PLOT_AXIS_WORKSPACE* pY);
static int drawGraph (cairo_t *cr, int refsize, PLOT_DIAG *pDiag,
                      PLOT_AXIS_WORKSPACE *pX, PLOT_AXIS_WORKSPACE *pY);
static void drawStyleCircleOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                                 unsigned lastFlags, unsigned curFlags,
                                 int x, int y, int size);
static void drawStyleCircleSample (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                                   unsigned lastFlags, unsigned curFlags,
                                   int x, int y, int size);
static void drawStyleLineOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                               unsigned lastFlags, unsigned curFlags,
                               int x, int y, int size);
static void drawStyleCrossOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                                unsigned lastFlags, unsigned curFlags,
                                int x, int y, int size);
static void drawStyleBoxOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                              unsigned lastFlags, unsigned curFlags,
                              int x, int y, int size);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** If \p errcode indicates an error then the function draws a message into the
 *  plot area.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param pDiag        Pointer to plot parameters.
 *  \param errcode      Error code if less than zero.
 *
 *  \return             The error code passed in by \p errcode.
 ******************************************************************************/
static int drawErrorMsg (cairo_t* cr, PLOT_DIAG *pDiag, int errcode)
{
    if ((errcode < 0) && (pDiag->area.width >= 20))
    {
        PangoRectangle rect;

        PangoLayout* layout = pango_cairo_create_layout (cr);
        gchar *message =
            g_strdup_printf (_("<b>Cannot draw the plot.</b>\n\n"
                               "<small>Maybe memory space is exhausted, there are too many"
                               " sample points or a mathematical operation has failed."
                               " Change the start and/or endpoint of ordinate"
                               " to circumvant this situation.</small>"));

        pango_layout_set_width (layout, (pDiag->area.width - 20) * PANGO_SCALE);
        pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
        pango_layout_set_markup (layout, message, -1);
        pango_layout_get_extents (layout, NULL, &rect);
        drawLayout (cr, layout,
                    pDiag->area.x - 10 + (pDiag->area.width - PANGO_PIXELS (rect.width)) / 2,
                    pDiag->area.y - 10 + (pDiag->area.height - PANGO_PIXELS (rect.height)) / 2);
        g_free (message);
    } /* if */

    return errcode;
} /* drawErrorMsg() */


/* FUNCTION *******************************************************************/
/** Returns the axis unit multiplier.
 *
 *  \param pAxis        Pointer to axis description.
 *
 *  \return             Unit multiplier of axis.
 ******************************************************************************/
static double getUnitFactor(PLOT_AXIS *pAxis)
{
    double factor = 1.0;

    if (pAxis->pUnit != NULL)
    {
        factor = pAxis->pUnit->multiplier;
    } /* if */

    return factor;
} /* getUnitFactor() */



/* FUNCTION *******************************************************************/
/** Calculates the ratio of GDK and world-coordinates. The function calculates
 *  the ratio between logical and world-coordinates and returns them.
 *  The ratio is the difference of world coordinates \p stop-start in the
 *  linear case or \p log10(stop/start) in the logarithmic case.
 *
 *  \param pAxis        Pointer to axis description.
 *  \param start        Start point of plot graph (box) in GDK coordinates.
 *  \param stop         Stop point of plot graph (box) in GDK coordinates.
 *
 *  \return             The calculated ratio (e.g. for PLOT_AXIS_WORKSPACE).
 *
 ******************************************************************************/
static double w2cRatio(PLOT_AXIS *pAxis, int start, int stop)
{
    double ratio;

    int delta = stop - start;

    if (pAxis->flags & PLOT_AXIS_FLAG_LOG)
    {
        ratio = delta / log10(pAxis->stop / pAxis->start);
    } /* if */
    else
    {
        ratio = delta / (pAxis->stop - pAxis->start);
    } /* else */

    return ratio;
} /* w2cRatio() */



/* FUNCTION *******************************************************************/
/** Returns a logical coordinate associated with a world-coordinate.
 *
 *  \param p            Pointer to axis workspace.
 *  \param coordinate   Real-world coordinate.
 *
 *  \return             Logical coordinate within this axis. This is a double
 *                      because \e Cairo uses coordinates of this type.
 ******************************************************************************/
static double w2c(PLOT_AXIS_WORKSPACE *p, double coordinate)
{
    PLOT_AXIS *pAxis = p->pAxis;

    if (pAxis->flags & PLOT_AXIS_FLAG_LOG)
    {
        return p->start + p->ratio * log10(coordinate / pAxis->start);
    } /* if */

    return (coordinate - pAxis->start) * p->ratio + p->start;
} /* w2c() */



/* FUNCTION *******************************************************************/
/** Calls init-function.
 *
 *  \param pDiag        Pointer to plot descriptor.
 *  \param pX           Pointer to axis workspace.
 *
 *  \return             The function returns:
 *                      - the number of samples in interval \a start - \a stop
 *                      - a negative number, if there was a critical error.
 ******************************************************************************/
static int callInitFunc (PLOT_DIAG *pDiag, PLOT_AXIS_WORKSPACE *pX)
{
    int tmp, num = pDiag->num;

    if (num <= 0)
    {
        num = pX->stop - pX->start + 1;

        if (num < 0)                               /* too small drawing area? */
        {
            num = 0;
        } /* if */
    } /* if */

    if (pDiag->initFunc != NULL)
    {
        tmp = pDiag->initFunc (pDiag->x.start, pDiag->x.stop, pDiag->pData);

        if (tmp != 0)
        {
            num = tmp;
        } /* if */
    } /* if */

    return num;
} /* callInitFunc() */



/* FUNCTION *******************************************************************/
/** Calls end-function.
 *
 *  \param pDiag        Pointer to plot descriptor.
 *
 ******************************************************************************/
static void callEndFunc (PLOT_DIAG *pDiag)
{
    if (pDiag->endFunc != NULL)
    {
        pDiag->endFunc (pDiag->pData);
    } /* if */
} /* callEndFunc() */


/* FUNCTION *******************************************************************/
/** Calls the plot progress-function.
 *
 *  \param pDiag        Pointer to plot descriptor.
 *  \param cnt          Current counter value (while plotting): 0 <= cnt < num
 *  \param num          Number of samples (see \p cnt).
 *
 *  \return             See type PLOT_FUNC_PROGRESS in cairoPlot.h.
 ******************************************************************************/
static int callProgressFunc(PLOT_DIAG *pDiag, int cnt, int num)
{
    int ret = 0;

    if (pDiag->progressFunc != NULL)
    {
        ret = pDiag->progressFunc (pDiag->pData, (double)(++cnt) / num);

        if (ret != 0)
        {
            callEndFunc (pDiag);
        } /* if */
    } /* if */

    return ret;
} /* callProgressFunc() */



/* FUNCTION *******************************************************************/
/** Creates an axis name \e Pango layout.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param p            Axis workspace.
 *
 ******************************************************************************/
static void createAxisNameLayout(cairo_t *cr, PLOT_AXIS_WORKSPACE *p)
{
    char string[256];
    PangoRectangle rect;

    PLOT_AXIS *pAxis = p->pAxis;

    p->width = 0;                                              /* set default */
    p->layout = NULL;

    if (pAxis->name != NULL)
    {
        p->layout = pango_cairo_create_layout (cr);

        if (pAxis->pUnit != NULL)
        {
            g_snprintf (string, sizeof(string), PLOT_AXISNAME_FORMAT("%s", "%s"),
                        gettext (pAxis->name), pAxis->pUnit->name);
            pango_layout_set_markup (p->layout, string, -1);
        } /* if */
        else
        {
            pango_layout_set_markup (p->layout, gettext (pAxis->name), -1);
        } /* else */

        pango_layout_get_extents (p->layout, NULL, &rect);
        p->width = PANGO_PIXELS(rect.width);
    } /* if */

} /* createAxisNameLayout() */




/* FUNCTION *******************************************************************/
/** Draws a layout on the current \e Cairo path and frees it.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param layout       The \e Pango layout to draw.
 *  \param x            x-position of layout.
 *  \param y            y-position of layout.
 *
 ******************************************************************************/
static void drawLayout(cairo_t *cr, PangoLayout* layout, int x, int y)
{
    cairo_move_to (cr, x, y);
    pango_cairo_show_layout (cr, layout);
    g_object_unref (layout);
} /* drawLayout() */



/* FUNCTION *******************************************************************/
/** Creates an axis label object.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param precision    Output precision (number of digits).
 *  \param divider      Prescaling divider (normally from axis units).
 *  \param label        Pointer to label description.
 *
 *  \return             The enclosing rectangle of the axis-label (it's extent).
 *  \todo               Parametrize margin by precision.
 ******************************************************************************/
static PangoRectangle createAxisLabel(cairo_t *cr, int precision,
                                      double divider, PLOT_LABEL *label)
{
    PangoRectangle labelRect;
    char labelText[128];

    double value = label->world / divider;

    if (fabs(value) < PLOT_TOLERANCE)
    {
        value = 0.0;
    } /* if */

    /* At cairo level the function gtk_widget_create_pango_layout () isn't
     * available. Therefore create a layout from the cairo context.
     */
    label->layout = pango_cairo_create_layout (cr);
    g_snprintf (labelText, sizeof(labelText), "%.*G", precision, value);
    pango_layout_set_text (label->layout, labelText, -1);
    pango_layout_get_extents (label->layout, NULL, &labelRect);

    labelRect.width = PANGO_PIXELS(labelRect.width);
    labelRect.height = PANGO_PIXELS(labelRect.height);

    return labelRect;
} /* createAxisLabel() */



/* FUNCTION *******************************************************************/
/** Searches the nearest human (decimal) value.
 *
 *  \param mantissa     Mantissa of real-world coordinate (may be negative).
 *
 *  \return             The nearest mantissa found. Notice that the sign is
 *                      always positive, independent of that of \p mantissa.
 ******************************************************************************/
static double searchNearestLin(double mantissa)
{
    static double human[] =                  /* typical (human) scale delta's */
    {
        0.1, 0.2, 0.25, 0.5, 1.0, 2.0, 2.5, 5.0, 10.0
    };


    int i, idx = N_ELEMENTS(human) - 1;
    mantissa = fabs(mantissa);

    for (i = 0; i < N_ELEMENTS(human); i++)
    {
        if (human[i] >= mantissa)
        {
            idx = i;
            break;                                         /* end of for loop */
        } /* if */
    } /* for */


    return human[idx];
} /* searchNearestLin() */


/* FUNCTION *******************************************************************/
/** Inserts a label into associated array.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param p            Pointer to axis workspace.
 *  \param margin       Minimum margin between two labels.
 *  \param vertical     Set TRUE if the axis labels are stacked vertical (and
 *                      this distance shall be monitored). Else the horizontal
 *                      distance between labels is checked wrt. overlapping.
 *  \param coordinate   World-coordinate at which the label is placed.
 *  \param idx          Index into label array \p p->labels to be used.
 *
 *  \return             TRUE if there is not enough space between labels (bad
 *                      case), else FALSE (good case).
 ******************************************************************************/
static BOOL insertLabel(cairo_t *cr, PLOT_AXIS_WORKSPACE *p, int margin,
                        BOOL vertical, double coordinate, int idx)
{
    int size;
    PangoRectangle labelRect;                 /* enclosing rectangle of label */

    BOOL bad = FALSE;
    PLOT_LABEL* pLabel = &p->labels[idx];                 /* pointer to label */
    double divider = getUnitFactor (p->pAxis);

    pLabel->world = coordinate;
    labelRect = createAxisLabel (cr, p->pAxis->prec, divider, pLabel);
    pLabel->grid = w2c(p, coordinate);

    size = labelRect.width;

    if (size > p->maxw)                               /* search maximum width */
    {
        p->maxw = size;
    } /* if */

    if (vertical)
    {
        MATH_SWAP_INT (labelRect.height, labelRect.width);
    } /* if */

    size = labelRect.width / 2;                               /* half of size */
    pLabel->pos = pLabel->grid - size;      /* label strings are left aligned */

    if (idx > 0)                                   /* not for the first label */
    {
        PLOT_LABEL* pLast = &p->labels[idx - 1];            /* previous label */

        bad = abs(pLabel->grid - pLast->grid) <        /* check space between */
            (margin + size + abs(pLast->grid - pLast->pos));
    } /* if */

    return bad;
} /* insertLabel() */



/* FUNCTION *******************************************************************/
/** Calculates the label/grid points for a linear axis.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param p            Pointer to axis workspace.
 *  \param margin       Minimum margin between two labels.
 *  \param vertical     Set TRUE if the axis labels are stacked vertical (and
 *                      this distance shall be monitored). Else the horizontal
 *                      distance between labels is checked wrt. overlapping.
 *
 *  \return             The number of labels to be used for axis. A number less
 *                      than 1 (maybe negative) indicates that there no labels
 *                      to place or an error has occured.
 ******************************************************************************/
static int scaleLin(cairo_t *cr, PLOT_AXIS_WORKSPACE *p, int margin, BOOL vertical)
{
    BOOL bad;
    int num;                                      /* number of labels to draw */
    double coordinate, delta;
    MATH_NORMDBL norm;

    int try = PLOT_LABELS_MAX - 1; /* number of labels to try (start with max.) */
    PLOT_AXIS *pAxis = p->pAxis;
    double diff = pAxis->stop - pAxis->start;

    do              /* try to put a (decreasing) number of labels at the axis */
    {
        norm = mathNorm10(diff / try);
        norm.mantissa = searchNearestLin(norm.mantissa);
        delta = mathDenorm10(norm);
        coordinate = floor(pAxis->start / delta) * delta; /* first coordinate */

        if (coordinate < pAxis->start)
        {                                  /* increment while less than start */
            coordinate += delta;
        } /* if */

        bad = FALSE;
        num = 0;
        p->maxw = 0;

        while ((coordinate <= pAxis->stop) && (num <= try) && !bad)
        {
            bad = insertLabel (cr, p, margin, vertical, coordinate, num++);
            coordinate += delta;                           /* next grid point */
        } /* while */


        if (bad)                                      /* some labels overlap? */
        {
            while (num-- > 0)       /* free all pango layouts (label strings) */
            {
                g_object_unref (p->labels[num].layout);
            } /* while */
        } /* if */
    } /* do */
    while ((--try > 0) && bad);

    return num;
} /* scaleLin() */



/* FUNCTION *******************************************************************/
/** Calculates the label/grid points for a logarithmic axis.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param p            Pointer to axis workspace.
 *  \param margin       Minimum margin between two labels.
 *  \param vertical     Set TRUE if the axis labels are stacked vertical (and
 *                      this distance shall be monitored). Else the horizontal
 *                      distance between labels is checked wrt. overlapping.
 *
 *  \return             The number of labels to be used for axis. A number less
 *                      than 1 indicates that there no labels to place or an
 *                      error has occured.
 ******************************************************************************/
static int scaleLog(cairo_t *cr, PLOT_AXIS_WORKSPACE *p, int margin, BOOL vertical)
{
    static const int plotGridNum[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

    static const double plotGridPoint[N_ELEMENTS(plotGridNum)][PLOT_LABELS_MAX] =
    {
        {1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0, 6.0, 8.0},
        {1.0, 1.25, 1.75, 2.5, 3.0, 4.0, 5.5, 7.5},
        {1.0, 1.5, 2.0, 2.75, 3.75, 5.0, 7.0},
        {1.0, 1.5, 2.25, 3.5, 5.0, 7.5},
        {1.0, 1.5, 2.5, 4.0, 6.0},
        {1.0, 1.75, 3.0, 6.0},
        {1.0, 2.0, 5.0},
        {1.0, 3.0},
        {1.0}
    };

    BOOL bad;
    int num;                                      /* number of labels to draw */
    int idx;                                        /* current index in scale */
    double coordinate;                                  /* current coordinate */
    MATH_NORMDBL norm;

    PLOT_AXIS *pAxis = p->pAxis;
    int scale = 0;

    do              /* try to put a (decreasing) number of labels at the axis */
    {
        bad = FALSE;
        idx = num = 0;
        p->maxw = 0;

        norm = mathNorm10(pAxis->start);
        norm.mantissa = plotGridPoint[scale][0];
        coordinate = mathDenorm10(norm);

        while (coordinate < pAxis->start)
        {                                  /* increment while less than start */
            if (++idx >= plotGridNum[scale])
            {
                idx = 0;                                       /* next decade */
                ++norm.exponent;
            } /* if */

            norm.mantissa = plotGridPoint[scale][idx];
            coordinate = mathDenorm10(norm);
        } /* if */


        while ((coordinate <= pAxis->stop) && (num < PLOT_LABELS_MAX) && !bad)
        {
            bad = insertLabel (cr, p, margin, vertical, coordinate, num++);

            if (++idx >= plotGridNum[scale])               /* next grid point */
            {
                idx = 0;
                ++norm.exponent;
            } /* if */

            norm.mantissa = plotGridPoint[scale][idx];
            coordinate = mathDenorm10 (norm);
        } /* while */


        if (bad)                                      /* some labels overlap? */
        {
            while (num-- > 0)       /* free all pango layouts (label strings) */
            {
                g_object_unref (p->labels[num].layout);
            } /* while */
        } /* if */
    } /* do */
    while ((++scale < N_ELEMENTS(plotGridNum)) && bad);

    return num;
} /* scaleLog() */



/* FUNCTION *******************************************************************/
/** Draws all the grid and axis labels wrt. associated colors. Notice that
 *  drawGridLabels() performs its own cairo_stroke() and modifies the color
 *  in \e Cairo drawing context if \p colors is unequal to NULL.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param colors       Pointer to array of GDK colors indexed by PLOT_COLOR.
 *                      If NULL, the current (default) colors are used.
 *  \param numx         Number of labels at x-axis.
 *  \param pX           Pointer to x-axis workspace.
 *  \param numy         Number of labels at y-axis.
 *  \param pY           Pointer to y-axis workspace.
 *
 *  \return             0 on success, else an error number from errno.h.
 ******************************************************************************/
static void drawGridLabels (cairo_t *cr, GdkColor *colors,
                            int numx, PLOT_AXIS_WORKSPACE* pX,
                            int numy, PLOT_AXIS_WORKSPACE* pY)
{
    static double dashes[] = {PLOT_GRID_DASH_LEN, PLOT_GRID_DASH_LEN};

    int i;
    PLOT_LABEL *pLabel;

    /* Set the color into Cairo context (using cairo_set_source_rgb) before
     * calling pango_cairo_show_layout(). It seems that a call to cairo_stroke
     * isn't needed, may be its implemented in pango_cairo_show_layout().
     */
    PLOT_COLOR_SET (cr, colors, PLOT_COLOR_LABELS);

    for (i = 0, pLabel = pX->labels; i < numx; i++, pLabel++)
    {                                                        /* x-axis labels */
        drawLayout (cr, pLabel->layout, pLabel->pos, pX->pos);
    } /* for */


    for (i = 0, pLabel = pY->labels; i < numy; i++, pLabel++)
    {                                                        /* y-axis labels */
        drawLayout (cr, pLabel->layout, pY->pos, pLabel->pos);
    } /* for */


    i = 0;                                      /* indicates any grid drawing */

    if (pX->pAxis->flags & PLOT_AXIS_FLAG_GRID)
    {
        for (i = 0, pLabel = pX->labels; i < numx; i++, pLabel++)
        {                                                      /* x-axis grid */
            cairo_move_to (cr, pLabel->grid, pY->start);
            cairo_line_to (cr, pLabel->grid, pY->stop);
        } /* for */
    } /* if */


    if (pY->pAxis->flags & PLOT_AXIS_FLAG_GRID)
    {
        for (i = 0, pLabel = pY->labels; i < numy; i++, pLabel++)
        {                                                      /* y-axis grid */
            cairo_move_to (cr, pX->start, pLabel->grid);
            cairo_line_to (cr, pX->stop, pLabel->grid);
        } /* for */
    } /* if */

    if (i > 0)                                             /* any grid drawn? */
    {
        PLOT_COLOR_SET (cr, colors, PLOT_COLOR_GRID);
        cairo_set_dash (cr, dashes, N_ELEMENTS(dashes), 0.0);
        cairo_set_line_width (cr, PLOT_GRID_LINE_WIDTH);
        cairo_stroke (cr);
    } /* if */


    for (i = 0, pLabel = pX->labels; i < numx; i++, pLabel++)
    {                                                        /* label markers */
        cairo_move_to (cr, pLabel->grid, pY->start);
        cairo_line_to (cr, pLabel->grid, pY->start - PLOT_LABEL_MARKER_LEN);

        if (pX->pAxis->flags & PLOT_AXIS_FLAG_GRID)
        {
            cairo_move_to (cr, pLabel->grid, pY->stop);
            cairo_line_to (cr, pLabel->grid, pY->stop + PLOT_LABEL_MARKER_LEN);
        } /* if */
    } /* for */


    for (i = 0, pLabel = pY->labels; i < numy; i++, pLabel++)
    {
        cairo_move_to (cr, pX->start, pLabel->grid);
        cairo_line_to (cr, pX->start + PLOT_LABEL_MARKER_LEN, pLabel->grid);

        if (pY->pAxis->flags & PLOT_AXIS_FLAG_GRID)
        {
            cairo_move_to (cr, pX->stop, pLabel->grid);
            cairo_line_to (cr, pX->stop - PLOT_LABEL_MARKER_LEN, pLabel->grid);
        } /* if */
    } /* for */

    PLOT_COLOR_SET (cr, colors, PLOT_COLOR_BOX);
    cairo_set_dash (cr, dashes, 0, 0);                           /* no dashes */
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
    cairo_set_line_width (cr, PLOT_LABEL_MARKER_WIDTH);
    cairo_stroke (cr);

} /* drawGridLabels() */



/* FUNCTION *******************************************************************/
/** The function tries to find the minimum and maximum y-coordinates used for
 *  auto-scaling.
 *
 *  \param pDiag        Pointer to plot descriptor. The members \a pDiag->y.start
 *                      and \a pDiag->y.stop are modified on success.
 *  \param pX           Pointer to x-axis workspace.
 *
 *  \return             0 on success, else an error number from errno.h.
 ******************************************************************************/
static int searchMinMaxY (PLOT_DIAG* pDiag, PLOT_AXIS_WORKSPACE *pX)
{
    int i;

    double y, x = pDiag->x.start;
    double minY = DBL_MAX;
    double maxY = DBL_MIN;
    double delta = PLOT_AXIS_MAX - PLOT_AXIS_MIN;

    int num = callInitFunc (pDiag, pX);

    if (num < 0)
    {
        return ENOMEM;
    } /* if */

    if (num > 1)
    {
        delta = (pDiag->x.stop - pDiag->x.start) / (num - 1);
    } /* if */


    for (i = 0; i < num; i++)                   /* try to process all samples */
    {
        y = pDiag->sampleFunc(&x, pDiag->pData);

        if (gsl_finite (y))
        {                                            /* only if y value exist */
            minY = GSL_MIN_DBL(minY, y);
            maxY = GSL_MAX_DBL(maxY, y);
        } /* if */

        if (callProgressFunc(pDiag, i, num))
        {
            return 0;
        } /* if */

        x += delta;
    } /* for */

    callEndFunc (pDiag);

    if (minY < maxY)                                    /* any valid values ? */
    {
        pDiag->y.start = minY;
        pDiag->y.stop = maxY;
    } /* if */

    return 0;
} /* searchMinMaxY() */



/* FUNCTION *******************************************************************/
/** Plot style draw function for PLOT_STYLE_CIRCLE_ONLY.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param pY           Pointer to y-axis workspace.
 *  \param lastFlags    Properties of last sample point.
 *  \param curFlags     Properties of current sample point.
 *  \param x            x coordinate.
 *  \param y            y coordinate (may be clamped to boundaries).
 *  \param size         x/y extent of the point pixmap (e.g. radius).
 *
 ******************************************************************************/
static void drawStyleCircleOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                                 unsigned lastFlags, unsigned curFlags,
                                 int x, int y, int size)
{
    cairo_move_to (cr, x, y + size);                     /* avoid line to arc */

    if (!curFlags)                         /* check preconditions for drawing */
    {
        cairo_arc(cr, x, y, size, -3 * M_PI_2, M_PI_2);
    } /* if */
} /* drawStyleCircleOnly() */


/* FUNCTION *******************************************************************/
/** Plot style draw function for PLOT_STYLE_CIRCLE_SAMPLE.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param pY           Pointer to y-axis workspace.
 *  \param lastFlags    Properties of last sample point.
 *  \param curFlags     Properties of current sample point.
 *  \param x            x coordinate.
 *  \param y            y coordinate.
 *  \param size         x/y extent of the point pixmap (e.g. radius).
 *
 ******************************************************************************/
static void drawStyleCircleSample (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                                   unsigned lastFlags, unsigned curFlags,
                                   int x, int y, int size)
{
    PLOT_AXIS *pAxisY = pY->pAxis;
    int ybase = pY->start;              /* base point of sample line (bottom) */

    if (!(curFlags & PLOT_FLAG_INVALID))   /* check preconditions for drawing */
    {
        /* Because a logarithmic axis always has start and stop greater than
         * zero, the following steps can be skipped (and this avoids a call to
         * w2c with argument 0)
         */
        if (!(pAxisY->flags & PLOT_AXIS_FLAG_LOG))
        {
            if (pAxisY->stop < 0.0)
            {
                ybase = pY->stop;
            } /* if */
            else
            {
                if (pAxisY->start < 0.0)
                {
                    ybase = w2c (pY, 0);
                } /* if */
            } /* else */
        } /* if */


        if (curFlags & PLOT_FLAG_OUTSIDE)
        {
            size = 0;      /* no circle to be drawn -> don't regard it's size */
        } /* if */

        drawStyleCircleOnly (cr, pY, lastFlags, curFlags, x, y, size);

        if (abs (y - ybase) >= size)         /* should we really draw a line? */
        {
            if (ybase < y)
            {
                cairo_move_to (cr, x, y - size); /* avoid line through circle */
            } /* if */

            cairo_line_to (cr, x, ybase);
        } /* if */
    } /* if */
} /* drawStyleCircleSample() */


/* FUNCTION *******************************************************************/
/** Plot style draw function for PLOT_STYLE_LINE_ONLY.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param pY           Pointer to y-axis workspace.
 *  \param lastFlags    Properties of last sample point.
 *  \param curFlags     Properties of current sample point.
 *  \param x            x coordinate.
 *  \param y            y coordinate.
 *  \param size         x/y extent of the point pixmap (e.g. radius).
 *
 ******************************************************************************/
static void drawStyleLineOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                               unsigned lastFlags, unsigned curFlags,
                               int x, int y, int size)
{
    if ((lastFlags & PLOT_FLAG_INVALID) ||     /* last point was invalid? */
        (lastFlags & curFlags & PLOT_FLAG_OUTSIDE))
    {
        cairo_move_to (cr, x, y);
    } /* if */
    else
    {
        cairo_line_to (cr, x, y);
    } /* else */
} /* drawStyleLineOnly() */


/* FUNCTION *******************************************************************/
/** Plot style draw function for PLOT_STYLE_CROSS_ONLY.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param pY           Pointer to y-axis workspace.
 *  \param lastFlags    Properties of last sample point.
 *  \param curFlags     Properties of current sample point.
 *  \param x            x coordinate.
 *  \param y            y coordinate.
 *  \param size         x/y extent of the point pixmap (e.g. radius).
 *
 ******************************************************************************/
static void drawStyleCrossOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                                unsigned lastFlags, unsigned curFlags,
                                int x, int y, int size)
{
    if (!curFlags)                         /* check preconditions for drawing */
    {
        cairo_move_to (cr, x - size, y - size);
        cairo_line_to (cr, x + size, y + size);
        cairo_move_to (cr, x + size, y - size);
        cairo_line_to (cr, x - size, y + size);
    } /* if */
} /* drawStyleCrossOnly() */


/* FUNCTION *******************************************************************/
/** Plot style draw function for PLOT_STYLE_BOX_ONLY.
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param pY           Pointer to y-axis workspace.
 *  \param lastFlags    Properties of last sample point.
 *  \param curFlags     Properties of current sample point.
 *  \param x            x coordinate.
 *  \param y            y coordinate.
 *  \param size         x/y extent of the point pixmap (e.g. radius).
 *
 ******************************************************************************/
static void drawStyleBoxOnly (cairo_t* cr, PLOT_AXIS_WORKSPACE* pY,
                              unsigned lastFlags, unsigned curFlags,
                              int x, int y, int size)
{
    if (!curFlags)                         /* check preconditions for drawing */
    {
        cairo_rectangle(cr, x - size, y - size, 2 *size, 2 * size);
    } /* if */
} /* drawStyleBoxOnly() */



/* FUNCTION *******************************************************************/
/** Draws a continuous graph on a \e Cairo context.
 *
 *
 *  \param cr           \e Cairo context for drawing.
 *  \param size         x/y extent of the point pixmap (e.g. radius).
 *  \param pDiag        Pointer to plot diagram descriptor.
 *  \param pX           Pointer to x-axis workspace.
 *  \param pY           Pointer to y-axis workspace.
 *
 *  \return             The number of samples taken to draw this plot
 *                      (independent of a possible break) or a negative
 *                      number on error.
 ******************************************************************************/
static int drawGraph (cairo_t *cr, int size, PLOT_DIAG *pDiag,
                      PLOT_AXIS_WORKSPACE *pX, PLOT_AXIS_WORKSPACE *pY)
{
    static const PLOT_FUNC_DRAW drawFunc[] =
    {
        drawStyleLineOnly,                            /* PLOT_STYLE_LINE_ONLY */
        drawStyleCircleOnly,                        /* PLOT_STYLE_CIRCLE_ONLY */
        drawStyleCircleSample,                    /* PLOT_STYLE_CIRCLE_SAMPLE */
        drawStyleCrossOnly,                          /* PLOT_STYLE_CROSS_ONLY */
        drawStyleBoxOnly                               /* PLOT_STYLE_BOX_ONLY */
    };

    int cx, cy;                           /* logical (user space) coordinates */
    double y, x = pDiag->x.start;                        /* world coordinates */
    unsigned lastFlags, curFlags = PLOT_FLAG_INVALID; /* properties of current/last point */
    double delta = PLOT_AXIS_MAX - PLOT_AXIS_MIN;
    int i, num = callInitFunc (pDiag, pX);

    if (num < 0)
    {
        return num;
    } /* if */

    if (num > 1)
    {
        delta = (pDiag->x.stop - pDiag->x.start) / (num - 1);
    } /* if */


    for (i = 0; i < num; i++)                              /* for all samples */
    {
        y = pDiag->sampleFunc(&x, pDiag->pData);
        lastFlags = curFlags;                         /* make current to last */

        if (gsl_finite (y))                      /* if no singularity draw it */
        {
            curFlags = 0;

            if (y < pDiag->y.start - PLOT_TOLERANCE)               /* check y */
            {
                cy = pY->start;           /* regard swapped start/stop values */
                curFlags |= PLOT_FLAG_OUTSIDE;
            } /* if */
            else
            {
                if (y > pDiag->y.stop + PLOT_TOLERANCE)
                {
                    curFlags |= PLOT_FLAG_OUTSIDE;
                    cy = pY->stop;        /* regard swapped start/stop values */
                } /* if */
                else
                {
                    cy = w2c(pY, y);                  /* logical y-coordinate */
                } /* else */
            } /* /else */


            if (x < pDiag->x.start - PLOT_TOLERANCE)               /* check x */
            {
                cx = pX->start;
                curFlags |= PLOT_FLAG_OUTSIDE;
            } /* if */
            else
            {
                if (x > pDiag->x.stop + PLOT_TOLERANCE)
                {
                    curFlags |= PLOT_FLAG_OUTSIDE;
                    cx = pX->stop;
                } /* if */
                else
                {
                    cx = w2c(pX, x);                  /* logical x-coordinate */
                } /* else */
            } /* /else */

            ASSERT(pDiag->style < N_ELEMENTS(drawFunc));
            drawFunc[pDiag->style](cr, pY, lastFlags, curFlags, cx, cy, size);
        } /* if */
        else
        {
            curFlags = PLOT_FLAG_INVALID;
        } /* else */

        if (callProgressFunc(pDiag, i, num))
        {
            return num;
        } /* if */

        x += delta;
    } /* for */

    callEndFunc (pDiag);

    return num;
} /* drawGraph() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


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
 *                      - gtk_print_context_get_cairo_context().
 *                      Notice, that the \e Cairo context is preserved (nothing
 *                      chenged).
 *  \param pDiag        Pointer to plot descriptor.
 *
 *  \return             0 on success, else an error number from errno.h.
 *  \todo               Make axisX.start and axisX.stop positions dependent of
 *                      number of digits. Especially think about the corrections
 *                      of axisX.stop (which is an estimation at the moment).
 ******************************************************************************/
int cairoPlot2d(cairo_t* cr, PLOT_DIAG *pDiag)
{
    int any, numx, numy;                  /* number of label strings per axis */
    PLOT_AXIS_WORKSPACE axisX, axisY;
    cairo_font_extents_t refsize;
    int width, height;                                        /* font extents */

    int ret = 0;

    axisX.pAxis = &pDiag->x;
    axisY.pAxis = &pDiag->y;

    (void)cairoPlotChkRange (&pDiag->x);
    (void)cairoPlotChkRange (&pDiag->y);

    cairo_font_extents (cr, &refsize);
    height = (int)refsize.height + 2;
    width = (int)refsize.max_x_advance + 2;

    axisX.start = pDiag->area.x + width;
    axisX.stop = pDiag->area.x + pDiag->area.width - width;
    axisY.start = pDiag->area.y + height;
    axisY.stop = pDiag->area.y + pDiag->area.height - 2 * height;

    if (pDiag->y.name != NULL)
    {
        axisY.start += 5 * height / 2;   /* reserve space for name of y-axis */
    } /* if */

    if (pDiag->x.name != NULL)
    {                                    /* reserve space for name of x-axis */
        axisY.stop -= 3 * height / 2;  /* expand by height + 1/2 extra space */
    } /* if */

    if ((axisX.stop > axisX.start) && (axisY.stop > axisY.start))
    {
        cairo_save(cr);

        axisX.ratio = w2cRatio (&pDiag->x, axisX.start, axisX.stop); /* preliminary */
        any = pDiag->y.flags & PLOT_AXIS_FLAG_AUTO;          /* auto scaling? */

        /* Try to find the minimum and maximum y-coordinates for auto-scaling.
         */
        while (any)           /* do it twice if log. y-axis changed to linear */
        {
            if (searchMinMaxY (pDiag, &axisX))
            {
                return drawErrorMsg (cr, pDiag, -1);
            } /* if */

            any = cairoPlotChkRange (&pDiag->y);

            if (any && (pDiag->y.flags & PLOT_AXIS_FLAG_LOG))
            {
                pDiag->y.flags &= ~PLOT_AXIS_FLAG_LOG; /* change to linear axis */
            } /* if */
            else
            {
                any = FALSE;
            } /* else */
        } /* while */


        axisX.pos = axisY.stop + height / 2;            /* position of labels */

        /* Change y-coordinate for equal handling (notice the cairo context has
         * south-east orientation).
         */
        MATH_SWAP_INT (axisY.start, axisY.stop);
        axisY.ratio = w2cRatio (&pDiag->y, axisY.start, axisY.stop);

        if (pDiag->y.flags & PLOT_AXIS_FLAG_LOG)       /* logarithmic y-axis? */
        {
            numy = scaleLog (cr, &axisY, height, TRUE);
        } /* if */
        else                                                 /* linear y-axis */
        {
            numy = scaleLin (cr, &axisY, height, TRUE);
        } /* else */

        axisY.pos = axisX.start;

        createAxisNameLayout (cr, &axisX);     /* create axis names (or NULL) */
        createAxisNameLayout (cr, &axisY);

        /* Note: For the following corrections axisX.maxw cannot be used,
         *       because it is valid only after scaleLin() for the x-axis.
         */
        axisX.start += MAX (axisY.maxw + width / 2, axisY.width / 2); 
        axisX.stop -= MAX (2 * width, axisX.width / 2);

        if ((axisX.stop > axisX.start) && (axisY.start > axisY.stop))
        {
            /* calculate x-ratio (again)
             */
            axisX.ratio = w2cRatio (&pDiag->x, axisX.start, axisX.stop);
            cairo_set_tolerance (cr, 1.0);       /* speed path calculation up */
            PLOT_COLOR_SET (cr, pDiag->colors, PLOT_COLOR_AXIS_NAME);

            if (axisX.layout != NULL)
            {
                drawLayout (cr, axisX.layout, axisX.stop - axisX.width / 2,
                            axisX.pos + 3 * height / 2);
            } /* if */

            if (axisY.layout != NULL)
            {
                /* put the name of y-axis 2.5 times the line height up the axis
                 * starting point (so regarding subscripts in the axis name)
                 */
                drawLayout (cr, axisY.layout, axisX.start - axisY.width / 2,
                            axisY.stop - 5 * height / 2);
            } /* if */


            if (pDiag->x.flags & PLOT_AXIS_FLAG_LOG)   /* logarithmic x-axis? */
            {
                numx = scaleLog (cr, &axisX, 2 * width, FALSE);
            } /* if */
            else                                             /* linear x-axis */
            {
                numx = scaleLin (cr, &axisX, 2 * width, FALSE);
            } /* else */

            pDiag->area.x = axisX.start;             /* prepare return values */
            pDiag->area.y = axisY.stop;
            pDiag->area.width = axisX.stop - axisX.start;
            pDiag->area.height = axisY.start - axisY.stop;

            /* Now draw grid and labels with associated colors. Notice that
             * drawGridLabels() performs its own cairo_stroke() and modifies
             * some colors in \e Cairo drawing context.
             */
            drawGridLabels (cr, pDiag->colors, numx, &axisX, numy, &axisY);

            cairo_rectangle(cr, axisX.start, axisY.stop,
                            pDiag->area.width, pDiag->area.height);
            PLOT_COLOR_SET (cr, pDiag->colors, PLOT_COLOR_BOX);
            cairo_set_line_width (cr, PLOT_BOX_LINE_WIDTH);
            cairo_stroke_preserve (cr);       /* retain the path for clipping */
            cairo_clip (cr);                                 /* clip the box  */

            ret = drawGraph (cr, height / 3, pDiag, &axisX, &axisY);

            PLOT_COLOR_SET (cr, pDiag->colors, PLOT_COLOR_GRAPH);
            cairo_set_line_width (cr,  pDiag->thickness);
            cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);
            cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
            /* cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE); */
            cairo_stroke (cr);
        } /* if */
        else                              /* drawing area for graph too small */
        {
            g_object_unref (axisX.layout);          /* free axis name layouts */
            g_object_unref (axisY.layout);

            while (--numy >= 0)                  /* free y-axis label layouts */
            {
                g_object_unref (axisY.labels[numy].layout);
            } /* for */
        } /* else */

        cairo_restore (cr);

    } /* if */


    return drawErrorMsg (cr, pDiag, ret);
} /* cairoPlot2d() */


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
int cairoPlotChkRange (PLOT_AXIS *pAxis)
{
    double result;
    int ret = 0;


    if (pAxis->flags & PLOT_AXIS_FLAG_LOG)                /* logarithmic axis */
    {
        pAxis->start = CLAMP (pAxis->start, PLOT_TOLERANCE, PLOT_AXIS_MAX);
        pAxis->stop = CLAMP (pAxis->stop, PLOT_TOLERANCE, PLOT_AXIS_MAX);

        result = mathTryDiv(pAxis->start, pAxis->stop);

        if (gsl_isinf(result) || (result  < PLOT_TOLERANCE))
        {
            pAxis->start = pAxis->stop * PLOT_TOLERANCE;
            ret = ERANGE;
        } /* if */

        result = mathTryDiv(pAxis->stop, pAxis->start);

        if (gsl_isinf(result) || (result  < (1.0 + PLOT_TOLERANCE)))
        {
            pAxis->stop = pAxis->start * (1.0 + PLOT_TOLERANCE);
            ret = ERANGE;
        } /* if */
    } /* if */
    else                                                       /* linear axis */
    {
        pAxis->start = CLAMP (pAxis->start, PLOT_AXIS_MIN, PLOT_AXIS_MAX);
        pAxis->stop = CLAMP (pAxis->stop, PLOT_AXIS_MIN, PLOT_AXIS_MAX);

        if ((pAxis->stop - pAxis->start) < PLOT_TOLERANCE)
        {
            pAxis->stop = pAxis->start + PLOT_TOLERANCE;
            ret = ERANGE;
        } /* if */
    } /* else */

    return ret;
} /* cairoPlotChkRange() */



/* FUNCTION *******************************************************************/
/** Returns a world-coordinate associated with a GDK coordinate.
 *
 *  \note  Because cairoPlotChkRange() has checked the ranges \p pAxis->start
 *         and \p pAxis->stop with respect to the operation \p pAxis->stop -
 *         \p pAxis->start (lin. case) and \p pAxis->stop / \p pAxis->start
 *         (log. case) there is no need to use math. function mathTryDiv().
 *
 *  \param pAxis        Pointer to axis description (filled from a previuous
 *                      call to cairoPlot2d).
 *  \param start        Start point of plot graph (box) in GDK coordinates.
 *  \param stop         Stop point of plot graph (box) in GDK coordinates.
 *  \param coordinate   The GDK coordinate to be transformed into world.
 *
 *  \attention          Due to internal processing optimization (associated
 *                      with the south-east orientation of y-axis) the
 *                      calculation of y-coordinates gives correct results
 *                      only when \p start and \p stop are exchanged.
 *
 *  \return             World-coordinate within this axis.
 ******************************************************************************/
double cairoPlotCoordinate (PLOT_AXIS *pAxis, int start, int stop, int coordinate)
{
    double offset = (coordinate - start) / w2cRatio (pAxis, start, stop);

    if (pAxis->flags & PLOT_AXIS_FLAG_LOG)                      /* log. axis? */
    {
        return pAxis->start * POW10(CLAMP (offset, 0.0,
                                           log10 (pAxis->stop / pAxis->start)));
    } /* if */

    return pAxis->start + offset;                              /* linear case */
} /* cairoPlotCoordinate() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
