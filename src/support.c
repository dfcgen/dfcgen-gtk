/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     support.c
 *
 * \brief    Support functions, defines and macros, mostly for \e gettext,
 *           widget and file handling.
 *
 * \author   Copyright (C) 2006-2021 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 *
 ******************************************************************************/

#include "gui.h"     /* includes base.h and config.h */
#include "support.h"

#include <string.h>  /* strlen() */


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
GtkWidget* lookup_widget (GtkWidget* widget, const gchar* name)
{
    GtkWidget *parent, *found_widget;

    for (;;)
    {
        if (GTK_IS_MENU (widget))
        {
            parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
        } /* if */
        else
        {
            parent = gtk_widget_get_parent (widget);
        } /* else */

        if (parent == NULL)
        {
            parent = (GtkWidget*) g_object_get_data (G_OBJECT (widget), "GladeParentKey");
        } /* if */

        if (parent == NULL)
        {
            break;
        } /* if */

        widget = parent;
    } /* for */

    found_widget = (GtkWidget*) g_object_get_data (G_OBJECT (widget), name);

#ifdef DEBUG
  if (found_widget == NULL)
  {
      g_warning ("Widget '%s' not found", name);
  } /* if */
#endif

  return found_widget;
} /* lookup_widget() */



/* FUNCTION *******************************************************************/
/** \brief This function returns a path to a directory, using UTF-8 encoding.
 *
 *  \param[in] dir_id   Directory identifier.
 *
 *  \return             A newly allocated string that must be freed with g_free().
 ******************************************************************************/
gchar* getPackageDirectory (DIRECTORY_ID dir_id)
{
    static const gchar* pkgdir[DIR_ID_SIZE] =
    {
        PACKAGE_TEMPLATES_DIR,                       /* (0) DIR_ID_TEMPLATES */
        PACKAGE_PIXMAPS_DIR,                           /* (1) DIR_ID_PIXMAPS */
        PACKAGE_FILTERS_DIR,                           /* (2) DIR_ID_FILTERS */
        PACKAGE_LOCALE_DIR                              /* (3) DIR_ID_LOCALE */
    };

    gchar* path;

#ifdef G_OS_WIN32
#if GTK_CHECK_VERSION (2, 18, 0)
    gchar* root = g_win32_get_package_installation_directory_of_module (NULL);
#else
    gchar* root = g_win32_get_package_installation_directory (NULL, NULL);
#endif

    if (root != NULL)                       /* installation directory found? */
    {
        const gchar* pStart = pkgdir[dir_id];
        const gchar* pEnd = pStart + strlen (pStart);

        DEBUG_LOG ("Trying to determine sub-directory from '%s'", pStart);

        while ((pEnd != pStart) && (*pEnd != '/'))
        {
            --pEnd;
        } /* while */

        if (*pEnd == '/')
        {
            ++pEnd;
        } /* if */

        path = g_build_filename (root, "share", pEnd, NULL);

        g_free (root);
        DEBUG_LOG ("Sub-directory '%s' mapped to '%s'", pEnd, path);
    } /* if */
    else /* use the MinGW path */
#endif
    path = g_strdup (pkgdir[dir_id]);

    if (path == NULL)
    {
        g_critical (_("Couldn't locate package sub-directory no. %d"), dir_id);
    } /* if */

    return path;
} /* getPackageDirectory() */




/* FUNCTION *******************************************************************/
/** \brief Create a \c GdkPixbuf from a pixmaps file.
 *
 *  \param[in] filename Pixmpas filename.
 *
 *  \return    A newly allocated \c GdkPixbuf, representing the pixmap file.
 ******************************************************************************/
GdkPixbuf* createPixbufFromFile (const gchar* filename)
{
    gchar *path;
    gchar *subdir;
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    ASSERT (filename != NULL);

    subdir = getPackageDirectory (DIR_ID_PIXMAPS);
    path = g_build_filename (subdir, filename, NULL);

    if (path == NULL)
    {
        g_free (subdir);
        g_warning (_("Couldn't find pixmap file '%s'"), filename);

        return NULL;
    } /* if */

    pixbuf = gdk_pixbuf_new_from_file (path, &error);

    if (pixbuf == NULL)
    {
        g_warning (_("Failed to create pixbuf from file '%s': %s"),
                   path, error->message);
        g_error_free (error);
    } /* if */

    g_free (path);
    g_free (subdir);

    return pixbuf;
} /* createPixbufFromFile() */


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
                               GtkAccelGroup* accel)
{
    GtkWidget *item = gtk_menu_item_new ();
    GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *icon = gtk_image_new_from_icon_name (img, GTK_ICON_SIZE_MENU);
    GtkWidget *label = gtk_accel_label_new (name);

    gtk_container_add (GTK_CONTAINER (box), icon);
    gtk_label_set_use_underline (GTK_LABEL (label), TRUE);
    gtk_label_set_xalign (GTK_LABEL (label), 0.0);
    gtk_widget_add_accelerator (item, "activate", accel,
                                GDK_KEY_m, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_accel_label_set_accel_widget (GTK_ACCEL_LABEL (label), item);
    gtk_box_pack_end (GTK_BOX (box), label, TRUE, TRUE, 0);
    gtk_container_add (GTK_CONTAINER (item), box);
    gtk_widget_show_all (item);

    return item;
}

#endif  /* GTK_CHECK_VERSION 3.10 */
