/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Main dialog management.
 * \author   Copyright (c) 2006 Ralf Hoppe
 *
 * \version  $Header: /home/cvs/dfcgen-gtk/src/mainDlg.c,v 1.1.1.1 2006-09-11 15:52:19 ralf Exp $
 *
 *
 * \note     Parts of code taken over from \e glade in interface.c, which itself
 *           isn't a project file.
 *
 * $Log: not supported by cvs2svn $
 *
 *
 ******************************************************************************/



/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dfcgen.h"
#include "dfcProject.h"
#include "mainDlg.h"
#include "designDlg.h"
#include "fileDlg.h"
#include "helpDlg.h"
#include "responseWin.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/

typedef enum
{
   MAINDLG_LIST_COLUMN_INDEX = 0,
   MAINDLG_LIST_COLUMN_COEFF = 1,

   MAINDLG_LIST_COLUMN_SIZE,
} MAINDLG_LIST_COLUMN;


/* LOCAL CONSTANT DEFINITIONS *************************************************/


#define MAINDLG_BTN_APPLY       "btnApply" /**< Name of \e Apply button widget */



/* LOCAL VARIABLE DEFINITIONS *************************************************/

/** Response window types as to be passed to g_signal_connect() and used in
 *  event callbacks.
 */
static RESPONSE_TYPE viewWinType[RESPONSE_TYPE_SIZE] =
{
    RESPONSE_TYPE_AMPLITUDE,                          /**< amplitude response */
    RESPONSE_TYPE_ATTENUATION,                               /**< attenuation */
    RESPONSE_TYPE_CHAR,                          /**< characteristic function */
    RESPONSE_TYPE_PHASE,                                  /**< phase response */
    RESPONSE_TYPE_DELAY,                                     /**< phase delay */
    RESPONSE_TYPE_GROUP,                                     /**< group delay */
    RESPONSE_TYPE_IMPULSE,                  /**< time-domain impulse response */
    RESPONSE_TYPE_STEP                         /**< time-domain step response */
};


static GtkWidget *topWidget;                            /**< Top level widget */
static GtkTreeView *treeDenominator; /**< Denominator coefficients tree widget */
static GtkTreeView *treeNumerator;    /**< Numerator coefficients tree widget */



/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void mainDlgDestroy (GtkObject* object, gpointer user_data);
static void mainDlgQuit (GtkWidget* widget, gpointer user_data);
static GtkTreeView* createCoeffListTreeView (void);
static void indexCellDataFunc (GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                               GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static void coeffCellDataFunc (GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                               GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static void fillCoeffListTreeView (GtkTreeView *tree,  GtkListStore *store,
                                   MATHPOLY *poly);


/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** This function is called if the main (top-level) widget is destroyed.
 *
 *  \param object       GtkWidget of top-level window which is destroyed.
 *  \param user_data    User data as passed to function g_signal_connect (unused).
 *
 *  \return     Top widget reference.
 ******************************************************************************/
static void mainDlgDestroy (GtkObject* object, gpointer user_data)
{
    gtk_main_quit ();
} /* mainDlgDestroy() */



/* FUNCTION *******************************************************************/
/** This function is called if either \e Quit is choosen from the main menu
 *  or the \e Exit button in toolbar clicked.
 *
 *  \param widget       \e GtkMenuItem or \e GtkToolItem for application quit.
 *  \param user_data    User data as passed to function g_signal_connect (unused).
 *
 ******************************************************************************/
static void mainDlgQuit (GtkWidget* widget, gpointer user_data)
{
    ASSERT (topWidget != NULL);
    gtk_widget_destroy (topWidget);
} /* mainDlgQuit() */


/* FUNCTION *******************************************************************/
/** This function sets the "markup" property of an index cell instead of just
 *  using the straight mapping between the cell and the model. This is useful
 *  for customizing the cell renderer - in that case to handle markup.
 *
 *  \param column       A GtkTreeColumn
 *  \param renderer     The GtkCellRenderer that is being rendered by \p column
 *  \param model        The GtkTreeModel being rendered
 *  \param iter         A GtkTreeIter of the current row rendered
 *  \param data         User data
 *
 ******************************************************************************/
static void indexCellDataFunc (GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                               GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
    int index;
    char buf[128];

    gtk_tree_model_get (model, iter, MAINDLG_LIST_COLUMN_INDEX, &index, -1);
    g_snprintf (buf, sizeof (buf), "z<sup>-%d</sup>", index);
    g_object_set (renderer, "markup", buf, NULL);
} /* indexCellDataFunc() */


/* FUNCTION *******************************************************************/
/** This function sets the "text" property of a coefficient cell instead of just
 *  using the straight mapping between the cell and the model. This is useful
 *  for customizing the cell renderer - in that case to customize the double
 *  output precision (which normally is 6 in \e libstdc).
 *
 *  \param column       A GtkTreeColumn
 *  \param renderer     The GtkCellRenderer that is being rendered by \p column
 *  \param model        The GtkTreeModel being rendered
 *  \param iter         A GtkTreeIter of the current row rendered
 *  \param data         User data
 *
 ******************************************************************************/
void coeffCellDataFunc (GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                        GtkTreeModel *model, GtkTreeIter *iter, gpointer data)
{
    double coeff;
    char buf[128];

    gtk_tree_model_get (model, iter, MAINDLG_LIST_COLUMN_COEFF, &coeff, -1);
    g_snprintf (buf, sizeof (buf), "%G", coeff);
    g_object_set (renderer, "text", buf, NULL);
} /* coeffCellDataFunc() */



/* FUNCTION *******************************************************************/
/** \e Realize event callback for denominator coefficients list.
 *
 *  \note If the GtkTreeView has \e fixed_height mode enabled, then all columns
 *        must have its \e sizing property set to be GTK_TREE_VIEW_COLUMN_FIXED.
 *
 *  \return GtkTreeView widget associated with the coefficients list.
 *
 ******************************************************************************/
static GtkTreeView* createCoeffListTreeView (void)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeView* tree;

    /* 1st step: create the model (GtkListStore)
     */
    GtkListStore *store = gtk_list_store_new (MAINDLG_LIST_COLUMN_SIZE,
                                              G_TYPE_INT, G_TYPE_DOUBLE);

    /* 2nd step: create the tree view
     */
    tree = GTK_TREE_VIEW (gtk_tree_view_new ());
    gtk_tree_view_set_rules_hint (tree, TRUE);
    gtk_tree_view_set_hover_expand (tree, TRUE);
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (tree), GTK_SELECTION_SINGLE);
    gtk_tree_view_set_search_column (tree, MAINDLG_LIST_COLUMN_INDEX);


    /* 3rd step: create all columns
     */
    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Tap"), renderer,
                                                       "markup", MAINDLG_LIST_COLUMN_INDEX,
                                                       NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             indexCellDataFunc, NULL, NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_sort_column_id (column, MAINDLG_LIST_COLUMN_INDEX);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column), GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column (tree, column);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes (_("Coefficient"), renderer,
                                                       "text", MAINDLG_LIST_COLUMN_COEFF,
                                                       NULL);
    gtk_tree_view_column_set_cell_data_func (column, renderer,
                                             coeffCellDataFunc, NULL, NULL);
    gtk_tree_view_column_set_resizable (column, TRUE);
    gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                     GTK_TREE_VIEW_COLUMN_AUTOSIZE);
    gtk_tree_view_append_column (tree, column);


    /* 4th step: set model into tree view (and show widget)
     */
    gtk_tree_view_set_model (tree, GTK_TREE_MODEL (store));
    g_object_unref (G_OBJECT (store));        /* view now holds the reference */

    return tree;
} /* createCoeffListTreeView() */


/* FUNCTION *******************************************************************/
/** Fills a GtkTreeView coefficients list with data (but does not clear it).
 *
 *  \param tree         The GtkTreeView widget to be filled with coefficients.
 *  \param store        The associated list store.
 *  \param poly         Pointer to polynomial coefficients to be filled in.
 *
 ******************************************************************************/
static void fillCoeffListTreeView (GtkTreeView *tree, GtkListStore *store,
                                   MATHPOLY *poly)
{
    int i;
    GtkTreeIter iter;                                       /* store iterator */

    for (i = 0; i <= poly->degree; i++)
    {
        gtk_list_store_append (store, &iter);   /* add new row & get iterator */
        gtk_list_store_set (store, &iter, MAINDLG_LIST_COLUMN_INDEX, i,
                            MAINDLG_LIST_COLUMN_COEFF, poly->coeff[i], -1);
    } /* for */
} /* fillCoeffListTreeView() */



/* EXPORTED FUNCTION DEFINITIONS **********************************************/


/* FUNCTION *******************************************************************/
/** \e dfcgen top widget creation.
 *
 *  This function is completely taken over from interface.c, where it has been
 *  generated by \e glade.
 *
 *
 *  \return     Top widget reference.
 ******************************************************************************/
GtkWidget* mainDlgCreate (void)
{
    GtkWidget *widget, *label;
    GtkWidget *toolbarMain;
    GtkWidget *menuMain, *menuMainItem, *menuContainer, *menuItem;

    GtkWidget *vbox1;

    GtkWidget *toolitem1;
    GtkWidget *btnOpen;
    GtkWidget *btnSave;
    GtkWidget *btnNew;
    GtkWidget *btnPreferences;
    GtkWidget *separatortoolitem2;
    GtkWidget *btnExit;
    GtkWidget *hbox1;
    GtkWidget *boxDesignDlg;
    GtkWidget *hbox3;
    GtkWidget *eventbox2;
    GtkWidget *comboFilterClass;
    GtkWidget *hbuttonbox1;
    GtkWidget *btnApply;
    GtkWidget *btnHelp;
    GtkWidget *boxFilterDlg;
    GtkWidget *vpaneFilter;
    GtkWidget *frame1;
    GtkWidget *alignment6;
    GtkWidget *hbox12;
    GtkWidget *aspectframe1;
    GtkWidget *drawRoots;
    GtkWidget *vbox4;
    GtkWidget *btnRootsEnable;
    GtkWidget *progressRoots;
    GtkWidget *scrolledwindow1;
    GtkWidget *scrolledwindow2;
    GtkWidget *table6;
    GtkWidget *hbuttonbox3;
    GtkWidget *btnCoeffModify;
    GtkWidget *btnRoeffRound;
    GtkWidget *btnCoeffScale;
    GtkWidget *alignment11;
    GtkWidget *hbox13;
    GtkWidget *statusbar1;
    GtkRequisition size;

    GtkTooltips *tooltips = gtk_tooltips_new ();
    GtkAccelGroup *accel_group = gtk_accel_group_new ();

    topWidget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (topWidget), 6);

#if 0
    gtk_window_set_default_size (GTK_WINDOW (topWidget), 800, 600);
#endif

    gtk_window_set_role (GTK_WINDOW (topWidget), PACKAGE);

    g_signal_connect ((gpointer) topWidget, "destroy",
                      G_CALLBACK (mainDlgDestroy),
                      NULL);

    vbox1 = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (topWidget), vbox1);

    menuMain = gtk_menu_bar_new ();
    gtk_box_pack_start (GTK_BOX (vbox1), menuMain, FALSE, FALSE, 0);

    menuMainItem = gtk_menu_item_new_with_mnemonic (_("_File"));
    gtk_container_add (GTK_CONTAINER (menuMain), menuMainItem);

    menuContainer = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuMainItem), menuContainer);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-new", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (fileDlgNewActivate),
                      NULL);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-open", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (fileDlgOpenActivate),
                      NULL);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-save", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (fileDlgSaveActivate),
                      NULL);
    GLADE_HOOKUP_OBJECT (topWidget, menuItem, "menuItemFileSave");

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-save-as", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (fileDlgSaveAsActivate),
                      NULL);
    GLADE_HOOKUP_OBJECT (topWidget, menuItem, "menuItemFileSaveAs");

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-print", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);

#if GTK_CHECK_VERSION(2, 10, 0)            /* print support requires GTK 2.10 */
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (fileDlgPrintActivate),
                      NULL);
#else
    gtk_widget_set_sensitive (GTK_WIDGET(menuItem), FALSE);
#endif

    widget = gtk_separator_menu_item_new ();
    gtk_container_add (GTK_CONTAINER (menuContainer), widget);
    gtk_widget_set_sensitive (widget, FALSE);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-quit", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (mainDlgQuit),
                      NULL);

    menuMainItem = gtk_menu_item_new_with_mnemonic (_("_Edit"));
    gtk_container_add (GTK_CONTAINER (menuMain), menuMainItem);

    menuContainer = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuMainItem), menuContainer);

    widget = gtk_separator_menu_item_new ();
    gtk_container_add (GTK_CONTAINER (menuContainer), widget);
    gtk_widget_set_sensitive (widget, FALSE);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-preferences", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);


    menuMainItem = gtk_menu_item_new_with_mnemonic (_("_View"));
    gtk_container_add (GTK_CONTAINER (menuMain), menuMainItem);

    menuContainer = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuMainItem), menuContainer);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("_Amplitude Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_AMPLITUDE]);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Attenuation"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_ATTENUATION]);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Characteristic Function"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_CHAR]);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Phase Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_PHASE]);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Phase Delay"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_DELAY]);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Group Delay"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_GROUP]);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Impulse Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_IMPULSE]);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Step Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      &viewWinType[RESPONSE_TYPE_STEP]);

    menuMainItem = gtk_menu_item_new_with_mnemonic (_("_Help"));
    gtk_container_add (GTK_CONTAINER (menuMain), menuMainItem);

    menuContainer = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuMainItem), menuContainer);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-help", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-about", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (helpDlgMenuActivate),
                      NULL);

    toolbarMain = gtk_toolbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox1), toolbarMain, FALSE, FALSE, 0);
    gtk_toolbar_set_style (GTK_TOOLBAR (toolbarMain), GTK_TOOLBAR_BOTH);

    toolitem1 = (GtkWidget*) gtk_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (toolbarMain), toolitem1);

    btnOpen = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-open");
    gtk_container_add (GTK_CONTAINER (toolbarMain), btnOpen);
    g_signal_connect ((gpointer) btnOpen, "clicked",
                      G_CALLBACK (fileDlgOpenActivate),
                      NULL);

    btnSave = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-save");
    gtk_container_add (GTK_CONTAINER (toolbarMain), btnSave);
    g_signal_connect ((gpointer) btnSave, "clicked",
                      G_CALLBACK (fileDlgSaveActivate),
                      NULL);
    GLADE_HOOKUP_OBJECT (topWidget, btnSave, "toolBtnSave");

    btnNew = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-new");
    gtk_container_add (GTK_CONTAINER (toolbarMain), btnNew);
    g_signal_connect ((gpointer) btnNew, "clicked",
                      G_CALLBACK (fileDlgNewActivate),
                      NULL);

    widget = (GtkWidget*) gtk_separator_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (toolbarMain), widget);

    btnPreferences = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-preferences");
    gtk_container_add (GTK_CONTAINER (toolbarMain), btnPreferences);

    separatortoolitem2 = (GtkWidget*) gtk_separator_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (toolbarMain), separatortoolitem2);

    btnExit = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-quit");
    gtk_container_add (GTK_CONTAINER (toolbarMain), btnExit);

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);

    boxDesignDlg = gtk_vbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX (hbox1), boxDesignDlg, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (boxDesignDlg), 0);

    hbox3 = gtk_hbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX (boxDesignDlg), hbox3, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox3), 6);

    label = gtk_label_new_with_mnemonic (_("<b>_Class</b>"));
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (hbox3), label, FALSE, FALSE, 0);

    eventbox2 = gtk_event_box_new ();
    gtk_box_pack_start (GTK_BOX (hbox3), eventbox2, FALSE, TRUE, 0);
    gtk_tooltips_set_tip (tooltips, eventbox2, _("Class of filter (or system)"), NULL);

    comboFilterClass = gtk_combo_box_new_text ();
    gtk_container_add (GTK_CONTAINER (eventbox2), comboFilterClass);
    gtk_label_set_mnemonic_widget (GTK_LABEL (label), comboFilterClass);

    hbuttonbox1 = gtk_hbutton_box_new ();
    gtk_box_pack_start (GTK_BOX (boxDesignDlg), hbuttonbox1, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox1), 6);
    gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox1), GTK_BUTTONBOX_SPREAD);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox1), 12);

    btnHelp = gtk_button_new_from_stock ("gtk-help");
    gtk_container_add (GTK_CONTAINER (hbuttonbox1), btnHelp);
    GTK_WIDGET_SET_FLAGS (btnHelp, GTK_CAN_DEFAULT);
    gtk_tooltips_set_tip (tooltips, btnHelp, _("Help"), NULL);

    btnApply = gtk_button_new_from_stock ("gtk-apply");
    gtk_container_add (GTK_CONTAINER (hbuttonbox1), btnApply);
    GTK_WIDGET_SET_FLAGS (btnApply, GTK_CAN_DEFAULT);
    g_signal_connect ((gpointer) btnApply, "clicked",
                      G_CALLBACK (designDlgApply), comboFilterClass);
    gtk_tooltips_set_tip (tooltips, btnApply, _("Apply input data"), NULL);

    boxFilterDlg = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox1), boxFilterDlg, TRUE, TRUE, 0);

    vpaneFilter = gtk_vpaned_new ();
    gtk_box_pack_start (GTK_BOX (boxFilterDlg), vpaneFilter, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vpaneFilter), 6);

    frame1 = gtk_frame_new (NULL);
    gtk_paned_pack1 (GTK_PANED (vpaneFilter), frame1, FALSE, TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (frame1), 6);

    alignment6 = gtk_alignment_new (0.5, 0.5, 1, 1);
    gtk_container_add (GTK_CONTAINER (frame1), alignment6);
    gtk_alignment_set_padding (GTK_ALIGNMENT (alignment6), 0, 0, 12, 0);

    hbox12 = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (alignment6), hbox12);

    aspectframe1 = gtk_aspect_frame_new (NULL, 0, 0, 1, FALSE);
    gtk_box_pack_start (GTK_BOX (hbox12), aspectframe1, TRUE, TRUE, 6);
    gtk_frame_set_shadow_type (GTK_FRAME (aspectframe1), GTK_SHADOW_NONE);

    drawRoots = gtk_drawing_area_new ();
    gtk_container_add (GTK_CONTAINER (aspectframe1), drawRoots);

    vbox4 = gtk_vbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX (hbox12), vbox4, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox4), 6);

    btnRootsEnable = gtk_check_button_new_with_mnemonic ("");
    gtk_box_pack_end (GTK_BOX (vbox4), btnRootsEnable, FALSE, FALSE, 0);
    GTK_WIDGET_UNSET_FLAGS (btnRootsEnable, GTK_CAN_FOCUS);
    gtk_button_set_focus_on_click (GTK_BUTTON (btnRootsEnable), FALSE);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btnRootsEnable), TRUE);

    progressRoots = gtk_progress_bar_new ();
    gtk_box_pack_end (GTK_BOX (vbox4), progressRoots, TRUE, TRUE, 0);
    gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (progressRoots), GTK_PROGRESS_BOTTOM_TO_TOP);

    label = gtk_label_new (_("<b>Roots</b>"));
    gtk_frame_set_label_widget (GTK_FRAME (frame1), label);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);

    table6 = gtk_table_new (2, 2, FALSE);
    gtk_paned_pack2 (GTK_PANED (vpaneFilter), table6, TRUE, TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (table6), 6);
    gtk_table_set_col_spacings (GTK_TABLE (table6), 12);

    scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
    gtk_table_attach (GTK_TABLE (table6), scrolledwindow2, 1, 2, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_IN);

    treeDenominator = createCoeffListTreeView ();
    gtk_container_add (GTK_CONTAINER (scrolledwindow2), GTK_WIDGET (treeDenominator));
    gtk_tooltips_set_tip (tooltips, GTK_WIDGET (treeDenominator),
                          _("Denominator coefficients"), NULL);

    scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
    gtk_table_attach (GTK_TABLE (table6), scrolledwindow1, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

    treeNumerator = createCoeffListTreeView ();
    gtk_container_add (GTK_CONTAINER (scrolledwindow1), GTK_WIDGET (treeNumerator));
    gtk_tooltips_set_tip (tooltips, GTK_WIDGET (treeNumerator),
                          _("Numerator coefficients"), NULL);

    label = gtk_label_new (_("<b>Numerator</b>"));
    gtk_table_attach (GTK_TABLE (table6), label, 0, 1, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 6);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);

    label = gtk_label_new (_("<b>Denominator</b>"));
    gtk_table_attach (GTK_TABLE (table6), label, 1, 2, 0, 1,
                      (GtkAttachOptions) (GTK_FILL),
                      (GtkAttachOptions) (0), 0, 6);
    gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);

    hbuttonbox3 = gtk_hbutton_box_new ();
    gtk_box_pack_start (GTK_BOX (boxFilterDlg), hbuttonbox3, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbuttonbox3), 6);
    gtk_box_set_spacing (GTK_BOX (hbuttonbox3), 12);

    btnCoeffModify = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (hbuttonbox3), btnCoeffModify);
    gtk_widget_set_sensitive (btnCoeffModify, FALSE);
    gtk_tooltips_set_tip (tooltips, btnCoeffModify, _("Modify coefficient"), NULL);

    widget = gtk_image_new_from_stock ("gtk-edit", GTK_ICON_SIZE_BUTTON);
    gtk_container_add (GTK_CONTAINER (btnCoeffModify), widget);

    btnRoeffRound = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (hbuttonbox3), btnRoeffRound);
    gtk_widget_set_sensitive (btnRoeffRound, FALSE);
    gtk_tooltips_set_tip (tooltips, btnRoeffRound, _("Round coefficient(s)"), NULL);

    widget = gtk_image_new_from_stock ("gtk-convert", GTK_ICON_SIZE_BUTTON);
    gtk_container_add (GTK_CONTAINER (btnRoeffRound), widget);

    btnCoeffScale = gtk_button_new ();
    gtk_container_add (GTK_CONTAINER (hbuttonbox3), btnCoeffScale);
    gtk_widget_set_sensitive (btnCoeffScale, FALSE);
    gtk_tooltips_set_tip (tooltips, btnCoeffScale, _("Scale coefficient(s)"), NULL);

    alignment11 = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_container_add (GTK_CONTAINER (btnCoeffScale), alignment11);

    hbox13 = gtk_hbox_new (FALSE, 2);
    gtk_container_add (GTK_CONTAINER (alignment11), hbox13);

    widget = gtk_image_new_from_stock ("gtk-fullscreen", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start (GTK_BOX (hbox13), widget, FALSE, FALSE, 0);

    label = gtk_label_new_with_mnemonic ("");
    gtk_box_pack_start (GTK_BOX (hbox13), label, FALSE, FALSE, 0);

    statusbar1 = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox1), statusbar1, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (statusbar1), 4);

    g_signal_connect ((gpointer) btnExit, "clicked",
                      G_CALLBACK (mainDlgQuit),
                      NULL);
    g_signal_connect ((gpointer) boxDesignDlg, "realize",
                      G_CALLBACK (designDlgBoxRealize),
                      NULL);
    g_signal_connect ((gpointer) comboFilterClass, "changed",
                      G_CALLBACK (designDlgOnFilterComboChanged),
                      NULL);

    /* Store pointers to all widgets, for use by lookup_widget(). */
    GLADE_HOOKUP_OBJECT_NO_REF (topWidget, topWidget, "topWidget");
    GLADE_HOOKUP_OBJECT (topWidget, toolbarMain, "toolbarMain");
    GLADE_HOOKUP_OBJECT (topWidget, toolitem1, "toolitem1");
    GLADE_HOOKUP_OBJECT (topWidget, btnOpen, "btnOpen");
    GLADE_HOOKUP_OBJECT (topWidget, btnSave, "btnSave");
    GLADE_HOOKUP_OBJECT (topWidget, btnNew, "btnNew");
    GLADE_HOOKUP_OBJECT (topWidget, btnPreferences, "btnPreferences");
    GLADE_HOOKUP_OBJECT (topWidget, separatortoolitem2, "separatortoolitem2");
    GLADE_HOOKUP_OBJECT (topWidget, btnExit, "btnExit");
    GLADE_HOOKUP_OBJECT (topWidget, hbox1, "hbox1");
    GLADE_HOOKUP_OBJECT (topWidget, boxDesignDlg, "boxDesignDlg");
    GLADE_HOOKUP_OBJECT (topWidget, hbox3, "hbox3");
    GLADE_HOOKUP_OBJECT (topWidget, eventbox2, "eventbox2");
    GLADE_HOOKUP_OBJECT (topWidget, comboFilterClass, "comboFilterClass");
    GLADE_HOOKUP_OBJECT (topWidget, hbuttonbox1, "hbuttonbox1");
    GLADE_HOOKUP_OBJECT (topWidget, btnApply, MAINDLG_BTN_APPLY);
    GLADE_HOOKUP_OBJECT (topWidget, btnHelp, "btnHelp");
    GLADE_HOOKUP_OBJECT (topWidget, boxFilterDlg, "boxFilterDlg");
    GLADE_HOOKUP_OBJECT (topWidget, vpaneFilter, "vpaneFilter");
    GLADE_HOOKUP_OBJECT (topWidget, frame1, "frame1");
    GLADE_HOOKUP_OBJECT (topWidget, alignment6, "alignment6");
    GLADE_HOOKUP_OBJECT (topWidget, hbox12, "hbox12");
    GLADE_HOOKUP_OBJECT (topWidget, drawRoots, "drawRoots");
    GLADE_HOOKUP_OBJECT (topWidget, vbox4, "vbox4");
    GLADE_HOOKUP_OBJECT (topWidget, btnRootsEnable, "btnRootsEnable");
    GLADE_HOOKUP_OBJECT (topWidget, progressRoots, "progressRoots");
    GLADE_HOOKUP_OBJECT (topWidget, scrolledwindow1, "scrolledwindow1");
    GLADE_HOOKUP_OBJECT (topWidget, scrolledwindow2, "scrolledwindow2");
    GLADE_HOOKUP_OBJECT (topWidget, hbuttonbox3, "hbuttonbox3");
    GLADE_HOOKUP_OBJECT (topWidget, btnCoeffModify, "btnCoeffModify");
    GLADE_HOOKUP_OBJECT (topWidget, btnRoeffRound, "btnRoeffRound");
    GLADE_HOOKUP_OBJECT (topWidget, btnCoeffScale, "btnCoeffScale");
    GLADE_HOOKUP_OBJECT (topWidget, alignment11, "alignment11");
    GLADE_HOOKUP_OBJECT (topWidget, hbox13, "hbox13");
    GLADE_HOOKUP_OBJECT (topWidget, statusbar1, "statusbar1");
    GLADE_HOOKUP_OBJECT_NO_REF (topWidget, tooltips, "tooltips");

    gtk_window_add_accel_group (GTK_WINDOW (topWidget), accel_group);
    gtk_widget_show_all (topWidget);
    gtk_widget_size_request (vpaneFilter, &size);
    gtk_paned_set_position (GTK_PANED (vpaneFilter), size.height / 2);

    mainDlgUpdateAll (NULL);                             /* set initial state */
    return topWidget;
}


/* FUNCTION *******************************************************************/
/** Updates the filter dialog in main widget from current project.
 *
 ******************************************************************************/
void mainDlgUpdateFilter ()
{
    GtkListStore *storeNum, *storeDen;

    FLTCOEFF* pFilter = dfcPrjGetFilter();
    BOOL valid = pFilter != NULL;

    gtk_widget_set_sensitive (lookup_widget (topWidget, "toolBtnSave"), valid);
    gtk_widget_set_sensitive (lookup_widget (topWidget, "menuItemFileSave"), valid);
    gtk_widget_set_sensitive (lookup_widget (topWidget, "menuItemFileSaveAs"), valid);

    storeNum = GTK_LIST_STORE (gtk_tree_view_get_model (treeNumerator));
    storeDen = GTK_LIST_STORE (gtk_tree_view_get_model (treeDenominator));
    gtk_list_store_clear (storeNum);
    gtk_list_store_clear (storeDen);

    if (valid)
    {
        fillCoeffListTreeView (treeNumerator, storeNum, &pFilter->num);
        fillCoeffListTreeView (treeDenominator, storeDen, &pFilter->den);
    } /* if */

    responseWinRedraw (RESPONSE_TYPE_SIZE);
} /* mainDlgUpdateFilter() */




/* FUNCTION *******************************************************************/
/** Adjustment of main filter dialog from a new project (may be read from file
 *  before).
 *
 *  \param filename     Associated filename in filesystem coding, or NULL to
 *                      reset.
 *
 ******************************************************************************/
void mainDlgUpdateAll (const char* filename)
{
    designDlgUpdate (topWidget);        /* update design from current project */
    mainDlgUpdateFilter ();          /* update coefficients and roots display */

    gtk_widget_grab_default (lookup_widget (topWidget, MAINDLG_BTN_APPLY));

    if (filename != NULL)
    {
        gchar *tmp, *utf8name = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

        if (utf8name == NULL)             /* may be an UTF-8 conversion error */
        {
            utf8name = g_strdup (_("<i>Unknown</i>"));
        } /* if */

        tmp = g_strdup_printf ("%s: %s", _(PACKAGE), utf8name);
        gtk_window_set_title (GTK_WINDOW (topWidget), tmp);
        FREE (utf8name);
        FREE (tmp);

        return;
    } /* if */

    gtk_window_set_title (GTK_WINDOW (topWidget), _(PACKAGE));
} /* mainDlgUpdateAll() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/
