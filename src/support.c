/*
 * DO NOT EDIT THIS FILE - it is generated by Glade.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include "support.h"
#include "base.h"    /* FREE() */


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
            parent = widget->parent;
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
/** \brief This function returns a path to a sub-directory of PACKAGE_DATA_DIR.
 *
 *  \param[in] subdir   Name of sub-directory (last path component).
 *
 *  \return             A newly allocated string that must be freed with g_free().
 ******************************************************************************/
gchar* getPackageDataSubdirPath (const gchar* subdir)
{
    gchar* path;

#ifdef G_OS_WIN32
#if GTK_CHECK_VERSION (2, 18, 0)
    gchar* pkgdatadir = g_win32_get_package_installation_directory_of_module (NULL);
#else
    gchar* pkgdatadir = g_win32_get_package_installation_directory (NULL, NULL);
#endif

    if (pkgdatadir != NULL)
    {
        path = g_build_filename (pkgdatadir, subdir, NULL);
    } /* if */
    else
#endif
    path = g_build_filename (PACKAGE_DATA_DIR, subdir, NULL);

    if (path == NULL)
    {
        g_critical (_("Couldn't locate package data sub-directory '%s'"), subdir);
    } /* if */

    return path;
} /* getPackageDataSubdirPath() */




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

    subdir = getPackageDataSubdirPath (PACKAGE_PIXMAPS_DIR);
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

