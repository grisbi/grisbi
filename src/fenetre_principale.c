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

/*START_INCLUDE*/
#include "fenetre_principale.h"
#include "navigation.h"
#include "operations_onglet.h"
#include "accueil.h"
#include "comptes_onglet.h"
#include "echeancier_onglet.h"
#include "etats_onglet.h"
#include "erreur.h"
#include "gsb_transactions_list.h"
#include "main.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "include.h"
#include "fenetre_principale.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
static  GtkWidget *create_main_notebook (void );
/*END_STATIC*/


/* adr du notebook de base */
GtkWidget *notebook_general;
GtkWidget *main_hpaned = NULL;
GtkWidget *main_vbox;

/* adr de l'onglet accueil */
GtkWidget *page_accueil;

GtkWidget *notebook_comptes_equilibrage;

/** Notebook of the account pane. */
GtkWidget *account_page;

/** Title for the heading bar. */
GtkWidget * headings_title;
/** Suffix for the heading bar.  */
GtkWidget * headings_suffix;


gint modif_categ;
gint modif_imputation;


/*START_EXTERN*/
extern GtkTreeStore *budgetary_line_tree_model;
extern GtkTreeStore * categ_tree_model;
extern AB_BANKING *gbanking;
extern GtkWidget * hpaned;
extern GtkWidget *main_statusbar ;
extern GtkTreeStore *payee_tree_model;
extern GtkWidget * scheduler_calendar;
/*END_EXTERN*/


#ifdef HAVE_G2BANKING
extern AB_BANKING *gbanking;
#endif




/**
 * Create the main widget that holds all the user interface save the
 * menus.
 *
 * \return A newly-allocated vbox holding all elements.
 */
GtkWidget * create_main_widget ( void )
{
    GtkWidget *hbox, *eb, *statusbar;
    GtkStyle * style;

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

    /* Define labels. */
    headings_title = gtk_label_new ( "" );
    gtk_label_set_justify ( GTK_LABEL(headings_title), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (headings_title), 0.0, 0.5);
    gtk_box_pack_start ( GTK_BOX(hbox), headings_title, TRUE, TRUE, 3 );
    headings_suffix = gtk_label_new ("");
    gtk_box_pack_start ( GTK_BOX(hbox), headings_suffix, FALSE, FALSE, 0 );

    /* Change color with an event box trick. */
    style = gtk_widget_get_style ( eb );
    gtk_widget_modify_bg ( eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
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
    if ( etat.largeur_colonne_comptes_operation )
	gtk_paned_set_position ( GTK_PANED ( main_hpaned ), 
				 etat.largeur_colonne_comptes_operation );

    gtk_widget_show ( notebook_general );
    gtk_widget_show ( main_vbox );
    gtk_widget_show ( main_hpaned );
    gtk_widget_show ( main_statusbar );

    /* Blank the transaction list. */
    gsb_transactions_list_set_store (NULL);

    return main_vbox;
}



/**
 * Create the main notebook.
 *
 * \return the notebook
 */
static GtkWidget *create_main_notebook (void )
{
    GtkWidget *page_operations, *page_echeancier, *page_prop, *page_tiers;
    GtkWidget *page_categories, *page_imputations, *page_etats;
#ifdef HAVE_G2BANKING
    GtkWidget *page_queue;
#endif

    devel_debug ( "create_main_notebook" );

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
    account_page = gtk_notebook_new ();
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(account_page), FALSE );
    gtk_widget_show ( account_page );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), account_page,
			       gtk_label_new (SPACIFY(_("Accounts"))) );

    page_operations = create_transaction_page ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ), page_operations,
			       gtk_label_new (SPACIFY(_("Transactions"))) );

    /*   création de la fenetre des comptes */
    page_prop = creation_onglet_comptes ();
    gtk_widget_show_all ( page_prop );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ), page_prop,
			       gtk_label_new (SPACIFY(_("Properties"))) );

    /*   création de la fenetre des echéances */
    page_echeancier = creation_onglet_echeancier();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_echeancier,
			       gtk_label_new (SPACIFY(_("Scheduler"))) );

    /* Création de la fenetre des tiers */
    page_tiers = onglet_tiers();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_tiers,
			       gtk_label_new (SPACIFY(_("Payee"))) );

    /* création de la fenetre des categories */
    page_categories = onglet_categories();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_categories,
			       gtk_label_new (SPACIFY(_("Categories"))) );

    /* création de la fenetre des imputations budgétaires */
    page_imputations = onglet_imputations();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_imputations,
			       gtk_label_new (SPACIFY(_("Budgetary lines"))) );

    /* création de la fenetre gbanking */
#ifdef HAVE_G2BANKING
    page_queue = GBanking_JobView_new(gbanking, 0);
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_queue,
			       gtk_label_new (SPACIFY(_("Outbox"))) );
#endif

    /* création de la fenetre des états */
    page_etats = creation_onglet_etats ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_etats,
			       gtk_label_new (SPACIFY(_("Reports"))) );

    gtk_signal_connect_after ( GTK_OBJECT ( notebook_general ), "switch_page",
			       GTK_SIGNAL_FUNC ( gsb_gui_on_notebook_switch_page ), NULL );

    return ( notebook_general );
}



/**
 * Handler triggered when the main notebook changed page.  It is
 * responsible for initial widgets fill, because it is not done at the
 * first time to speed up startup.
 *
 * \param notebook	Widget that triggered event.
 * \param page		Not used.
 * \param numero_page	Page set.
 * \param null		Not used.
 *
 * \return		FALSE
 */
gboolean gsb_gui_on_notebook_switch_page ( GtkNotebook *notebook,
					   GtkNotebookPage *page,
					   guint numero_page,
					   gpointer null )
{
    GtkTreeIter dummy_iter;

    if ( numero_page != GSB_SCHEDULER_PAGE ) 
    {
	gtk_widget_hide_all ( scheduler_calendar );
    }

    switch ( numero_page )
    {
    case GSB_HOME_PAGE:
	mise_a_jour_accueil (FALSE);
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
 * Update headings bar with a new title and a new suffix.
 *
 * \param title		String to display as a title in headings bar.
 * \param suffix	String to display as a suffix in headings bar.
 *
 */
void gsb_gui_headings_update ( gchar * title, gchar * suffix )
{
    gtk_label_set_markup ( GTK_LABEL(headings_title), 
			   g_strconcat ( "<b>", title, "</b>", NULL ) );
    gtk_label_set_markup ( GTK_LABEL(headings_suffix), 
			   g_strconcat ( "<b>", suffix, "</b>", NULL ) );
}



/**
 * Set the main notebook page.
 * 
 * \param page		Page to set.
 */
void gsb_gui_notebook_change_page ( GsbGeneralNotebookPages page )
{
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), page );
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
