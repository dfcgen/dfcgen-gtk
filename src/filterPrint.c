/******************************************************************************/
/**
 * \file     filterPrint.c
 * \brief    Filter print functions.
 *
 * \author   Copyright (C) 2011 Ralf Hoppe <ralf.hoppe@ieee.org> 
 * \version  $Id$
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "filterResponse.h"
#include "dfcProject.h"
#include "cfgSettings.h" /* cfgGetDesktopPrefs() */

#include "filterPrint.h"


#if GTK_CHECK_VERSION(2, 10, 0)           /* print support requires GTK 2.10 */


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/** \brief Internal context for a print job
 */
typedef struct
{
    int pages;                                 /**< number of pages to print */
    int lines;         /**< number of lines needed to print all coefficients */
    int lines1;            /**< max. number of coefficient lines on 1st page */
    int lines2; /**< max. number of coefficient lines on 2nd and other pages */
    int icoeff;                            /**< index of current coefficient */
    int pgwidth;                                          /**< width of page */
    int lmargin;                                            /**< left margin */
    int maxwidth;                                            /**< max. width */
} FILTERPRINT_CONTEXT;



/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL VARIABLE DEFINITIONS *************************************************/

static FILTERPRINT_CONTEXT filterPrintCtx;



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static int filterPrintf (GtkPrintContext *ctx, BOOL doprint,
                         int maxwidth, const gchar* format, ...);
static int filterPrintPageHeader (GtkPrintContext *ctx, BOOL doprint, int pgno);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** \brief Printf-like function for a variable list of parameters.
 *  \note  Affects the \e Cairo path in the sense that it modifies the cursor
 *         position.
 *
 * \param[in] ctx       the \c GtkPrintContext for the current operation
 * \param[in] doprint   really print the passed parameters, else only do
 *                      a size calculation (and return the vertical space used)
 * \param[in] maxwidth  maximum width available for printing the parameters
 * \param[in] format    printf-like format string
 * \param[in] ...       variable parameter list associated with \p format
 *
 * \return    vertical space used for output (height of printed parameters)
 ******************************************************************************/
static int filterPrintf (GtkPrintContext *ctx, BOOL doprint,
                         int maxwidth, const gchar* format, ...)
{
    gchar* text;
    va_list args;

    PangoRectangle rect = {.height = 0};
    PangoLayout *layout = gtk_print_context_create_pango_layout (ctx);

    va_start(args, format);
    text = g_strdup_vprintf(format, args);

    pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);
    pango_layout_set_indent (layout, -maxwidth / 10 * PANGO_SCALE); /* hanging indent */
    pango_layout_set_width (layout, maxwidth * PANGO_SCALE);
    pango_layout_set_markup (layout, text, -1);
    pango_layout_get_extents (layout, NULL, &rect);

    if (doprint)
    {
        pango_cairo_show_layout (
            gtk_print_context_get_cairo_context(ctx), layout);
    } /* if */

    FREE (text);
    g_object_unref (layout);

    return (PANGO_PIXELS(rect.height) + 1); /* some more space */
} /* filterPrintf() */



/* FUNCTION *******************************************************************/
/** \brief Prints the top of a page (page number, filter info, etc)
 *  \note  Top of page means all above the coefficients list.
 *
 *  \param[in] ctx      the \c GtkPrintContext for the current operation
 *  \param[in] doprint  really print the page, else only do a size
 *                      calculation (and return the vertical space used)
 *  \param[in] pgno     the number of the currently printed page 
 *
 *  \return    vertical space used for output (height of printed area)
 *
 ******************************************************************************/
static int filterPrintPageHeader (GtkPrintContext *ctx, BOOL doprint, int pgno)
{
    PangoRectangle rect;
    gchar *text;
    int yoffset, lheight;

    cairo_t* cr = gtk_print_context_get_cairo_context(ctx);
    PangoLayout *layout = gtk_print_context_create_pango_layout (ctx);

    /* start with the page number (use default font)
     */
    pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);
    text = g_strdup_printf (_("Page %d / %d"), pgno + 1, filterPrintCtx.pages);
    pango_layout_set_text (layout, text, -1);
    cairo_move_to (cr, filterPrintCtx.pgwidth / 2, 0);

    if (doprint)
    {
        pango_cairo_show_layout (cr, layout);
    } /* if */

    pango_layout_get_extents (layout, NULL, &rect);
    lheight = PANGO_PIXELS(rect.height) + 1;     /* some more space per line */
    yoffset = 2 * lheight;
    FREE (text);
    g_object_unref (layout);
    pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);

    if (pgno == 0)                     /* on first page put the project info */
    {
        int maxwidth2;                        /* half of the printable width */

        DFCPRJ_INFO* info = dfcPrjGetInfo ();
        FLTCOEFF* pFilter = dfcPrjGetFilter();

        if (g_utf8_strlen (info->title, -1) > 0)   /* project title defined? */
        {
            cairo_move_to (cr, filterPrintCtx.lmargin, yoffset);
            yoffset += filterPrintf (ctx, doprint, filterPrintCtx.maxwidth,
                                     _("<b>Title: </b>%s"), info->title);
        } /* if */

        if (g_utf8_strlen (info->author, -1) > 0)         /* author defined? */
        {
            cairo_move_to (cr, filterPrintCtx.lmargin, yoffset);
            yoffset += filterPrintf (ctx, doprint, filterPrintCtx.maxwidth,
                                     _("<b>Author: </b>%s"), info->author);
        } /* if */

        if (g_utf8_strlen (info->desc, -1) > 0)    /* description available? */
        {
            cairo_move_to (cr, filterPrintCtx.lmargin, yoffset);
            yoffset += filterPrintf (ctx, doprint, filterPrintCtx.maxwidth,
                                     _("<b>Description: </b>%s"), info->desc);
        } /* if */

        if (yoffset > 2 * lheight)              /* any project info printed? */
        {
            yoffset += lheight;            /* then add some additional space */
        } /* if */

        cairo_move_to (cr, filterPrintCtx.lmargin, yoffset);

        /* finally the coefficients list header
         */
        if (pFilter != NULL)                 /* sanity (should never happen) */
        {
            yoffset += filterPrintf (ctx, doprint, filterPrintCtx.maxwidth,
                                     _("<b>Coefficients:</b>")) + lheight / 2;
            maxwidth2 = filterPrintCtx.maxwidth / 2;

            cairo_move_to (cr, filterPrintCtx.lmargin, yoffset);
            (void)filterPrintf (ctx, doprint, maxwidth2,
                                _("Numerator (%d)"),
                                pFilter->num.degree + 1);

            cairo_move_to (cr, (filterPrintCtx.lmargin + filterPrintCtx.pgwidth) / 2, yoffset);
            yoffset += filterPrintf (ctx, doprint, maxwidth2,
                                     _("Denominator (%d)"),
                                     pFilter->den.degree + 1) + lheight / 4;
        } /* if */
    } /* if */

    return yoffset;
} /* filterPrintPageHeader() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** \brief Callback function for the \e begin-print event.
 *
 *  The \e begin-print event is emitted after the user has finished changing
 *  print settings in the dialog, before the actual rendering starts. A typical
 *  use for \e begin-print is to use the parameters from the \c GtkPrintContext
 *  and paginate the document accordingly, and then set the number of pages
 *  with \c gtk_print_operation_set_n_pages().
 *
 *
 * \param[in] op        the GtkPrintOperation on which the signal was emitted
 * \param[in] ctx       the GtkPrintContext for the current operation
 * \param[in] ref       user data set when the signal handler was connected
 *
 ******************************************************************************/
void filterPrintCoeffsInit (GtkPrintOperation *op, GtkPrintContext *ctx, gpointer ref)
{
    int yofs1, yofs2;

    FLTCOEFF* pFilter = dfcPrjGetFilter();

    gtk_print_operation_set_use_full_page (op, FALSE); /* set cairo transform */
    gtk_print_operation_set_unit (op, GTK_UNIT_PIXEL);

    filterPrintCtx.icoeff = 0;           /* reset index of first coefficient */
    filterPrintCtx.pages = 1;                        /* defaults to one page */
    filterPrintCtx.pgwidth = (int) gtk_print_context_get_width (ctx);
    filterPrintCtx.lmargin = filterPrintCtx.pgwidth / 10;
    filterPrintCtx.maxwidth = filterPrintCtx.pgwidth - filterPrintCtx.lmargin;
    filterPrintCtx.lines = filterPrintCtx.lines1 = filterPrintCtx.lines2 = 0;

    yofs1 = filterPrintPageHeader (ctx, FALSE, 0);       /* 1st page y-start */
    yofs2 = filterPrintPageHeader (ctx, FALSE, 1);       /* 2nd page y-start */

    if (pFilter != NULL)                     /* sanity (should never happen) */
    {
        int pgheight, lheight;                  /* page height & line height */
        PangoRectangle rect;

        PangoLayout *layout = gtk_print_context_create_pango_layout (ctx);

        pango_layout_set_markup (layout, "z<sup>-1</sup>  -&gt;  1.0", -1);
        pango_layout_get_extents (layout, NULL, &rect);
        lheight = PANGO_PIXELS (rect.height) + 1;         /* some more space */
        g_object_unref (layout);

        pgheight = (int) gtk_print_context_get_height (ctx) - lheight;

        if (yofs1 >= pgheight)                                     /* sanity */
        {
            yofs1 = pgheight;
        } /* if */

        if (yofs2 >= pgheight)                                     /* sanity */
        {
            yofs2 = pgheight;
        } /* if */

        filterPrintCtx.lines = MAX (pFilter->num.degree, pFilter->den.degree) + 1;
        filterPrintCtx.lines1 = (pgheight - yofs1) / lheight;
        filterPrintCtx.lines2 = (pgheight - yofs2) / lheight;

        filterPrintCtx.pages = (filterPrintCtx.lines - filterPrintCtx.lines1 +
                                filterPrintCtx.lines2 - 1) / filterPrintCtx.lines2 + 1;
    } /* if */

    gtk_print_operation_set_n_pages (op, filterPrintCtx.pages);
} /* filterPrintCoeffsInit() */




/* FUNCTION *******************************************************************/
/** \brief Callback function for the \e draw-page event.
 *
 *  This function (signal handler for \e draw-page) is called for every page
 *  that is printed. It renders the page onto the cairo context of page.
 *
 * \param[in] op        the GtkPrintOperation on which the signal was emitted
 * \param[in] ctx       the GtkPrintContext for the current operation
 * \param[in] pgno      the number of the currently printed page 
 * \param[in] ref       user data set when the signal handler was connected
 *
 ******************************************************************************/
void filterPrintCoeffsDo (GtkPrintOperation *op, GtkPrintContext *ctx, int pgno, gpointer ref)
{
    FLTCOEFF* pFilter = dfcPrjGetFilter();
    cairo_t* cr = gtk_print_context_get_cairo_context(ctx);
    int yoffset = filterPrintPageHeader (ctx, TRUE, pgno);

    cairo_move_to (cr, filterPrintCtx.lmargin, yoffset);

    if (pFilter != NULL)
    {
        int i;

        int maxwidth2 = filterPrintCtx.maxwidth / 2;
        int pglines = (pgno == 0) ? filterPrintCtx.lines1 : filterPrintCtx.lines2;

        for (i = 0;
             (i < pglines) && (filterPrintCtx.icoeff < filterPrintCtx.lines);
             ++i, ++filterPrintCtx.icoeff)
        {
            int dy = 0;

            if (filterPrintCtx.icoeff <= pFilter->num.degree)
            {
                cairo_move_to (cr, filterPrintCtx.lmargin, yoffset);
                dy = filterPrintf (ctx, TRUE, maxwidth2,
                                   "z<sup>-%d</sup>  -&gt;  %.*G",
                                   filterPrintCtx.icoeff,
                                   cfgGetDesktopPrefs()->outprec,
                                   pFilter->num.coeff[filterPrintCtx.icoeff]);
            } /* if */

            if (filterPrintCtx.icoeff <= pFilter->den.degree)
            {
                cairo_move_to (cr, (filterPrintCtx.lmargin + filterPrintCtx.pgwidth) / 2, yoffset);
                dy = MAX(dy, filterPrintf (ctx, TRUE, maxwidth2,
                                           "z<sup>-%d</sup>  -&gt;  %.*G",
                                           filterPrintCtx.icoeff,
                                           cfgGetDesktopPrefs()->outprec,
                                           pFilter->den.coeff[filterPrintCtx.icoeff]));
            } /* if */

            yoffset += dy;

        } /* while */
    } /* if */

    cairo_stroke (cr);

} /* filterPrintCoeffsDo() */




#else

static int filterPrintDummy = 0; /* empty source files are not allowed */

#endif  /* GTK_CHECK_VERSION(2, 10, 0) */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
