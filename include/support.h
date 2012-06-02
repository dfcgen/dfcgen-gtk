/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file     support.h
 * \brief    Support functions, defines and macros, mostly for \e gettext,
 *           widget and file handling.
 *
 * \author   Copyright (C) 2006, 2011-2012 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Id$
 *
 ******************************************************************************/


#ifndef SUPPORT_H
#define SUPPORT_H

/* INCLUDE FILES **************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

/*
 * Standard gettext macros.
 */
#ifdef ENABLE_NLS
#  include <libintl.h>
#  undef _
#  define _(String) dgettext (PACKAGE, String)
#  define Q_(String) g_strip_context ((String), gettext (String))
#  ifdef gettext_noop
#    define N_(String) gettext_noop (String)
#  else
#    define N_(String) (String)
#  endif
#else  /* !ENABLE_NLS */
#  define textdomain(String) (String)
#  define gettext(String) (String)
#  define dgettext(Domain,Message) (Message)
#  define dcgettext(Domain,Message,Type) (Message)
#  define bindtextdomain(Domain,Directory) (Domain)
#  define _(String) (String)
#  define Q_(String) g_strip_context ((String), (String))
#  define N_(String) (String)
#endif  /* ENABLE_NLS */


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
/** \brief This function returns a path to a sub-directory of PACKAGE_DATA_DIR.
 *
 *  \param[in] subdir   Name of sub-directory (last path component).
 *
 *  \return             A newly allocated string that must be freed with g_free().
 ******************************************************************************/
gchar* getPackageDataSubdirPath (const gchar* subdir);



/* FUNCTION *******************************************************************/
/** \brief Create a \c GdkPixbuf from a pixmaps file.
 *
 *  \param[in] filename Pixmpas filename.
 *
 *  \return    A newly allocated \c GdkPixbuf, representing the pixmap file.
 ******************************************************************************/
GdkPixbuf* createPixbufFromFile (const gchar* filename);



#endif /* SUPPORT_H */

