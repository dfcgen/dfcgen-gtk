/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     cfgSettings.c
 * \brief    DFCGen configuration settings.
 *
 * \note     All double values written to the configuration file are formated
 *           in the "C" locale for LC_NUMERIC.
 *
 * \author   Copyright (C) 2006, 2011, 2012, 2020 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


/* INCLUDE FILES **************************************************************/

#include "cfgSettings.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h> /* memcpy() */

#include <gsl/gsl_const.h>



/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/


/** Settings to be saved/restored for a (response window) plot axis. The
 *  \a flags member is used as an attribute.
 */
typedef struct
{
    double start;               /**< real-world coordinate of start-point */
    double stop;                  /**< real-world coordinate of end-point */
    unsigned flags; /**< flags (e.g. PLOT_AXIS_FLAG_LOG, PLOT_AXIS_FLAG_GRID) */
} CFG_AXIS_SETTINGS;


/** Settings to be saved/restored for the response plot.
 */
typedef struct
{
    char *key;                               /**< Configuration file key name */
    CFG_AXIS_SETTINGS x;
    CFG_AXIS_SETTINGS y;
    PLOT_STYLE style;                                     /**< Style of graph */
    int num;                                   /**< Number of samples to take */
    unsigned flags;                          /**< Flags, see CFG_FLAG_VISIBLE */
    GdkColor color[PLOT_COLOR_SIZE];                         /**< Color array */
} CFG_RESPONSE_SETTINGS;




/* LOCAL CONSTANT DEFINITIONS *************************************************/

#define CFG_DEFAULT_F_STOP      10E3              /**< Default stop frequency */
#define CFG_DEFAULT_T_STOP      (10 / CFG_DEFAULT_F_STOP) /**< Default stop time */
#define CFG_FLAG_VISIBLE        1             /**< Response window is visible */


/** Color format string (old X11 spec). Because GDK 2.8 does not accept the
 *  newer one, we use the old format when writing the config file. This may
 *  change in the future if GDK understands both syntax.
 */
#define COLOR_FORMAT_STRING     "#%04X%04X%04X"
/* #define COLOR_FORMAT_STRING     "rgb:%04x/%04x/%04x"*/ /* New X11 color spec. format */

#ifdef G_OS_WIN32
#define CFG_FILE_NAME           PACKAGE ".ini"        /**< Configuration file */
#else
#define CFG_FILE_NAME           PACKAGE ".conf"       /**< Configuration file */
#endif

#define CFG_GROUP_APPLICATION   "Application" /**< Group key for \e Application */
#define CFG_KEY_APPNAME         "AppName"           /**< Application name key */
#define CFG_KEY_APPVERSION      "Version"        /**< Application version key */
#define CFG_GROUP_DESKTOP       "Desktop"       /**< Group key for \e Desktop */
#define CFG_KEY_UNIT_F          "Frequency"               /**< frequency unit */
#define CFG_KEY_UNIT_T          "Time"                         /**< Time unit */
#define CFG_KEY_PRECISION       "Precision"    /**< Output precision (digits) */
#define CFG_GROUP_WINDOW        "Window-"  /**< (Partial) group key for plots */
#define CFG_KEY_POINTS          "Points"   /**< Number of points used in plot */
#define CFG_KEY_STYLE           "Style"                       /**< Plot style */
#define CFG_KEY_COLORS          "Colors"                  /**< Colors of plot */
#define CFG_KEY_VISIBLE         "Visibility"         /**< Flag for visibility */
#define CFG_KEY_X_START         "xStart"              /**< Start value x-axis */
#define CFG_KEY_X_STOP          "xStop"                /**< Stop value x-axis */
#define CFG_KEY_X_LOG           "xLogarithmic"    /**< Log. option for x-axis */
#define CFG_KEY_X_GRID          "xGrid"           /**< Grid option for x-axis */
#define CFG_KEY_Y_START         "yStart"              /**< Start value y-axis */
#define CFG_KEY_Y_STOP          "yStop"                /**< Stop value y-axis */
#define CFG_KEY_Y_LOG           "yLogarithmic"    /**< Log. option for y-axis */
#define CFG_KEY_Y_GRID          "yGrid"           /**< Grid option for y-axis */
#define CFG_KEY_Y_AUTO          "yAuto"           /**< Auto option for y-axis */


/* LOCAL VARIABLE DEFINITIONS *************************************************/


/** Desktop configuration settings (initialized with defaults).
 */
static CFG_DESKTOP deskPrefs =
{
    {"µs", GSL_CONST_NUM_MICRO},                                  /* timeUnit */
    {"kHz", GSL_CONST_NUM_KILO},                                 /* frequUnit */
    6                                             /* default output precision */
};



/** Response windows settings.
 */
static CFG_RESPONSE_SETTINGS respSet[RESPONSE_TYPE_SIZE] =
{
    [RESPONSE_TYPE_MAGNITUDE] =
    {
        CFG_GROUP_WINDOW "Magnitude",
        {0.0, CFG_DEFAULT_F_STOP, PLOT_AXIS_FLAG_GRID},
        {0, 1, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_LINE_ONLY, 0, 0
    },
    [RESPONSE_TYPE_ATTENUATION] =
    {
        CFG_GROUP_WINDOW "Attenuation",
        {0.0, CFG_DEFAULT_F_STOP, PLOT_AXIS_FLAG_GRID},
        {0, 60, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_LINE_ONLY, 0, 0
    },
    [RESPONSE_TYPE_CHAR] =
    {
        CFG_GROUP_WINDOW "Char",
        {0.0, CFG_DEFAULT_F_STOP, PLOT_AXIS_FLAG_GRID},
        {0, 360, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_LINE_ONLY, 0, 0
    },
    [RESPONSE_TYPE_PHASE] =
    {
        CFG_GROUP_WINDOW "Phase",
        {0.0, CFG_DEFAULT_F_STOP, PLOT_AXIS_FLAG_GRID},
        {0, 360, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_LINE_ONLY, 0, 0
    },
    [RESPONSE_TYPE_DELAY] =
    {
        CFG_GROUP_WINDOW "Delay",
        {0.0, CFG_DEFAULT_F_STOP, PLOT_AXIS_FLAG_GRID},
        {0, CFG_DEFAULT_T_STOP, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_LINE_ONLY, 0, 0
    },
    [RESPONSE_TYPE_GROUP] =
    {
        CFG_GROUP_WINDOW "Group",
        {0.0, CFG_DEFAULT_F_STOP, PLOT_AXIS_FLAG_GRID},
        {0, CFG_DEFAULT_T_STOP, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_LINE_ONLY, 0, 0
    },
    [RESPONSE_TYPE_IMPULSE] =
    {
        CFG_GROUP_WINDOW "Impulse",
        {0.0, CFG_DEFAULT_T_STOP, PLOT_AXIS_FLAG_GRID},
        {0, 1, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_CIRCLE_SAMPLE, 0, 0
    },
    [RESPONSE_TYPE_STEP] =
    {
        CFG_GROUP_WINDOW "Step",
        {0.0, CFG_DEFAULT_T_STOP, PLOT_AXIS_FLAG_GRID},
        {0, 1, PLOT_AXIS_FLAG_GRID | PLOT_AXIS_FLAG_AUTO},
        PLOT_STYLE_CIRCLE_SAMPLE, 0, 0
    }
};



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* MACRO **********************************************************************/
/** Gets a widgets color.
 *
 * \note                The widget attribute \p var accessed by this macro may
 *                      deprecated in later revision of GTK. In that case they
 *                      will be replaced e.g. by functions as gtk_style_get_fg().
 *
 * \param widget        Widget.
 * \param var           Name of (may be private) color variable: fg, bg, light,
 *                      dark, mid, text, base, text_aa
 * \param state         State of widget: GTK_STATE_NORMAL, ...
 *
 * \return              GdkColor associated with style and state.
 */
#define CFG_WIDGET_COLOR(widget, var, state) (widget)->style->var[state]




/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void cfgReadFlag (GKeyFile *keyFile, const gchar *group, const gchar *key,
                         unsigned mask, unsigned *pFlags);
static void cfgReadInteger (GKeyFile *keyFile, const gchar *group, const gchar *key,
                            int *pResult);
static void cfgReadDouble (GKeyFile *keyFile, const gchar *group, const gchar *key,
                           double *pResult);
static void cfgReadUnit (GKeyFile *keyFile, const gchar *group, const gchar *key,
                         PLOT_UNIT *pUnit);



/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** Reads an integer value from key file associated with \a key under \a group.
 *  If the key is not found then target of \a pResult is unchanged.
 *
 *  \param keyFile      The key file (GKeyFile) to be used for search operation.
 *  \param group        The group name.
 *  \param key          Key inside \a group which is searched.
 *  \param pUnit        Pointer to unit buffer.
 *
 ******************************************************************************/
static void cfgReadUnit (GKeyFile *keyFile, const gchar *group, const gchar *key,
                         PLOT_UNIT *pUnit)
{
    static PLOT_UNIT units[] =                             /* all known units */
    {
        {"Hz", 1.0},
        {"kHz", GSL_CONST_NUM_KILO},
        {"MHz", GSL_CONST_NUM_MEGA},
        {"GHz", GSL_CONST_NUM_GIGA},
        {"s", 1.0},
        {"ms", GSL_CONST_NUM_MILLI},
        {"µs", GSL_CONST_NUM_MICRO},
        {"ns", GSL_CONST_NUM_NANO},
        {"ps", GSL_CONST_NUM_PICO}
    };

    int i;

    char *unitName = g_key_file_get_string (keyFile, group, key, NULL);

    if (unitName != NULL)
    {
        for (i = 0; i < N_ELEMENTS (units); i++)
        {
            if (g_strcmp0 (unitName, units[i].name) == 0)          /* found? */
            {
                pUnit->name = units[i].name;
                pUnit->multiplier = units[i].multiplier;

                g_free (unitName);
                return;
            } /* if */
        } /* for */

        g_free (unitName);
    } /* if */
} /* cfgReadUnit() */



/* FUNCTION *******************************************************************/
/** Reads an integer value from key file associated with \a key under \a group.
 *  If the key is not found then target of \a pResult is unchanged.
 *
 *  \param keyFile      The key file (GKeyFile) to be used for search operation.
 *  \param group        The group name.
 *  \param key          Key inside \a group which is searched.
 *  \param pResult      Pointer to result buffer.
 *
 ******************************************************************************/
static void cfgReadInteger (GKeyFile *keyFile, const gchar *group, const gchar *key,
                            int *pResult)
{
    GError *error = NULL;                                  /* preset required */
    int result = g_key_file_get_integer (keyFile, group, key, &error);

    if (error == NULL)
    {
        *pResult = result;
    } /* if */
    else
    {
        g_error_free (error);
    } /* else */

} /* cfgReadInteger() */


/* FUNCTION *******************************************************************/
/** Reads a double from key file associated with \a key under \a group.
 *  If the key is not found then target of \a pResult is unchanged.
 *
 *  \param keyFile      The key file (GKeyFile) to be used for search operation.
 *  \param group        The group name.
 *  \param key          Key inside \a group which is searched.
 *  \param pResult      Pointer to result buffer.
 *
 ******************************************************************************/
static void cfgReadDouble (GKeyFile *keyFile, const gchar *group, const gchar *key,
                           double *pResult)
{
    GError *error = NULL;                                  /* preset required */
    double result = g_key_file_get_double(keyFile, group, key, &error);

    if (error == NULL)
    {
        *pResult = result;
    } /* if */
    else
    {
        g_error_free(error);
    } /* else */

} /* cfgReadDouble() */



/* FUNCTION *******************************************************************/
/** Updates a flag associated with a boolean value, which itself is associated
 *  with \a key under \a group. If the key is not found then target of \a pFlags
 *  is unchanged.
 *
 *  \param keyFile      The key file (GKeyFile) to be used for search operation.
 *  \param group        The group name.
 *  \param key          Key inside \a group which is searched.
 *  \param mask         Mask associated with the boolean in key file.
 *  \param pFlags       Pointer to flags to be modified (on success).
 *
 ******************************************************************************/
static void cfgReadFlag (GKeyFile *keyFile, const gchar *group, const gchar *key,
                         unsigned mask, unsigned *pFlags)
{
    GError *error = NULL;

    gboolean flag = g_key_file_get_boolean (keyFile, group, key, &error);

    if (error == NULL)                          /* all seems okay (key found) */
    {
        if (flag)
        {
            *pFlags |= mask;
        } /* if */
        else
        {
            *pFlags &= ~mask;
        } /* else */
    } /* if */
    else
    {
        g_error_free(error);
    } /* else */

} /* cfgReadFlag() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** Reads the DFCGen configuration from XDG_CONFIG_DIR, the XDG user
 *  configuration directory (see XDG specification at
 *  http://www.freedesktop.org/Standards/basedir-spec).
 *
 *  \param widget       Top level widget used for default colors assignment.
 *
 ******************************************************************************/
void cfgCacheSettings (GtkWidget *widget)
{
    CFG_RESPONSE_SETTINGS* pSet;
    RESPONSE_TYPE i;
    GdkColor color;
    char **colorList;  /* pointer to array of char pointers (NULL-terminated) */
    int colorIdx;

    GKeyFile* keyFile = g_key_file_new ();
    char *name = g_build_filename (g_get_user_config_dir (), CFG_FILE_NAME, NULL);

    /* First set default colors
     */
    for (i = 0, pSet = respSet; i < RESPONSE_TYPE_SIZE; i++, pSet++)
    {
        pSet->color[PLOT_COLOR_LABELS] =
        pSet->color[PLOT_COLOR_AXIS_NAME] =
        pSet->color[PLOT_COLOR_BOX] =
        pSet->color[PLOT_COLOR_GRID] =
        pSet->color[PLOT_COLOR_GRAPH] =
            CFG_WIDGET_COLOR (widget, text, GTK_STATE_NORMAL);


        pSet->color[PLOT_COLOR_NOTE_TEXT] =
        pSet->color[PLOT_COLOR_NOTE_BOX] =
            CFG_WIDGET_COLOR (widget, text_aa, GTK_STATE_NORMAL);
    } /* for */


    if (g_key_file_load_from_file (keyFile, name, G_KEY_FILE_NONE, NULL))
    {

        /* Read desktop group
         */
        cfgReadUnit (keyFile, CFG_GROUP_DESKTOP, CFG_KEY_UNIT_T, &deskPrefs.timeUnit);
        cfgReadUnit (keyFile, CFG_GROUP_DESKTOP, CFG_KEY_UNIT_F, &deskPrefs.frequUnit);
        cfgReadInteger (keyFile, CFG_GROUP_DESKTOP, CFG_KEY_PRECISION, &deskPrefs.outprec);


        /* Read window group
         */
        for (i = 0, pSet = respSet; i < RESPONSE_TYPE_SIZE; i++, pSet++)
        {
            cfgReadFlag (keyFile, pSet->key, CFG_KEY_VISIBLE,
                         CFG_FLAG_VISIBLE, &pSet->flags);

            cfgReadInteger (keyFile, pSet->key, CFG_KEY_POINTS, &pSet->num);
            cfgReadInteger (keyFile, pSet->key, CFG_KEY_STYLE, (int *) &pSet->style);

            colorList = g_key_file_get_string_list (keyFile, pSet->key,
                                                    CFG_KEY_COLORS, NULL, NULL);

            if (colorList != NULL)                              /* key found? */
            {
                colorIdx = 0;

                while ((colorList[colorIdx] != NULL) && (colorIdx < PLOT_COLOR_SIZE))
                {                                 /* get all colors from list */
                    if (gdk_color_parse (colorList[colorIdx], &color))
                    {
                        pSet->color[colorIdx] = color;
                    } /* if */
                    else
                    {
                        DEBUG_LOG("Color syntax \"%s\" in group %s bad)",
                                  colorList[colorIdx], pSet->key);
                    } /* else */

                    ++colorIdx;
                } /* while */

                g_strfreev(colorList);    /* frees a NULL-terminated array of */
            }                             /* strings (incl. the array itself) */

            cfgReadDouble (keyFile, pSet->key, CFG_KEY_X_START, &pSet->x.start);
            cfgReadDouble (keyFile, pSet->key, CFG_KEY_X_STOP, &pSet->x.stop);

            cfgReadFlag (keyFile, pSet->key, CFG_KEY_X_LOG,
                         PLOT_AXIS_FLAG_LOG, &pSet->x.flags);
            cfgReadFlag (keyFile, pSet->key, CFG_KEY_X_GRID,
                         PLOT_AXIS_FLAG_GRID, &pSet->x.flags);

            cfgReadDouble (keyFile, pSet->key, CFG_KEY_Y_START, &pSet->y.start);
            cfgReadDouble (keyFile, pSet->key, CFG_KEY_Y_STOP, &pSet->y.stop);

            cfgReadFlag (keyFile, pSet->key, CFG_KEY_Y_LOG,
                         PLOT_AXIS_FLAG_LOG, &pSet->y.flags);
            cfgReadFlag (keyFile, pSet->key, CFG_KEY_Y_GRID,
                         PLOT_AXIS_FLAG_GRID, &pSet->y.flags);
            cfgReadFlag (keyFile, pSet->key, CFG_KEY_Y_AUTO,
                         PLOT_AXIS_FLAG_AUTO, &pSet->y.flags);

        } /* for */

    } /* if */

    g_key_file_free (keyFile);
    g_free (name);

} /* cfgCacheSettings() */



/* FUNCTION *******************************************************************/
/** Writes the DFCGen configuration to the XDG user configuration directory
 *  (see XDG specification at http://www.freedesktop.org/Standards/basedir-spec).
 *  The syntax of key files is described in the \e Desktop \e Entry \e Specification
 *  at http://freedesktop.org/Standards/desktop-entry-spec.
 *
 *  \return             Zero on success, else an error number.
 ******************************************************************************/
int cfgFlushSettings ()
{
    CFG_RESPONSE_SETTINGS* pSet;
    RESPONSE_TYPE i;
    int colorIdx;
    gsize size;
    const char* colorStr[PLOT_COLOR_SIZE];        /* array of string pointers */

    GKeyFile* keyFile = g_key_file_new ();
    char *data = g_build_filename (g_get_user_config_dir (), CFG_FILE_NAME, NULL);
    FILE *f = fopen(data, "w");

    if (f == NULL)
    {
        DEBUG_LOG("Cannot open configuration file");
        return errno;
    } /* if */


    /* First write application group
     */
    g_key_file_set_comment (keyFile, NULL, NULL,
                            " -*- mode: conf; coding: utf-8 -*-\n"
                            " " PACKAGE " " VERSION " session settings (UTF-8 coded)",
                            NULL);

    g_key_file_set_string (keyFile, CFG_GROUP_APPLICATION, CFG_KEY_APPNAME, PACKAGE);
    g_key_file_set_string (keyFile, CFG_GROUP_APPLICATION, CFG_KEY_APPVERSION, VERSION);


    /* Write desktop group
     */
    g_key_file_set_string (keyFile, CFG_GROUP_DESKTOP, CFG_KEY_UNIT_T, deskPrefs.timeUnit.name);
    g_key_file_set_string (keyFile, CFG_GROUP_DESKTOP, CFG_KEY_UNIT_F, deskPrefs.frequUnit.name);
    g_key_file_set_integer (keyFile, CFG_GROUP_DESKTOP, CFG_KEY_PRECISION, deskPrefs.outprec);
    g_key_file_set_comment (keyFile, CFG_GROUP_DESKTOP, NULL, "", NULL);


    /* Write all window groups
     */
    for (i = 0, pSet = respSet; i < RESPONSE_TYPE_SIZE; i++, pSet++)
    {
        g_key_file_set_boolean (keyFile, pSet->key, CFG_KEY_VISIBLE,
                                pSet->flags & CFG_FLAG_VISIBLE);

        g_key_file_set_integer (keyFile, pSet->key, CFG_KEY_POINTS, pSet->num);
        g_key_file_set_integer (keyFile, pSet->key, CFG_KEY_STYLE, pSet->style);

        for (colorIdx = 0; colorIdx < PLOT_COLOR_SIZE; colorIdx++)
        {
            colorStr[colorIdx] = g_strdup_printf (COLOR_FORMAT_STRING,
                                                  pSet->color[colorIdx].red,
                                                  pSet->color[colorIdx].green,
                                                  pSet->color[colorIdx].blue);
        } /* for */

        g_key_file_set_string_list (keyFile, pSet->key, CFG_KEY_COLORS,
                                    colorStr, PLOT_COLOR_SIZE);

        for (colorIdx = 0; colorIdx < PLOT_COLOR_SIZE; colorIdx++)
        {
            g_free ((gpointer) colorStr[colorIdx]);
        } /* for */


        g_key_file_set_double (keyFile, pSet->key, CFG_KEY_X_START, pSet->x.start);
        g_key_file_set_double (keyFile, pSet->key, CFG_KEY_X_STOP, pSet->x.stop);
        g_key_file_set_boolean (keyFile, pSet->key, CFG_KEY_X_LOG,
                                pSet->x.flags & PLOT_AXIS_FLAG_LOG);
        g_key_file_set_boolean (keyFile, pSet->key, CFG_KEY_X_GRID,
                                pSet->x.flags & PLOT_AXIS_FLAG_GRID);

        g_key_file_set_double (keyFile, pSet->key, CFG_KEY_Y_START, pSet->y.start);
        g_key_file_set_double (keyFile, pSet->key, CFG_KEY_Y_STOP, pSet->y.stop);
        g_key_file_set_boolean (keyFile, pSet->key, CFG_KEY_Y_LOG,
                                pSet->y.flags & PLOT_AXIS_FLAG_LOG);
        g_key_file_set_boolean (keyFile, pSet->key, CFG_KEY_Y_GRID,
                                pSet->y.flags & PLOT_AXIS_FLAG_GRID);
        g_key_file_set_boolean (keyFile, pSet->key, CFG_KEY_Y_AUTO,
                                pSet->y.flags & PLOT_AXIS_FLAG_AUTO);

        g_key_file_set_comment (keyFile, respSet[i].key, NULL, "", NULL);

    } /* for */

    data = g_key_file_to_data (keyFile, &size, NULL);
    fwrite (data, size, 1, f);
    g_key_file_free (keyFile);

    if (fclose (f) != 0)
    {
        return ENOENT;
    } /* if */

    return 0;
} /* cfgFlushSettings() */



/* FUNCTION *******************************************************************/
/** Saves the response window configuration settings. It is assumed that the
 *  response window is closed when calling this function.
 *
 *  \param type         Type of response plot/window.
 *  \param pDiag        Pointer to plot which holds the current settings.
 *
 ******************************************************************************/
void cfgSaveResponseSettings(RESPONSE_TYPE type, PLOT_DIAG* pDiag)
{
    CFG_RESPONSE_SETTINGS* pSet = &respSet[type];

    pSet->x.start = pDiag->x.start;
    pSet->x.stop = pDiag->x.stop;
    pSet->x.flags = pDiag->x.flags;
    pSet->y.start = pDiag->y.start;
    pSet->y.stop = pDiag->y.stop;
    pSet->y.flags = pDiag->y.flags;

    pSet->style = pDiag->style;
    pSet->num = pDiag->num;

    memcpy (pSet->color, pDiag->colors, PLOT_COLOR_SIZE * sizeof (pSet->color[0]));
    pSet->flags &= ~CFG_FLAG_VISIBLE;              /* assume window is closed */

} /* cfgSaveResponseSettings() */


/* FUNCTION *******************************************************************/
/** Restores the response window configuration settings. It is assumed that the
 *  response window is visible when calling this function with a valid \a pDiag
 *  pointer.
 *
 *  \param type         Type of response plot/window.
 *  \param pDiag        Pointer to plot which gets the settings from last
 *                      session. If \a pDiag is NULL, then the last
 *                      \e Visibility state is returned. But notice that
 *                      colors are always unallocated.
 *
 *  \return             TRUE if the response window should be visible (on),
 *                      FALSE if not (off).
 *
 ******************************************************************************/
BOOL cfgRestoreResponseSettings(RESPONSE_TYPE type, PLOT_DIAG* pDiag)
{
    BOOL state;

    CFG_RESPONSE_SETTINGS* pSet = &respSet[type];

    state = pSet->flags & CFG_FLAG_VISIBLE;            /* save old visibility */

    if (pDiag != NULL)
    {
        pDiag->x.start = pSet->x.start;
        pDiag->x.stop = pSet->x.stop;
        pDiag->x.flags = pSet->x.flags;
        pDiag->y.start = pSet->y.start;
        pDiag->y.stop = pSet->y.stop;
        pDiag->y.flags = pSet->y.flags;

        pDiag->style = pSet->style;
        pDiag->num = pSet->num;

        memcpy (pDiag->colors, pSet->color, PLOT_COLOR_SIZE * sizeof (pSet->color[0]));
        pSet->flags |= CFG_FLAG_VISIBLE;                  /* prepare for save */

    } /* if */


    return state;

} /* cfgRestoreResponseSettings() */



/* FUNCTION *******************************************************************/
/** Sets new desktop configuration settings (preferences).
 *
 *  \param newPrefs     Pointer to new preferences.
 *
 ******************************************************************************/
void cfgSetDesktopPrefs (CFG_DESKTOP* newPrefs)
{
    memcpy (&deskPrefs, newPrefs, sizeof (deskPrefs));
} /* cfgSetDesktopPrefs() */


/* FUNCTION *******************************************************************/
/** Gets the current desktop configuration settings (preferences).
 *
 *  \return    Pointer to current preferences.
 ******************************************************************************/
const CFG_DESKTOP* cfgGetDesktopPrefs ()
{
    return &deskPrefs;
} /* cfgGetDesktopPrefs() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
