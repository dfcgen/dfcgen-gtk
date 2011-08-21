/******************************************************************************/
/**
 * \file
 *           GUI header files.
 *
 * \author   Copyright (C) 2006, 2011 Ralf Hoppe <ralf.hoppe@ieee.org>
 * \version  $Id$
 *
 ******************************************************************************/


#ifndef GUI_H
#define GUI_H


/* INCLUDE FILES **************************************************************/

#include "base.h"    /* includes config.h (include before GNU system headers) */
#include "support.h"        /* gettext macros (and others generated by glade) */

#include <gtk/gtk.h>


#ifdef  __cplusplus
extern "C" {
#endif


/* GLOBAL TYPE DECLARATIONS ***************************************************/


/* GLOBAL CONSTANT DECLARATIONS ***********************************************/

#define GUI_ENTRY_WIDTH_CHARS   8 /**< Width of all entry fields for numerics */
#define PACKAGE_ICON            "dfcgen.png"    /**< Icon filename of package */


/* GLOBAL VARIABLE DECLARATIONS ***********************************************/


/* GLOBAL MACRO DEFINITIONS ***************************************************/

/* Some macros moved here from interface.c, which is generated by glade (but not
 * used directly in this application).
 */
#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
    g_object_set_data (G_OBJECT (component), name, widget)


/* EXPORTED FUNCTIONS *********************************************************/


#ifdef  __cplusplus
}
#endif


#endif /* GUI_H */


/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
