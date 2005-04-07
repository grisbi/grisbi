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
#include "fenetre_principale_constants.h"
#include "accueil.h"
#include "operations_onglet.h"
#include "comptes_onglet.h"
#include "echeancier_onglet.h"
#include "etats_onglet.h"
#include "gsb_account.h"
#include "main.h"
#include "categories_onglet.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"
#include "utils_buttons.h"
#include "operations_comptes.h"
#include "comptes_gestion.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean gsb_gui_select_page ( GtkWidget * button, gpointer page );
static gboolean gsb_gui_select_account_page ( GtkWidget * button, gpointer account );
static GtkWidget * create_account_list ( void );
static GtkWidget * create_management_list ( void );
/*END_STATIC*/


/* adr du notebook de base */
GtkWidget *notebook_general;
GtkWidget *main_hpaned, *main_vbox, *main_statusbar;

/* adr de l'onglet accueil */
GtkWidget *page_accueil;


GtkWidget *notebook_comptes_equilibrage;


gint modif_tiers;
gint modif_categ;
gint modif_imputation;


/*START_EXTERN*/
extern GtkTreeStore *budgetary_line_tree_model;
extern GtkTreeStore * categ_tree_model;
extern AB_BANKING *gbanking;
extern gint id_temps;
extern GtkWidget *label_temps;
extern GtkTreeStore *payee_tree_model;
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
			   g_strconcat ( "<span weight=\"bold\">",
					 "Opérations du compte : compte courant",
					 "</span>", NULL ) );
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
    gtk_box_pack_start ( GTK_BOX(hbox), label, TRUE, TRUE, 3 );

    label = gtk_label_new ("");
    gtk_label_set_markup ( GTK_LABEL(label), g_strconcat ( "<span weight=\"bold\">",
							   "Solde: 1000 €",
							   "</span>", NULL ) );
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
    GtkWidget * notebook;

    notebook = gtk_notebook_new ();

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), create_account_list (),
			       gtk_label_new (_("Accounts")) );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ), create_management_list (),
			       gtk_label_new (_("Manage")) );

    gtk_widget_show_all ( notebook );
    return notebook;
}



/**
 * Create a list of buttons that are shortcuts to accounts.
 *
 * \return A newly allocated vbox containing buttons.
 */
static GtkWidget * create_account_list ( void )
{
    GtkWidget * vbox, *button;
    GSList *list_tmp;

    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER(vbox), 3 );

    /* Create homepage shortcut. */
    button = new_button_with_label_and_image ( GSB_BUTTON_BOTH,
					       _("Resume"), "resume.png",
					       NULL, NULL );
    g_signal_connect ( G_OBJECT(button), "clicked", 
		       G_CALLBACK ( gsb_gui_select_page ), GSB_HOME_PAGE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    gtk_box_pack_start ( GTK_BOX ( vbox ), gtk_hseparator_new(), FALSE, FALSE, 6 );

    /* Fill in with accounts. */
    list_tmp = gsb_account_get_list_accounts ();
    while ( list_tmp )
    {
	gint i = gsb_account_get_no_account ( list_tmp -> data );

	if ( !gsb_account_get_closed_account ( i ) )
	{
	    button = new_button_with_label_and_image ( GSB_BUTTON_BOTH,
						       gsb_account_get_name(i),
						       "money.png",
						       NULL, NULL );
	    g_signal_connect ( G_OBJECT(button), "clicked", 
			       G_CALLBACK ( gsb_gui_select_account_page ), (gpointer) i );
	    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );
	}

	list_tmp = list_tmp -> next;
    }

    return vbox;
}



/**
 * Create a list of buttons that are shortcuts to accounts.
 *
 * \return A newly allocated vbox containing buttons.
 */
static GtkWidget * create_management_list ( void )
{
    GtkWidget * vbox, *button;

    vbox = gtk_vbox_new ( FALSE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 3 );

    /* Create payees shortcut. */
    button = new_button_with_label_and_image ( GSB_BUTTON_BOTH,
					       _("Payees"), "payees.png",
					       NULL, NULL );
    g_signal_connect ( G_OBJECT(button), "clicked", 
		       G_CALLBACK ( gsb_gui_select_page ), 
		       (gpointer) GSB_PAYEES_PAGE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    /* Create categories shortcut. */
    button = new_button_with_label_and_image ( GSB_BUTTON_BOTH,
					       _("Categories"), "categories.png",
					       NULL, NULL );
    g_signal_connect ( G_OBJECT(button), "clicked", 
		       G_CALLBACK ( gsb_gui_select_page ), 
		       (gpointer) GSB_CATEGORIES_PAGE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    /* Create budgetary lines shortcut. */
    button = new_button_with_label_and_image ( GSB_BUTTON_BOTH,
					       _("Budgetary lines"), "budgetary_lines.png",
					       NULL, NULL );
    g_signal_connect ( G_OBJECT(button), "clicked", 
		       G_CALLBACK ( gsb_gui_select_page ), 
		       (gpointer) GSB_BUDGETARY_LINES_PAGE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

    /* Create reports shortcut. */
    button = new_button_with_label_and_image ( GSB_BUTTON_BOTH,
					       _("Reports"), "reports.png",
					       NULL, NULL );
    g_signal_connect ( G_OBJECT(button), "clicked", 
		       G_CALLBACK ( gsb_gui_select_page ), 
		       (gpointer) GSB_REPORTS_PAGE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );

#ifdef HAVE_G2BANKING
    /* Create aqbanking shortcut. */
    button = new_button_with_label_and_image ( GSB_BUTTON_BOTH,
					       _("AQ Banking"), "hbci.png",
					       NULL, NULL );
    g_signal_connect ( G_OBJECT(button), "clicked", 
		       G_CALLBACK ( gsb_gui_select_page ), 
		       (gpointer) GSB_AQBANKING_PAGE );
    gtk_box_pack_start ( GTK_BOX ( vbox ), button, FALSE, FALSE, 0 );
#endif

    return vbox;
}



/**
 * Create the main notebook.
 *
 * \return the notebook
 */
GtkWidget *create_main_notebook (void )
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

    /* Création de la page d'accueil */
    page_accueil = creation_onglet_accueil();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_accueil,
			       gtk_label_new (SPACIFY(_("Main page"))) );

    /*  Céation de la fenêtre principale qui contient d'un côté */
    /*  les comptes, et de l'autre les opérations */
    page_compte = gtk_notebook_new ();
    gtk_widget_show ( page_compte );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ), page_compte,
			       gtk_label_new (SPACIFY(_("Accounts"))) );

    page_operations = create_transaction_page ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( page_compte ), page_operations,
			       gtk_label_new (SPACIFY(_("Transactions"))) );

    /*   création de la fenetre des echéances */
    page_echeancier = creation_onglet_echeancier();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( page_compte ), page_echeancier,
			       gtk_label_new (SPACIFY(_("Scheduler"))) );

    /*   création de la fenetre des comptes */
    page_prop = creation_onglet_comptes ();
    gtk_widget_show_all ( page_prop );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( page_compte ), page_prop,
			       gtk_label_new (SPACIFY(_("Properties"))) );

/*     gtk_widget_show_all ( page_compte ); */

    /* Création de la fenetre des tiers */
    page_tiers = onglet_tiers();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_tiers,
			       gtk_label_new (SPACIFY(_("Third party"))) );

    /* création de la fenetre des categories */
    page_categories = onglet_categories();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_categories,
			       gtk_label_new (SPACIFY(_("Categories"))) );

    /* création de la fenetre des imputations budgétaires */
    page_imputations = onglet_imputations();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_imputations,
			       gtk_label_new (SPACIFY(_("Budgetary lines"))) );

    /* création de la fenetre des états */
    page_etats = creation_onglet_etats ();
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
			       page_etats,
			       gtk_label_new (SPACIFY(_("Reports"))) );

#ifdef HAVE_G2BANKING
    page_queue = GBanking_JobView_new(gbanking, 0);
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook_general ),
                              page_queue,
                              gtk_label_new (SPACIFY(_("Outbox"))) );
#endif

    gtk_signal_connect_after ( GTK_OBJECT ( notebook_general ),
			       "switch_page",
			       GTK_SIGNAL_FUNC ( change_page_notebook),
			       NULL );

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

    switch ( numero_page )
    {
	case GSB_HOME_PAGE:
	    mise_a_jour_accueil ();
	    break;

	case GSB_PAYEES_PAGE:
	    if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (payee_tree_model), &dummy_iter ) )
		remplit_arbre_tiers ();
	    break;

	case GSB_CATEGORIES_PAGE:
	    if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (categ_tree_model), &dummy_iter ) )
		remplit_arbre_categ ();
	    break;

	case GSB_BUDGETARY_LINES_PAGE:
	    if ( ! gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (budgetary_line_tree_model), &dummy_iter ) )
		remplit_arbre_imputation ();
	    break;

	default:
	    break;
    }

    return ( FALSE );
}



/**
 * Selects the home page in general notebook.
 *
 * \param button	Widget that triggered event.
 * \param page		Page to select.
 *
 * \return		FALSE
 */
static gboolean gsb_gui_select_page ( GtkWidget * button, gpointer page )
{
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), (gint) page );
    return FALSE;
}



/**
 * Selects the account page with an account selected in general
 * notebook.
 *
 * \param button	Widget that triggered event.
 * \param account	Account number to display.
 * \return		FALSE
 */
static gboolean gsb_gui_select_account_page ( GtkWidget * button, gpointer account )
{
    extern gint compte_courant_onglet;

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ), GSB_ACCOUNT_PAGE );
    gsb_account_list_gui_change_current_account ( account );

    compte_courant_onglet = GPOINTER_TO_INT ( account );
    remplissage_details_compte ();

    return FALSE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
