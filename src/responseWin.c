/**
 * \file        responseWin.c
 * \brief       Digital filter response window creation and callbacks.
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 */


/* INCLUDE FILES **************************************************************/

#include "responsePlot.h"
#include "responseWin.h"
#include "responseDlg.h"
#include "dfcProject.h"  /* dfcPrjGetFilter() */
#include "filterPrint.h" /* filterPrintResponse() */
#include "cfgSettings.h" /* cfgSaveResponseSettings(), cfgRestoreResponseSettings */
#include "mathFuncs.h"


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/** Response window description
 */
typedef struct
{
    RESPONSE_TYPE type;                     /**< Type of response plot/window */
    char *iconFile;                              /**< Name of icon (png) file */
    PLOT_DIAG diag;                                            /**< Plot data */
    GdkRGBA colors[PLOT_COLOR_SIZE];         /**< Allocated colors to be used */
    cairo_surface_t *surface;   /**< surface to store current painting (diag) */
    int points;       /**< Number of points drawed on last responsePlotDraw() */
    GdkRectangle zoom;                 /**< Zoom coordinates (last rectangle) */
#if GTK_CHECK_VERSION(3, 20, 0)
    GdkSeat* grab;          /**< mouse grab in zoom mode, \c NULL if inactive */
#else
    GdkDevice* grab;
#endif
    GtkCheckMenuItem *menuref;            /**< (Backward) menu item reference */
    GtkWidget* btnPrint;                  /**< Print button widget reference */
    GtkWidget* topWidget; /**< response plot top-level widget (NULL if not exists) */
    GtkWidget* draw;                            /**< \e GtkDrawingArea widget */
    GtkWidget* label;             /**< Label which shows the number of points */
} RESPONSE_WIN;


/* LOCAL CONSTANT DEFINITIONS *************************************************/


#define RESPONSE_WIN_GRAPH_THICKNESS    (2.0)           /**< Graph line width */



/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void plotToSurface (RESPONSE_WIN *pDesc, GtkWidget *widget);
static void drawZoomRect (RESPONSE_WIN *pDesc, cairo_t* gc);
static void cancelZoomMode (RESPONSE_WIN *pDesc);
static void responseWinCreate (RESPONSE_WIN *pDesc);
static gboolean responseWinDrawHandler (GtkWidget *widget, cairo_t *gc, gpointer user_data);
static void responseWinExpose (RESPONSE_WIN* pDesc);
static void responseWinMapped (GtkWidget* widget, gpointer user_data);
static void responseWinDestroyed (GtkWidget* object, gpointer user_data);
static void responseWinBtnPropActivate (GtkButton *button, gpointer user_data);
static void responseWinBtnPrintActivate (GtkWidget* srcWidget, gpointer user_data);
static gboolean responseWinKeyPress (GtkWidget *widget, GdkEventKey *event,
                                     gpointer user_data);
static gboolean responseWinButtonPress (GtkWidget* widget, GdkEventButton* event, gpointer user_data);
static gboolean responseWinButtonRelease (GtkWidget* widget, GdkEventButton* event, gpointer user_data);
static gboolean responseWinMotionNotify (GtkWidget *widget, GdkEventMotion *event,
                                         gpointer user_data);


/* LOCAL VARIABLE DEFINITIONS *************************************************/

static PLOT_UNIT plotUnitDB = {"dB", 1.0};     /**< Constant attenuation unit */
static PLOT_UNIT plotUnitDeg = {"°", 1.0};          /**< Constant degree unit */


/** All predefined response plots (partially intialized).
 */
static RESPONSE_WIN responseWidget[RESPONSE_TYPE_SIZE] =
{
    [RESPONSE_TYPE_MAGNITUDE] =
    {
        RESPONSE_TYPE_MAGNITUDE,                                      /* type */
        "amplitude.png",                                          /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>f</b>")},
            .y = {.name = N_("<b>H(f)</b>")},
            .pData = &responseWidget[RESPONSE_TYPE_MAGNITUDE], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        },
    },
    [RESPONSE_TYPE_ATTENUATION] =
    {
        RESPONSE_TYPE_ATTENUATION,
        "attenuation.png",                                        /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>f</b>")},
            .y = {.name = N_("<b>A(f)</b>"), .pUnit = &plotUnitDB},
            .pData = &responseWidget[RESPONSE_TYPE_ATTENUATION], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        },
    },
    [RESPONSE_TYPE_CHAR] =
    {
        RESPONSE_TYPE_CHAR,
        "charfunc.png",                                           /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>f</b>")},
            .y = {.name = N_("<b>D(f)</b>")},
            .pData = &responseWidget[RESPONSE_TYPE_CHAR], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        },
    },
    [RESPONSE_TYPE_PHASE] =
    {
        RESPONSE_TYPE_PHASE,
        "phase.png",                                              /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>f</b>")},
            .y = {.name = N_("<b>B(f)</b>"), .pUnit = &plotUnitDeg},
            .pData = &responseWidget[RESPONSE_TYPE_PHASE], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        },
    },
    [RESPONSE_TYPE_DELAY] =
    {
        RESPONSE_TYPE_DELAY,
        "phasedelay.png",                                         /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>f</b>")},
            .y = {.name = N_("<b>T<sub>p</sub>(f)</b>")},
            .pData = &responseWidget[RESPONSE_TYPE_DELAY], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        }
    },
    [RESPONSE_TYPE_GROUP] =
    {
        RESPONSE_TYPE_GROUP,
        "grpdelay.png",                                           /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>f</b>")},
            .y = {.name = N_("<b>T<sub>g</sub>(f)</b>")},
            .pData = &responseWidget[RESPONSE_TYPE_GROUP], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        }
    },
    [RESPONSE_TYPE_IMPULSE] =
    {
        RESPONSE_TYPE_IMPULSE,
        "impulse.png",                                            /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>t</b>")},
            .y = {.name = N_("<b>h(t)</b>")},
            .pData = &responseWidget[RESPONSE_TYPE_IMPULSE], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        },
    },
    [RESPONSE_TYPE_STEP] =
    {
        RESPONSE_TYPE_STEP,
        "step.png",                                               /* iconFile */
        {                                                             /* diag */
            .x = {.name = N_("<b>t</b>")},
            .y = {.name = N_("<b>g(t)</b>")},
            .pData = &responseWidget[RESPONSE_TYPE_STEP], /* pData (backward ptr) */
            .thickness = RESPONSE_WIN_GRAPH_THICKNESS,    /* graph thickness */
        }
    }
}; /* responseWidget[] */



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/**
 * \brief           (Re-) Create the surface and plot the response diagram into.
 * \note            This function should be called after change of plot area size.
 *
 * \param[in,out]   pDesc       Pointer to response window/widget descriptor.
 * \param[in,out]   widget      \GTkDrawingArea widget (which is the final target,
 *                              but used only as a template for \p pDesc->surface).
 */
static void plotToSurface (RESPONSE_WIN *pDesc, GtkWidget *widget)
{
    cairo_t *gc;

    int width = gtk_widget_get_allocated_width (widget);
    int height = gtk_widget_get_allocated_height (widget);

    if (pDesc->surface != NULL)
    {
        cairo_surface_destroy (pDesc->surface);
    }

    pDesc->surface = gdk_window_create_similar_surface (
        gtk_widget_get_window (widget), CAIRO_CONTENT_COLOR_ALPHA, width, height);

    pDesc->diag.area.x = pDesc->diag.area.y = 0;   /* set size of plot area */
    pDesc->diag.area.width = width;
    pDesc->diag.area.height = height;

    gc = cairo_create (pDesc->surface);
    pDesc->points = responsePlotDraw (gc, pDesc->type, &pDesc->diag);
    cairo_destroy (gc);
}


/**
 * \brief   Draw zoom rubberband into graphics context.
 *
 *  \param  pDesc       Pointer to response window/widget descriptor.
 *  \param  gc          Graphics context.
 */
static void drawZoomRect (RESPONSE_WIN *pDesc, cairo_t* gc)
{
    static double dashes = 4.0;

    GdkRectangle rect = pDesc->zoom;

    if (rect.width < 0)
    {
        rect.x += rect.width;
        rect.width = -rect.width;
    } /* if */

    if (rect.height < 0)
    {
        rect.y += rect.height;
        rect.height = -rect.height;
    } /* if */

    cairo_set_operator (gc, CAIRO_OPERATOR_OVER);
    cairo_set_line_width (gc, 1);
    cairo_set_dash (gc, &dashes, 1, 0.0);
    cairo_set_line_cap (gc, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join (gc, CAIRO_LINE_JOIN_MITER);

    gdk_cairo_rectangle (gc, &rect);                   /* cairo_rectangle() */
    cairo_stroke (gc);
} /* drawZoomRect() */



/** 
 * \brief   Cancels the zoom mode, then exposes the drawing widget.
 *
 * \param   pDesc       Pointer to response window/widget description.
 */
static void cancelZoomMode (RESPONSE_WIN *pDesc)
{
    if (pDesc->grab != NULL)
    {
#if GTK_CHECK_VERSION(3, 20, 0)
        gdk_seat_ungrab (pDesc->grab);
#else
        gdk_device_ungrab (pDesc->grab, GDK_CURRENT_TIME);
#endif
        pDesc->grab = NULL;
        responseWinExpose (pDesc);
    }
} /* cancelZoomMode() */



/**
 * \brief   \e Clicked event callback emitted when a print menuitem/button is pressed.
 *
 * \param   srcWidget   \c GtkToolButton on event \e clicked, which causes
 *                      this call.
 * \param   user_data   Pointer to response description (::RESPONSE_WIN) as
 *                      supplied to function g_signal_connect().
 */
static void responseWinBtnPrintActivate (GtkWidget* srcWidget, gpointer user_data)
{
    RESPONSE_WIN* pDesc = (RESPONSE_WIN*)user_data;

    filterPrintResponse (gtk_widget_get_toplevel (srcWidget), &pDesc->diag, pDesc->type);

} /* responseWinBtnPrintActivate() */



/**
 * \brief   Creates a filter response widget/window. Such a window is used to display
 *          - magnitude reponse,
 *          - phase response,
 *          - step response etc.
 *
 * \param   pDesc       Pointer to response window/widget description.
 *
 * \todo    Restore size of drawing area from last session.
 */
static void responseWinCreate (RESPONSE_WIN *pDesc)
{
    GtkWidget *vbox;
    GtkWidget *separator;
    GtkWidget *hbox;
    GtkWidget *btnSettings;
    GdkPixbuf *iconPixbuf;

    pDesc->topWidget = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title (GTK_WINDOW (pDesc->topWidget),
                          gtk_label_get_text (
                              GTK_LABEL (
                                  gtk_bin_get_child (
                                      GTK_BIN (pDesc->menuref)))));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (pDesc->topWidget), TRUE);
    gtk_window_set_focus_on_map (GTK_WINDOW (pDesc->topWidget), FALSE);

    iconPixbuf = createPixbufFromFile (pDesc->iconFile);

    if (iconPixbuf != NULL)
    {
        gtk_window_set_icon (GTK_WINDOW (pDesc->topWidget), iconPixbuf);
        g_object_unref (iconPixbuf);
    } /* if */

    vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add (GTK_CONTAINER (pDesc->topWidget), vbox);

    pDesc->draw = gtk_drawing_area_new ();
    gtk_widget_set_size_request (pDesc->draw, 350, 240);
    gtk_widget_add_events (pDesc->draw, GDK_EXPOSURE_MASK | GDK_BUTTON_MOTION_MASK |
                           GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_box_pack_start (GTK_BOX (vbox), pDesc->draw, TRUE, TRUE, 6);

    separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 6);

    pDesc->label = gtk_label_new ("");
    gtk_box_pack_start (GTK_BOX (hbox), pDesc->label, FALSE, FALSE, 6);
    gtk_label_set_single_line_mode (GTK_LABEL (pDesc->label), TRUE);

    pDesc->btnPrint = createImageButton (GUI_BUTTON_LABEL_PRINT, GUI_BUTTON_IMAGE_PRINT);
    gtk_box_pack_end (GTK_BOX (hbox), pDesc->btnPrint, FALSE, FALSE, 6);
    g_signal_connect ((gpointer) pDesc->btnPrint, "clicked",
                      G_CALLBACK (responseWinBtnPrintActivate),
                      pDesc);

    gtk_widget_set_can_default (pDesc->btnPrint, TRUE);
    gtk_widget_set_tooltip_text (pDesc->btnPrint, _("Print this response plot"));

    btnSettings = createImageButton (GUI_BUTTON_LABEL_PREFS, GUI_BUTTON_IMAGE_PREFS);
    gtk_box_pack_end (GTK_BOX (hbox), btnSettings, FALSE, FALSE, 6);
    gtk_widget_set_can_default (btnSettings, TRUE);
    gtk_widget_set_tooltip_text (btnSettings, _("Set response plot preferences"));

    gtk_widget_add_events (pDesc->topWidget, GDK_KEY_PRESS_MASK);

    g_signal_connect ((gpointer) pDesc->draw, "button_press_event",
                      G_CALLBACK (responseWinButtonPress),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "button_release_event",
                      G_CALLBACK (responseWinButtonRelease),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "motion_notify_event",
                      G_CALLBACK (responseWinMotionNotify),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->topWidget, "key_press_event",
                      G_CALLBACK (responseWinKeyPress),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "draw",
                      G_CALLBACK (responseWinDrawHandler),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "destroy",
                      G_CALLBACK (responseWinDestroyed),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "map",
                      G_CALLBACK (responseWinMapped),
                      pDesc);
    g_signal_connect ((gpointer) btnSettings, "clicked",
                      G_CALLBACK (responseWinBtnPropActivate),
                      pDesc);


    gtk_widget_show_all (pDesc->topWidget);
    gtk_widget_grab_focus (btnSettings);
    gtk_widget_grab_default (btnSettings);

} /* responseWinCreate() */



/**
 * \brief   Invalidates the drawing area of a response window.
 *
 * \param   pDesc   Pointer to response window description.
 */
static void responseWinExpose (RESPONSE_WIN* pDesc)
{
    if ((pDesc->topWidget != NULL) && (pDesc->draw != NULL))
    {
        gdk_window_invalidate_rect (
            gtk_widget_get_window (pDesc->draw), NULL, FALSE);
    } /* if */
} /* responseWinExpose() */



/**
 * \brief   Redraws a response widget in case an \e draw event is received.
 *
 * \param   widget      Pointer to \c GtkDrawingArea widget, which has to be
 *                      redrawn with a particular filter response.
 * \param   gc          \e Cairo graphics context.
 * \param   user_data   Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect() for \e draw
 *                      event.
 * \return  \c TRUE if the callback handled the signal, and no further
 *          handling is needed.
 * \see     Documentation of \e GtkDrawingArea.
 */
static gboolean responseWinDrawHandler (GtkWidget *widget, cairo_t *gc,
                                        gpointer user_data)
{
    char labelString[128];

    RESPONSE_WIN* pDesc = user_data;
    GdkWindow* topWindow = gtk_widget_get_window (pDesc->topWidget);

    ASSERT (pDesc != NULL);
    ASSERT (widget == pDesc->draw);

    if ((pDesc->grab != NULL) && (pDesc->surface != NULL))
    {
        cairo_save (gc);
        cairo_set_source_surface (gc, pDesc->surface, 0, 0);
        cairo_paint (gc);                 /* copy surface into drawing area */
        cairo_restore (gc);

        drawZoomRect (pDesc, gc);
    }
    else
    {
        GdkCursor* cursor = gdk_cursor_new_from_name (
            gtk_widget_get_display (pDesc->topWidget), GUI_CURSOR_IMAGE_WATCH);
        gdk_window_set_cursor (topWindow, cursor);
        g_object_unref (cursor);

        plotToSurface (pDesc, widget);
        // gtk_render_background (gtk_widget_get_style_context (widget), gc, ...
        cairo_set_source_surface (gc, pDesc->surface, 0, 0);
        cairo_paint (gc);                 /* copy surface into drawing area */

        if (pDesc->points >= 0)
        {
            g_snprintf (labelString, sizeof (labelString), _("%d Points"), pDesc->points);
            gtk_label_set_text (GTK_LABEL (pDesc->label), labelString);
        } /* if */
        else
        {
            gtk_label_set_text (GTK_LABEL (pDesc->label), "");
        } /* else */

        gdk_window_set_cursor (topWindow, NULL);   /* restore parent cursor */
    }

    return TRUE;                                           /* stop emission */
} /* responseWinDrawHandler() */



/**
 * \brief   This function is called when a response widget (GtkDrawingArea) is
 *          destroyed.
 *
 * \note    The function isn't called if the window is destroyed because of
 *          main application exit (e.g. by pressing Ctrl-Q).
 *
 * \param   object      Response widget which was destroyed.
 * \param   user_data   Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect() for \e destroy
 *                      event.
 */
static void responseWinDestroyed (GtkWidget* object, gpointer user_data)
{
    RESPONSE_WIN* pDesc = user_data;

    ASSERT(pDesc != NULL);

    pDesc->topWidget = NULL;
    gtk_check_menu_item_set_active (pDesc->menuref, FALSE);    /* update menu */

} /* responseWinDestroyed() */



/**
 * \brief   This function is called when a response widget is realized (becomes
 *          visible).
 *
 * \param   widget      Response widget (\e GtkDrawingArea) which was realized.
 * \param   user_data   Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect() for \e realize
 *                      event.
 */
static void responseWinMapped (GtkWidget* widget, gpointer user_data)
{
    RESPONSE_WIN* pDesc = user_data;

    ASSERT(pDesc != NULL);
    pDesc->diag.colors = pDesc->colors;      /* set pointer for color restore */
    cfgRestoreResponseSettings (pDesc->type, &pDesc->diag);

} /* responseWinMapped() */



/**
 * \brief   This function should be called when then properties button is clicked.
 *
 * \param   button      Button widget.
 * \param   user_data   Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect().
 */
static void responseWinBtnPropActivate (GtkButton *button, gpointer user_data)
{
    gint result;

    RESPONSE_WIN *pDesc = user_data;
    GtkWidget *dialog = responseDlgCreate (GTK_WINDOW (pDesc->topWidget), &pDesc->diag);

    do
    {
        result = gtk_dialog_run (GTK_DIALOG (dialog));

        switch (result)
        {
            case GTK_RESPONSE_APPLY:
            case GTK_RESPONSE_OK:
                if (responseDlgApply (dialog, &pDesc->diag) == 0)
                {
                    cfgSaveResponseSettings (pDesc->type, &pDesc->diag);
                    responseWinRedraw (pDesc->type);
                } /* if */
                else
                {
                    result = GTK_RESPONSE_APPLY;        /* dialog in progress */
                } /* else */

                break;

            default:
                break;
        } /* switch */
    }
    while ((result == GTK_RESPONSE_APPLY) || (result == GTK_RESPONSE_HELP));

    gtk_widget_destroy (dialog);

} /* responseWinBtnPropActivate */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/**
 * \brief   Toggles visibility of a filter response widget/window.
 * \note    This function should be called if a \e GtkCheckMenuItem from the
 *          \e View menu receives an \e activate event.
 *
 * \param   menuitem    Menu item which has received the \e activate event.
 * \param   user_data   Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 */
void responseWinMenuActivate (GtkMenuItem* menuitem, gpointer user_data)
{
    RESPONSE_WIN *pDesc;
    RESPONSE_TYPE type;

    type = GPOINTER_TO_INT (user_data);            /* type of response window */
    pDesc = &responseWidget[type];
    pDesc->menuref = GTK_CHECK_MENU_ITEM(menuitem);        /* store reference */

    if (gtk_check_menu_item_get_active (pDesc->menuref))
    {
        if (pDesc->topWidget == NULL)
        {
            pDesc->surface = NULL;
            pDesc->grab = NULL;
            responseWinCreate (pDesc);                   /* and create window */

            /* Because sometimes the expose event is missing after creation,
             * force redraw now.
             */
            responseWinRedraw (type);
        } /* if */
    } /* if */
    else
    {
        if (pDesc->topWidget != NULL)
        {
            gtk_widget_destroy (pDesc->topWidget);
        } /* if */
    } /* else */
} /* responseWinMenuActivate() */



/**
 * \brief   Invalidates one or all response windows for redrawing.
 *
 * \param   type        The response window which shall be redrawn. Set this
 *                      parameter to RESPONSE_TYPE_SIZE to redraw all.
 */
void responseWinRedraw (RESPONSE_TYPE type)
{
    RESPONSE_TYPE start, stop;
    BOOL filterValid = (dfcPrjGetFilter () != NULL);

    if (type < RESPONSE_TYPE_SIZE)
    {
        start = stop = type;
    } /* if */
    else                                                        /* redraw all */
    {
        start = 0;
        stop = RESPONSE_TYPE_SIZE - 1;
    } /* else */

    for (type = start; type <= stop; type++)
    {
        if (responseWidget[type].topWidget != NULL)
        {
            gtk_widget_set_sensitive (responseWidget[type].btnPrint, filterValid);
        } /* if */

        responseWinExpose (&responseWidget[type]);
    } /* for */
} /* responseWinRedraw() */


/**
 * \brief   Top widget callback function on \e key_press_event.
 * \note    This function is used to detect zoom mode cancel.
 *
 * \param   widget      Response plot widget handle (top-level).
 * \param   event       GDK event structure associated with \e key_press_event.
 * \param   user_data   Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 * \return  \c TRUE if the signal shall be propagated to the GDK default handler,
 *          else \c FALSE.
 */
static gboolean responseWinKeyPress (GtkWidget *widget, GdkEventKey *event,
                                     gpointer user_data)
{
    RESPONSE_WIN* pDesc = user_data;

    cancelZoomMode (pDesc);

    return FALSE;                    /* propagate signal to default handler */
} /* responseWinKeyPress() */


/**
 * \brief   Drawing area widget callback function on \e button_press_event (mouse
 *          button press).
 * \note    This function is used to detect start of zoom mode.
 *
 * \param   widget      Drawing area widget handle.
 * \param   event       GDK event structure associated with \e button_press_event.
 * \param   user_data   Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 * \return  \c TRUE if the event shall be propagated to the GDK default handler,
 *          else \c FALSE.
 */
static gboolean responseWinButtonPress (GtkWidget* widget, GdkEventButton* event,
                                        gpointer user_data)
{
    GdkCursor* cursor;

    RESPONSE_WIN* pDesc = user_data;
    GdkDisplay* display = gtk_widget_get_display (pDesc->draw);

    cancelZoomMode (pDesc);

    if ((event->button == 1) &&                         /* left mouse button? */
        (pDesc->points > 0) &&               /* else drawing area isn't valid */
        (event->x >= pDesc->diag.area.x) &&         /* inside plot/graph box? */
        (event->x < pDesc->diag.area.x + pDesc->diag.area.width) &&
        (event->y >= pDesc->diag.area.y) &&
        (event->y < pDesc->diag.area.y + pDesc->diag.area.height))
    {
#if GTK_CHECK_VERSION(3, 20, 0)
        GdkSeat* mouse = gdk_display_get_default_seat (display);
#else
        GdkDevice* mouse = gdk_device_manager_get_client_pointer (
            gdk_display_get_device_manager (display));
#endif

        DEBUG_LOG ("Mouse button 1 pressed in plot box (x = %G, y = %G)",
                   cairoPlotCoordinate (&pDesc->diag.x, pDesc->diag.area.x,
                                        pDesc->diag.area.x + pDesc->diag.area.width,
                                        event->x),
                   cairoPlotCoordinate (&pDesc->diag.y,
                                        pDesc->diag.area.y + pDesc->diag.area.height,
                                        pDesc->diag.area.y, event->y));

        cursor = gdk_cursor_new_from_name (display, GUI_CURSOR_IMAGE_CROSS);

#if GTK_CHECK_VERSION(3, 20, 0)
        if (gdk_seat_grab (mouse,
                           gtk_widget_get_window (pDesc->draw),
                           GDK_SEAT_CAPABILITY_ALL_POINTING,
                           TRUE,
                           cursor,
                           (GdkEvent*) event,
                           NULL, NULL) == GDK_GRAB_SUCCESS)
#else
        if (gdk_device_grab (mouse,
                             gtk_widget_get_window (pDesc->draw),
                             GDK_OWNERSHIP_NONE,
                             FALSE,
                             GDK_BUTTON_MOTION_MASK |              /* mouse */
                             GDK_BUTTON_PRESS_MASK |
                             GDK_BUTTON_RELEASE_MASK |
                             GDK_KEY_PRESS_MASK,                /* keyboard */
                             cursor,
                             gdk_event_get_time ((GdkEvent *) event)) == GDK_GRAB_SUCCESS)
#endif
        {
            pDesc->grab = mouse;
            pDesc->zoom.x = event->x;

            if (pDesc->diag.y.flags & PLOT_AXIS_FLAG_AUTO)
            {
                pDesc->zoom.y = pDesc->diag.area.y;
            } /* if */
            else
            {
                pDesc->zoom.y = event->y;
            } /* else */

            pDesc->zoom.width = pDesc->zoom.height = 0;
        } /* if */

        g_object_unref (cursor);   /* free client side resource immediately */
    } /* if */

    return FALSE;                    /* propagate signal to default handler */
} /* responseWinButtonPress() */



/**
 * \brief   Drawing area widget callback function on \e button_release_event.
 * \note    This function is used to detect (regular) end of zoom mode.
 *
 * \param   widget      Drawing area widget handle.
 * \param   event       GDK event structure associated with \e button_release_event.
 * \param   user_data   Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 * \return  \c TRUE if the event shall be propagated to the GDK default handler,
 *          else \c FALSE.
 */
static gboolean responseWinButtonRelease (GtkWidget* widget, GdkEventButton* event,
                                          gpointer user_data)
{
    int x, y;
    double tmpx, tmpy;

    RESPONSE_WIN *pDesc = user_data;

    if (pDesc->grab != NULL)
    {
        cancelZoomMode (pDesc);

        if (event->button == 1)                         /* left mouse button? */
        {
            x = event->x;
            y = event->y;

            if (x < pDesc->zoom.x)
            {
                MATH_SWAP_INT (x, pDesc->zoom.x);
            } /* if */

            if (y < pDesc->zoom.y)
            {
                MATH_SWAP_INT (y, pDesc->zoom.y);
            } /* if */

            DEBUG_LOG ("Mouse button 1 released while zooming (x = %G, y = %G)",
                       cairoPlotCoordinate (&pDesc->diag.x, pDesc->diag.area.x,
                                            pDesc->diag.area.x + pDesc->diag.area.width,
                                            x),
                       cairoPlotCoordinate (&pDesc->diag.y,
                                            pDesc->diag.area.y + pDesc->diag.area.height,
                                            pDesc->diag.area.y, y));

            if ((x > pDesc->zoom.x) && (y > pDesc->zoom.y) && /* not too small? */
                (pDesc->zoom.x >= pDesc->diag.area.x) && /* in graph area (box)? */
                (x < pDesc->diag.area.x + pDesc->diag.area.width) &&
                (pDesc->zoom.y >= pDesc->diag.area.y) &&
                (y < pDesc->diag.area.y + pDesc->diag.area.height))
            {
                tmpx = cairoPlotCoordinate (
                    &pDesc->diag.x, pDesc->diag.area.x,
                    pDesc->diag.area.x + pDesc->diag.area.width, pDesc->zoom.x);

                pDesc->diag.x.stop = cairoPlotCoordinate (
                    &pDesc->diag.x, pDesc->diag.area.x,
                    pDesc->diag.area.x + pDesc->diag.area.width, x);

                pDesc->diag.x.start = tmpx;

                if (!(pDesc->diag.y.flags & PLOT_AXIS_FLAG_AUTO))
                {
                    tmpy = cairoPlotCoordinate (
                        &pDesc->diag.y,
                        pDesc->diag.area.y + pDesc->diag.area.height,
                        pDesc->diag.area.y, pDesc->zoom.y);

                    pDesc->diag.y.start = cairoPlotCoordinate (
                        &pDesc->diag.y,
                        pDesc->diag.area.y + pDesc->diag.area.height,
                        pDesc->diag.area.y, y);

                    pDesc->diag.y.stop = tmpy;
                } /* if */

                responseWinExpose (pDesc);          /* redraw with new ranges */

            } /* if */
        } /* if */
    } /* if */


    return FALSE;                      /* propagate signal to default handler */

} /* responseWinButtonRelease() */




/**
 * \brief   Drawing area widget callback function on \e motion_notify_event.
 * \note    This function is used to track pointer movements while in zoom mode.
 *
 * \param   widget      Drawing area widget handle.
 * \param   event       GDK event structure associated with \e motion_notify_event.
 * \param   user_data   Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 * \return  \c TRUE if the event shall be propagated to the GDK default handler,
 *          else \c FALSE.
 */
static gboolean responseWinMotionNotify (GtkWidget *widget, GdkEventMotion *event,
                                         gpointer user_data)
{
    RESPONSE_WIN* pDesc = user_data;

    if (pDesc->grab != NULL)                                 /* in zoom mode? */
    {
        pDesc->zoom.width = event->x - pDesc->zoom.x;

        if (pDesc->diag.y.flags & PLOT_AXIS_FLAG_AUTO)
        {
            pDesc->zoom.height = pDesc->diag.area.height;
        } /* if */
        else
        {                                       /* height may become negative */
            pDesc->zoom.height = event->y - pDesc->zoom.y;
        } /* else */

        responseWinExpose (pDesc);                          /* draw rectangle */
    } /* if */

    return FALSE;                      /* propagate signal to default handler */
} /* responseWinMotionNotify() */
