/**
 * \file        gui.h
 * \brief       GUI header files.
 * \copyright   Copyright (C) 2006-2022 Ralf Hoppe <ralf.hoppe@dfcgen.de>
 */

#ifndef GUI_H
#define GUI_H


#include "base.h"   /* includes config.h (include before GNU system headers) */
#include "support.h"           /* gettext macros and file handling functions */

#include <gtk/gtk.h>


#ifdef  __cplusplus
extern "C" {
#endif


#if (GTK_MAJOR_VERSION == 3)
#define GUI_GTK_GETTEXT_DOMAIN  "gtk30"       /**< \e gettext domain of GTK */
#else
#error "Adopt GUI_GTK_GETTEXT_DOMAIN to GTK version!"
#endif


#define GUI_ENTRY_WIDTH_CHARS   8 /**< Width of all entry fields for numerics */
#define GUI_LABEL_WRAP_CHARS    32 /**< Wrap of long \e Label widgets in dialogs */
#define GUI_INDENT_CHILD_PIXEL  12 /**< Extra left/start indent of child widget */



/**
 * \brief       Translate string from GTK \e gettext domain to current language.
 * \param[in]   str     String to translate.
 * \return      Translated string.
 */
#define G_(str) dgettext (GUI_GTK_GETTEXT_DOMAIN, (str))



/** @defgroup gui_icons Icons
 *
 *  \note   Some icon names conform to the \e Icon \e Naming
 *          \e Specification of freedektop.org
 *  @{
 */

#define GUI_CURSOR_IMAGE_CROSS          "cross"
#define GUI_CURSOR_IMAGE_WATCH          "watch"

#define GUI_ICON_IMAGE_PREFS            "preferences-other"

#define GUI_BUTTON_IMAGE_OK             "gtk-ok"
#define GUI_BUTTON_IMAGE_CANCEL         "gtk-cancel"
#define GUI_BUTTON_IMAGE_APPLY          "gtk-apply"
#define GUI_BUTTON_IMAGE_HELP           "gtk-help"
#define GUI_BUTTON_IMAGE_PRINT          "document-print"
#define GUI_BUTTON_IMAGE_OPEN           "document-open"
#define GUI_BUTTON_IMAGE_SAVE           "document-save"
#define GUI_BUTTON_IMAGE_PREFS          GUI_ICON_IMAGE_PREFS
#define GUI_BUTTON_IMAGE_COEFF_EDIT     "gtk-edit"
#define GUI_BUTTON_IMAGE_COEFF_MULTIPLY "gtk-fullscreen"
#define GUI_BUTTON_IMAGE_COEFF_ROUND    "gtk-convert"


#define GUI_MENU_IMAGE_HELP             "help-contents"
#define GUI_MENU_IMAGE_ABOUT            "help-about"
#define GUI_MENU_IMAGE_INFO             "help-info"
#define GUI_MENU_IMAGE_PRINT            "document-print"
#define GUI_MENU_IMAGE_OPEN             "document-open"
#define GUI_MENU_IMAGE_SAVE             "document-save"
#define GUI_MENU_IMAGE_SAVE_AS          "document-save-as"
#define GUI_MENU_IMAGE_NEW              "document-new"
#define GUI_MENU_IMAGE_EXPORT           "document-export"
#define GUI_MENU_IMAGE_PREFS            GUI_ICON_IMAGE_PREFS
#define GUI_MENU_IMAGE_QUIT             "application-exit"

/** @} */


/** @defgroup gui_labels Re-used Labels
 *
 *  @{
 */
#define GUI_BUTTON_LABEL_OK             G_("_OK")
#define GUI_BUTTON_LABEL_CANCEL         G_("_Cancel")
#define GUI_BUTTON_LABEL_APPLY          G_("_Apply")
#define GUI_BUTTON_LABEL_HELP           G_("_Help")
#define GUI_BUTTON_LABEL_PRINT          G_("_Print")
#define GUI_BUTTON_LABEL_PREFS          _("_Preferences")

#define GUI_MENU_LABEL_HELP             G_("_Help")
#define GUI_MENU_LABEL_ABOUT            G_("About")
#define GUI_MENU_LABEL_PRINT            GUI_BUTTON_LABEL_PRINT
#define GUI_MENU_LABEL_OPEN             G_("_Open")
#define GUI_MENU_LABEL_SAVE             G_("_Save")
#define GUI_MENU_LABEL_SAVE_AS          _("Save _As")
#define GUI_MENU_LABEL_NEW              _("_New")
#define GUI_MENU_LABEL_EXPORT           _("_Export")
#define GUI_MENU_LABEL_PREFS            GUI_BUTTON_LABEL_PREFS
#define GUI_MENU_LABEL_INFO             _("Project Info")
#define GUI_MENU_LABEL_QUIT             _("_Quit")

/** @} */


#ifdef  __cplusplus
}
#endif


#endif /* GUI_H */
