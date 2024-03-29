/**
 * \file        support.h
 * \brief       Support functions, defines and macros, mostly for \e gettext,
 *              widget and file handling.
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <dfcgen@rho62.de>
 */

#ifndef SUPPORT_H
#define SUPPORT_H

/* INCLUDE FILES **************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>


/* MACROS *********************************************************************/

/* If gettext.m4 has detected GNU gettext (libintl.h), then ENABLE_NLS is
 * defined. In that case a translation to the user's natural language may
 * be possible (if there are catalog files for that language installed).
 */
#ifdef ENABLE_NLS

#include <libintl.h>

/*
 * Macros _() and N_() are keywords which trigger `gettext' (see also Make
 * variable XGETTEXT_OPTIONS).
 */
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



/* Some macros moved here from interface.c, which is generated by Glade (but
 * not used directly in this application).
 */
#define GLADE_HOOKUP_OBJECT(component,widget,name)                             \
    g_object_set_data_full (G_OBJECT (component), name,                        \
                            g_object_ref (widget), (GDestroyNotify) g_object_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
    g_object_set_data (G_OBJECT (component), name, widget)



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



/* FUNCTION *******************************************************************/
/** \brief Replacement for gtk_image_menu_item_new_from_stock() which is
 *         deprecated since GTK 3.10.
 *
 *  \param[in]      name    Label to be applied.
 *  \param[in]      img     Icon image name.
 *  \param[in,out]  accel_group   The accelerator group.
 *  \param[in]      accel_key     The accelerator key, or \c GDK_KEY_VoidSymbol
 *                          in case there is none.
 *
 *  \return    A newly created menu item.
 ******************************************************************************/
GtkWidget* createImageMenuItem (const gchar* name, const gchar* img,
                                GtkAccelGroup* accel_group, guint accel_key);


/* FUNCTION *******************************************************************/
/** \brief Replacement for gtk_button_new_from_stock() which is deprecated
 *         since GTK 3.10.
 *
 *  \param[in]      name    Label to be applied (translated by \e gettext).
 *  \param[in]      img     Icon image (file) name.
 *
 *  \return    A newly created \e GtkButton.
 ******************************************************************************/
GtkWidget* createImageButton (const gchar* name, const gchar* img);


#endif /* SUPPORT_H */
