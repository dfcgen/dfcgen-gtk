/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Digital filter response window creation and callbacks.
 *
 * \author   Copyright (c) 2006 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Header: /home/cvs/dfcgen-gtk/src/responseWin.c,v 1.1.1.1 2006-09-11 15:52:19 ralf Exp $
 *
 *
 * History:
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "responsePlot.h"
#include "responseWin.h"
#include "responseDlg.h"
#include "cfgSettings.h"

#include <gsl/gsl_const.h>


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

/** Response window description
 */
typedef struct
{
    RESPONSE_TYPE type;                     /**< Type of response plot/window */
    char *name;                             /**< Name of response plot/window */
    PLOT_DIAG diag;                                            /**< Plot data */
    GdkColor colors[PLOT_COLOR_SIZE];        /**< Allocated colors to be used */
    int recursion;      /**< \e expose event draw recursion reference counter */
    GtkCheckMenuItem *menuref;            /**< (Backward) menu item reference */
    GtkWidget* main;             /**< response plot window (top-level widget) */
    GtkWidget* draw;                            /**< \e GtkDrawingArea widget */
    GtkWidget* pbar;               /**< Progress bar pointer (GtkProgressBar) */
    GtkWidget* label;             /**< Label which shows the number of points */
} RESPONSE_WIN;


/* LOCAL CONSTANT DEFINITIONS *************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static int progressCallback(void *pData, double percent);
static void responseWinCreate (RESPONSE_WIN *pDesc);
static gboolean responseWinDraw (GtkWidget *widget, GdkEventExpose *event, gpointer user_data);
static BOOL responseWinInvalidate (RESPONSE_WIN* pDesc);
static void responseWinDrawAreaMapped (GtkWidget* widget, gpointer user_data);
static void responseWinDrawAreaDestroy (GtkObject* object, gpointer user_data);
static void responseWinProgressBarUnrealize (GtkWidget* widget, gpointer user_data);
static void responseWinBtnProperties (GtkButton *button, gpointer user_data);
gboolean responseWinButtonPress (GtkWidget* widget, GdkEventButton* event, gpointer user_data);
gboolean responseWinButtonRelease (GtkWidget* widget, GdkEventButton* event, gpointer user_data);


/* LOCAL VARIABLE DEFINITIONS *************************************************/

/** Frequency units
 */
static PLOT_UNIT unitF[] =
{
    {N_("Hz"), 1.0},
    {N_("kHz"), GSL_CONST_NUM_KILO},
    {N_("MHz"), GSL_CONST_NUM_MEGA},
    {N_("GHz"), GSL_CONST_NUM_GIGA}
};


/** Time units
 */
static PLOT_UNIT unitT[] =
{
    {N_("s"), 1.0},
    {N_("ms"), GSL_CONST_NUM_MILLI},
    {N_("µs"), GSL_CONST_NUM_MICRO},
    {N_("ns"), GSL_CONST_NUM_NANO},
    {N_("ps"), GSL_CONST_NUM_PICO}
};



/** All predefined response plots (partially intialized).
 */
static RESPONSE_WIN responseWidget[RESPONSE_TYPE_SIZE] =
{
    {
        RESPONSE_TYPE_AMPLITUDE,                                      /* type */
        N_("Amplitude Response"),                                     /* name */
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>"), &unitF[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>H</b>"), NULL,                          /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_AMPLITUDE], /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        },
    },
    {
        RESPONSE_TYPE_ATTENUATION,
        N_("Attenuation Response"),
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>"), &unitF[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                N_(PLOT_AXISNAME_FORMAT("<b>A</b>", "dB")), NULL, /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_ATTENUATION], /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        },
    },
    {
        RESPONSE_TYPE_CHAR,
        N_("Characteristic Function"),
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>"), &unitF[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>D</b>"), NULL,                          /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_CHAR],      /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        },
    },
    {
        RESPONSE_TYPE_PHASE,
        N_("Phase Response"),
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>"), &unitF[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                N_(PLOT_AXISNAME_FORMAT("<b>B</b>", "°")), NULL, /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_PHASE],     /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        },
    },
    {
        RESPONSE_TYPE_DELAY,
        N_("Phase Delay"),
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>"), &unitF[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>T<sub>p</sub></b>"), &unitT[1],         /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_DELAY],     /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        }
    },
    {
        RESPONSE_TYPE_GROUP,
        N_("Group Delay"),
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>f</b>"), &unitF[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                N_("<b>T<sub>g</sub></b>"), &unitT[1],         /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_GROUP],     /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        }
    },
    {
        RESPONSE_TYPE_IMPULSE,
        N_("Impulse Response"),
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>t</b>"), &unitT[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                NULL, NULL,                                    /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_IMPULSE],   /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        },
    },
    {
        RESPONSE_TYPE_STEP,
        N_("Step Response"),
        {                                                             /* diag */
            {                                                            /* x */
                N_("<b>t</b>"), &unitT[1],                     /* name, pUnit */
            },
            {                                                            /* y */
                NULL, NULL,                                    /* name, pUnit */
            },
            &responseWidget[RESPONSE_TYPE_STEP],      /* pData (backward ptr) */
            progressCallback,                                 /* progressFunc */
        }
    }
}; /* responseWidget[] */



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Progress bar callback function. This function must match the type
 *  declaration PLOT_FUNC_PROGRESS in cairoPlot.h.
 *
 *  \param pData        User application data pointer as stored in member
 *                      \a pData of structure PLOT_DIAG (as passed to function
 *                      cairoPlot2d().
 *  \param percent      A value between 0.0 and 1.0 which indicates the
 *                      percentage of completion.
 *
 *  \return             The function returns a value unequal to 0, if the
 *                      plot has to be cancelled.
 *  \todo               Avoid the call \e g_main_context_iteration (use threads?)
 *                      This may lead to recursion on \e expose events and other
 *                      unexpected behaviour (e.g. on \e destroy events).
 ******************************************************************************/
static int progressCallback(void *pData, double percent)
{
    RESPONSE_WIN *pDesc = pData;

    if (pDesc->pbar != NULL)
    {
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pDesc->pbar), percent);
/*        while (g_main_context_iteration (NULL, FALSE)); */
    } /* if */

    return 0;
} /* progressCallback() */



/* FUNCTION *******************************************************************/
/** Creates a filter response widget/window. Such a window is used to display
 *  - amplitude reponse
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

    GtkTooltips *tooltips = gtk_tooltips_new ();

    pDesc->main = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (pDesc->main), TRUE); /* added */
    gtk_window_set_skip_pager_hint (GTK_WINDOW (pDesc->main), TRUE); /* added */

    gtk_window_set_title (GTK_WINDOW (pDesc->main), gettext (pDesc->name));
    gtk_window_set_destroy_with_parent (GTK_WINDOW (pDesc->main), TRUE);
    gtk_window_set_focus_on_map (GTK_WINDOW (pDesc->main), FALSE);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (pDesc->main), vbox);

    pDesc->draw = gtk_drawing_area_new ();
    gtk_widget_set_size_request (pDesc->draw, 350, 240);
    gtk_widget_add_events (pDesc->draw, GDK_EXPOSURE_MASK | GDK_BUTTON1_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_box_pack_start (GTK_BOX (vbox), pDesc->draw, TRUE, TRUE, 6);
    gtk_widget_set_sensitive (pDesc->draw, FALSE);

    hseparator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, TRUE, 0);

    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 6);

    pDesc->pbar = gtk_progress_bar_new ();
    gtk_box_pack_start (GTK_BOX (hbox), pDesc->pbar, FALSE, FALSE, 6);
    gtk_progress_bar_set_ellipsize (GTK_PROGRESS_BAR (pDesc->pbar), PANGO_ELLIPSIZE_END);

    pDesc->label = gtk_label_new ("");
    gtk_box_pack_start (GTK_BOX (hbox), pDesc->label, FALSE, FALSE, 6);
    gtk_label_set_single_line_mode (GTK_LABEL (pDesc->label), TRUE);

    btnSettings = gtk_button_new_from_stock ("gtk-preferences");
    gtk_box_pack_start (GTK_BOX (hbox), btnSettings, TRUE, FALSE, 6);
    GTK_WIDGET_SET_FLAGS (btnSettings, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip (tooltips, btnSettings, _("Set response plot preferences"), NULL);

    btnPrint = gtk_button_new_from_stock ("gtk-print");
    gtk_box_pack_start (GTK_BOX (hbox), btnPrint, TRUE, FALSE, 6);

#if !GTK_CHECK_VERSION(2, 10, 0)            /* print support requires GTK 2.10 */
    gtk_widget_set_sensitive (GTK_WIDGET(btnPrint), FALSE);
#endif

    GTK_WIDGET_SET_FLAGS (btnPrint, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip (tooltips, btnPrint, _("Print this response plot"), NULL);

    g_signal_connect ((gpointer) pDesc->draw, "button_press_event",
                      G_CALLBACK (responseWinButtonPress),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "button_release_event",
                      G_CALLBACK (responseWinButtonRelease),
                      pDesc);
                      
    g_signal_connect ((gpointer) pDesc->draw, "expose_event",
                      G_CALLBACK (responseWinDraw),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "destroy",
                      G_CALLBACK (responseWinDrawAreaDestroy),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->draw, "map",
                      G_CALLBACK (responseWinDrawAreaMapped),
                      pDesc);
    g_signal_connect ((gpointer) pDesc->pbar, "unrealize",
                      G_CALLBACK (responseWinProgressBarUnrealize),
                      pDesc);
    g_signal_connect ((gpointer) btnSettings, "clicked",
                      G_CALLBACK (responseWinBtnProperties),
                      pDesc);

    gtk_widget_show_all (pDesc->main);

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
static BOOL responseWinInvalidate (RESPONSE_WIN* pDesc)
{
    if (pDesc->main != NULL)
    {
        gdk_window_invalidate_region (pDesc->draw->window,
                                      gdk_drawable_get_visible_region (
                                          GDK_DRAWABLE(pDesc->draw->window)),
                                      FALSE);
        return TRUE;
    } /* if */

    return FALSE;
} /* responseWinInvalidate() */



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
static gboolean responseWinDraw (GtkWidget *widget, GdkEventExpose *event, gpointer user_data)
{
    int points;
    cairo_t* cr;
    char labelString[128];

    RESPONSE_WIN* pDesc = user_data;
    GdkDrawable* drawable = GDK_DRAWABLE(widget->window);
    GdkCursor* cursor = gdk_cursor_new (GDK_WATCH);

    ASSERT(pDesc != NULL);
    gdk_window_set_cursor (pDesc->main->window, cursor);
    gdk_cursor_unref (cursor);       /* free client side resource immediately */

    if ((++pDesc->recursion == 1) && (pDesc->pbar != NULL))
    {
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pDesc->pbar), 0.0);
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (pDesc->pbar), _("Calculating"));
    } /* if */

    pDesc->diag.area.x = pDesc->diag.area.y = 0;     /* set size of plot area */
    gdk_drawable_get_size (drawable, &pDesc->diag.area.width, &pDesc->diag.area.height);

    cr = gdk_cairo_create (drawable);                 /* create cairo context */
    points = responsePlotDraw (cr, pDesc->type, &pDesc->diag);

    if (points >= 0)
    {
        g_snprintf (labelString, sizeof(labelString), _("%d Points"), points);
        gtk_label_set_text (GTK_LABEL(pDesc->label), labelString);
    } /* if */
    else
    {
        gtk_label_set_text (GTK_LABEL(pDesc->label), "");
    } /* else */


    cairo_destroy(cr);                                  /* free cairo context */

    if ((pDesc->pbar != NULL) && (--pDesc->recursion == 0))
    {
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (pDesc->pbar), NULL);
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(pDesc->pbar), 0.0);
    } /* if */

    gdk_window_set_cursor(pDesc->main->window, NULL); /* restore parent cursor */

    return TRUE;                                             /* stop emission */
} /* responseWinDraw() */



/* FUNCTION *******************************************************************/
/** This function is called when a response widget (GtkDrawingArea) is destroyed.
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

    pDesc->diag.colors = pDesc->colors;    /* set current colors into diag... */
    cfgSaveResponseSettings (pDesc->type, &pDesc->diag);      /* ...then save */

    pDesc->main = NULL;
    pDesc->recursion = 0;

    gdk_colormap_free_colors(gdk_colormap_get_system (), pDesc->colors,
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
/** This function should be called when a progree bar is unrealized.
 *
 *  \param widget       Response widget which was destroyed.
 *  \param user_data    Pointer to response description (of type RESPONSE_WIN)
 *                      as supplied to function g_signal_connect() for the
 *                      \e unrealize event.
 *
 ******************************************************************************/
static void responseWinProgressBarUnrealize (GtkWidget* widget, gpointer user_data)
{
    ASSERT(user_data != NULL);

    ((RESPONSE_WIN*)user_data)->pbar = NULL;
} /* responseWinProgressBarUnrealize() */


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
    GtkWidget *dialog = responseDlgCreate ();

    do
    {
        result = gtk_dialog_run (GTK_DIALOG (dialog));
    }
    while (result == GTK_RESPONSE_APPLY);

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

    ASSERT(user_data != NULL);

    type = *(RESPONSE_TYPE*)user_data;             /* type of response window */
    pDesc = &responseWidget[type];
    pDesc->menuref = GTK_CHECK_MENU_ITEM(menuitem);    /* store reference */

    if (gtk_check_menu_item_get_active (pDesc->menuref))
    {
        if (pDesc->main == NULL)
        {
            pDesc->recursion = 0;
            responseWinCreate (pDesc);               /* and create window */
        } /* if */
    } /* if */
    else
    {
        if (pDesc->main != NULL)
        {
            gtk_widget_destroy (pDesc->main);
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
        (void)responseWinInvalidate (&responseWidget[type]);
    } /* if */
    else                                                        /* redraw all */
    {
        for (type = 0; type < RESPONSE_TYPE_SIZE; type++)
        {
            (void)responseWinInvalidate (&responseWidget[type]);
        } /* for */
    } /* else */
} /* responseWinRedraw() */



gboolean responseWinButtonPress (GtkWidget* widget, GdkEventButton* event, gpointer user_data)
{

  return FALSE;
}


gboolean responseWinButtonRelease (GtkWidget* widget, GdkEventButton* event, gpointer user_data)
{

  return FALSE;
}



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
