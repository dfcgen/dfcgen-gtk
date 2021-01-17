/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     support.h
 * \brief    Support functions, defines and macros, mostly for \e gettext,
 *           widget and file handling.
 *
 * \author   Copyright (C) 2006, 2011-2012, 2020, 2021 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/


#ifndef SUPPORT_H
#define SUPPORT_H

/* INCLUDE FILES **************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>


/* MACROS *********************************************************************/

#if GTK_CHECK_VERSION (3, 10, 0)

#define DFCGEN_GTK_IMAGE_BUTTON_NEW(id)                         \
    gtk_button_new_from_icon_name (id, GTK_ICON_SIZE_BUTTON)
#define DFCGEN_GTK_IMAGE_MENUITEM_NEW(id, label, accel) \
    createImageMenuItem ((label), (id), accel)
#define DFCGEN_GTK_TOOL_BUTTON_NEW(id)                                        \
    ((GtkWidget*) gtk_tool_button_new (                                       \
        gtk_image_new_from_icon_name (id, GTK_ICON_SIZE_LARGE_TOOLBAR), NULL))


#define DFCGEN_GTK_STOCK_BUTTON_OK      "gtk-ok"
#define DFCGEN_GTK_STOCK_BUTTON_CANCEL  "gtk-cancel"
#define DFCGEN_GTK_STOCK_BUTTON_APPLY   "gtk-apply"
#define DFCGEN_GTK_STOCK_BUTTON_HELP    "gtk-help"

#define DFCGEN_GTK_STOCK_MENUITEM_ABOUT "gtk-about"
#define DFCGEN_GTK_STOCK_MENUITEM_HELP  "gtk-help"
#define DFCGEN_GTK_STOCK_MENUITEM_INFO  "gtk-info"

#define DFCGEN_GTK_STOCK_NEW            "gtk-new"
#define DFCGEN_GTK_STOCK_OPEN           "gtk-open"
#define DFCGEN_GTK_STOCK_SAVE           "gtk-save"
#define DFCGEN_GTK_STOCK_SAVE_AS        "gtk-save-as"
#define DFCGEN_GTK_STOCK_PRINT          "gtk-print"
#define DFCGEN_GTK_STOCK_PREFERENCES    "gtk-preferences"
#define DFCGEN_GTK_STOCK_QUIT           "gtk-quit"
#define DFCGEN_GTK_STOCK_COEFF_EDIT     "gtk-edit"
#define DFCGEN_GTK_STOCK_COEFF_MULTIPLY "gtk-fullscreen"
#define DFCGEN_GTK_STOCK_COEFF_ROUND    "gtk-convert"

#else /* GTK version < 3.10 */

#define DFCGEN_GTK_IMAGE_BUTTON_NEW(id) \
    gtk_button_new_from_stock (id)

#define DFCGEN_GTK_IMAGE_MENUITEM_NEW(id, label, accel) \
    gtk_image_menu_item_new_from_stock ((id), (accel));

#define DFCGEN_GTK_TOOL_BUTTON_NEW(id)                  \
    ((GtkWidget*) gtk_tool_button_new_from_stock (id))


#define DFCGEN_GTK_STOCK_BUTTON_OK      GTK_STOCK_OK
#define DFCGEN_GTK_STOCK_BUTTON_CANCEL  GTK_STOCK_CANCEL
#define DFCGEN_GTK_STOCK_BUTTON_APPLY   GTK_STOCK_APPLY
#define DFCGEN_GTK_STOCK_BUTTON_HELP    GTK_STOCK_HELP

#define DFCGEN_GTK_STOCK_MENUITEM_ABOUT GTK_STOCK_ABOUT
#define DFCGEN_GTK_STOCK_MENUITEM_HELP  GTK_STOCK_HELP
#define DFCGEN_GTK_STOCK_MENUITEM_INFO  GTK_STOCK_INFO

#define DFCGEN_GTK_STOCK_NEW            GTK_STOCK_NEW
#define DFCGEN_GTK_STOCK_OPEN           GTK_STOCK_OPEN
#define DFCGEN_GTK_STOCK_SAVE           GTK_STOCK_SAVE
#define DFCGEN_GTK_STOCK_SAVE_AS        GTK_STOCK_SAVE_AS
#define DFCGEN_GTK_STOCK_PRINT          GTK_STOCK_PRINT
#define DFCGEN_GTK_STOCK_PREFERENCES    GTK_STOCK_PREFERENCES
#define DFCGEN_GTK_STOCK_QUIT           GTK_STOCK_QUIT
#define DFCGEN_GTK_STOCK_COEFF_EDIT     GTK_STOCK_EDIT
#define DFCGEN_GTK_STOCK_COEFF_MULTIPLY GTK_STOCK_FULLSCREEN
#define DFCGEN_GTK_STOCK_COEFF_ROUND    GTK_STOCK_CONVERT

#endif  /* GTK_CHECK_VERSION */


/* If gettext.m4 has detected GNU gettext (libintl.h), then ENABLE_NLS is
 * defined. In that case a translation to the user's natural language may
 * be possible (if there are catalog files for that language installed).
 */
#ifdef ENABLE_NLS

#include <libintl.h>

#undef _
#define _(String) dgettext (PACKAGE, String)
#define Q_(String) g_strip_context ((String), gettext (String))

#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif

#else  /* !ENABLE_NLS (no translation) */

#define textdomain(String) (String)
#define gettext(String) (String)
#define dgettext(Domain,Message) (Message)
#define dcgettext(Domain,Message,Type) (Message)
#define bindtextdomain(Domain,Directory) (Domain)

#define _(String) (String)
#define Q_(String) g_strip_context ((String), (String))
#define N_(String) (String)
#endif  /* ENABLE_NLS */


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/** Directory Path Identifiers.
 *
 *  \attention Don't change the enums, because order must match array indices
 *             in support.c.
 */
typedef enum
{
    DIR_ID_INVALID = -1,                     /**< Invalid ID (unused so far) */
    DIR_ID_TEMPLATES = 0,                           /**< Templates directory */
    DIR_ID_PIXMAPS = 1,                               /**< Pixmaps directory */
    DIR_ID_FILTERS = 2,                    /**< Predefined filters directory */
    DIR_ID_LOCALE = 3,                                 /**< Locale directory */

    DIR_ID_SIZE                               /**< Size of DIRECTORY_ID enum */
} DIRECTORY_ID;



/* EXPORTED FUNCTIONS *********************************************************/


/* FUNCTION *******************************************************************/
/** \brief This function returns a widget in a component created by \e Glade.
 *
 *         Call it with the toplevel widget in the component, or alternatively
 *         any widget in the component, and the name of the widget you want
 *         returned.
 *
 *  \param[in] widget   Top level widget pointer.
 *  \param[in] name     Name of widget to be searched for.
 *
 *  \return             Pointer to widget, if found, else NULL.
 ******************************************************************************/
GtkWidget* lookup_widget (GtkWidget* widget, const gchar* widget_name);


/* FUNCTION *******************************************************************/
/** \brief This function returns a path to a directory, using UTF-8 encoding.
 *
 *  \param[in] dir_id   Directory identifier.
 *
 *  \return             A newly allocated string that must be freed with g_free().
 ******************************************************************************/
gchar* getPackageDirectory (DIRECTORY_ID dir_id);



/* FUNCTION *******************************************************************/
/** \brief Create a \c GdkPixbuf from a pixmaps file.
 *
 *  \param[in] filename Pixmpas filename.
 *
 *  \return    A newly allocated \c GdkPixbuf, representing the pixmap file.
 ******************************************************************************/
GdkPixbuf* createPixbufFromFile (const gchar* filename);



#if GTK_CHECK_VERSION (3, 10, 0)
/* FUNCTION *******************************************************************/
/** \brief Replacement for gtk_image_menu_item_new_from_stock() which is
 *         deprecated since GTK 3.10.
 *
 *  \param[in]      name    Label to be applied.
 *  \param[in]      img     Icon image name.
 *  \param[in,out]  accel   The associated accelerator group.
 *
 *  \return    A newly created menu item.
 ******************************************************************************/
GtkWidget* createImageMenuItem(const gchar* name, const gchar* img,
                               GtkAccelGroup* accel);
#endif


#endif /* SUPPORT_H */
