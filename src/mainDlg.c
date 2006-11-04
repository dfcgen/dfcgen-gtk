/********************* -*- mode: C; coding: utf-8 -*- *************************/
/**
 * \file
 *           Main dialog elements (right, bottom of window) handling.
 * \author   Copyright (c) 2006 Ralf Hoppe
 *
 * \version  $Header: /home/cvs/dfcgen-gtk/src/mainDlg.c,v 1.2 2006-11-04 18:26:27 ralf Exp $
 *
 *
 * \note     Parts of code taken over from \e glade in interface.c, which itself
 *           isn't a project file.
 *
 * $Log: not supported by cvs2svn $
 * Revision 1.1.1.1  2006/09/11 15:52:19  ralf
 * Initial CVS import
 *
 *
 *
 ******************************************************************************/



/* INCLUDE FILES **************************************************************/

#include "gui.h"
#include "dfcgen.h"
#include "dfcProject.h"
#include "cfgSettings.h"
#include "mainDlg.h"
#include "designDlg.h"
#include "fileDlg.h"
#include "editDlg.h"
#include "helpDlg.h"
#include "rootsPlot.h"
#include "responseWin.h"
#include "filterSupport.h"
#include "dialogSupport.h"


/* GLOBAL CONSTANT DEFINITIONS ************************************************/


/* GLOBAL VARIABLE DEFINITIONS ************************************************/


/* LOCAL TYPE DECLARATIONS ****************************************************/



/** Function that operates on selected coefficients.
 */
typedef BOOL (* MAINDLG_COEFF_OPERATION) (GtkWidget *top, FLTCOEFF *pFilter,
                                          MATHPOLY *poly, int index);



/** Coefficients modification action descriptor 
 */
typedef struct
{
    GtkWidget *btn;
    GtkWidget *menu;
    MAINDLG_COEFF_OPERATION op;
    char *text;                    /**< Menuitem text (describing the action) */
    char *stockimg;
    char *tooltip;
} MAINDLG_COEFF_ACTION;



/** Column identifiers for coefficients list GtkTreeview model.
 */
typedef enum
{
   MAINDLG_LIST_COLUMN_INDEX = 0,
   MAINDLG_LIST_COLUMN_COEFF = 1,

   MAINDLG_LIST_COLUMN_SIZE
} MAINDLG_LIST_COLUMN;




/* LOCAL MACRO DEFINITIONS ****************************************************/


/* LOCAL FUNCTION DECLARATIONS ************************************************/

static void mainDlgDestroy (GtkObject* object, gpointer user_data);
static void mainDlgQuit (GtkWidget* widget, gpointer user_data);
static BOOL mainDlgCoeffEdit (GtkWidget *topWidget, FLTCOEFF *pFilter,
                              MATHPOLY *poly, int index);
static BOOL mainDlgCoeffsRound (GtkWidget *topWidget, FLTCOEFF *pFilter,
                                MATHPOLY *poly, int index);
static BOOL mainDlgCoeffsFactor (GtkWidget *topWidget, FLTCOEFF *pFilter,
                                 MATHPOLY *poly, int index);
static void allowCoeffActions (BOOL active);
static int getSelectedCoeff (GtkTreeSelection *selection);
static void treeSelectionCallback (GtkTreeSelection *treeselection, gpointer user_data);

static GtkTreeView* createCoeffListTreeView (GtkTreeView **pOther);
static void indexCellDataFunc (GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                               GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static void coeffCellDataFunc (GtkTreeViewColumn *column, GtkCellRenderer *renderer,
                               GtkTreeModel *model, GtkTreeIter *iter, gpointer data);
static void fillCoeffListTreeView (GtkTreeView *tree,  GtkListStore *store,
                                   MATHPOLY *poly);
static MATHPOLY *getSelectedPoly (FLTCOEFF* pFilter, int *pIndex);



/* LOCAL CONSTANT DEFINITIONS *************************************************/


#define MAINDLG_BTN_APPLY       "btnApply" /**< Name of \e Apply button widget */



/* LOCAL VARIABLE DEFINITIONS *************************************************/

static GtkWidget *topWidget;                            /**< Top level widget */
static GtkTreeView *treeDenominator; /**< Denominator coefficients tree widget */
static GtkTreeView *treeNumerator;    /**< Numerator coefficients tree widget */
static GtkWidget *statusbar;      /**< Statusbar in bottom line of top widget */


static MAINDLG_COEFF_ACTION mainDlgCoeffBtn[] =
{
    {
        NULL, NULL, mainDlgCoeffEdit, N_("Change"),
        "gtk-edit", N_("Edit a single coefficient")
    },
    {
        NULL, NULL, mainDlgCoeffsFactor, N_("Multiply"),
        "gtk-fullscreen", N_("Multiply all coefficients with a constant")
    },
    {
        NULL, NULL, mainDlgCoeffsRound, N_("Round"),
        "gtk-convert", N_("Round all coefficients")
    }
};



/* LOCAL FUNCTION DEFINITIONS *************************************************/


/* FUNCTION *******************************************************************/
/** This function is called if the main (top-level) widget is destroyed.
 *
 *  \param object       GtkWidget of top-level window which is destroyed.
 *  \param user_data    User data as passed to function g_signal_connect (unused).
 *
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
/** This function determines the index of the associated coefficient from a
 *  numerator or denominator GtkTreeview list.
 *
 *  \param selection    The current selection from numerator or denominator list.
 *
 *  \return             Returns the index of selected coefficient (if there is
 *                      one selected) or -1 if no coefficient is selected.
 ******************************************************************************/
static int getSelectedCoeff (GtkTreeSelection *selection)
{
    int index, *pIndex;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkTreePath* path;

    if (!gtk_tree_selection_get_selected (selection, &model, &iter))
    {
        return -1;                                        /* nothing selected */
    } /* if */

    path = gtk_tree_model_get_path (model, &iter);
    pIndex = gtk_tree_path_get_indices (path);

    if (pIndex == NULL)
    {
        return -1;
    } /* if */

    index = *pIndex;
    gtk_tree_path_free (path);

    return index;
} /* getSelectedCoeff() */



/* FUNCTION *******************************************************************/
/** Determines selected polynomial and coefficient from the numerator and
 *  denominator (GtkTreeView) list.
 *
 *  \param pFilter      Pointer to filter which is currently displayed in the
 *                      GtkTreeView coefficients list.
 *  \param pIndex       Pointer to buffer which gets the index of selected
 *                      coefficient (if there is one).
 *
 *  \return             Selected polynomial (\p pFilter->den or \p Filter->num)
 *                      or NULL if nothing is selected.
 ******************************************************************************/
static MATHPOLY *getSelectedPoly (FLTCOEFF* pFilter, int *pIndex)
{
    int idx;

    if (pIndex == NULL)
    {
        pIndex = &idx;
    } /* if */

    *pIndex = getSelectedCoeff (gtk_tree_view_get_selection (treeNumerator));

    if (*pIndex >= 0)
    {
        return &pFilter->num;
    } /* if */

    *pIndex = getSelectedCoeff (gtk_tree_view_get_selection (treeDenominator));

    if (*pIndex >= 0)
    {
        return &pFilter->den;
    } /* if */

    return NULL;
} /* getSelectedPoly() */



/* FUNCTION *******************************************************************/
/** Operation which is performed when the \e Edit button is clicked (called from
 *  function mainDlgCoeffAction(), type is MAINDLG_COEFF_OPERATION).
 *
 *  \param topWidget    Topwidget reference.
 *  \param pFilter      Pointer to filter which is currently displayed in the
 *                      GtkTreeView coefficients list.
 *  \param poly         Pointer to selected coefficients (\p pFilter->num or
 *                      \p pFilter->den)
 *  \param index        Index of selected coefficient in vector \p poly->coeff.
 *
 *  \return             TRUE if the operation was really performed, else
 *                      FALSE (means no coefficient has changed).
 ******************************************************************************/
static BOOL mainDlgCoeffEdit (GtkWidget *topWidget, FLTCOEFF *pFilter,
                              MATHPOLY *poly, int index)
{
    char *intro =
        g_strdup_printf (_("Changes coefficient of tap z<sup>-%d</sup> in"
                           " the selected list to a new value."), index);

    BOOL ret = dlgPopupDouble (_("Change coefficient"),
                               _("_New"), intro, poly->coeff + index);
    g_free (intro);

    return ret;
} /* mainDlgCoeffEdit() */



/* FUNCTION *******************************************************************/
/** Operation which is performed when the \e Factor button is clicked (called
 *  from function mainDlgCoeffAction(), type is MAINDLG_COEFF_OPERATION).
 *
 *  \param topWidget    Topwidget reference.
 *  \param pFilter      Pointer to filter which is currently displayed in the
 *                      GtkTreeView coefficients list.
 *  \param poly         Pointer to selected coefficients (\p pFilter->num or
 *                      \p pFilter->den)
 *  \param index        Index of selected coefficient in vector \p poly->coeff.
 *
 *  \return             TRUE if the operation was really performed, else
 *                      FALSE (means no coefficient has changed).
 ******************************************************************************/
static BOOL mainDlgCoeffsFactor (GtkWidget *topWidget, FLTCOEFF *pFilter,
                                 MATHPOLY *poly, int index)
{
    int i;
    double factor = 1.0;

    BOOL ret = dlgPopupDouble (_("Multiply coefficients"),
                               _("_Factor"),
                               _("Multiplies all coefficients in the"
                                 " selected list with the given factor."),
                               &factor);
    if (ret)
    {
        for (i = 0; i <= poly->degree; i++)
        {
            poly->coeff[i] *= factor;
        } /* for */
    } /* if */

    return ret;
} /* mainDlgCoeffsFactor() */



/* FUNCTION *******************************************************************/
/** Operation which is performed when the \e Round button is clicked (called
 *  from function mainDlgCoeffAction(), type is MAINDLG_COEFF_OPERATION).
 *
 *  \param topWidget    Topwidget reference.
 *  \param pFilter      Pointer to filter which is currently displayed in the
 *                      GtkTreeView coefficients list.
 *  \param poly         Pointer to selected coefficients (\p pFilter->num or
 *                      \p pFilter->den)
 *  \param index        Index of selected coefficient in vector \p poly->coeff.
 *
 *  \return             TRUE if the operation was really performed, else
 *                      FALSE (means no coefficient has changed).
 ******************************************************************************/
static BOOL mainDlgCoeffsRound (GtkWidget *topWidget, FLTCOEFF *pFilter,
                                MATHPOLY *poly, int index)
{
    int i;

    GtkWidget *dialog =
        gtk_message_dialog_new (GTK_WINDOW (topWidget),
                                GTK_DIALOG_DESTROY_WITH_PARENT,
                                GTK_MESSAGE_WARNING,
                                GTK_BUTTONS_YES_NO,
                                _("Do you really want to round all"
                                  " coefficients in the selected list?"));

    i = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);

    if (i != GTK_RESPONSE_YES)                      /* user wants it (really) */
    {
        return FALSE;
    } /* if */

    for (i = 0; i <= poly->degree; i++)
    {
        poly->coeff[i] = round (poly->coeff[i]);
    } /* for */

    return TRUE;
} /* mainDlgCoeffsRound() */



/* FUNCTION *******************************************************************/
/** This function is called if a coefficients button (\e Edit, \e Factor,
 *  \e Round) was clicked.
 *
 *  \param widget       Button that was clicked.
 *  \param user_data    User data as passed to function g_signal_connect. Here
 *                      it is a pointer to the button descriptor in array
 *                      mainDlgCoeffBtn.
 *
 ******************************************************************************/
static void mainDlgCoeffAction (GtkButton *widget, gpointer user_data)
{
    int index, result;
    FLTCOEFF tmp;

    MAINDLG_COEFF_ACTION *pAction = user_data;

    FLTCOEFF *pFilter = dfcPrjGetFilter ();
    MATHPOLY *poly = getSelectedPoly (&tmp, &index);

    if ((pFilter != NULL) && (poly != NULL))                 /* sanity checks */
    {
        if (filterDuplicate (&tmp, pFilter) == 0)        /* make temp. filter */
        {
            if (pAction->op (topWidget, &tmp, poly, index))     /* performed? */
            {
                result = filterCheck (&tmp);             /* check realization */

                if (FLTERR_CRITICAL (result))
                {
                    filterFree(&tmp);
                    dlgError (topWidget,
                              _("Cannot implement such a filter."
                                " Maybe the result of such an operation"
                                " leads to vanishing coefficients at all."));
                } /* if */
                else                   /* seems okay (do not free any memory) */
                {
                    dfcPrjSetFilter (FLTCLASS_NOTDEF, &tmp, NULL);
                    mainDlgUpdateFilter (result);
                } /* else */
            } /* if */
        } /* if */
        else
        {
            dlgError (topWidget,
                      _("Cannot perform the desired operation."
                        " It seems that all the memory is exhausted."));
        } /* else */
    } /* if */
} /* mainDlgCoeffAction() */




/* FUNCTION *******************************************************************/
/** Activates (or deactivates) the coefficients buttons.
 *
 *  \param active       TRUE if the buttons shall be activated, FALSE to
 *                      deactivate.
 *
 ******************************************************************************/
static void allowCoeffActions (BOOL active)
{
    int i;

    for (i = 0; i < N_ELEMENTS (mainDlgCoeffBtn); i++)
    {
        gtk_widget_set_sensitive (mainDlgCoeffBtn[i].btn, active);
        gtk_widget_set_sensitive (mainDlgCoeffBtn[i].menu, active);
    } /* for */
} /* allowCoeffActions() */



/* FUNCTION *******************************************************************/
/** Callback function for \e changed signal on a GtkTreeSelection associated
 *  with a coefficients list GtkTreeView.
 *
 *  \param selection    Selection of the GtkTreeView list.
 *  \param user_data    Selection of the other GtkTreeView list (user data as
 *                      passed to function g_signal_connect).
 *
 *  \todo  Process double-clicks (see the tree-view tutorial on how to make this)
 ******************************************************************************/
static void treeSelectionCallback (GtkTreeSelection *selection, gpointer user_data)
{
    int idx;
    GtkTreeSelection *other;

    ASSERT (user_data != NULL);

    idx = getSelectedCoeff (selection);
    other = gtk_tree_view_get_selection (*(GtkTreeView**)user_data);

    ASSERT (other != NULL);

    if (idx >= 0)                      /* unselected -> selected transition ? */
    {
        gtk_tree_selection_unselect_all (other);       /* deselect other list */
    } /* if */

    allowCoeffActions ((getSelectedCoeff (other) >= 0) || (idx >= 0));

} /* treeSelectionCallback() */



/* FUNCTION *******************************************************************/
/** This function sets the \e markup property of an index cell instead of just
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
    g_snprintf (buf, sizeof (buf), "%.*G", cfgGetDesktopPrefs()->outprec, coeff);
    g_object_set (renderer, "text", buf, NULL);
} /* coeffCellDataFunc() */



/* FUNCTION *******************************************************************/
/** \e Realize event callback for denominator coefficients list.
 *
 *  \note If the GtkTreeView has \e fixed_height mode enabled, then all columns
 *        must have its \e sizing property set to be GTK_TREE_VIEW_COLUMN_FIXED.
 *
 *  \param pOther   Pointer to a variable which holds the GtkTreeView reference.
 *
 *  \return         GtkTreeView widget associated with the coefficients list.
 *
 ******************************************************************************/
static GtkTreeView* createCoeffListTreeView (GtkTreeView **pOther)
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


    /* 5th step: Connect to "changed" signal
     */
    g_signal_connect_after ((gpointer) gtk_tree_view_get_selection (tree), "changed",
                            G_CALLBACK (treeSelectionCallback), pOther);

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
 *  \todo       Reduce number of variables used (optimize the function)
 ******************************************************************************/
GtkWidget* mainDlgCreate (void)
{
    int i;
    MAINDLG_COEFF_ACTION *pAction;

    GtkWidget *widget, *label, *toolbarMain, *menuItem;
    GtkWidget *menuMain, *menuMainItem, *menuContainer, *submenuContainer;

    GdkPixbuf *iconPixbuf;
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
    GtkWidget *scrolledwindow1;
    GtkWidget *scrolledwindow2;
    GtkWidget *table6;
    GtkWidget *hbuttonbox3;
    GtkRequisition size;

    GtkTooltips *tooltips = gtk_tooltips_new ();
    GtkAccelGroup *accel_group = gtk_accel_group_new ();

    topWidget = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (topWidget), 6);

    iconPixbuf = create_pixbuf (PACKAGE_ICON);

    if (iconPixbuf != NULL)
    {
        gtk_window_set_icon (GTK_WINDOW (topWidget), iconPixbuf);
        gdk_pixbuf_unref (iconPixbuf);
    } /* if */

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

    menuItem = gtk_image_menu_item_new_with_mnemonic (_("Project Info"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    widget = gtk_image_new_from_stock ("gtk-info", GTK_ICON_SIZE_MENU);
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuItem), widget);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (editDlgInfoActivate), NULL);

    widget = gtk_menu_item_new_with_mnemonic (_("Coefficient(s)"));
    gtk_container_add (GTK_CONTAINER (menuContainer), widget);
    submenuContainer = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (widget), submenuContainer);


    for (i = 0, pAction = mainDlgCoeffBtn;
         i < N_ELEMENTS (mainDlgCoeffBtn);
         i++, pAction++)
    {
        pAction->menu =
            gtk_image_menu_item_new_with_mnemonic (gettext (pAction->text));
        gtk_container_add (GTK_CONTAINER (submenuContainer), pAction->menu);
        widget = gtk_image_new_from_stock (pAction->stockimg, GTK_ICON_SIZE_MENU);
        gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (pAction->menu), widget);
        gtk_widget_set_sensitive (GTK_WIDGET(pAction->menu), FALSE);
        g_signal_connect ((gpointer) pAction->menu, "activate",
                          G_CALLBACK (mainDlgCoeffAction), pAction);
    } /* for */


    widget = gtk_separator_menu_item_new ();
    gtk_container_add (GTK_CONTAINER (menuContainer), widget);
    gtk_widget_set_sensitive (widget, FALSE);

    menuItem = gtk_image_menu_item_new_from_stock ("gtk-preferences", accel_group);
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (editDlgSettingsActivate), NULL);

    menuMainItem = gtk_menu_item_new_with_mnemonic (_("_View"));
    gtk_container_add (GTK_CONTAINER (menuMain), menuMainItem);

    menuContainer = gtk_menu_new ();
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuMainItem), menuContainer);

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("_Amplitude Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_AMPLITUDE));

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("A_ttenuation"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_ATTENUATION));

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("_Characteristic Function"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_CHAR));

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("_Phase Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_PHASE));

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("Phase _Delay"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_DELAY));

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("_Group Delay"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_GROUP));

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("_Impulse Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_IMPULSE));

    menuItem = gtk_check_menu_item_new_with_mnemonic (_("_Step Response"));
    gtk_container_add (GTK_CONTAINER (menuContainer), menuItem);
    g_signal_connect ((gpointer) menuItem, "activate",
                      G_CALLBACK (responseWinMenuActivate),
                      GINT_TO_POINTER (RESPONSE_TYPE_STEP));

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
                      G_CALLBACK (fileDlgNewActivate), NULL);

    widget = (GtkWidget*) gtk_separator_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (toolbarMain), widget);

    btnPreferences = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-preferences");
    gtk_container_add (GTK_CONTAINER (toolbarMain), btnPreferences);
    g_signal_connect ((gpointer) btnPreferences, "clicked",
                      G_CALLBACK (editDlgSettingsActivate), NULL);

    separatortoolitem2 = (GtkWidget*) gtk_separator_tool_item_new ();
    gtk_container_add (GTK_CONTAINER (toolbarMain), separatortoolitem2);

    btnExit = (GtkWidget*) gtk_tool_button_new_from_stock ("gtk-quit");
    gtk_container_add (GTK_CONTAINER (toolbarMain), btnExit);

    hbox1 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox1), 6);

    boxDesignDlg = gtk_vbox_new (FALSE, 12);
    gtk_box_pack_start (GTK_BOX (hbox1), boxDesignDlg, FALSE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (boxDesignDlg), 6);

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
    gtk_container_set_border_width (GTK_CONTAINER (boxFilterDlg), 6);

    vpaneFilter = gtk_vpaned_new ();
    gtk_box_pack_start (GTK_BOX (boxFilterDlg), vpaneFilter, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vpaneFilter), 6);

    frame1 = gtk_frame_new (NULL);
    gtk_paned_pack1 (GTK_PANED (vpaneFilter), frame1, FALSE, TRUE);
    gtk_container_set_border_width (GTK_CONTAINER (frame1), 6);

    gtk_container_add (GTK_CONTAINER (frame1), rootsPlotCreate ());

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

    treeDenominator = createCoeffListTreeView (&treeNumerator);
    gtk_container_add (GTK_CONTAINER (scrolledwindow2), GTK_WIDGET (treeDenominator));
    gtk_tooltips_set_tip (tooltips, GTK_WIDGET (treeDenominator),
                          _("Denominator coefficients"), NULL);

    scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
    gtk_table_attach (GTK_TABLE (table6), scrolledwindow1, 0, 1, 1, 2,
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL),
                      (GtkAttachOptions) (GTK_EXPAND | GTK_SHRINK | GTK_FILL), 0, 0);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_IN);

    treeNumerator = createCoeffListTreeView (&treeDenominator);
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
    gtk_box_set_spacing (GTK_BOX (hbuttonbox3), 6);

    for (i = 0, pAction = mainDlgCoeffBtn;
         i < N_ELEMENTS (mainDlgCoeffBtn);
         i++, pAction++)
    {
        pAction->btn = gtk_button_new ();
        gtk_container_add (GTK_CONTAINER (hbuttonbox3), pAction->btn);
        gtk_widget_set_sensitive (pAction->btn, FALSE);
        gtk_tooltips_set_tip (tooltips, pAction->btn,
                              gettext (pAction->tooltip), NULL);
        widget = gtk_image_new_from_stock (pAction->stockimg,
                                           GTK_ICON_SIZE_BUTTON);
        gtk_container_add (GTK_CONTAINER (pAction->btn), widget);
        g_signal_connect ((gpointer) pAction->btn, "clicked",
                          G_CALLBACK (mainDlgCoeffAction), pAction);
    } /* for */


    statusbar = gtk_statusbar_new ();
    gtk_box_pack_start (GTK_BOX (vbox1), statusbar, FALSE, FALSE, 0);

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
    GLADE_HOOKUP_OBJECT (topWidget, scrolledwindow1, "scrolledwindow1");
    GLADE_HOOKUP_OBJECT (topWidget, scrolledwindow2, "scrolledwindow2");
    GLADE_HOOKUP_OBJECT (topWidget, hbuttonbox3, "hbuttonbox3");
    GLADE_HOOKUP_OBJECT_NO_REF (topWidget, tooltips, "tooltips");

    gtk_window_add_accel_group (GTK_WINDOW (topWidget), accel_group);
    gtk_widget_show_all (topWidget);
    gtk_widget_grab_focus (comboFilterClass);

    mainDlgUpdateAll (NULL);                             /* set initial state */

    gtk_widget_size_request (boxDesignDlg, &size);
    gtk_paned_set_position (GTK_PANED (vpaneFilter), size.height / 3);

    return topWidget;
} /* mainDlgCreate() */



/* FUNCTION *******************************************************************/
/** Updates the project information in statusbar.
 *
 ******************************************************************************/
void mainDlgUpdatePrjInfo ()
{
    static guint prjInfoMsgId, prjInfoContextId;
    static BOOL firstCall = TRUE;

    char *msg;
    const DFCPRJ_INFO *pInfo = dfcPrjGetInfo ();

    if (firstCall)
    {
        prjInfoContextId =
            gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar),
                                          PACKAGE "Project Info");
        firstCall = FALSE;
    } /* if */
    else
    {
        gtk_statusbar_remove (GTK_STATUSBAR (statusbar), prjInfoContextId,
                              prjInfoMsgId);
    } /* else */


    if (pInfo->title == NULL)                              /* no title given? */
    {
        if (pInfo->author == NULL)
        {
            msg = g_strdup ("");      /* prepare unconditional g_free() later */
        } /* if */
        else
        {
            msg = g_strdup (pInfo->author);               /* only show author */
        } /* else */
    } /* if */
    else                                                  /* there is a title */
    {
        if (pInfo->author == NULL)                           /* but no author */
        {
            msg = g_strdup (pInfo->title);
        } /* if */
        else                                                /* both specified */
        {
            msg = g_strdup_printf ("%s (%s)", pInfo->title, pInfo->author);
        } /* else */
    } /* else */

    prjInfoMsgId = gtk_statusbar_push (GTK_STATUSBAR (statusbar),
                                       prjInfoContextId, msg);
    g_free (msg);
} /* mainDlgUpdatePrjInfo() */




/* FUNCTION *******************************************************************/
/** Updates the main filter dialog from current project (if there is no error
 *  passed in via \p err). If there is an critical error (coded as from function
 *  filterCheck(), then it displays an error box. If \c FLTERR_WARNING(err)
 *  indicates loss of coefficients then a warning will be shown.
 *
 *  \param err      Error indicator with coding as from filterCheck().
 *
 *  \return         Returns TRUE if the error is not FLTERR_CRITICAL(err),
 *                  else FALSE.
 ******************************************************************************/
BOOL mainDlgUpdateFilter (int err)
{
    GtkListStore *storeNum, *storeDen;

    FLTCOEFF* pFilter = dfcPrjGetFilter();
    BOOL valid = pFilter != NULL;

    if (!FLTERR_CRITICAL (err))
    {
        if (FLTERR_WARNING (err))
        {
            GtkWidget *dialog =
                gtk_message_dialog_new (GTK_WINDOW (topWidget),
                                        GTK_DIALOG_DESTROY_WITH_PARENT,
                                        GTK_MESSAGE_WARNING,
                                        GTK_BUTTONS_CLOSE,
                                        _("Filter generation has dropped some"
                                          " (near zero) coefficients, but the"
                                          " filter is still valid."));
            gtk_dialog_run (GTK_DIALOG (dialog));
            gtk_widget_destroy (dialog);
        } /* if */


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
        rootsPlotUpdate (pFilter);
        return TRUE;
    } /* if */

    return FALSE;
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
    mainDlgUpdateFilter (0);         /* update coefficients and roots display */
    mainDlgUpdatePrjInfo ();

    gtk_widget_grab_default (lookup_widget (topWidget, MAINDLG_BTN_APPLY));

    if (filename != NULL)
    {
        gchar *tmp, *utf8name = g_filename_to_utf8 (filename, -1, NULL, NULL, NULL);

        if (utf8name == NULL)             /* may be an UTF-8 conversion error */
        {
            utf8name = g_strdup (_("<i>Unknown</i>"));
        } /* if */

        tmp = g_strdup_printf ("%s: %s", PACKAGE, utf8name);
        gtk_window_set_title (GTK_WINDOW (topWidget), tmp);
        g_free (utf8name);
        g_free (tmp);

        return;
    } /* if */

    gtk_window_set_title (GTK_WINDOW (topWidget), PACKAGE);
} /* mainDlgUpdateAll() */



/******************************************************************************/
/* END OF FILE                                                                */
/******************************************************************************/