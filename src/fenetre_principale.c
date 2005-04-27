/* ************************************************************************** */
/*  Fichier qui s'occupe de former les différentes fenêtres de travail      */
/*                                                                            */
/*     Copyright (C)	2000-2005 CÃ©dric Auger (cedric@grisbi.org)      */
/*			     2005 Benjamin Drieu (bdrieu@april.org)	      */
/* 			http://www.grisbi.org				      */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

#include "include.h"
#include "fenetre_principale_constants.h"
#include "fichier_configuration_constants.h"

/*START_INCLUDE*/
#include "fenetre_principale.h"
#include "etats_onglet.h"
#include "operations_onglet.h"
#include "accueil.h"
#include "comptes_onglet.h"
#include "echeancier_onglet.h"
#include "echeancier_infos.h"
#include "gsb_account.h"
#include "operations_comptes.h"
#include "main.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "comptes_gestion.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void create_account_list ( GtkTreeModel * model, GtkTreeIter * account_iter );
static  GtkWidget *create_main_notebook (void );
static GtkWidget * create_navigation_pane ( void );
static void create_report_list ( GtkTreeModel * model, GtkTreeIter * reports_iter );
static  gboolean gsb_gui_select_navigation_link ( GtkTreeSelection * selection,
						 GtkTreeModel * model );
/*END_STATIC*/


/* adr du notebook de base */
GtkWidget *notebook_general;
GtkWidget *main_hpaned, *main_vbox, *main_statusbar;

/* adr de l'onglet accueil */
GtkWidget *page_accueil;

GtkWidget *notebook_comptes_equilibrage;

/** Widget that hold the scheduler calendar. */
GtkWidget * scheduler_calendar;

gint modif_tiers;
gint modif_categ;
gint modif_imputation;


/*START_EXTERN*/
extern GtkTreeStore *budgetary_line_tree_model;
extern GtkTreeStore * categ_tree_model;
extern gint compte_courant_onglet;
extern AB_BANKING *gbanking;
extern GtkWidget * hpaned;
extern gint id_temps;
extern GSList *liste_struct_etats;
extern GtkTreeStore *model;
extern GtkTreeStore *payee_tree_model;
extern GtkTreeSelection * selection;
extern GtkWidget *tree_view;
/*END_EXTERN*/

#ifdef HAVE_G2BANKING
extern AB_BANKING *gbanking;
#endif


/** Holds data for the navigation tree.  */
enum navigation_cols { 
    NAVIGATION_PIX, NAVIGATION_PIX_VISIBLE,
    NAVIGATION_TEXT, NAVIGATION_FONT,
    NAVIGATION_PAGE, NAVIGATION_ACCOUNT,
    NAVIGATION_REPORT, NAVIGATION_TOTAL,
};



/**
 * Create the main widget that holds all the user interface save the
 * menus.
 *
 * \return A newly-allocated vbox holding all elements.
 */
GtkWidget * create_main_widget ( void )
{
    GtkWidget *label, *hbox, *eb;
    GdkColor bg = { 0, 54016, 54016, 54016 };

    /* All stuff will be put in a huge vbox, with an hbox containing
     * quick summary. */
    main_vbox = gtk_vbox_new ( FALSE, 0 );
    eb = gtk_event_box_new ();
    hbox = gtk_hbox_new ( FALSE, 0 );

    /* Create two arrows (dummy atm). */
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_arrow_new ( GTK_ARROW_LEFT,GTK_SHADOW_OUT ), 
			 FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_arrow_new ( GTK_ARROW_RIGHT,GTK_SHADOW_OUT ), 
			 FALSE, FALSE, 3 );

    /* Define labels (dummy atm). */
    label = gtk_label_new ( "" );
    gtk_label_set_markup ( GTK_LABEL(label), 
			   g_strconcat ( "<b>", "Opérations du compte : compte courant",
					 "</b>", NULL ) );
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start ( GTK_BOX(hbox), label, TRUE, TRUE, 3 );

    label = gtk_label_new ("");
    gtk_label_set_markup ( GTK_LABEL(label), g_strconcat ( "<b>",
							   "Solde: 1000 €",
							   "</b>", NULL ) );
    gtk_box_pack_start ( GTK_BOX(hbox), label, FALSE, FALSE, 0 );

    /* Change color with an event box trick. */
    gtk_widget_modify_bg ( eb, 0, &bg );
    gtk_container_add ( GTK_CONTAINER ( eb ), hbox );
    gtk_container_set_border_width ( GTK_CONTAINER ( hbox ), 6 );

    gtk_box_pack_start ( GTK_BOX(main_vbox), eb, FALSE, FALSE, 0 );
    gtk_widget_show_all ( eb );

    /* Then create and fill the main hpaned. */
    main_hpaned = gtk_hpaned_new ();
    gtk_box_pack_start ( GTK_BOX(main_vbox), main_hpaned, TRUE, TRUE, 0 );
    gtk_paned_add1 ( GTK_PANED( main_hpaned ), create_navigation_pane ( ) );
    gtk_paned_add2 ( GTK_PANED( main_hpaned ), create_main_notebook ( ) );
    gtk_container_set_border_width ( GTK_CONTAINER ( main_hpaned ), 6 );

    main_statusbar = gtk_statusbar_new ();
    gtk_box_pack_start ( GTK_BOX(main_vbox), main_statusbar, FALSE, FALSE, 0 );

    gtk_widget_show ( notebook_general );
    gtk_widget_show ( main_vbox );
    gtk_widget_show ( main_hpaned );
    gtk_widget_show ( main_statusbar );

    return main_vbox;
}



/**
 * Create the navigation pane on the left of the GUI.  It contains
 * account list as well as shortcuts.
 *
 * \return The newly allocated pane.
 */
GtkWidget * create_navigation_pane ( void )
{
    GtkWidget * sw, * tree_view, *column, *vbox;
    GtkTreeModel * model;
    GdkPixbuf * pixbuf;
    GtkTreeIter iter, account_iter, reports_iter;
    GtkCellRenderer * renderer;

    vbox = gtk_vbox_new ( FALSE, 6 );

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER,
				    GTK_POLICY_AUTOMATIC);

    model = GTK_TREE_MODEL ( gtk_tree_store_new (NAVIGATION_TOTAL, GDK_TYPE_PIXBUF,
						 G_TYPE_BOOLEAN, G_TYPE_STRING, 
						 G_TYPE_INT, G_TYPE_INT, G_TYPE_INT, 
						 G_TYPE_POINTER ) );
    tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL(model));
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW(tree_view), FALSE );
    gtk_container_add ( GTK_CONTAINER(sw), tree_view );

    g_signal_connect (gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view)), "changed", 
		      ((GCallback) gsb_gui_select_navigation_link), model );

    renderer = gtk_cell_renderer_pixbuf_new ();
    column = GTK_WIDGET ( gtk_tree_view_column_new_with_attributes ( "", renderer,
								     "visible", 
								     NAVIGATION_PIX_VISIBLE, 
								     "pixbuf", 
								     NAVIGATION_PIX, NULL) );
    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, TRUE);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "text", NAVIGATION_TEXT);
    gtk_tree_view_column_add_attribute(GTK_TREE_VIEW_COLUMN(column), renderer, 
				       "weight", NAVIGATION_FONT);

    gtk_tree_view_column_pack_start(GTK_TREE_VIEW_COLUMN(column), renderer, FALSE);

    gtk_tree_view_append_column ( GTK_TREE_VIEW ( tree_view ), 
				  GTK_TREE_VIEW_COLUMN ( column ) );
    /* Account list */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "resume.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(model), &account_iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(model), &account_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Accounts"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_HOME_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       -1);
    create_account_list ( model, &account_iter );

    /* Scheduler */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "scheduler.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Scheduler"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_SCHEDULER_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       -1 );

    /* Payees */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "payees.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Payees"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_PAYEES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       -1 );

    /* Categories */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "categories.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Categories"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_CATEGORIES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       -1 );

    /* Budgetary lines */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "budgetary_lines.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(model), &iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Budgetary lines"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_BUDGETARY_LINES_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       -1 );

    /* Reports */
    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, C_DIRECTORY_SEPARATOR,
						     "reports.png", NULL ), NULL );
    gtk_tree_store_append(GTK_TREE_STORE(model), &reports_iter, NULL);
    gtk_tree_store_set(GTK_TREE_STORE(model), &reports_iter, 
		       NAVIGATION_PIX, pixbuf,
		       NAVIGATION_TEXT, _("Reports"), 
		       NAVIGATION_PIX_VISIBLE, TRUE, 
		       NAVIGATION_FONT, 800,
		       NAVIGATION_PAGE, GSB_REPORTS_PAGE,
		       NAVIGATION_ACCOUNT, -1,
		       -1 );
    create_report_list ( model, &reports_iter );

    gtk_tree_view_expand_all ( GTK_TREE_VIEW(tree_view) );

    gtk_box_pack_start ( GTK_BOX(vbox), sw, TRUE, TRUE, 0 );

    scheduler_calendar = creation_partie_gauche_echeancier();
    gtk_box_pack_end ( GTK_BOX(vbox), scheduler_calendar, FALSE, FALSE, 0 );

    gtk_widget_show_all ( vbox );
    gtk_widget_hide_all ( scheduler_calendar );

    return vbox;
}



/**
 * Create a list of tree items that are shortcuts to accounts.
 *
 * \param model		Tree model to insert items into.
 * \param account_iter	Parent iter.
 */
void create_account_list ( GtkTreeModel * model, GtkTreeIter * account_iter )
{
    GSList *list_tmp;

    /* Fill in with accounts. */
    list_tmp = gsb_account_get_list_accounts ();
    while ( list_tmp )
    {
	gint i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !gsb_account_get_closed_account ( i ) )
	{
	    GdkPixbuf * pixbuf;
	    GtkTreeIter iter;
	    gchar * account_icon;
	    
	    switch ( gsb_account_get_kind(i) )
		{
		case GSB_TYPE_BANK:
		    account_icon = "bank-account";
		    break;

		case GSB_TYPE_CASH:
		    account_icon = "money";
		    break;

		case GSB_TYPE_ASSET:
		case GSB_TYPE_LIABILITIES:
		    account_icon = "asset";
		    break;

		default:
		    account_icon = "warnings";
		    break;
		}

	    pixbuf = gdk_pixbuf_new_from_file ( g_strconcat( PIXMAPS_DIR, 
							     C_DIRECTORY_SEPARATOR,
							     account_icon, ".png", NULL ),
						NULL );
	    gtk_tree_store_append(GTK_TREE_STORE(model), &iter, account_iter);
	    gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
			       NAVIGATION_PIX, pixbuf,
			       NAVIGATION_PIX_VISIBLE, TRUE, 
			       NAVIGATION_TEXT, gsb_account_get_name(i), 
			       NAVIGATION_FONT, 400,
			       NAVIGATION_PAGE, GSB_ACCOUNT_PAGE,
			       NAVIGATION_ACCOUNT, i,
			       -1 );
	}

	list_tmp = list_tmp -> next;
    }
}



/**
 * Create a list of tree items that are shortcuts to reports.
 *
 * \param model		Tree model to insert items into.
 * \param reports_iter	Parent iter.
 */
void create_report_list ( GtkTreeModel * model, GtkTreeIter * reports_iter )
{
    GSList *tmp_list;
    GtkTreeIter iter;

    /* Fill in with accounts. */
    tmp_list = liste_struct_etats;
    while ( tmp_list )
    {
	struct struct_etat * etat = tmp_list -> data;

	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, reports_iter);
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, 
			   NAVIGATION_PIX_VISIBLE, TRUE, 
			   NAVIGATION_TEXT, etat -> nom_etat,
			   NAVIGATION_FONT, 400,
			   NAVIGATION_PAGE, GSB_REPORTS_PAGE,
			   NAVIGATION_ACCOUNT, -1,
			   NAVIGATION_REPORT, etat,
			   -1 );
	
	tmp_list = tmp_list -> next;
    }
}



/**
 * Create the main notebook.
 *
 * \return the notebook
 */
static GtkWidget *create_main_notebook (void )
{
    GtkWidget *page_operations, *page_echeancier, *page_prop, *page_tiers;
    GtkWidget *page_categories, *page_imputations, *page_etats, *page_compte;
#ifdef HAVE_G2BANKING
    GtkWidget *page_queue;
#endif

    if ( DEBUG )
	printf ( "create_main_notebook\n" );

    /* création du notebook de base */

    notebook_general = gtk_notebook_new();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(notebook_general), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(notebook_general), FALSE );

    /* Création de la page d'accueil */
    page_accueil = creation_onglet_accueil();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_accueil,
			       gtk_label_new (SPACIFY(_("Main page"))) );

    /*  Céation de la fenêtre principale qui contient d'un côté */
    /*  les comptes, et de l'autre les opérations */
    page_compte = gtk_notebook_new ();
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(page_compte), FALSE );
    gtk_widget_show ( page_compte );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_compte,
			       gtk_label_new (SPACIFY(_("Accounts"))) );

    page_operations = create_transaction_page ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( page_compte ), page_operations,
			       gtk_label_new (SPACIFY(_("Transactions"))) );

    /*   création de la fenetre des comptes */
    page_prop = creation_onglet_comptes ();
    gtk_widget_show_all ( page_prop );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( page_compte ), page_prop,
			       gtk_label_new (SPACIFY(_("Properties"))) );

/*     gtk_widget_show_all ( page_compte ); */

    /*   création de la fenetre des echéances */
    page_echeancier = creation_onglet_echeancier();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_echeancier,
			       gtk_label_new (SPACIFY(_("Scheduler"))) );

    /* Création de la fenetre des tiers */
    page_tiers = onglet_tiers();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_tiers,
			       gtk_label_new (SPACIFY(_("Third party"))) );

    /* création de la fenetre des categories */
    page_categories = onglet_categories();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_categories,
			       gtk_label_new (SPACIFY(_("Categories"))) );

    /* création de la fenetre des imputations budgétaires */
    page_imputations = onglet_imputations();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_imputations,
			       gtk_label_new (SPACIFY(_("Budgetary lines"))) );

    /* création de la fenetre des états */
    page_etats = creation_onglet_etats ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_etats,
			       gtk_label_new (SPACIFY(_("Reports"))) );

#ifdef HAVE_G2BANKING
    page_queue = GBanking_JobView_new(gbanking, 0);
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_queue,
                              gtk_label_new (SPACIFY(_("Outbox"))) );
#endif

    gtk_signal_connect_after ( GTK_OBJECT ( notebook_general ), "switch_page",
			       GTK_SIGNAL_FUNC ( change_page_notebook), NULL );

    return ( notebook_general );
}
/***********************************************************************************************************/



/***********************************************************************************************************/

gboolean change_page_notebook ( GtkNotebook *notebook,
				GtkNotebookPage *page,
				guint numero_page,
				gpointer null )
{
    GtkTreeIter dummy_iter;

    /* retire l'horloge si part de l'accueil */

    if ( id_temps )
    {
	gtk_timeout_remove ( id_temps );
	id_temps = 0;
    }

    if ( numero_page != GSB_SCHEDULER_PAGE ) 
	gtk_widget_hide_all ( scheduler_calendar );

    switch ( numero_page )
    {
    case GSB_HOME_PAGE:
	mise_a_jour_accueil ();
	break;

    case GSB_PAYEES_PAGE:
	if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (payee_tree_model), 
					       &dummy_iter ) )
	    remplit_arbre_tiers ();
	break;

    case GSB_CATEGORIES_PAGE:
	if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (categ_tree_model), 
					       &dummy_iter ) )
	    remplit_arbre_categ ();
	break;

    case GSB_BUDGETARY_LINES_PAGE:
	if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (budgetary_line_tree_model), 
					       &dummy_iter ) )
	    remplit_arbre_imputation ();
	break;

    case GSB_SCHEDULER_PAGE:
	gtk_widget_show_all ( scheduler_calendar );
	break;
	
    default:
	break;
    }

    return ( FALSE );
}



/**
 * Selects the account or management page with an optional account
 * selected in general notebook.
 *
 * \param selection	Selection of tree that triggered event.
 * \param selection	Model of tree that triggered event.
 *
 * \return		FALSE
 */
static gboolean gsb_gui_select_navigation_link ( GtkTreeSelection * selection,
						 GtkTreeModel * model )
{
    GtkTreeIter iter;
    GValue value = {0, }, value2 = {0, };

    if (! gtk_tree_selection_get_selected (selection, NULL, &iter))
	return(FALSE);

    gtk_tree_model_get_value (model, &iter, NAVIGATION_PAGE, &value);
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), g_value_get_int(&value) );
    g_value_unset (&value);

    gtk_tree_model_get_value (model, &iter, NAVIGATION_ACCOUNT, &value);
    if ( g_value_get_int(&value) >= 0 )
    {
	gsb_account_list_gui_change_current_account ( GINT_TO_POINTER(g_value_get_int(&value) ));
	compte_courant_onglet = g_value_get_int(&value);
	remplissage_details_compte ();
    }

    gtk_tree_model_get_value (model, &iter, NAVIGATION_REPORT, &value2);
    if ( g_value_get_pointer(&value2) > 0 )
    {
	changement_etat ( g_value_get_pointer(&value2) );
    }

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
