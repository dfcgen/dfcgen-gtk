/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     filterPrint.c
 * \brief    Filter print functions.
 *
 * \author   Copyright (C) 2011-2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de> 
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "filterResponse.h"
#include "dfcProject.h"
#include "cfgSettings.h" /* cfgGetDesktopPrefs() */

#include "filterPrint.h"



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/** \brief Internal context for a coefficients print job
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
} FILTERPRINT_COEFF_CONTEXT;


/** \brief Internal context for a response print job
 *  \note  This structure is not used in the sense of a context. Instead
 *         it's purpose is to map two parameters to one variable,
 *         which then can be used as a single pointer to an event
 *         callback function.
 */
typedef struct
{
    RESPONSE_TYPE type;                                   /**< response type */
    PLOT_DIAG diag;                                       /**< response plot */
} FILTERPRINT_RESPONSE_CONTEXT;



/* LOCAL CONSTANT DEFINITIONS ************************************************/


/* LOCAL VARIABLE DEFINITIONS ************************************************/

static FILTERPRINT_COEFF_CONTEXT filterPrintCoeffCtx;



/* LOCAL MACRO DEFINITIONS ****************************************************/


#define FILTER_PRINT_LMARGIN(pgwidth) ((pgwidth) / 10)



/* LOCAL FUNCTION DECLARATIONS ************************************************/

static int filterPrintf (GtkPrintContext *ctx, BOOL doprint,
                         int maxwidth, const gchar* format, ...);
static int filterPrintPageHeader (GtkPrintContext *ctx, BOOL doprint, int pgno);
static void filterPrintCoeffsInit (GtkPrintOperation *op, GtkPrintContext *ctx, gpointer ref);
static void filterPrintCoeffsDo (GtkPrintOperation *op, GtkPrintContext *ctx, int pgno, gpointer ref);
static void filterPrintResponseInit (GtkPrintOperation *op, GtkPrintContext *ctx, gpointer ref);
static void filterPrintResponseDo (GtkPrintOperation *op, GtkPrintContext *ctx, int pgno, gpointer ref);
static void filterPrintDo (GtkWidget* topWidget, GtkPrintOperation* print);


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

    va_start (args, format);
    text = g_strdup_vprintf (format, args);

    pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);
    pango_layout_set_indent (layout, -maxwidth / 10 * PANGO_SCALE); /* hanging indent */
    pango_layout_set_width (layout, maxwidth * PANGO_SCALE);
    pango_layout_set_markup (layout, text, -1);
    pango_layout_get_extents (layout, NULL, &rect);

    if (doprint)
    {
        pango_cairo_show_layout (
            gtk_print_context_get_cairo_context (ctx), layout);
    } /* if */

    g_free (text);
    g_object_unref (layout);

    return (PANGO_PIXELS (rect.height) + 1); /* some more space */
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
    text = g_strdup_printf (_("Page %d / %d"), pgno + 1, filterPrintCoeffCtx.pages);
    pango_layout_set_text (layout, text, -1);
    cairo_move_to (cr, filterPrintCoeffCtx.pgwidth / 2, 0);

    if (doprint)
    {
        pango_cairo_show_layout (cr, layout);
    } /* if */

    pango_layout_get_extents (layout, NULL, &rect);
    lheight = PANGO_PIXELS (rect.height) + 1;     /* some more space per line */
    yoffset = 2 * lheight;
    g_free (text);
    g_object_unref (layout);
    pango_layout_set_alignment (layout, PANGO_ALIGN_LEFT);

    if (pgno == 0)                     /* on first page put the project info */
    {
        int maxwidth2;                        /* half of the printable width */

        DFCPRJ_INFO* info = dfcPrjGetInfo ();
        FLTCOEFF* pFilter = dfcPrjGetFilter ();

        if (info->title != NULL)
        {
            if (g_utf8_strlen (info->title, -1) > 0)   /* project title defined? */
            {
                cairo_move_to (cr, filterPrintCoeffCtx.lmargin, yoffset);
                yoffset += filterPrintf (ctx, doprint, filterPrintCoeffCtx.maxwidth,
                                         _("<b>Title: </b>%s"), info->title);
            } /* if */
        } /* if */

        if (info->author != NULL)
        {
            if (g_utf8_strlen (info->author, -1) > 0)         /* author defined? */
            {
                cairo_move_to (cr, filterPrintCoeffCtx.lmargin, yoffset);
                yoffset += filterPrintf (ctx, doprint, filterPrintCoeffCtx.maxwidth,
                                         _("<b>Author: </b>%s"), info->author);
            } /* if */
        } /* if */

        if (info->desc != NULL)
        {
            if (g_utf8_strlen (info->desc, -1) > 0)    /* description available? */
            {
                cairo_move_to (cr, filterPrintCoeffCtx.lmargin, yoffset);
                yoffset += filterPrintf (ctx, doprint, filterPrintCoeffCtx.maxwidth,
                                         _("<b>Description: </b>%s"), info->desc);
            } /* if */
        } /* if */

        if (yoffset > 2 * lheight)              /* any project info printed? */
        {
            yoffset += lheight;            /* then add some additional space */
        } /* if */

        cairo_move_to (cr, filterPrintCoeffCtx.lmargin, yoffset);

        /* finally the coefficients list header
         */
        if (pFilter != NULL)                 /* sanity (should never happen) */
        {
            yoffset += filterPrintf (ctx, doprint, filterPrintCoeffCtx.maxwidth,
                                     _("<b>Coefficients:</b>")) + lheight / 2;
            maxwidth2 = filterPrintCoeffCtx.maxwidth / 2;

            cairo_move_to (cr, filterPrintCoeffCtx.lmargin, yoffset);
            (void)filterPrintf (ctx, doprint, maxwidth2,
                                _("Numerator (%d)"),
                                pFilter->num.degree + 1);

            cairo_move_to (cr, (filterPrintCoeffCtx.lmargin + filterPrintCoeffCtx.pgwidth) / 2, yoffset);
            yoffset += filterPrintf (ctx, doprint, maxwidth2,
                                     _("Denominator (%d)"),
                                     pFilter->den.degree + 1) + lheight / 4;
        } /* if */
    } /* if */

    return yoffset;
} /* filterPrintPageHeader() */



/* FUNCTION *******************************************************************/
/** \brief Common part of print functions and wrapper of
 *         gtk_print_operation_run().
 *
 *  \param[in] topWidget Top widget associated with the \c GtkToolButton widget
 *                       which has caused the \e clicked event.
 *  \param[in] print     GTK print context.
 *
 ******************************************************************************/
static void filterPrintDo (GtkWidget* topWidget, GtkPrintOperation* print)
{
    static GtkPrintSettings *settings = NULL;

    GtkWidget* widget;
    GError *error;
    GtkPrintOperationResult result;

    gtk_print_operation_set_print_settings (print, settings);
    gtk_print_operation_set_default_page_setup (print, NULL);

    result = gtk_print_operation_run (print,
                                      GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                      NULL, NULL);
    switch (result)
    {
        case GTK_PRINT_OPERATION_RESULT_ERROR:
            gtk_print_operation_get_error (print, &error);
            widget = gtk_message_dialog_new (GTK_WINDOW (topWidget),
                                             GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_MESSAGE_ERROR,
                                             GTK_BUTTONS_CLOSE,
                                             _("Error printing: %s"),
                                             error->message);
            g_signal_connect (widget, "response", 
                              G_CALLBACK (gtk_widget_destroy), NULL);
            gtk_widget_show (widget);
            g_error_free (error);
            break; /* GTK_PRINT_OPERATION_RESULT_ERROR */


        case GTK_PRINT_OPERATION_RESULT_APPLY:   /* store the print settings */
            if (settings != NULL)
            {
                g_object_unref (settings);
            } /* if */

            settings = g_object_ref (gtk_print_operation_get_print_settings (print));
            break;

        default: /* GTK_PRINT_OPERATION_RESULT_CANCEL */
            break;
    } /* switch */

} /* filterPrintDo() */



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
static void filterPrintCoeffsInit (GtkPrintOperation *op, GtkPrintContext *ctx, gpointer ref)
{
    int yofs1, yofs2;

    FLTCOEFF* pFilter = dfcPrjGetFilter ();

    gtk_print_operation_set_use_full_page (op, FALSE); /* set cairo transform */
    gtk_print_operation_set_unit (op, GTK_UNIT_PIXEL);

    filterPrintCoeffCtx.icoeff = 0;           /* reset index of first coefficient */
    filterPrintCoeffCtx.pages = 1;                        /* defaults to one page */
    filterPrintCoeffCtx.pgwidth = (int) gtk_print_context_get_width (ctx);
    filterPrintCoeffCtx.lmargin = FILTER_PRINT_LMARGIN (filterPrintCoeffCtx.pgwidth);
    filterPrintCoeffCtx.maxwidth = filterPrintCoeffCtx.pgwidth - filterPrintCoeffCtx.lmargin;
    filterPrintCoeffCtx.lines = filterPrintCoeffCtx.lines1 = filterPrintCoeffCtx.lines2 = 0;

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

        filterPrintCoeffCtx.lines = MAX (pFilter->num.degree, pFilter->den.degree) + 1;
        filterPrintCoeffCtx.lines1 = (pgheight - yofs1) / lheight;
        filterPrintCoeffCtx.lines2 = (pgheight - yofs2) / lheight;

        filterPrintCoeffCtx.pages = (filterPrintCoeffCtx.lines - filterPrintCoeffCtx.lines1 +
                                filterPrintCoeffCtx.lines2 - 1) / filterPrintCoeffCtx.lines2 + 1;
    } /* if */

    gtk_print_operation_set_n_pages (op, filterPrintCoeffCtx.pages);
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
static void filterPrintCoeffsDo (GtkPrintOperation *op, GtkPrintContext *ctx, int pgno, gpointer ref)
{
    FLTCOEFF* pFilter = dfcPrjGetFilter ();
    cairo_t* cr = gtk_print_context_get_cairo_context (ctx);
    int yoffset = filterPrintPageHeader (ctx, TRUE, pgno);

    cairo_move_to (cr, filterPrintCoeffCtx.lmargin, yoffset);

    if (pFilter != NULL)
    {
        int i;

        int maxwidth2 = filterPrintCoeffCtx.maxwidth / 2;
        int pglines = (pgno == 0) ? filterPrintCoeffCtx.lines1 : filterPrintCoeffCtx.lines2;

        for (i = 0;
             (i < pglines) && (filterPrintCoeffCtx.icoeff < filterPrintCoeffCtx.lines);
             ++i, ++filterPrintCoeffCtx.icoeff)
        {
            int dy = 0;

            if (filterPrintCoeffCtx.icoeff <= pFilter->num.degree)
            {
                cairo_move_to (cr, filterPrintCoeffCtx.lmargin, yoffset);
                dy = filterPrintf (ctx, TRUE, maxwidth2,
                                   "z<sup>-%d</sup>  -&gt;  %.*G",
                                   filterPrintCoeffCtx.icoeff,
                                   cfgGetDesktopPrefs ()->outprec,
                                   pFilter->num.coeff[filterPrintCoeffCtx.icoeff]);
            } /* if */

            if (filterPrintCoeffCtx.icoeff <= pFilter->den.degree)
            {
                cairo_move_to (cr, (filterPrintCoeffCtx.lmargin + filterPrintCoeffCtx.pgwidth) / 2, yoffset);
                dy = MAX(dy, filterPrintf (ctx, TRUE, maxwidth2,
                                           "z<sup>-%d</sup>  -&gt;  %.*G",
                                           filterPrintCoeffCtx.icoeff,
                                           cfgGetDesktopPrefs ()->outprec,
                                           pFilter->den.coeff[filterPrintCoeffCtx.icoeff]));
            } /* if */

            yoffset += dy;

        } /* while */
    } /* if */

    cairo_stroke (cr);

} /* filterPrintCoeffsDo() */



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
static void filterPrintResponseInit (GtkPrintOperation *op, GtkPrintContext *ctx, gpointer ref)
{
    gtk_print_operation_set_use_full_page (op, FALSE);
    gtk_print_operation_set_unit (op, GTK_UNIT_PIXEL); /* set cairo transform */

    gtk_print_operation_set_n_pages (op, 1);
} /* filterPrintResponseInit() */



/* FUNCTION *******************************************************************/
/** \brief Callback function for the \e draw-page event.
 *
 *  This function (signal handler for \e draw-page) is called for every page
 *  that is printed. It renders the page onto the cairo context of page.
 *
 * \param[in] op        the GtkPrintOperation on which the signal was emitted
 * \param[in] ctx       the GtkPrintContext for the current operation
 * \param[in] pgno      the number of the currently printed page 
 * \param[in] ref       reference pointer to response plot context
 *                      ::FILTERPRINT_RESPONSE_CONTEXT
 *
 ******************************************************************************/
static void filterPrintResponseDo (GtkPrintOperation *op, GtkPrintContext *ctx,
                                   int pgno, gpointer ref)
{
    FLTCOEFF* pFilter = dfcPrjGetFilter ();

    if (pFilter != NULL)
    {
        FILTERPRINT_RESPONSE_CONTEXT* response = (FILTERPRINT_RESPONSE_CONTEXT*)ref;
        PLOT_DIAG* pDiag = &response->diag;                           /* shortcut */
        cairo_t* cr = gtk_print_context_get_cairo_context (ctx);

        pDiag->area.y = 0;  /* set size of plot area */
        pDiag->area.height = gtk_print_context_get_height (ctx) / 2;
        pDiag->area.width = gtk_print_context_get_width (ctx);
        pDiag->area.x = FILTER_PRINT_LMARGIN (pDiag->area.width);
        pDiag->area.width -= pDiag->area.x;

        (void)responsePlotDraw (cr, response->type, pDiag);
    } /* if */

} /* filterPrintResponseDo() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/



/* FUNCTION *******************************************************************/
/** \brief Response plot print function.
 *
 *  \param[in] topWidget Top widget associated with the \c GtkToolButton widget
 *                       which has caused the \e clicked event.
 *  \param[in] pDiag     Pointer to plot diag of associated response \p type.
 *  \param[in] type     ::RESPONSE_TYPE which identifies what response to print.
 *
 ******************************************************************************/
void filterPrintResponse (GtkWidget* topWidget, const PLOT_DIAG* pDiag,
                          RESPONSE_TYPE type)
{
    FILTERPRINT_RESPONSE_CONTEXT response = {.diag = *pDiag, .type = type};
    GtkPrintOperation* print = gtk_print_operation_new ();

    g_signal_connect (print, "begin-print",
                      G_CALLBACK (filterPrintResponseInit), NULL);
    g_signal_connect (print, "draw-page",
                      G_CALLBACK (filterPrintResponseDo), &response);
 
    filterPrintDo (topWidget, print);
    g_object_unref (print);

} /* filterPrintResponse() */



/* FUNCTION *******************************************************************/
/** \brief Coefficients print function.
 *
 *  \note  The signature of this function was chosen in a way that it is usual
 *         as \e clicked event callback (when a print menuitem/button is pressed).
 *
 *  \param[in] srcWidget \c GtkMenuItem on event \e activate or \c GtkToolButton
 *                      on event \e clicked, which causes this call.
 *  \param[in] user_data Pointer to user data (unused).
 *
 ******************************************************************************/
void filterPrintCoeffs (GtkWidget* srcWidget, gpointer user_data)
{
    GtkPrintOperation* print = gtk_print_operation_new ();

    g_signal_connect (print, "begin-print",
                      G_CALLBACK (filterPrintCoeffsInit), NULL);
    g_signal_connect (print, "draw-page",
                      G_CALLBACK (filterPrintCoeffsDo), NULL);

    filterPrintDo (gtk_widget_get_toplevel (srcWidget), print);
    g_object_unref (print);

} /* filterPrintCoeffs() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
