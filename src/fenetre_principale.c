/* ************************************************************************** */
/*  Fichier qui s'occupe de former les différentes fenêtres de travail      */
/*                                                                            */
/*     Copyright (C)	2000-2006 CÃ©dric Auger (cedric@grisbi.org)      */
/*			2005-2006 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "./balance_estimate_tab.h"
#include "./navigation.h"
#include "./gsb_transactions_list.h"
#include "./accueil.h"
#include "./comptes_onglet.h"
#include "./etats_onglet.h"
#include "./gsb_form.h"
#include "./gsb_plugins.h"
#include "./gsb_scheduler_list.h"
#include "./main.h"
#include "./categories_onglet.h"
#include "./imputation_budgetaire.h"
#include "./tiers_onglet.h"
#include "./structures.h"
#include "./include.h"
#include "./erreur.h"
#include "./gsb_plugins.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *create_main_notebook (void );
static gboolean gsb_gui_fill_main_notebook ( GtkWidget *notebook );
static gboolean gsb_gui_on_account_switch_page ( GtkNotebook *notebook,
					  GtkNotebookPage *page,
					  guint page_number,
					  gpointer null );
static gboolean gsb_gui_update_show_headings ();
static gboolean on_simpleclick_event_run ( GtkWidget * button, GdkEvent * button_event,
				    GCallback cb );
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *window ;
/*END_EXTERN*/


/* adr du notebook de base */
GtkWidget *notebook_general = NULL;
GtkWidget *main_hpaned = NULL;
GtkWidget *main_vbox = NULL;

/** Notebook of the account pane. */
GtkWidget *account_page = NULL;

static GtkWidget * headings_eb = NULL;

/** Title for the heading bar. */
static GtkWidget * headings_title = NULL;
/** Suffix for the heading bar.  */
static GtkWidget * headings_suffix = NULL;



/**
 * Create the main widget that holds all the user interface save the
 * menus.
 *
 * \return A newly-allocated vbox holding all elements.
 */
GtkWidget * create_main_widget ( void )
{
    GtkWidget * hbox, * arrow_eb, * arrow_left, * arrow_right;
    GtkStyle * style;

    /* All stuff will be put in a huge vbox, with an hbox containing
     * quick summary. */
    main_vbox = gtk_vbox_new ( FALSE, 0 );
    g_signal_connect ( G_OBJECT (main_vbox ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &main_vbox );

    headings_eb = gtk_event_box_new ();
    g_signal_connect ( G_OBJECT (headings_eb ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &headings_eb );
    style = gtk_widget_get_style ( headings_eb );

    hbox = gtk_hbox_new ( FALSE, 0 );

    /* Create two arrows. */
    arrow_left = gtk_arrow_new ( GTK_ARROW_LEFT, GTK_SHADOW_OUT );
    arrow_eb = gtk_event_box_new ();
    gtk_widget_modify_bg ( arrow_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
    gtk_container_add ( GTK_CONTAINER ( arrow_eb ), arrow_left );
    g_signal_connect ( G_OBJECT ( arrow_eb ), "button-press-event", 
		       G_CALLBACK ( on_simpleclick_event_run ), 
		       gsb_gui_navigation_select_prev );
    gtk_box_pack_start ( GTK_BOX ( hbox ), arrow_eb, FALSE, FALSE, 0 );

    arrow_right = gtk_arrow_new ( GTK_ARROW_RIGHT, GTK_SHADOW_OUT );
    arrow_eb = gtk_event_box_new ();
    gtk_widget_modify_bg ( arrow_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
    gtk_container_add ( GTK_CONTAINER ( arrow_eb ), arrow_right );
    g_signal_connect ( G_OBJECT ( arrow_eb ), "button-press-event", 
		       G_CALLBACK ( on_simpleclick_event_run ), 
		       gsb_gui_navigation_select_next );
    gtk_box_pack_start ( GTK_BOX(hbox), arrow_eb, FALSE, FALSE, 3 );

    /* Define labels. */
    headings_title = gtk_label_new ( NULL );
    g_signal_connect ( G_OBJECT (headings_title ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &headings_title );
    gtk_label_set_justify ( GTK_LABEL(headings_title), GTK_JUSTIFY_LEFT );
    gtk_misc_set_alignment (GTK_MISC (headings_title), 0.0, 0.5);
    gtk_box_pack_start ( GTK_BOX(hbox), headings_title, TRUE, TRUE, 3 );
    headings_suffix = gtk_label_new (NULL);
    g_signal_connect ( G_OBJECT (headings_suffix ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &headings_suffix );
    gtk_box_pack_start ( GTK_BOX(hbox), headings_suffix, FALSE, FALSE, 0 );

    /* Change color with an event box trick. */
    gtk_container_add ( GTK_CONTAINER ( headings_eb ), hbox );
    gtk_widget_modify_bg ( headings_eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );
    gtk_container_set_border_width ( GTK_CONTAINER ( hbox ), 6 );

    gtk_box_pack_start ( GTK_BOX(main_vbox), headings_eb, FALSE, FALSE, 0 );
    gsb_gui_update_show_headings ();

    /* Then create and fill the main hpaned. */
    main_hpaned = gtk_hpaned_new ();
    g_signal_connect ( G_OBJECT (main_hpaned ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &main_hpaned );
    gtk_box_pack_start ( GTK_BOX(main_vbox), main_hpaned, TRUE, TRUE, 0 );
    gtk_paned_add1 ( GTK_PANED( main_hpaned ), create_navigation_pane ( ) );
    gtk_paned_add2 ( GTK_PANED( main_hpaned ), create_main_notebook ( ) );
    gtk_container_set_border_width ( GTK_CONTAINER ( main_hpaned ), 6 );
    if ( etat.largeur_colonne_comptes_operation )
    {
	gtk_paned_set_position ( GTK_PANED ( main_hpaned ), etat.largeur_colonne_comptes_operation );
    }
    else
    {
	gint width, height;

	gtk_window_get_size ( GTK_WINDOW ( window ), &width, &height );
	gtk_paned_set_position ( GTK_PANED ( main_hpaned ), (gint) width / 4 );
    }
    gtk_widget_show ( main_hpaned );

    gtk_widget_show ( main_vbox );

    /* Blank the transaction list. */
    gsb_transactions_list_set_store (NULL);

    return main_vbox;
}



/**
 * Create the main notebook : 
 * a notebook wich contains the pages : main page, accounts, scheduler... and
 * the form on the bottom, the form will be showed only for accounts page and
 * scheduler page
 *
 * \return the notebook
 */
GtkWidget *create_main_notebook (void )
{
    GtkWidget * vbox, * form;

    devel_debug ( "create_main_notebook" );

    /* the main right page is a vbox with a notebook on the top
     * and the form on the bottom */

    vbox = gtk_vbox_new ( FALSE,
			  0);

    /* append the notebook */
    notebook_general = gtk_notebook_new();
    g_signal_connect ( G_OBJECT (notebook_general ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &notebook_general );
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(notebook_general), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(notebook_general), FALSE );
    gtk_box_pack_start ( GTK_BOX (vbox),
			 notebook_general,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show (notebook_general);

    /* append the form */
    form = gsb_form_new ();
    gtk_box_pack_start ( GTK_BOX (vbox),
			 form,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_hide (form);

    /* fill the notebook */
    gsb_gui_fill_main_notebook(notebook_general);

    gtk_widget_show (vbox);
    return (vbox);
}


/**
 * fill the notebook given in param
 *
 * \param notebook a notebook (usually the main_notebook...)
 *
 * \return FALSE
 * */
gboolean gsb_gui_fill_main_notebook ( GtkWidget *notebook )
{
    gsb_plugin * plugin;

    /* append the main page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       creation_onglet_accueil(),
			       gtk_label_new (SPACIFY(_("Main page"))) );

    /* append the account page : a notebook with the account configuration
     * and the transactions page */
    account_page = gtk_notebook_new ();
    g_signal_connect ( G_OBJECT (account_page ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &account_page );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(account_page), FALSE );
    gtk_widget_show ( account_page );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       account_page,
			       gtk_label_new (SPACIFY(_("Accounts"))) );

    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ),
			       creation_fenetre_operations (),
			       gtk_label_new (SPACIFY(_("Transactions"))) );
    gtk_notebook_append_page ( GTK_NOTEBOOK ( account_page ),
			       creation_onglet_comptes (),
			       gtk_label_new (SPACIFY(_("Properties"))) );
    g_signal_connect ( G_OBJECT (account_page),
		       "switch_page",
		       G_CALLBACK (gsb_gui_on_account_switch_page),
		       NULL );
    g_object_set_data ( G_OBJECT (notebook), "account_notebook", account_page );

    /* append the scheduled transactions page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       gsb_scheduler_list_create_list (),
			       gtk_label_new (SPACIFY(_("Scheduler"))) );

    /* append the payee page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       onglet_tiers(),
			       gtk_label_new (SPACIFY(_("Payee"))) );


    /* append the balance estimate page */
#ifdef ENABLE_BALANCE_ESTIMATE 
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       bet_create_balance_estimate_tab(),
			       gtk_label_new (SPACIFY(_("Balance estimate"))) );
#endif /* ENABLE_BALANCE_ESTIMATE */

    /* append the categories page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       onglet_categories(),
			       gtk_label_new (SPACIFY(_("Categories"))) );

    /* append the budget page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       onglet_imputations(),
			       gtk_label_new (SPACIFY(_("Budgetary lines"))) );

    /* append the reports page */
    gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
			       creation_onglet_etats (),
			       gtk_label_new (SPACIFY(_("Reports"))) );

    /* append the g2banking page */
#ifdef HAVE_PLUGINS
    plugin = gsb_plugin_find ( "g2banking" );
    if ( plugin )
    {
	gtk_notebook_append_page ( GTK_NOTEBOOK ( notebook ),
				   GTK_WIDGET ( plugin -> plugin_run ( ) ),
				   gtk_label_new (SPACIFY(_("Outbox")) ) );
    }

#endif

    return FALSE;
}





/**
 * called when the account notebook changed page between
 * transactions list and account description
 *
 * \param notebook	Widget that triggered event.
 * \param page		Not used.
 * \param page_number	Page set.
 * \param null		Not used.
 *
 * \return		FALSE
 */
gboolean gsb_gui_on_account_switch_page ( GtkNotebook *notebook,
					  GtkNotebookPage *page,
					  guint page_number,
					  gpointer null )
{
    switch ( page_number )
    {
	case 0:
	    gsb_form_set_expander_visible (TRUE,
					   TRUE );
	    break;

	case 1:
	    gsb_form_set_expander_visible (FALSE,
					   FALSE );
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
    gchar* tmpstr = g_strconcat ( "<b>", title, "</b>", NULL );
    gtk_label_set_markup ( GTK_LABEL(headings_title), tmpstr);
    g_free ( tmpstr );
    tmpstr = g_strconcat ( "<b>", suffix, "</b>", NULL );
    gtk_label_set_markup ( GTK_LABEL(headings_suffix), tmpstr);
    g_free ( tmpstr );
}

/**
 * Update headings bar with a new suffix.
 *
 * \param suffix	String to display as a suffix in headings bar.
 *
 */
void gsb_gui_headings_update_suffix ( gchar * suffix )
{
    gchar* tmpstr = g_strconcat ( "<b>", suffix, "</b>", NULL );
    gtk_label_set_markup ( GTK_LABEL(headings_suffix), tmpstr );
    g_free ( tmpstr );
}




/**
 * Display or hide the headings bar depending on configuration.
 *
 * return		FALSE
 */
gboolean gsb_gui_update_show_headings ()
{
    if ( etat.show_headings_bar )
    {
	gtk_widget_show_all ( headings_eb );
    }
    else
    {
	gtk_widget_hide_all ( headings_eb );
    }
    return FALSE;
}

/**
 * sensitive or unsensitive the headings
 *
 * \param sensitive TRUE to sensitive
 *
 * \return
 * */
void gsb_gui_sensitive_headings ( gboolean sensitive )
{
    if (headings_eb)
	gtk_widget_set_sensitive ( headings_eb,
				   sensitive );
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



/**
 * Trigger a callback functions only if button event that triggered it
 * was a simple click.
 *
 */
gboolean on_simpleclick_event_run ( GtkWidget * button, GdkEvent * button_event,
				    GCallback cb )
{
    if ( button_event -> type == GDK_BUTTON_PRESS )
    {
	cb ();
    }

    return TRUE;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
