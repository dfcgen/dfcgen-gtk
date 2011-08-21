/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Digital filter response window creation and callbacks.
 *
 * \author   Copyright (C) 2006, 2011 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Id$
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "responsePlot.h"
#include "responseWin.h"
#include "responseDlg.h"
#include "cfgSettings.h"
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
    GdkColor colors[PLOT_COLOR_SIZE];        /**< Allocated colors to be used */
    int points;                 /**< Points drawed on last responsePlotDraw() */
    GdkRectangle zoom;                 /**< Zoom coordinates (last rectangle) */
    GdkGCValues original;                /**< Saved GdkGC values when zooming */
    GtkCheckMenuItem *menuref;            /**< (Backward) menu item reference */
    GtkWidget* topWidget;        /**< response plot window (top-level widget) */
    GtkWidget* draw;                            /**< \e GtkDrawingArea widget */
    GtkWidget* label;             /**< Label which shows the number of points */
} RESPONSE_WIN;


/* LOCAL CONSTANT DEFINITIONS *************************************************/


#define RESPONSE_WIN_GRAPH_THICKNESS    (1.5)           /**< Graph line width */


/** Used GDK graphics context for zoom rectangle
 */
#define RESPONSE_WIN_ZOOM_GC(widget)  (widget)->style->dark_gc[GTK_STATE_NORMAL]


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void drawZoomRect (RESPONSE_WIN *pDesc);
static void cancelZoomMode (RESPONSE_WIN *pDesc);
static void responseWinCreate (RESPONSE_WIN *pDesc);
static gboolean exposeHandler (GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
static BOOL responseWinExpose (RESPONSE_WIN* pDesc);
static void responseWinDrawAreaMapped (GtkWidget* widget, gpointer user_data);
static void responseWinDrawAreaDestroy (GtkObject* object, gpointer user_data);
static void responseWinBtnProperties (GtkButton *button, gpointer user_data);
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
 *
 *  \todo Try to move some of these data here into responsePlot.c
 */
static RESPONSE_WIN responseWidget[RESPONSE_TYPE_SIZE] =
{
    {
        RESPONSE_TYPE_MAGNITUDE,                                      /* type */
        "amplitude.png",                                          /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>"),                                /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>H</b>")                                 /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_MAGNITUDE], /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        },
    },
    {
        RESPONSE_TYPE_ATTENUATION,
        "attenuation.png",                                        /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>")                                 /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>A</b>"), &plotUnitDB                    /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_ATTENUATION], /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        },
    },
    {
        RESPONSE_TYPE_CHAR,
        "charfunc.png",                                           /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>")                                 /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>D</b>")                                 /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_CHAR],      /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        },
    },
    {
        RESPONSE_TYPE_PHASE,
        "phase.png",                                              /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>")                                 /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>B</b>"), &plotUnitDeg                   /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_PHASE],     /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        },
    },
    {
        RESPONSE_TYPE_DELAY,
        "phasedelay.png",                                         /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>")                                 /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>T<sub>p</sub></b>")                     /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_DELAY],     /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        }
    },
    {
        RESPONSE_TYPE_GROUP,
        "grpdelay.png",                                           /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>")                                 /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>T<sub>g</sub></b>")                     /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_GROUP],     /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        }
    },
    {
        RESPONSE_TYPE_IMPULSE,
        "impulse.png",                                            /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>t</b>")                                 /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>h</b>")                                 /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_IMPULSE],   /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        },
    },
    {
        RESPONSE_TYPE_STEP,
        "step.png",                                               /* iconFile */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>t</b>")                                 /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>g</b>")                                 /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_STEP],      /* pData (backward ptr) */
            RESPONSE_WIN_GRAPH_THICKNESS,                  /* graph thickness */
        }
    }
}; /* responseWidget[] */



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/



/* FUNCTION *******************************************************************/
/** Cancels the zoom mode by restoring the original graphics context and
 *  erasing the last zoom rubberband. Call this function only if really in
 *  zoom mode, means the pointer is grabbed.
 *
 *  \param pDesc        Pointer to response window/widget description.
 *
 ******************************************************************************/
static void drawZoomRect (RESPONSE_WIN *pDesc)
{
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

    gdk_draw_rectangle (GDK_DRAWABLE(pDesc->draw->window),  /* erase old rect */
                        RESPONSE_WIN_ZOOM_GC(pDesc->draw), FALSE,
                        rect.x, rect.y, rect.width, rect.height);
} /* drawZoomRect() */



/* FUNCTION *******************************************************************/
/** Cancels the zoom mode by restoring the original graphics context and
 *  erasing the last zoom rubberband. Call this function only if really in
 *  zoom mode, means the pointer is grabbed.
 *
 *  \param pDesc        Pointer to response window/widget description.
 *
 ******************************************************************************/
static void cancelZoomMode (RESPONSE_WIN *pDesc)
{
    drawZoomRect (pDesc);
    gdk_pointer_ungrab (GDK_CURRENT_TIME);       /* clear zoom mode indicator */
    gdk_keyboard_ungrab (GDK_CURRENT_TIME);

    gdk_gc_set_values (RESPONSE_WIN_ZOOM_GC(pDesc->draw), &pDesc->original,
                       GDK_GC_FUNCTION);
} /* cancelZoomMode() */



/* FUNCTION *******************************************************************/
/** Creates a filter response widget/window. Such a window is used to display
 *  - magnitude reponse
 *  - phase response
 *  - step response etc.
 *
 *  \note               This function has changed after \e glade code generation.
 *
 *  \param pDesc        Pointer to response window/widget description.
 *
 *  \todo               Restore size of drawing area from last session
 ******************************************************************************/
static void responseWinCreate (RESPONSE_WIN *pDesc)
{
    GtkWidget *vbox;
    GtkWidget *hseparator;
    GtkWidget *hbox;
    GtkWidget *btnSettings, *btnPrint;
    GdkPixbuf *iconPixbuf;

    GtkTooltips *tooltips = gtk_tooltips_new ();

    pDesc->topWidget = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (pDesc->topWidget), TRUE); /* added */
    gtk_window_set_skip_pager_hint (GTK_WINDOW (pDesc->topWidget), TRUE); /* added */

    gtk_window_set_title (GTK_WINDOW (pDesc->topWidget),
                          gtk_label_get_text (
                              GTK_LABEL (
                                  gtk_bin_get_child (
                                      GTK_BIN (pDesc->menuref)))));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (pDesc->topWidget), TRUE);
    gtk_window_set_focus_on_map (GTK_WINDOW (pDesc->topWidget), FALSE);

    iconPixbuf = create_pixbuf (pDesc->iconFile);

    if (iconPixbuf != NULL)
    {
        gtk_window_set_icon (GTK_WINDOW (pDesc->topWidget), iconPixbuf);
        gdk_pixbuf_unref (iconPixbuf);
    } /* if */

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (pDesc->topWidget), vbox);

    pDesc->draw = gtk_drawing_area_new ();
    gtk_widget_set_size_request (pDesc->draw, 350, 240);
    gtk_widget_add_events (pDesc->draw, GDK_EXPOSURE_MASK | GDK_BUTTON_MOTION_MASK |
                           GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_box_pack_start (GTK_BOX (vbox), pDesc->draw, TRUE, TRUE, 6);

    hseparator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, TRUE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 6);

    pDesc->label = gtk_label_new ("");
    gtk_box_pack_start (GTK_BOX (hbox), pDesc->label, FALSE, FALSE, 6);
    gtk_label_set_single_line_mode (GTK_LABEL (pDesc->label), TRUE);

    btnPrint = gtk_button_new_from_stock ("gtk-print");
    gtk_box_pack_end (GTK_BOX (hbox), btnPrint, FALSE, FALSE, 6);

#if 1 || !GTK_CHECK_VERSION(2, 10, 0)            /* print support requires GTK 2.10 */
    gtk_widget_set_sensitive (GTK_WIDGET(btnPrint), FALSE);
#endif

    GTK_WIDGET_SET_FLAGS (btnPrint, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip (tooltips, btnPrint, _("Print this response plot"), NULL);

    btnSettings = gtk_button_new_from_stock ("gtk-preferences");
    gtk_box_pack_end (GTK_BOX (hbox), btnSettings, FALSE, FALSE, 6);
    GTK_WIDGET_SET_FLAGS (btnSettings, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip (tooltips, btnSettings, _("Set response plot preferences"), NULL);

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

    g_signal_connect ((gpointer) pDesc->draw, "expose_event",
                      G_CALLBACK (exposeHandler),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "destroy",
                      G_CALLBACK (responseWinDrawAreaDestroy),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "map",
                      G_CALLBACK (responseWinDrawAreaMapped),
                      pDesc);
    g_signal_connect ((gpointer) btnSettings, "clicked",
                      G_CALLBACK (responseWinBtnProperties),
                      pDesc);


    gtk_widget_show_all (pDesc->topWidget);

    gtk_widget_grab_focus (btnSettings);
    gtk_widget_grab_default (btnSettings);

} /* responseWinCreate() */



/* FUNCTION *******************************************************************/
/** Invalidates the drawing area of a response window.
 *
 *  \param pDesc    Pointer to response window description.
 *
 *  \return             TRUE if the associated response does really exist, else
 *                      FALSE.
 ******************************************************************************/
static BOOL responseWinExpose (RESPONSE_WIN* pDesc)
{
    if (pDesc->topWidget != NULL)
    {
        gdk_window_invalidate_region (pDesc->draw->window,
                                      gdk_drawable_get_visible_region (
                                          GDK_DRAWABLE(pDesc->draw->window)),
                                      FALSE);
        return TRUE;
    } /* if */

    return FALSE;
} /* responseWinExpose() */



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
static gboolean exposeHandler (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    cairo_t* cr;
    char labelString[128];

    RESPONSE_WIN* pDesc = user_data;
    GdkDrawable* drawable = GDK_DRAWABLE(widget->window);
    GdkCursor* cursor = gdk_cursor_new (GDK_WATCH);

    ASSERT(pDesc != NULL);
    gdk_window_set_cursor (pDesc->topWidget->window, cursor);
    gdk_cursor_unref (cursor);       /* free client side resource immediately */

    pDesc->diag.area.x = pDesc->diag.area.y = 0;     /* set size of plot area */
    gdk_drawable_get_size (drawable, &pDesc->diag.area.width, &pDesc->diag.area.height);

    cr = gdk_cairo_create (drawable);                 /* create cairo context */
    pDesc->points = responsePlotDraw (cr, pDesc->type, &pDesc->diag);

    if (pDesc->points >= 0)
    {
        g_snprintf (labelString, sizeof(labelString), _("%d Points"), pDesc->points);
        gtk_label_set_text (GTK_LABEL(pDesc->label), labelString);
    } /* if */
    else
    {
        gtk_label_set_text (GTK_LABEL(pDesc->label), "");
    } /* else */


    cairo_destroy(cr);                                  /* free cairo context */
    gdk_window_set_cursor(pDesc->topWidget->window, NULL); /* restore parent cursor */

    return TRUE;                                             /* stop emission */
} /* exposeHandler() */



/* FUNCTION *******************************************************************/
/** This function is called when a response widget (GtkDrawingArea) is
 *  destroyed.
 *
 *  \note  The function isn't called if the window is destroyed because of
 *         main application exit (e.g. by pressing Ctrl-Q).
 *
 *  \param object       Response widget which was destroyed.
 *  \param user_data    Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect() for \e destroy
 *                      event.
 *
 ******************************************************************************/
static void responseWinDrawAreaDestroy(GtkObject* object, gpointer user_data)
{
    RESPONSE_WIN* pDesc = user_data;

    ASSERT(pDesc != NULL);

    pDesc->topWidget = NULL;
    gdk_colormap_free_colors (gdk_colormap_get_system (), pDesc->colors,
                              PLOT_COLOR_SIZE);
    gtk_check_menu_item_set_active (pDesc->menuref, FALSE);    /* update menu */

} /* responseWinDrawAreaDestroy() */



/* FUNCTION *******************************************************************/
/** This function is called when a response widget (GtkDrawingArea) is realized.
 *
 *  \param widget       Response widget which was realized.
 *  \param user_data    Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect() for \e realize
 *                      event.
 *
 ******************************************************************************/
static void responseWinDrawAreaMapped (GtkWidget* widget, gpointer user_data)
{
    gboolean success[PLOT_COLOR_SIZE];

    RESPONSE_WIN* pDesc = user_data;

    ASSERT(pDesc != NULL);
    pDesc->diag.colors = pDesc->colors;      /* set pointer for color restore */
    cfgRestoreResponseSettings (pDesc->type, &pDesc->diag);

    if (gdk_colormap_alloc_colors (gdk_colormap_get_system (), pDesc->colors,
                                   PLOT_COLOR_SIZE, TRUE, TRUE, success) != 0)
    {
        DEBUG_LOG ("Could not allocate colors in system colormap");
    } /* if */

} /* responseWinDrawAreaMapped() */



/* FUNCTION *******************************************************************/
/** This function should be called when then properties button is clicked.
 *
 *  \param button       Button widget.
 *  \param user_data    Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect().
 *
 ******************************************************************************/
static void responseWinBtnProperties (GtkButton *button, gpointer user_data)
{
    gint result;

    RESPONSE_WIN *pDesc = user_data;
    GtkWidget *dialog = responseDlgCreate (&pDesc->diag);

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

} /* responseWinBtnProperties */


/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Toggles visibility of a filter response widget/window. This function
 *  should be called if a \e GtkCheckMenuItem from the \e View menu receives
 *  an \e activate event.
 *
 *  \param menuitem     Menu item which has received the \e activate event.
 *  \param user_data    Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 ******************************************************************************/
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



/* FUNCTION *******************************************************************/
/** Invalidates one or all response windows for redrawing.
 *
 *  \param type         The response window which shall be redrawn. Set this
 *                      parameter to RESPONSE_TYPE_SIZE to redraw all.
 *
 ******************************************************************************/
void responseWinRedraw (RESPONSE_TYPE type)
{
    if (type < RESPONSE_TYPE_SIZE)
    {
        (void)responseWinExpose (&responseWidget[type]);
    } /* if */
    else                                                        /* redraw all */
    {
        for (type = 0; type < RESPONSE_TYPE_SIZE; type++)
        {
            (void)responseWinExpose (&responseWidget[type]);
        } /* for */
    } /* else */
} /* responseWinRedraw() */


/* FUNCTION *******************************************************************/
/** Top widget callback function on \e key_press_event. This function is used
 *  to detect zoom mode cancel.
 *
 *  \param widget       Response plot widget handle (top-level).
 *  \param event        GDK event structure associated with \e key_press_event.
 *  \param user_data    Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 *  \return             TRUE if the signal shall be propagated to the GDK
 *                      default handler, else FALSE.
 ******************************************************************************/
static gboolean responseWinKeyPress (GtkWidget *widget, GdkEventKey *event,
                                     gpointer user_data)
{
    if (gdk_pointer_is_grabbed ())                           /* in zoom mode? */
    {
        cancelZoomMode (user_data);
    } /* if */

    return FALSE;                      /* propagate signal to default handler */
} /* responseWinKeyPress() */


/* FUNCTION *******************************************************************/
/** Drawing area widget callback function on \e button_press_event. This
 *  function is used to detect start of zoom mode.
 *
 *  \param widget       Drawing area widget handle.
 *  \param event        GDK event structure associated with \e button_press_event.
 *  \param user_data    Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 *  \return             TRUE if the event shall be propagated to the GDK
 *                      default handler, else FALSE.
 ******************************************************************************/
static gboolean responseWinButtonPress (GtkWidget* widget, GdkEventButton* event,
                                        gpointer user_data)
{
    GdkCursor* cursor;
    GdkGC* gc;

    RESPONSE_WIN *pDesc = user_data;

    if (gdk_pointer_is_grabbed ())                           /* in zoom mode? */
    {
        cancelZoomMode (user_data);
    } /* if */

    if ((event->button == 1) &&                         /* left mouse button? */
        (pDesc->points > 0) &&               /* else drawing area isn't valid */
        (event->x >= pDesc->diag.area.x) &&         /* inside plot/graph box? */
        (event->x < pDesc->diag.area.x + pDesc->diag.area.width) &&
        (event->y >= pDesc->diag.area.y) &&
        (event->y < pDesc->diag.area.y + pDesc->diag.area.height))
    {
        DEBUG_LOG ("Mouse button 1 pressed in plot box (x = %G, y = %G)",
                   cairoPlotCoordinate (&pDesc->diag.x, pDesc->diag.area.x,
                                        pDesc->diag.area.x + pDesc->diag.area.width,
                                        event->x),
                   cairoPlotCoordinate (&pDesc->diag.y,
                                        pDesc->diag.area.y + pDesc->diag.area.height,
                                        pDesc->diag.area.y, event->y));

        cursor = gdk_cursor_new (GDK_CROSS);

        if (gdk_pointer_grab (pDesc->draw->window, FALSE,
                              GDK_BUTTON_MOTION_MASK |
                              GDK_BUTTON_PRESS_MASK |
                              GDK_BUTTON_RELEASE_MASK,
                              pDesc->draw->window,
                              cursor, event->time) == GDK_GRAB_SUCCESS)
        {
            if (gdk_keyboard_grab (pDesc->topWidget->window, GDK_KEY_PRESS_MASK,
                                   event->time) == GDK_GRAB_SUCCESS)
            {
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

                gc = RESPONSE_WIN_ZOOM_GC(pDesc->draw);
                gdk_gc_get_values (gc, &pDesc->original);
                gdk_gc_set_function (gc, GDK_XOR);
                gdk_gc_set_line_attributes (gc, 1, GDK_LINE_ON_OFF_DASH,
                                            GDK_CAP_BUTT, GDK_JOIN_MITER);
                gdk_gc_set_clip_rectangle (gc, &pDesc->diag.area);
                drawZoomRect (pDesc);
            } /* if */
            else                                                     /* error */
            {
                gdk_pointer_ungrab (GDK_CURRENT_TIME);
            } /* else */
        } /* if */

        gdk_cursor_unref (cursor);   /* free client side resource immediately */
    } /* if */

    return FALSE;                      /* propagate signal to default handler */
} /* responseWinButtonPress() */



/* FUNCTION *******************************************************************/
/** Drawing area widget callback function on \e button_release_event. This
 *  function is used to detect (regular) end of zoom mode.
 *
 *  \param widget       Drawing area widget handle.
 *  \param event        GDK event structure associated with \e button_release_event.
 *  \param user_data    Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 *  \return             TRUE if the event shall be propagated to the GDK
 *                      default handler, else FALSE.
 ******************************************************************************/
static gboolean responseWinButtonRelease (GtkWidget* widget, GdkEventButton* event,
                                          gpointer user_data)
{
    int x, y;
    double tmpx, tmpy;

    RESPONSE_WIN *pDesc = user_data;

    if (gdk_pointer_is_grabbed ())                           /* in zoom mode? */
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

                (void)responseWinExpose (pDesc);    /* redraw with new ranges */

            } /* if */
        } /* if */
    } /* if */


    return FALSE;                      /* propagate signal to default handler */

} /* responseWinButtonRelease() */




/* FUNCTION *******************************************************************/
/** Drawing area widget callback function on \e motion_notify_event. This
 *  function is used to track pointer movements while in zoom mode.
 *
 *  \param widget       Drawing area widget handle.
 *  \param event        GDK event structure associated with \e motion_notify_event.
 *  \param user_data    Pointer to response window type (RESPONSE_WIN_TYPE).
 *
 *  \return             TRUE if the event shall be propagated to the GDK
 *                      default handler, else FALSE.
 ******************************************************************************/
static gboolean responseWinMotionNotify (GtkWidget *widget, GdkEventMotion *event,
                                         gpointer user_data)
{
    if (gdk_pointer_is_grabbed ())                           /* in zoom mode? */
    {
        RESPONSE_WIN *pDesc = user_data;

        drawZoomRect (pDesc);                          /* erase old rectangle */
        pDesc->zoom.width = event->x - pDesc->zoom.x;

        if (pDesc->diag.y.flags & PLOT_AXIS_FLAG_AUTO)
        {
            pDesc->zoom.height = pDesc->diag.area.height;
        } /* if */
        else
        {                                       /* height may become negative */
            pDesc->zoom.height = event->y - pDesc->zoom.y;
        } /* else */


        drawZoomRect (pDesc);                           /* draw new rectangle */

    } /* if */

    return FALSE;                      /* propagate signal to default handler */
} /* responseWinMotionNotify() */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
