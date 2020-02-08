/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Roots plot functions.
 *
 * \author   Copyright (C) 2006, 2011, 2018, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include <errno.h>

#include "dfcProject.h"
#include "rootsPlot.h"
#include "cairoPlot.h"
#include "cfgSettings.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/


static int plotRootIndex;                     /**< Index of next root to plot */
static GtkWidget *plotDrawable;        /**< Roots plot drawable widget handle */


/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static int rootsPlotInit (double start, double stop, void *pData);
static double plotRoot (double *px, void *pData);
static double returnZeroRoot (double *px, void *pData);
static void updateRootsMinMax (MATHPOLY *poly, gsl_complex *rmin, gsl_complex *rmax);
static int calcRoots (MATHPOLY *poly);
static gboolean rootsPlotExposeHandler (GtkWidget *widget, GdkEventExpose *event,
                                        gpointer user_data);
static double plotUnitCircleBottom (double *px, void *pData);
static double plotUnitCircleTop (double *px, void *pData);



/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Initialization function for a roots plot. This function must be of type
 *  PLOT_FUNC_INIT from cairoPlot.h (see there).
 *
 *  \param start        The real-world start x-coordinate.
 *  \param stop         The real-world stop x-coordinate.
 *  \param pData        Pointer to polynomial (MATHPOLY) for which the roots
 *                      shall be plotted. If \p pData is NULL the function
 *                      returns -1.
 *
 *  \return  The function returns:
 *           - normally the degree of polynomial referenced by \p pData
 *           - a negative number, if \p pData is NULL.
 ******************************************************************************/
static int rootsPlotInit (double start, double stop, void *pData)
{
    MATHPOLY *poly = pData;

    if (poly == NULL)
    {
        return -1;
    } /* if */

    plotRootIndex = 0;

    return poly->degree;
} /* rootsPlotInit() */


/* FUNCTION *******************************************************************/
/** Plot function for a polynomial root. This function must be of type
 *  PLOT_FUNC_GET from cairoPlot.h (see there).
 *
 *  \param px           Pointer to buffer, which gets the x-coordinate (real
 *                      part) of the next root.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. Here it
 *                      must point to the polynomial (MATHPOLY) for which the
 *                      next root shall be plotted.
 *
 *  \return             The y-coordinate (imag. part) of the next root. If there
 *                      is no value at \p px, then it returns GSL_POSINF.
 ******************************************************************************/
static double plotRoot (double *px, void *pData)
{
    double y;
    MATHPOLY *poly = pData;

    if (plotRootIndex >= poly->degree)                        /* sanity check */
    {
        return GSL_POSINF;
    } /* if */

    *px = GSL_REAL (poly->root[plotRootIndex]);
    y = GSL_IMAG (poly->root[plotRootIndex]);
    ++plotRootIndex;

    return y;
} /* plotRoot() */



/* FUNCTION *******************************************************************/
/** Plot function for a polynomial root at 0,0. This function must be of type
 *  PLOT_FUNC_GET from cairoPlot.h (see there).
 *
 *  \param px           Pointer to buffer, which gets the x-coordinate (real
 *                      part) of the root.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. Here it
 *                      is unused, because return ZeroRoot() always returns 0,0.
 *
 *  \return             The y-coordinate (imag. part) of the next root (always 0).
 ******************************************************************************/
static double returnZeroRoot (double *px, void *pData)
{
    *px = 0.0;

    return 0.0;
} /* returnZeroRoot() */



/* FUNCTION *******************************************************************/
/** Plot function for a north-oriented semicircle with unity radius. This
 *  function must be of type PLOT_FUNC_GET from cairoPlot.h (see there).
 *
 *  \param px           Pointer to real-world x-coordinate.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. Here it is
 *                      unused.
 *
 *  \return             The y-coordinate \f$y=\sqrt{1-x^2}\f$ of the semicircle
 *                      associated with \p *px.
 ******************************************************************************/
static double plotUnitCircleTop (double *px, void *pData)
{
    double y;

    if (fabs (*px) <= 1.0)
    {
        y = sqrt (1 - *px * *px);
    } /* if */
    else
    {
        y = GSL_POSINF;
    } /* else */

    return y;
} /* plotUnitCircleTop() */



/* FUNCTION *******************************************************************/
/** Plot function for a south-oriented semicircle with unity radius. This
 *  function must be of type PLOT_FUNC_GET from cairoPlot.h (see there).
 *
 *  \param px           Pointer to real-world x-coordinate.
 *  \param pData        User application data pointer as passed to cairoPlot2d()
 *                      in element \a pData of structure PLOT_DIAG. Here it is
 *                      unused.
 *
 *  \return             The y-coordinate \f$y=-\sqrt{1-x^2}\f$ of the semicircle
 *                      associated with \p *px.
 ******************************************************************************/
static double plotUnitCircleBottom (double *px, void *pData)
{
    double y;

    if (fabs (*px) <= 1.0)
    {
        y = -sqrt (1 - *px * *px);
    } /* if */
    else
    {
        y = GSL_NEGINF;
    } /* else */

    return y;
} /* plotUnitCircleBottom() */



/* FUNCTION *******************************************************************/
/** Updates real/imag maximum and minimum values from roots positions.
 *
 *  \param poly         Polynomial which roots shall be compared with the
 *                      current maximum and minimum values.
 *  \param rmin         Current minimum value to be updated from \p poly->root.
 *  \param rmax         Current maximum value to be updated from \p poly->root.
 *
 ******************************************************************************/
static void updateRootsMinMax (MATHPOLY *poly, gsl_complex *rmin, gsl_complex *rmax)
{
    int i;

    for (i = 0; i < poly->degree; i++)
    {
        if (GSL_REAL (poly->root[i]) > GSL_REAL (*rmax))    /* max. real part */
        {
            GSL_SET_REAL (rmax, GSL_REAL (poly->root[i]));
        } /* if */

        if (GSL_IMAG (poly->root[i]) > GSL_IMAG (*rmax))       /* max. imag. part */
        {
            GSL_SET_IMAG (rmax, GSL_IMAG (poly->root[i]));
        } /* if */

        if (GSL_REAL (poly->root[i]) < GSL_REAL (*rmin))        /* min. real part */
        {
            GSL_SET_REAL (rmin, GSL_REAL (poly->root[i]));
        } /* if */

        if (GSL_IMAG (poly->root[i]) < GSL_IMAG (*rmin))       /* min. imag. part */
        {
            GSL_SET_IMAG (rmin, GSL_IMAG (poly->root[i]));
        } /* if */
    } /* for */
} /* updateRootsMinMax() */



/* FUNCTION *******************************************************************/
/** The function calculates the roots of a polynomial. The function assumes
 *  that all coefficients \f$c_i\f$ in \p poly are based on the following
 *  representation in \e Z domain:
    \f[
    p(z)=c_n z^{-n} + c_{n-1} z^{-(n-1)}+ \cdots + c_2 z^{-2} + c_1 z^{-1} + c_0
    \f]
 *  If we substitute \f$w=z^{-1}\f$ it gives
    \f[
    p(w)=c_n w^n + c_{n-1} w^{n-1}+ \cdots + c_2 w^2 + c_1 w + c_0
    \f]
 *  In that sense the function first calculates the roots \f$w_i\f$, then it
 *  transforms all back into \f$z_i\f$. So the linear factor presentation of
 *  \f$p(z)\f$ by the help of \f$z_i\f$ is equivalent to the polynomial
 *  presentation in \e Z domain.
 *
 *  \param poly         Pointer to polynomial that holds the coefficients in
 *                      \p poly->coeff and gets the roots in \p poly->roots.
 *
 *  \return             0 on success, else an error number.
 ******************************************************************************/
static int calcRoots (MATHPOLY *poly)
{
    int ret, i;

    ret = mathPolyCoeffs2Roots (poly);

    if (ret == 0)
    {
        for (i = 0; i < poly->degree; i++)
        {
            poly->root[i] = gsl_complex_inverse (poly->root[i]);
        } /* for */
    } /* if */

    return ret;
} /* calcRoots */



/* FUNCTION *******************************************************************/
/** Redraws a response widget in case an \e expose event is received.
 *
 *  \param widget       Pointer to widget (GtkDrawingArea, GDK drawable), which
 *                      have to be redrawn with a particular filter response.
 *  \param event        \e expose event data.
 *  \param user_data    Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect() for \e expose
 *                      event.
 *  \return             TRUE if the callback handled the signal, and no further
 *                      handling is needed.
 ******************************************************************************/
static gboolean rootsPlotExposeHandler (GtkWidget *widget, GdkEventExpose *event,
                                        gpointer user_data)
{
    static const char realText[] = N_("Re(z)");
    static const char imagText[] = N_("Im(z)");

    static GdkColor circleColor[PLOT_COLOR_SIZE];
    static GdkColor rootsColor[PLOT_COLOR_SIZE];


    static PLOT_DIAG rplots[] =
    {
        {                                  /* stability boundary circle (top) */
            {realText}, {imagText}, NULL,                      /* x, y, pData */
            1.0, PLOT_STYLE_LINE_ONLY, 0,            /* thickness, style, num */
            NULL, NULL,                             /* progressFunc, initFunc */
            plotUnitCircleTop, NULL,                   /* sampleFunc, endFunc */
            circleColor,                                            /* colors */
        },
        {                               /* stability boundary circle (bottom) */
            {realText}, {imagText}, NULL,                      /* x, y, pData */
            1.0, PLOT_STYLE_LINE_ONLY, 0,            /* thickness, style, num */
            NULL, NULL,                             /* progressFunc, initFunc */
            plotUnitCircleBottom, NULL,                /* sampleFunc, endFunc */
            circleColor,                                            /* colors */
        },
        {                                                            /* zeros */
            {realText}, {imagText}, NULL,                      /* x, y, pData */
            2.0, PLOT_STYLE_CIRCLE_ONLY, 0,          /* thickness, style, num */
            NULL, rootsPlotInit, plotRoot, NULL, /* progress-, init-, sample-, endFunc */
            rootsColor,                                             /* colors */
        },
        {                                                            /* poles */
            {N_(realText)}, {imagText}, NULL,                  /* x, y, pData */
            2.0, PLOT_STYLE_CROSS_ONLY, 0,           /* thickness, style, num */
            NULL, rootsPlotInit, plotRoot, NULL, /* progress-, init-, sample-, endFunc */
            rootsColor,                                             /* colors */
        },
        {                                                            /* poles */
            {N_(realText)}, {imagText}, NULL,                  /* x, y, pData */
            2.0, PLOT_STYLE_CROSS_ONLY, 1,           /* thickness, style, num */
            NULL, NULL, returnZeroRoot, NULL, /* progress-, init-, sample-, endFunc */
            rootsColor,                                             /* colors */
        }
    };


    FLTCOEFF *pFilter = dfcPrjGetFilter ();

    if (pFilter != NULL)
    {
        int i, width, height;
        gsl_complex rmax, rmin, delta;
        cairo_t* cr;

        int points = 0;
        int numPlots = N_ELEMENTS (rplots);
        GtkWidget *topWidget = gtk_widget_get_toplevel (widget);
        const CFG_DESKTOP* pPrefs = cfgGetDesktopPrefs ();
        GdkDrawable* drawable = GDK_DRAWABLE(widget->window);
        GdkCursor* cursor = gdk_cursor_new (GDK_WATCH);

        gdk_window_set_cursor (topWidget->window, cursor);
        gdk_cursor_unref (cursor);   /* free client side resource immediately */

        GSL_SET_COMPLEX (&rmax, 1, 1);        /* at least in interval [-1,+1] */
        GSL_SET_COMPLEX (&rmin, -1, -1);
        updateRootsMinMax (&pFilter->den, &rmin, &rmax);
        updateRootsMinMax (&pFilter->num, &rmin, &rmax);
        delta = gsl_complex_mul_real (gsl_complex_sub (rmax, rmin), 0.05);
        rmin = gsl_complex_sub (rmin, delta);                   /* 5% stretch */
        rmax = gsl_complex_add (rmax, delta);

        if (pFilter->factor != 0.0)                       /* roots are valid? */
        {
            rplots[2].pData = &pFilter->num; /* set the expected data pointer */
            rplots[3].pData = &pFilter->den;
        } /* if */
        else
        {
            rplots[2].pData = rplots[3].pData = NULL;
        } /* else */

        if (pFilter->num.degree == pFilter->den.degree)
        {                                     /* polynomials with same degree */
            --numPlots;        /* do not plot the last diagram (roots at 0,0) */
        } /* if */
        else
        {                                  /* note: polynomials are in z^{-1} */
            if (pFilter->den.degree > pFilter->num.degree)
            {
                rplots[4].style = PLOT_STYLE_CIRCLE_ONLY;
            } /* if */
            else
            {
                rplots[4].style = PLOT_STYLE_CROSS_ONLY;
            } /* else */
        } /* else */

        circleColor[PLOT_COLOR_LABELS] =                 /* set all invisible */
            circleColor[PLOT_COLOR_GRID] =
            circleColor[PLOT_COLOR_BOX] = 
            circleColor[PLOT_COLOR_AXIS_NAME] = widget->style->bg[GTK_STATE_NORMAL];

        circleColor[PLOT_COLOR_GRAPH] = widget->style->fg[GTK_STATE_INSENSITIVE];

        rootsColor[PLOT_COLOR_GRID] =
            rootsColor[PLOT_COLOR_AXIS_NAME] =
            rootsColor[PLOT_COLOR_BOX] = widget->style->fg[GTK_STATE_INSENSITIVE];

        rootsColor[PLOT_COLOR_LABELS] =
            rootsColor[PLOT_COLOR_GRAPH] = widget->style->fg[GTK_STATE_NORMAL];

        gdk_drawable_get_size (drawable, &width, &height);
        cr = gdk_cairo_create (drawable);             /* create cairo context */

        for (i = 0; i < numPlots; i++)
        {
            rplots[i].area.x = rplots[i].area.y = 0;
            rplots[i].area.width = width;            /* set size of plot area */
            rplots[i].area.height = height;
            rplots[i].x.prec = rplots[i].y.prec = pPrefs->outprec;
            rplots[i].x.start = GSL_REAL (rmin);
            rplots[i].x.stop = GSL_REAL (rmax);
            rplots[i].y.start = GSL_IMAG (rmin);
            rplots[i].y.stop = GSL_IMAG (rmax);

            points = cairoPlot2d (cr, &rplots[i]);
        } /* for */

        if (points < 0)
        {
        } /* if */

        cairo_destroy(cr);                              /* free cairo context */
        gdk_window_set_cursor(topWidget->window, NULL);     /* restore cursor */
    } /* if */

    return TRUE;                                             /* stop emission */
} /* rootsPlotExposeHandler() */




/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Creates a \e GtkDrawingArea used for roots display.
 *
 *  \return             Pointer to widget (\e GtkDrawingArea, GDK drawable), which
 *                      is used in function rootsPlotUpdate() to draw polynomial
 *                      roots.
 ******************************************************************************/
GtkWidget *rootsPlotCreate ()
{
    plotDrawable = gtk_drawing_area_new ();

    g_signal_connect ((gpointer) plotDrawable, "expose_event",
                      G_CALLBACK (rootsPlotExposeHandler), NULL);

    return plotDrawable;
} /* rootsPlotCreate() */




/* FUNCTION *******************************************************************/
/** Re-calculates the roots of transfer \f$H(z)\f$ of a filter.
 *
 *  \param pFilter      Pointer to filter coefficients, for which the roots
 *                      shall be calculated. Set this to NULL, if the filter
 *                      (and therefore the roots too) is invalid.
 *
 ******************************************************************************/
void rootsPlotUpdate (FLTCOEFF *pFilter)
{
    if (pFilter != NULL)
    {
        if ((calcRoots (&pFilter->num) != 0) || (calcRoots (&pFilter->den) != 0))
        {
            pFilter->factor = 0.0;                       /* roots are invalid */
        } /* if */
        else
        {
            pFilter->factor = pFilter->num.coeff[0] / pFilter->den.coeff[0];
        } /* else */
    } /* if */

    rootsPlotRedraw ();

} /* rootsPlotUpdate() */



/* FUNCTION *******************************************************************/
/** Forces a asynchronous redraw of all transfer function roots.
 *
 ******************************************************************************/
void rootsPlotRedraw ()
{
    GdkWindow *win = plotDrawable->window;

    gdk_window_invalidate_region (
        win, gdk_drawable_get_visible_region (GDK_DRAWABLE (win)), FALSE);

} /* rootsPlotRedraw() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
