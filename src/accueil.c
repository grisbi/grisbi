/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*            2003-2008 Benjamin Drieu (bdrieu@april.org)                     */
/*             http://www.grisbi.org                                          */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "accueil.h"
#include "classement_echeances.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_real.h"
#include "gsb_scheduler.h"
#include "gsb_scheduler_list.h"
#include "gsb_select_icon.h"
#include "gsb_transactions_list.h"
#include "main.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void affiche_solde_des_comptes ( GtkWidget *table,
                        gint i,
                        gint nb_comptes,
                        gint currency_number,
                        gsb_real solde_global_courant,
                        gsb_real solde_global_pointe );
static gint affiche_soldes_additionnels ( GtkWidget *table, gint i, GSList *liste );
static gint affiche_soldes_partiels ( GtkWidget *table,
                        gint i,
                        gint nb_comptes,
                        GSList *liste,
                        gint currency_number,
                        gint type_compte );
static gboolean gsb_config_scheduler_switch_balances_with_scheduled ( void );
static void gsb_main_page_affiche_ligne_du_compte ( GtkWidget *pTable,
                        gint account_number,
                        gint i );
static gboolean gsb_main_page_click_on_account ( gint *account_number );
static GtkStyle *gsb_main_page_get_default_label_style ( );
static gboolean gsb_main_page_get_devise_is_used ( gint currency_number, gint type_compte );
static GtkWidget *gsb_main_page_get_table_for_accounts ( gint nb_lignes, gint nb_col );
static gboolean saisie_echeance_accueil ( GtkWidget *event_box,
                        GdkEventButton *event,
                        gint scheduled_number );
static void update_fin_comptes_passifs ( gboolean force );
static void update_liste_comptes_accueil ( gboolean force );
static void update_liste_echeances_auto_accueil ( gboolean force );
static void update_liste_echeances_manuelles_accueil ( gboolean force );
static void update_soldes_minimaux ( gboolean force );
/*END_STATIC*/

/*START_EXTERN*/
extern GdkColor couleur_bleue;
extern GdkColor couleur_jaune;
extern GdkColor couleur_nom_compte_normal;
extern GdkColor couleur_nom_compte_prelight;
extern GdkColor couleur_solde_alarme_orange_normal;
extern GdkColor couleur_solde_alarme_orange_prelight;
extern GdkColor couleur_solde_alarme_rouge_normal;
extern GdkColor couleur_solde_alarme_rouge_prelight;
extern GdkColor couleur_solde_alarme_verte_normal;
extern GdkColor couleur_solde_alarme_verte_prelight;
extern GtkWidget *form_transaction_part;
extern gsb_real null_real;
extern GSList *scheduled_transactions_taken;
extern GSList *scheduled_transactions_to_take;
extern GtkWidget *window;
/*END_EXTERN*/

GtkWidget *logo_accueil = NULL;
GtkWidget *hbox_title = NULL;
GtkWidget *label_titre_fichier = NULL;
static GtkWidget *frame_etat_comptes_accueil = NULL;
static GtkWidget *frame_etat_fin_compte_passif = NULL;
static GtkWidget *frame_etat_echeances_manuelles_accueil = NULL;
static GtkWidget *frame_etat_echeances_auto_accueil = NULL;
static GtkWidget *main_page_finished_scheduled_transactions_part = NULL;
static GtkWidget *frame_etat_soldes_minimaux_autorises = NULL;
static GtkWidget *frame_etat_soldes_minimaux_voulus = NULL;
static GtkStyle *style_label;

#define show_paddingbox(child) gtk_widget_show_all (gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(child)))))
#define hide_paddingbox(child) gtk_widget_hide_all (gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(child)))))


/* ces 5 variables sont mises à 1 lorsqu'il est nécessaire de rafraichir cette */
/* partie la prochaine fois qu'on va sur l'accueil */

gint mise_a_jour_liste_comptes_accueil;
gint mise_a_jour_liste_echeances_manuelles_accueil;
gint mise_a_jour_liste_echeances_auto_accueil;
gint mise_a_jour_soldes_minimaux;
gint mise_a_jour_fin_comptes_passifs;

GtkSizeGroup * size_group_accueil;
gchar *chaine_espace = "                         ";

/**
 * Create the home page of Grisbi
 *
 *
 *
 * */
GtkWidget *creation_onglet_accueil ( void )
{
    GtkWidget *paddingbox, *base, *base_scroll;
    GtkWidget * eb;
    GtkStyle * style;

    devel_debug ( NULL );

    /* on crée à ce niveau base_scroll qui est aussi une vbox mais qui peut
       scroller verticalement */

    base_scroll = gtk_scrolled_window_new ( NULL, NULL);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( base_scroll ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( base_scroll ),
					  GTK_SHADOW_NONE );

    base = gtk_vbox_new ( FALSE, 15 );
    gtk_container_set_border_width ( GTK_CONTAINER ( base ), 12 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( base_scroll ), base );
    gtk_widget_show ( base_scroll );
    gtk_widget_show ( base );

    /* en dessous, on met le titre du fichier */
    hbox_title = gtk_hbox_new ( FALSE, 0 );

    eb = gtk_event_box_new ();
    style = gtk_widget_get_style ( eb );
    gtk_widget_modify_bg ( eb, 0, &(style -> bg[GTK_STATE_ACTIVE]) );

    label_titre_fichier = gtk_label_new ( NULL );

    if ( etat.utilise_logo )
    {
        logo_accueil =  gtk_image_new_from_pixbuf ( gsb_select_icon_get_logo_pixbuf ( ) );

        gtk_box_pack_start ( GTK_BOX ( hbox_title ), logo_accueil, FALSE, FALSE, 20 );
        gtk_widget_set_size_request ( hbox_title, -1, LOGO_HEIGHT + 20 );
    }

    gtk_box_pack_end ( GTK_BOX ( hbox_title ), label_titre_fichier, TRUE, TRUE, 20 );
    gtk_container_set_border_width ( GTK_CONTAINER ( hbox_title ), 6 );
    gtk_container_add ( GTK_CONTAINER ( eb ), hbox_title );
    gtk_box_pack_start ( GTK_BOX ( base ), eb, FALSE, FALSE, 0 );
    gtk_widget_show_all ( eb );

    /* on crée le size_group pour l'alignement des tableaux */
    size_group_accueil = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    /* on crée la première frame dans laquelle on met les états des comptes */
    frame_etat_comptes_accueil = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_comptes_accueil), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_comptes_accueil), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_comptes_accueil), 0 );
    gtk_box_pack_start ( GTK_BOX(base), frame_etat_comptes_accueil, FALSE, FALSE, 0 );

    /* on met la liste des comptes et leur état dans la frame */
    mise_a_jour_liste_comptes_accueil = 1;
    gtk_widget_show_all ( frame_etat_comptes_accueil );


    /* mise en place de la partie fin des comptes passif */
    paddingbox = new_paddingbox_with_title ( base, FALSE,
                         _("Closed liabilities accounts") );
    frame_etat_fin_compte_passif = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_fin_compte_passif), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_fin_compte_passif), FALSE );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_fin_compte_passif, FALSE, FALSE, 0 );
    mise_a_jour_fin_comptes_passifs = 1;


    /* mise en place de la partie des échéances manuelles ( non affiché ) */
    /*     sera mis à jour automatiquement si nécessaire */
    paddingbox = new_paddingbox_with_title ( base, FALSE,
					     _("Run out manual scheduled transactions") );
    frame_etat_echeances_manuelles_accueil = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_echeances_manuelles_accueil),
				 FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_echeances_manuelles_accueil),
				   FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_echeances_manuelles_accueil),
				     0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_echeances_manuelles_accueil,
			 FALSE, FALSE, 6 );


    /* mise en place de la partie des échéances auto  ( non affiché )*/
    /*     sera mis à jour automatiquement si nécessaire */
    paddingbox = new_paddingbox_with_title ( base, FALSE,
					     _("Automatic scheduled transactions entered") );
    frame_etat_echeances_auto_accueil = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_echeances_auto_accueil), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_echeances_auto_accueil), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_echeances_auto_accueil), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_echeances_auto_accueil, FALSE, FALSE, 6 );


    /* partie des fin d'échéances */
    paddingbox = new_paddingbox_with_title ( base, FALSE,
					     _("Closed scheduled transactions") );
    main_page_finished_scheduled_transactions_part = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(main_page_finished_scheduled_transactions_part), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(main_page_finished_scheduled_transactions_part), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(main_page_finished_scheduled_transactions_part), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), main_page_finished_scheduled_transactions_part, FALSE, FALSE, 6 );


    /* partie des soldes minimaux autorisés */
    paddingbox = new_paddingbox_with_title ( base, FALSE,
					     _("Accounts under authorized balance") );
    frame_etat_soldes_minimaux_autorises = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_autorises), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_autorises), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_soldes_minimaux_autorises), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_soldes_minimaux_autorises, FALSE, FALSE, 6 );


    /* partie des soldes minimaux voulus */
    paddingbox = new_paddingbox_with_title ( base, FALSE,
					     _("Accounts under desired balance") );
    frame_etat_soldes_minimaux_voulus = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_voulus), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_voulus), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_soldes_minimaux_voulus), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_soldes_minimaux_voulus, FALSE, FALSE, 6 );

    mise_a_jour_soldes_minimaux = 1;

    return ( base_scroll );
}


/**
 * update the first page, force the updating if asked,
 * else, each function will decide if it need to be
 * updated or not
 *
 * \param force TRUE if we want to update all
 *
 * \return
 * */
void mise_a_jour_accueil ( gboolean force )
{
    update_liste_comptes_accueil (force);
    update_liste_echeances_manuelles_accueil (force);
    update_liste_echeances_auto_accueil (force);
    update_soldes_minimaux (force);
    update_fin_comptes_passifs (force);
}


/* ************************************************************************* */
/*        Fonction appelée lorsqu'on clicke sur une échéance à saisir        */
/* ************************************************************************* */
gboolean saisie_echeance_accueil ( GtkWidget *event_box,
                        GdkEventButton *event,
                        gint scheduled_number )
{
    GtkWidget *parent_save, *dialog, *hbox;
    GtkWidget *button;
    gint result;

    parent_save = form_transaction_part -> parent;

    /* Create the dialog */
    dialog = gtk_dialog_new_with_buttons ( _("Enter a scheduled transaction"),
					   GTK_WINDOW ( window ),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );

    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER_ON_PARENT );
    gtk_widget_set_size_request ( dialog, 700, -1 );
    gtk_window_set_resizable ( GTK_WINDOW ( dialog ), TRUE );
    gtk_dialog_set_default_response ( GTK_DIALOG (dialog), GTK_RESPONSE_OK );

	/* first we reparent the form in the dialog */
	hbox = gtk_hbox_new ( FALSE, 0 );
	gtk_box_pack_start ( GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox, TRUE, TRUE, 0 );
	gtk_container_set_border_width ( GTK_CONTAINER(hbox), 12 );
	gtk_widget_reparent ( form_transaction_part, hbox );
    gtk_widget_show_all ( hbox );

    /* next we fill the form,
     * don't use gsb_form_show because we are neither on transactions list, neither scheduled list */
    button = gsb_form_scheduler_get_element_widget( SCHEDULED_FORM_ACCOUNT );
    g_signal_handlers_block_by_func ( G_OBJECT ( button ),
                        G_CALLBACK (gsb_form_scheduler_change_account),
                        NULL );

    gsb_form_fill_from_account (gsb_data_scheduled_get_account_number (scheduled_number));

    /* fill the form with the scheduled transaction */
    gsb_scheduler_list_execute_transaction(scheduled_number);

    g_signal_handlers_unblock_by_func ( G_OBJECT ( button ),
                        G_CALLBACK (gsb_form_scheduler_change_account),
                        NULL );

	result = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( result == GTK_RESPONSE_OK )
	 gsb_form_finish_edition ();

    gtk_widget_reparent ( form_transaction_part, parent_save );
    gtk_widget_destroy ( dialog );

    /* update the home page */
    update_liste_echeances_manuelles_accueil (TRUE);
    update_liste_comptes_accueil (TRUE);
    return FALSE;
}
/* ************************************************************************* */

/* ************************************************************************* */
/* Fonction update_liste_comptes_accueil                                     */
/* affiche la liste des comptes et leur solde courant dans la frame qui leur */
/* est réservée dans l'accueil                                               */
/* ************************************************************************* */
void update_liste_comptes_accueil ( gboolean force )
{
    GtkWidget *pTable, *vbox, *paddingbox;
    GList *children;
    GSList *devise;
    GSList *list_tmp;
    gsb_real solde_global_courant, solde_global_pointe;
    gchar* tmpstr;
    gint i = 0;
    gint nb_comptes_bancaires=0, nb_comptes_passif=0, nb_comptes_actif=0;
    gint new_comptes_bancaires=0, new_comptes_passif=0, new_comptes_actif=0;
    gint soldes_mixtes = 0;

    if ( !force
	 &&
	 !( mise_a_jour_liste_comptes_accueil
	   &&
	   gsb_data_account_get_accounts_amount ( ) ) )
	return;

    mise_a_jour_liste_comptes_accueil = 0;

    /* Remove previous child */
    children = gtk_container_get_children(GTK_CONTAINER(frame_etat_comptes_accueil));
    if ( children && children -> data)
        gtk_container_remove ( GTK_CONTAINER(frame_etat_comptes_accueil),
                       GTK_WIDGET(children -> data) );

    /* Create the handle vbox  */
    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_add ( GTK_CONTAINER ( frame_etat_comptes_accueil ), vbox );

    /* Préparation de la séparation de l'affichage des comptes en fonction
       de leur type */

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
        gint i;

        i = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( !gsb_data_account_get_closed_account (i) )
        {
            if ( gsb_data_account_get_kind (i) == GSB_TYPE_ASSET )
            {
                nb_comptes_actif++;
            }
            else if ( gsb_data_account_get_kind (i) == GSB_TYPE_LIABILITIES )
            {
                nb_comptes_passif++;
            }
            else
            {
                nb_comptes_bancaires++;
            }
        }
        list_tmp = list_tmp -> next;
    }
    
    list_tmp = gsb_data_partial_balance_get_list ( );
    if ( list_tmp )
    {
        while ( list_tmp )
        {
            kind_account i;

            i = gsb_data_partial_balance_get_number ( list_tmp -> data );

            if ( gsb_data_partial_balance_get_kind ( i ) == GSB_TYPE_ASSET )
            {
                new_comptes_actif++;
            }
            else if ( gsb_data_partial_balance_get_kind ( i ) == GSB_TYPE_LIABILITIES )
            {
                new_comptes_passif++;
            }
            else if ( gsb_data_partial_balance_get_kind ( i ) == -1 )
            {
                soldes_mixtes++;
            }
            else
            {
                new_comptes_bancaires++;
            }
            list_tmp = list_tmp -> next;
        }
   }

    /* Affichage des comptes bancaires et de caisse */
    for ( devise = gsb_data_currency_get_currency_list (); devise ; devise = devise->next )
    {
        gint currency_number;

        currency_number = gsb_data_currency_get_no_currency (devise -> data);

        if ( !gsb_main_page_get_devise_is_used ( currency_number, GSB_TYPE_BANK )
            && 
            !gsb_main_page_get_devise_is_used ( currency_number, GSB_TYPE_CASH ) )
            continue;


        /* Creating the table which will store accounts with their balances. */
        tmpstr = g_strdup_printf ( _("Account balances in %s"),
                        gsb_data_currency_get_name ( currency_number ) );
        if ( conf.balances_with_scheduled == FALSE )
            tmpstr = g_strconcat ( tmpstr, _(" at "), gsb_date_today (), NULL );

        paddingbox = new_paddingbox_with_title ( vbox, FALSE, tmpstr );
        g_free ( tmpstr );

        pTable = gsb_main_page_get_table_for_accounts (
                        nb_comptes_bancaires + 3 + new_comptes_bancaires + 2, 3 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), pTable, FALSE, FALSE, 0 );

        /* Affichage des comptes et de leur solde */
        i = 1;
        solde_global_courant = null_real ;
        solde_global_pointe = null_real;

        /* Pour chaque compte non cloturé (pour chaque ligne), */
        /* créer toutes les colonnes et les remplir            */

        list_tmp = gsb_data_account_get_list_accounts ();

        while ( list_tmp )
        {
            gint account_number;

            account_number = gsb_data_account_get_no_account ( list_tmp -> data );

            if ( !gsb_data_account_get_closed_account (account_number) &&
             gsb_data_account_get_currency (account_number) == currency_number
             && gsb_data_account_get_kind (account_number) < GSB_TYPE_LIABILITIES )
            {
                /* on affiche la ligne du compte avec les soldes pointé et courant */
                gsb_main_page_affiche_ligne_du_compte  ( pTable, account_number, i );

                /* ATTENTION : les sommes effectuées ici présupposent que
                   TOUS les comptes sont dans la MÊME DEVISE !!!!!        */
                solde_global_courant = gsb_real_add ( solde_global_courant,
                                      gsb_data_account_get_current_balance (account_number));
                solde_global_pointe = gsb_real_add ( solde_global_pointe,
                                     gsb_data_account_get_marked_balance (account_number));
            }
            i++;
            list_tmp = list_tmp -> next;
        }

        /* affichage des soldes partiels s'ils existent */
        if ( new_comptes_bancaires > 0 )
        {
            list_tmp = gsb_data_partial_balance_get_list ( );
            if ( list_tmp )
                i += affiche_soldes_partiels ( pTable, i, new_comptes_bancaires, list_tmp,
                        currency_number, GSB_TYPE_BANK | GSB_TYPE_CASH );
        }

        /* Création et remplissage de la (nb_comptes + 3)ième ligne du tableau :
           elle contient la somme des soldes de chaque compte */
        affiche_solde_des_comptes ( pTable, i, nb_comptes_bancaires, currency_number, 
                            solde_global_courant, solde_global_pointe );

        gtk_widget_show_all ( paddingbox );
        gtk_widget_show_all ( pTable );
    }


    /* Affichage des comptes de passif */
    for ( devise = gsb_data_currency_get_currency_list (); devise ; devise = devise->next )
    {
        GSList *list_tmp;
        gint currency_number;
        gchar* tmpstr;

        currency_number = gsb_data_currency_get_no_currency (devise -> data);

        if ( !gsb_main_page_get_devise_is_used ( currency_number, GSB_TYPE_LIABILITIES ) )
            continue;

        /* Creating the table which will store accounts with their balances   */
		tmpstr = g_strdup_printf (_("Liabilities accounts balances in %s"),
                         gsb_data_currency_get_name (currency_number) );
        if ( conf.balances_with_scheduled == FALSE )
            tmpstr = g_strconcat ( tmpstr, _(" at "), gsb_date_today (), NULL );

        paddingbox = new_paddingbox_with_title ( vbox, FALSE, tmpstr );
        g_free ( tmpstr );

        pTable = gsb_main_page_get_table_for_accounts (
                        nb_comptes_passif + 3 + new_comptes_passif + 2, 3 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), pTable, FALSE, FALSE, 0 );

        /* Affichage des comptes et de leur solde */
        i = 1;
        solde_global_courant = null_real;
        solde_global_pointe = null_real;

        /* Pour chaque compte non cloturé (pour chaque ligne), */
        /* créer toutes les colonnes et les remplir            */
        list_tmp = gsb_data_account_get_list_accounts ();

        while ( list_tmp )
        {
            gint account_number;

            account_number = gsb_data_account_get_no_account ( list_tmp -> data );
            if ( !gsb_data_account_get_closed_account (account_number) &&
             gsb_data_account_get_currency (account_number) == currency_number
             &&
             gsb_data_account_get_kind (account_number) == GSB_TYPE_LIABILITIES )
            {
                /* on affiche la ligne du compte avec les soldes pointé et courant */
                gsb_main_page_affiche_ligne_du_compte  ( pTable, account_number, i );

                /* ATTENTION : les sommes effectuées ici présupposent que
                   TOUS les comptes sont dans la MÊME DEVISE !!!!!        */
                solde_global_courant = gsb_real_add ( solde_global_courant,
                                      gsb_data_account_get_current_balance (account_number));
                solde_global_pointe = gsb_real_add ( solde_global_pointe,
                                     gsb_data_account_get_marked_balance (account_number));
            }
            i++;
            list_tmp = list_tmp -> next;
        }

        /* affichage des soldes partiels s'ils existent */
        if ( new_comptes_passif > 0 )
        {
            list_tmp = gsb_data_partial_balance_get_list ( );
            if ( list_tmp )
                i += affiche_soldes_partiels ( pTable, i, new_comptes_passif, list_tmp,
                        currency_number, GSB_TYPE_LIABILITIES );
        }

        /* Création et remplissage de la (nb_comptes + 3)ième ligne du tableau :
           elle contient la somme des soldes de chaque compte */
        affiche_solde_des_comptes ( pTable, i, nb_comptes_passif, currency_number, 
                            solde_global_courant, solde_global_pointe );

        gtk_widget_show_all ( paddingbox );
        gtk_widget_show_all ( pTable );
    }


    /* Affichage des comptes d'actif */
    for ( devise = gsb_data_currency_get_currency_list (); devise ; devise = devise->next )
    {
        GSList *list_tmp;
        gint currency_number;
		gchar* tmpstr;

        currency_number = gsb_data_currency_get_no_currency (devise -> data);

       if ( !gsb_main_page_get_devise_is_used ( currency_number, GSB_TYPE_ASSET ) )
            continue;

        /* Creating the table which will store accounts with their balances    */
        tmpstr = g_strdup_printf (_("Assets accounts balances in %s"),
                         gsb_data_currency_get_name (currency_number));
        if ( conf.balances_with_scheduled == FALSE )
            tmpstr = g_strconcat ( tmpstr, _(" at "), gsb_date_today (), NULL );

        paddingbox = new_paddingbox_with_title ( vbox, FALSE, tmpstr );
        g_free ( tmpstr );

        pTable = gsb_main_page_get_table_for_accounts (
                        nb_comptes_actif + 3 + new_comptes_passif  + 2, 3 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), pTable, FALSE, FALSE, 0 );

        /* Affichage des comptes et de leur solde */
        i = 1;
        solde_global_courant = null_real;
        solde_global_pointe = null_real;

        /* Pour chaque compte non cloturé (pour chaque ligne), */
        /* créer toutes les colonnes et les remplir            */

        list_tmp = gsb_data_account_get_list_accounts ();

        while ( list_tmp )
        {
            gint account_number;

            account_number = gsb_data_account_get_no_account ( list_tmp -> data );

            if ( !gsb_data_account_get_closed_account (account_number) &&
             gsb_data_account_get_currency (account_number) == currency_number
             &&
             gsb_data_account_get_kind (account_number) == GSB_TYPE_ASSET )
            {
                /* on affiche la ligne du compte avec les soldes pointé et courant */
                gsb_main_page_affiche_ligne_du_compte ( pTable, account_number, i );

                /* ATTENTION : les sommes effectuées ici présupposent que
                   TOUS les comptes sont dans la MÊME DEVISE !!!!!        */
                solde_global_courant = gsb_real_add ( solde_global_courant,
                                      gsb_data_account_get_current_balance (account_number));
                solde_global_pointe = gsb_real_add ( solde_global_pointe,
                                     gsb_data_account_get_marked_balance (account_number));
            }
            i++;
            list_tmp = list_tmp -> next;
        }

        /* affichage des soldes partiels s'ils existent */
        if ( new_comptes_actif > 0 )
        {
            list_tmp = gsb_data_partial_balance_get_list ( );
            if ( list_tmp )
                i += affiche_soldes_partiels ( pTable, i, new_comptes_actif, list_tmp,
                        currency_number, GSB_TYPE_ASSET );
        }

        /* Création et remplissage de la (nb_comptes + 3)ième ligne du tableau :
           elle contient la somme des soldes de chaque compte */
        affiche_solde_des_comptes ( pTable, i, nb_comptes_actif, currency_number, 
                            solde_global_courant, solde_global_pointe );
        gtk_widget_show_all ( paddingbox );
        gtk_widget_show_all ( pTable );
    }

    /* Affichage des soldes mixtes */
    if ( soldes_mixtes > 0 )
    {
        if ( soldes_mixtes == 1 )
            tmpstr = g_strdup ( _("Additional balance") );
        else
            tmpstr = g_strdup ( _("Additional balances") );
        if ( conf.balances_with_scheduled == FALSE )
            tmpstr = g_strconcat ( tmpstr, _(" at "), gsb_date_today (), NULL );

        paddingbox = new_paddingbox_with_title ( vbox, FALSE, tmpstr );
        g_free ( tmpstr );

        pTable = gsb_main_page_get_table_for_accounts ( (3 * soldes_mixtes) , 3 );
        gtk_box_pack_start ( GTK_BOX ( paddingbox ), pTable, FALSE, FALSE, 0 );

        list_tmp = gsb_data_partial_balance_get_list ( );

        if ( list_tmp )
            i += affiche_soldes_additionnels ( pTable, i, list_tmp );
    }
    gtk_widget_show_all (vbox);
}


/**
 * teste si au moins 1 compte utilise la devise passée en paramètre.
 *
 * \param currency_number
 * 
 * \return TRUE si un compte utilise la devise FALSE sinon;
 * */
gboolean gsb_main_page_get_devise_is_used ( gint currency_number, gint type_compte )
{
    GSList *list_tmp;

    list_tmp = gsb_data_account_get_list_accounts ( );

    while ( list_tmp )
    {
        gint i;

        i = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( gsb_data_account_get_currency ( i ) == currency_number
         &&
         !gsb_data_account_get_closed_account ( i ) 
         &&
         gsb_data_account_get_kind ( i ) == type_compte )
        return TRUE;

        list_tmp = list_tmp -> next;
    }

    return FALSE;
}


/**
 * 
 * 
 * \return style_label
 * */
GtkStyle *gsb_main_page_get_default_label_style ( )
{
    GtkWidget *label;
    GtkStyle * style_label;

    /* Création d'un label juste pour en récupérer le style */
    label = gtk_label_new (NULL);

    /* Initialisation du style « Nom du compte » */
    style_label = gtk_style_copy ( gtk_widget_get_style ( label ) );
    style_label -> fg[GTK_STATE_NORMAL] = couleur_nom_compte_normal;
    style_label ->fg[GTK_STATE_PRELIGHT] = couleur_nom_compte_prelight;
    gtk_widget_destroy ( label );

    return style_label;
}


/**
 * Crée la table et sa première ligne
 * 
 * \return table
 * */
GtkWidget *gsb_main_page_get_table_for_accounts ( gint nb_lignes, gint nb_col )
{
    GtkWidget *table, *label;

	table = gtk_table_new ( nb_lignes, nb_col, FALSE );

	/* Création et remplissage de la première ligne du tableau */
    label = gtk_label_new ( chaine_espace );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, 0, 1 );
	gtk_widget_show ( label );
	label = gtk_label_new (_("Reconciled balance"));
	gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 1, 2, 0, 1 );
	gtk_widget_show ( label );
	label = gtk_label_new (_("Current balance"));
	gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 2, 4, 0, 1 );
	gtk_widget_show ( label );

    return table;

}
/**
 * Affiche une ligne pour le compte donné en paramètre
 * 
 * \param table
 * \param account number
 * \param ligne dans la table
 *
 * \return FALSE
 * */
void gsb_main_page_affiche_ligne_du_compte ( GtkWidget *pTable,
                        gint account_number,
                        gint i )
{
    GtkWidget *pEventBox, *pLabel;
    GtkStyle *pStyleLabelNomCompte, *pStyleLabelSoldeCourant,
	     *pStyleLabelSoldePointe;
    GSList *list = NULL;
	gchar* tmpstr;

    /* Initialisation du style « Nom du compte » */
    pStyleLabelNomCompte = gsb_main_page_get_default_label_style ( );

    /* Première colonne : elle contient le nom du compte */
    tmpstr = g_strconcat ( gsb_data_account_get_name (account_number), " : ", NULL );
    pLabel = gtk_label_new ( tmpstr );
    g_free ( tmpstr );
    gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), pLabel );
    gtk_widget_set_style ( pLabel, pStyleLabelNomCompte );

    /* Création d'une boite à évènement qui sera rattachée au nom du compte */
    pEventBox = gtk_event_box_new ();
    list = g_slist_append ( list, pEventBox );
    g_signal_connect ( G_OBJECT ( pEventBox ),
                 "enter-notify-event",
                 G_CALLBACK ( met_en_prelight ),
                 list );
    g_signal_connect ( G_OBJECT ( pEventBox ),
                 "leave-notify-event",
                 G_CALLBACK ( met_en_normal ),
                 list );
    g_signal_connect_swapped ( G_OBJECT ( pEventBox ),
                    "button-press-event",
                    G_CALLBACK ( gsb_main_page_click_on_account ),
                    GINT_TO_POINTER (account_number) );
    gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pEventBox,
                    0, 1, i, i+1 );
    gtk_widget_show ( pEventBox );
    gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
    gtk_widget_show ( pLabel );

    /* Deuxième colonne : elle contient le solde pointé du compte */
    tmpstr = gsb_real_get_string_with_currency (
            gsb_data_account_get_marked_balance ( account_number ),
            gsb_data_account_get_currency (account_number), TRUE);
    pLabel = gtk_label_new ( tmpstr );
    g_free ( tmpstr );
    gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

    /* Mise en place du style du label en fonction du solde pointé */
    pStyleLabelSoldePointe = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
    if ( gsb_real_cmp ( gsb_data_account_get_marked_balance (account_number),
                gsb_data_account_get_mini_balance_wanted (account_number)) != -1)
    {
        pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = couleur_solde_alarme_verte_normal;
        pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = couleur_solde_alarme_verte_prelight;
    }
    else
    {
        if ( gsb_real_cmp ( gsb_data_account_get_marked_balance (account_number),
                gsb_data_account_get_mini_balance_authorized (account_number)) != -1 )
        {
        pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = couleur_solde_alarme_orange_normal;
        pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = couleur_solde_alarme_orange_prelight;
        }
        else
        {
        pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = couleur_solde_alarme_rouge_normal;
        pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = couleur_solde_alarme_rouge_prelight;
        }
    }
    gtk_widget_set_style ( pLabel, pStyleLabelSoldePointe );

    /* Création d'une boite à évènement qui sera rattachée au solde pointé du compte */
    pEventBox = gtk_event_box_new ();
    list = g_slist_append ( list, pEventBox );
    g_signal_connect ( G_OBJECT ( pEventBox ),
                 "enter-notify-event",
                 G_CALLBACK ( met_en_prelight ),
                 list );
    g_signal_connect ( G_OBJECT ( pEventBox ),
                 "leave-notify-event",
                 G_CALLBACK ( met_en_normal ),
                 list );
    g_signal_connect_swapped ( G_OBJECT ( pEventBox ),
                    "button-press-event",
                    G_CALLBACK ( gsb_main_page_click_on_account ),
                    GINT_TO_POINTER (account_number) );
    gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pEventBox,
                    1, 2, i, i+1 );
    gtk_widget_show ( pEventBox );
    gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
    gtk_widget_show ( pLabel );

    /* Troisième colonne : elle contient le solde courant du compte */
    tmpstr = gsb_real_get_string_with_currency (
            gsb_data_account_get_current_balance (account_number),
            gsb_data_account_get_currency (account_number), TRUE);
    pLabel = gtk_label_new ( tmpstr );
    g_free ( tmpstr );
    gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

    /* Mise en place du style du label en fonction du solde courant */
    pStyleLabelSoldeCourant = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
    if ( gsb_real_cmp ( gsb_data_account_get_current_balance (account_number),
                gsb_data_account_get_mini_balance_wanted (account_number)) != -1)
    {
        pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = couleur_solde_alarme_verte_normal;
        pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = couleur_solde_alarme_verte_prelight;
    }
    else
    {
        if ( gsb_real_cmp ( gsb_data_account_get_current_balance (account_number),
                gsb_data_account_get_mini_balance_authorized (account_number)) != -1 )
        {
        pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = couleur_solde_alarme_orange_normal;
        pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = couleur_solde_alarme_orange_prelight;
        }
        else
        {
        pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = couleur_solde_alarme_rouge_normal;
        pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = couleur_solde_alarme_rouge_prelight;
        }
    }
    gtk_widget_set_style ( pLabel, pStyleLabelSoldeCourant );

    /* Création d'une boite à évènement qui sera rattachée au solde courant du compte */
    pEventBox = gtk_event_box_new ();
    list = g_slist_append ( list, pEventBox );
    g_signal_connect ( G_OBJECT ( pEventBox ),
                 "enter-notify-event",
                 G_CALLBACK ( met_en_prelight ),
                 list );
    g_signal_connect ( G_OBJECT ( pEventBox ),
                 "leave-notify-event",
                 G_CALLBACK ( met_en_normal ),
                 list );
    g_signal_connect_swapped ( G_OBJECT ( pEventBox ),
                    "button-press-event",
                    G_CALLBACK (gsb_main_page_click_on_account),
                    GINT_TO_POINTER (account_number) );
    gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pEventBox,
                    2, 3, i, i+1 );
    gtk_widget_show ( pEventBox );
    gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
    gtk_widget_show ( pLabel );
}


/**
 * Création de ou des lignes de solde partiels 
 *
 * */
gint affiche_soldes_partiels ( GtkWidget *table,
                        gint i,
                        gint nb_comptes,
                        GSList *liste,
                        gint currency_number,
                        gint type_compte )
{
    GtkWidget *label;
    gchar *tmpstr;
    gint nbre_lignes = 0;
    gboolean concerne = FALSE;

    while ( liste )
    {
        gint partial_number;
        kind_account kind;

        partial_number = gsb_data_partial_balance_get_number ( liste -> data );
        kind = gsb_data_partial_balance_get_kind ( partial_number );

        if ( ( kind == type_compte
         || ( kind < GSB_TYPE_LIABILITIES && type_compte < GSB_TYPE_LIABILITIES ) )
         &&
         gsb_data_partial_balance_get_currency ( partial_number ) == currency_number )
        {
            if ( concerne == FALSE )
            {
                    /* on commence par une ligne vide */
                label = gtk_label_new ( chaine_espace );
                gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
                gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
                gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, i, i+1 );
                gtk_widget_show ( label );
                i ++;
                nbre_lignes ++;

                /* On met les titres du sous ensemble solde(s) partiel(s) */
                if ( nb_comptes == 1 )
                    label = gtk_label_new ( _("Partial balance: ") );
                else
                    label = gtk_label_new ( _("Partial balances: ") );
                gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
                gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
                gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, i, i+1 );
                gtk_widget_show ( label );
                    label = gtk_label_new (_("Reconciled balance"));
                gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
                gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 1, 2, i, i+1 );
                gtk_widget_show ( label );
                label = gtk_label_new (_("Current balance"));
                gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
                gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 2, 4, i, i+1 );
                gtk_widget_show ( label );
                i ++;
                nbre_lignes ++;
                concerne = TRUE;
            }
            /* Première colonne : elle contient le nom du solde partiel */
            tmpstr = g_strconcat ( gsb_data_partial_balance_get_name ( partial_number ),
                            " : ", NULL );
            label = gtk_label_new ( tmpstr );
            g_free ( tmpstr );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
            gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
            gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, i, i+1 );
            gtk_widget_show ( label );

            /* Deuxième colonne : elle contient le solde pointé du solde partiel */
            tmpstr = gsb_data_partial_balance_get_marked_balance ( partial_number );
            label = gtk_label_new ( tmpstr );
            gtk_label_set_markup ( GTK_LABEL ( label ), tmpstr );
            g_free ( tmpstr );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
            gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 1, 2, i, i+1 );
            gtk_widget_show ( label );

            /* Troisième colonne : elle contient le solde courant du solde partiel */
            tmpstr = gsb_data_partial_balance_get_current_balance ( partial_number );
            label = gtk_label_new ( NULL );
            gtk_label_set_markup ( GTK_LABEL ( label ), tmpstr );
            g_free ( tmpstr );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
            gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 2, 3, i, i+1 );
            gtk_widget_show ( label );

            i++;
            nbre_lignes ++;
        }
        liste = liste -> next;
    }

    return nbre_lignes;
}


/**
 * Création de la ligne de solde des comptes 
 *
 * */
void affiche_solde_des_comptes ( GtkWidget *table,
                        gint i,
                        gint nb_comptes,
                        gint currency_number,
                        gsb_real solde_global_courant,
                        gsb_real solde_global_pointe )
{
    GtkWidget *label;
    gchar *tmpstr;

    /* on commence par une ligne vide */
    label = gtk_label_new ( chaine_espace );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, i, i+1 );
	gtk_widget_show ( label );
    i ++;

	/* Première colonne */
    if ( nb_comptes == 1 )
        label = gtk_label_new ( _("Global balance: ") );
    else if ( conf.pluriel_final )
        label = gtk_label_new ( ("Soldes finaux: ") );
    else
        label = gtk_label_new ( _("Global balances: ") );
	gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
	gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, i, i+1 );
	gtk_widget_show ( label );

	/* Deuxième colonne : elle contient le solde total pointé des comptes */
	tmpstr = gsb_real_get_string_with_currency (solde_global_pointe,
								    currency_number, TRUE);
	label = gtk_label_new ( tmpstr );
	g_free ( tmpstr );
	gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 1, 2, i, i+1 );
	gtk_widget_show ( label );

	/* Troisième colonne : elle contient le solde total courant des comptes */
	tmpstr = gsb_real_get_string_with_currency (solde_global_courant, currency_number, TRUE);
	label = gtk_label_new ( tmpstr );
	g_free ( tmpstr );
	gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 2, 3, i, i+1 );
	gtk_widget_show ( label );
}

/**
 * Création de ou des lignes de solde additionnel
 *
 * */
gint affiche_soldes_additionnels ( GtkWidget *table, gint i, GSList *liste )
{
    GtkWidget *label;
    gchar *tmpstr;
    gint nbre_lignes = 0;
    gint currency_number;

    /* on commence par une ligne vide */
    label = gtk_label_new ( chaine_espace );
    gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
    gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, i, i+1 );
    gtk_widget_show ( label );
    i ++;
    nbre_lignes ++;

    while ( liste )
    {
        gint partial_number;
        kind_account kind;

        partial_number = gsb_data_partial_balance_get_number ( liste -> data );
        kind = gsb_data_partial_balance_get_kind ( partial_number );

        if ( kind == - 1 )
        {
            /* Première colonne : elle contient le nom du solde partiel */
            currency_number = gsb_data_partial_balance_get_currency ( partial_number );
            tmpstr = g_strdup_printf (_(" in %s"), gsb_data_currency_get_name (
                        currency_number ) );
            tmpstr = g_strconcat ( gsb_data_partial_balance_get_name ( partial_number ),
                            tmpstr,
                            " : ", NULL );
            label = gtk_label_new ( tmpstr );
            g_free ( tmpstr );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
            gtk_size_group_add_widget ( GTK_SIZE_GROUP ( size_group_accueil ), label );
            gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 0, 1, i, i+1 );
            gtk_widget_show ( label );

            /* Deuxième colonne : elle contient le solde pointé du solde partiel */
            tmpstr = gsb_data_partial_balance_get_marked_balance ( partial_number );
            label = gtk_label_new ( tmpstr );
            gtk_label_set_markup ( GTK_LABEL ( label ), tmpstr );
            g_free ( tmpstr );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
            gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 1, 2, i, i+1 );
            gtk_widget_show ( label );

            /* Troisième colonne : elle contient le solde courant du solde partiel */
            tmpstr = gsb_data_partial_balance_get_current_balance ( partial_number );
            label = gtk_label_new ( NULL );
            gtk_label_set_markup ( GTK_LABEL ( label ), tmpstr );
            g_free ( tmpstr );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
            gtk_table_attach_defaults ( GTK_TABLE ( table ), label, 2, 3, i, i+1 );
            gtk_widget_show ( label );

            i++;
            nbre_lignes ++;
        }
        liste = liste -> next;
    }

    return nbre_lignes;
}

/**
 * called by a click on an account name or balance of accounts on the main page
 * it's just a mediator between the signal and gsb_navigation_show_account
 *
 * \param account_number a pointer wich is the number of account we want to switch
 *
 * \return FALSE
 * */
gboolean gsb_main_page_click_on_account ( gint *account_number )
{
    devel_debug_int (GPOINTER_TO_INT (account_number));
    gsb_gui_navigation_set_selection ( GSB_ACCOUNT_PAGE,
				       GPOINTER_TO_INT (account_number),
				       NULL );
    return FALSE;
}


/**
 * 
 *
 *
 *
 * */
void update_liste_echeances_manuelles_accueil ( gboolean force )
{
    devel_debug_int (force);

    /* need to set that in first because can change mise_a_jour_liste_echeances_manuelles_accueil */
    gsb_scheduler_check_scheduled_transactions_time_limit ();

    if ( !force
	 &&
	 !mise_a_jour_liste_echeances_manuelles_accueil )
	return;


    mise_a_jour_liste_echeances_manuelles_accueil = 0;

    if ( scheduled_transactions_to_take )
    {
	GtkWidget *vbox;
	GtkWidget *label;
	GSList *pointeur_liste;
	GtkWidget *event_box;
	GtkWidget *hbox;
    gint manual = 1;

	/* s'il y avait déjà un fils dans la frame, le détruit */
	gtk_notebook_remove_page ( GTK_NOTEBOOK(frame_etat_echeances_manuelles_accueil), 0 );

	/* on affiche la seconde frame dans laquelle on place les
	   échéances à saisir */
	show_paddingbox ( frame_etat_echeances_manuelles_accueil );

	/* on y place la liste des échéances */
	vbox = gtk_vbox_new ( FALSE, 6 );
	gtk_container_add ( GTK_CONTAINER(frame_etat_echeances_manuelles_accueil), vbox );
	gtk_widget_show ( vbox );

	/* on met une ligne vide pour faire joli */
	label = gtk_label_new (NULL);

	/* création du style normal -> bleu */
	/* pointeur dessus -> jaune-rouge */

	style_label = gtk_style_copy ( gtk_widget_get_style (label));
	gtk_widget_destroy (label);

	style_label->fg[GTK_STATE_PRELIGHT] = couleur_jaune;
	style_label->fg[GTK_STATE_NORMAL] = couleur_bleue;
	style_label->fg[GTK_STATE_INSENSITIVE] = couleur_bleue;
	style_label->fg[GTK_STATE_SELECTED] = couleur_bleue;
	style_label->fg[GTK_STATE_ACTIVE] = couleur_bleue;


	pointeur_liste = g_slist_sort_with_data ( scheduled_transactions_to_take,
				        (GCompareDataFunc) classement_sliste_echeance_par_date,
                        GINT_TO_POINTER ( manual ) );

	while ( pointeur_liste )
	{
	    gint scheduled_number;
	    gint account_number;
	    gint currency_number;
		gchar* tmpstr;

	    scheduled_number = GPOINTER_TO_INT (pointeur_liste -> data);
	    account_number = gsb_data_scheduled_get_account_number (scheduled_number);
	    currency_number = gsb_data_scheduled_get_currency_number (scheduled_number);

	    hbox = gtk_hbox_new ( TRUE, 5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );
	    gtk_widget_show (  hbox );

	    /* bouton à gauche */

	    event_box = gtk_event_box_new ();
	    g_signal_connect ( G_OBJECT ( event_box ),
				 "enter-notify-event",
				 G_CALLBACK ( met_en_prelight ),
				 NULL );
	    g_signal_connect ( G_OBJECT ( event_box ),
				 "leave-notify-event",
				 G_CALLBACK ( met_en_normal ),
				 NULL );
	    g_signal_connect ( G_OBJECT ( event_box ),
				 "button-press-event",
				 (GtkSignalFunc) saisie_echeance_accueil,
				 GINT_TO_POINTER (scheduled_number));
	    gtk_box_pack_start ( GTK_BOX ( hbox ), event_box, TRUE, TRUE, 5 );
	    gtk_widget_show ( event_box  );

            tmpstr = g_strconcat ( gsb_format_gdate (gsb_data_scheduled_get_date (scheduled_number)),
				  " : ",
				  gsb_data_payee_get_name (gsb_data_scheduled_get_party_number (scheduled_number),
								       FALSE ),
						  NULL );
	    label = gtk_label_new ( tmpstr );
	    g_free ( tmpstr );

	    gtk_widget_set_style ( label, style_label );
	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
	    gtk_container_add ( GTK_CONTAINER ( event_box ), label );
	    gtk_widget_show ( label  );

	    /* label à droite */
	    if ( gsb_data_scheduled_get_amount (scheduled_number).mantissa >= 0 )
	    {
		gchar* tmpstr2 = gsb_real_get_string_with_currency (
			gsb_data_scheduled_get_amount (scheduled_number), currency_number, TRUE);
                gchar* tmpstr = g_strdup_printf ( _("%s credited on %s"), tmpstr2,
							  gsb_data_account_get_name (account_number));
		g_free ( tmpstr2 );
		label = gtk_label_new ( tmpstr );
		g_free ( tmpstr );
	    }
	    else
	    {
		gchar* tmpstr2 = gsb_real_get_string_with_currency (gsb_real_abs (
				  gsb_data_scheduled_get_amount (scheduled_number)), currency_number, TRUE);
	        gchar* tmpstr = g_strdup_printf ( _("%s debited on %s"), tmpstr2,
							  gsb_data_account_get_name (account_number));
		g_free ( tmpstr2 );
		label = gtk_label_new ( tmpstr );
		g_free ( tmpstr );
	    }

	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, TRUE, 0 );
	    gtk_widget_show (  label );

	    pointeur_liste = pointeur_liste -> next;
	}
    }
    else
    {
	hide_paddingbox ( frame_etat_echeances_manuelles_accueil );
    }
}


void update_liste_echeances_auto_accueil ( gboolean force )
{
    if ( !force
	 &&
	 !mise_a_jour_liste_echeances_auto_accueil )
	return;

    devel_debug_int (force);

    mise_a_jour_liste_echeances_auto_accueil = 0;

    if ( scheduled_transactions_taken )
    {
	GtkWidget *vbox, *label, *event_box, *hbox;
	GSList *pointeur_liste;
	GtkStyle *style_selectable;
	GdkColor gray_color;
    gint manual = 0;

	/* s'il y avait déjà un fils dans la frame, le détruit */
	gtk_notebook_remove_page ( GTK_NOTEBOOK ( frame_etat_echeances_auto_accueil ), 0 );
	/* on affiche la seconde frame dans laquelle on place les échéances à saisir */
	show_paddingbox ( frame_etat_echeances_auto_accueil );

	gray_color.red =   0.61 * 65535 ;
	gray_color.green = 0.61 * 65535 ;
	gray_color.blue =  0.61 * 65535 ;
	gray_color.pixel = 1;

	style_selectable = gtk_style_copy ( gtk_widget_get_style ( frame_etat_echeances_auto_accueil ));
	style_selectable->fg[GTK_STATE_PRELIGHT] = gray_color;

	/* on y place la liste des échéances */

	vbox = gtk_vbox_new ( FALSE, 5 );
	gtk_container_add ( GTK_CONTAINER ( frame_etat_echeances_auto_accueil ), vbox);
	gtk_widget_show ( vbox);

	pointeur_liste = g_slist_sort_with_data ( scheduled_transactions_taken,
				      (GCompareDataFunc) classement_sliste_echeance_par_date,
                       GINT_TO_POINTER ( manual ) );

	while ( pointeur_liste )
	{
	    gint transaction_number;
	    gint account_number;
	    gint currency_number;
		gchar* tmpstr;

	    transaction_number = GPOINTER_TO_INT ( pointeur_liste -> data );
	    account_number = gsb_data_transaction_get_account_number (transaction_number);
	    currency_number = gsb_data_transaction_get_currency_number (transaction_number);


	    hbox = gtk_hbox_new ( TRUE, 5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );
	    gtk_widget_show (  hbox );

	    event_box = gtk_event_box_new ();
	    g_signal_connect ( G_OBJECT ( event_box ),
				 "enter-notify-event",
				 G_CALLBACK ( met_en_prelight ),
				 NULL );
	    g_signal_connect ( G_OBJECT ( event_box ),
				 "leave-notify-event",
				 G_CALLBACK ( met_en_normal ),
				 NULL );
	    g_signal_connect_swapped ( G_OBJECT ( event_box ),
				       "button-press-event",
				       G_CALLBACK (gsb_transactions_list_edit_transaction_by_pointer),
				       GINT_TO_POINTER (transaction_number));
	    gtk_widget_show ( event_box );

	    /* label à gauche */
            tmpstr = g_strconcat ( gsb_format_gdate ( gsb_data_transaction_get_date (transaction_number)),
					  " : ",
					  gsb_data_payee_get_name (gsb_data_transaction_get_party_number (transaction_number), FALSE),
					  NULL );
	    label = gtk_label_new ( tmpstr );
	    g_free ( tmpstr );

	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
	    gtk_widget_set_style ( label, style_selectable );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), event_box, TRUE, TRUE, 5 );
	    gtk_container_add ( GTK_CONTAINER ( event_box ), label );
	    gtk_widget_show ( label  );

	    /* label à droite */

	    if ( gsb_data_transaction_get_amount (transaction_number).mantissa >= 0 )
	    {
		gchar* tmpstr2 = gsb_real_get_string_with_currency (
			gsb_data_transaction_get_amount (transaction_number), currency_number, TRUE);
	        gchar* tmpstr = g_strdup_printf ( _("%s credited on %s"), tmpstr2 ,
							  gsb_data_account_get_name (account_number));
		g_free ( tmpstr2 );
		label = gtk_label_new ( tmpstr );
		g_free ( tmpstr );
	    }
	    else
	    {
		gchar* tmpstr2 = gsb_real_get_string_with_currency (gsb_real_abs (
			gsb_data_transaction_get_amount (transaction_number)), currency_number, TRUE);
	        gchar* tmpstr = g_strdup_printf ( _("%s debited on %s"),
					  tmpstr2, gsb_data_account_get_name (account_number));
		g_free ( tmpstr2 );
		label = gtk_label_new ( tmpstr );
	        g_free ( tmpstr );
	    }

	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), label, TRUE, TRUE, 5 );
	    gtk_widget_show ( label );

	    pointeur_liste = pointeur_liste -> next;
	}
    }
    else
    {
	hide_paddingbox ( frame_etat_echeances_auto_accueil );
    }
}


/**
 * Fonction update_soldes_minimaux
 * vérifie les soldes de tous les comptes, affiche un message d'alerte si nécessaire
 * 
 *
 *
 *
 * */
void update_soldes_minimaux ( gboolean force )
{
    GtkWidget *vbox_1;
    GtkWidget *vbox_2;
    GtkWidget *label;
    GSList *liste_autorise;
    GSList *liste_voulu;
    GSList *liste_autorise_et_voulu;
    GSList *list_tmp;


    if ( !force
	 &&
	 !mise_a_jour_soldes_minimaux  )
	return;

    devel_debug ( "update_soldes_minimaux" );

    mise_a_jour_soldes_minimaux = 0;

    liste_autorise = NULL;
    liste_voulu = NULL;
    liste_autorise_et_voulu = NULL;

    /* s'il y avait déjà un fils dans la frame, le détruit */

    gtk_notebook_remove_page ( GTK_NOTEBOOK ( frame_etat_soldes_minimaux_autorises ), 0 );
    gtk_notebook_remove_page ( GTK_NOTEBOOK ( frame_etat_soldes_minimaux_voulus ), 0 );

    hide_paddingbox ( frame_etat_soldes_minimaux_autorises );
    hide_paddingbox ( frame_etat_soldes_minimaux_voulus );

    vbox_1 = NULL;
    vbox_2 = NULL;

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
        gint i;

        i = gsb_data_account_get_no_account ( list_tmp -> data );

        if ( gsb_data_account_get_closed_account ( i ) && !etat.show_closed_accounts )
        {
            list_tmp = list_tmp -> next;
            continue;
        }

        if ( gsb_real_cmp ( gsb_data_account_get_current_balance (i),
                    gsb_data_account_get_mini_balance_authorized (i)) == -1
             &&
             gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES )
        {
            if ( !vbox_1 )
            {
            vbox_1 = gtk_vbox_new ( TRUE, 5 );
            gtk_container_add ( GTK_CONTAINER ( frame_etat_soldes_minimaux_autorises ), vbox_1 );
            gtk_widget_show ( vbox_1 );
            show_paddingbox ( frame_etat_soldes_minimaux_autorises );
            }
            label = gtk_label_new ( gsb_data_account_get_name (i) );
            gtk_box_pack_start ( GTK_BOX ( vbox_1 ), label, FALSE, FALSE, 0 );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_TOP );
            gtk_widget_show ( label );

            show_paddingbox ( frame_etat_soldes_minimaux_autorises );
        }

        if ( gsb_real_cmp ( gsb_data_account_get_current_balance (i),
                    gsb_data_account_get_mini_balance_wanted (i)) == -1
             &&
             gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES
             &&
             gsb_real_cmp ( gsb_data_account_get_current_balance (i),
                    gsb_data_account_get_mini_balance_authorized (i)) == -1
             &&
             gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES)
        {
            if ( !vbox_2 )
            {
            vbox_2 = gtk_vbox_new ( TRUE, 5 );
            gtk_container_add ( GTK_CONTAINER ( frame_etat_soldes_minimaux_voulus ), vbox_2 );
            gtk_widget_show ( vbox_2 );
            show_paddingbox ( frame_etat_soldes_minimaux_voulus );
            }

            label = gtk_label_new ( gsb_data_account_get_name (i) );
            gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
            gtk_box_pack_start ( GTK_BOX ( vbox_2 ), label, FALSE, FALSE, 0 );
            gtk_widget_show ( label );

            show_paddingbox ( frame_etat_soldes_minimaux_voulus );
        }

	    list_tmp = list_tmp -> next;
    }


    /*     on affiche une boite d'avertissement si nécessaire */

    affiche_dialogue_soldes_minimaux ();
    mise_a_jour_liste_comptes_accueil = 1;
}
/* ************************************************************************* */

/* ************************************************************************* */
/* cette fonction vérifie les soldes minimaux et affiche une boite de dialogue */
/* avec les comptes en dessous des seuils si non désactivé */
/* ************************************************************************* */

void affiche_dialogue_soldes_minimaux ( void )
{
    GSList *liste_autorise;
    GSList *liste_voulu;
    GSList *liste_autorise_et_voulu;
    GSList *liste_tmp;
    gchar *texte_affiche;
    GSList *list_tmp;

    if ( !mise_a_jour_soldes_minimaux  )
	return;

    liste_autorise = NULL;
    liste_voulu = NULL;
    liste_autorise_et_voulu = NULL;

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( gsb_real_cmp ( gsb_data_account_get_current_balance (i),
			    gsb_data_account_get_mini_balance_authorized (i)) == -1
	     &&
	     gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES
	     &&
	     !gsb_data_account_get_mini_balance_authorized_message (i) )
	{
	    if ( gsb_real_cmp ( gsb_data_account_get_current_balance (i),
				gsb_data_account_get_mini_balance_wanted (i)) == -1 )
	    {
		liste_autorise_et_voulu = g_slist_append ( liste_autorise_et_voulu,
							   gsb_data_account_get_name (i) );
		gsb_data_account_set_mini_balance_wanted_message ( i,
								   1 );
	    }
	    else
	    {
		liste_autorise = g_slist_append ( liste_autorise,
						  gsb_data_account_get_name (i) );
	    }
	    gsb_data_account_set_mini_balance_authorized_message ( i,
								   1 );
	}

	if ( gsb_real_cmp ( gsb_data_account_get_current_balance (i),
			    gsb_data_account_get_mini_balance_wanted (i)) == -1
	     &&
	     gsb_real_cmp ( gsb_data_account_get_current_balance (i),
			    gsb_data_account_get_mini_balance_authorized (i)) == 1
	     &&
	     gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES
	     &&
	     !gsb_data_account_get_mini_balance_wanted_message (i) )
	{
	    liste_voulu = g_slist_append ( liste_voulu,
					   gsb_data_account_get_name (i) );
	    gsb_data_account_set_mini_balance_wanted_message ( i,
							       1 );
	}

	/* 	si on repasse au dessus des seuils, c'est comme si on n'avait rien affiché */

	if ( gsb_real_cmp ( gsb_data_account_get_current_balance (i),
			    gsb_data_account_get_mini_balance_authorized (i)) == 1 )
	    gsb_data_account_set_mini_balance_authorized_message ( i,
								   0 );
	if ( gsb_real_cmp ( gsb_data_account_get_current_balance (i),
			    gsb_data_account_get_mini_balance_wanted (i)) == 1 )
	    gsb_data_account_set_mini_balance_wanted_message ( i,
							       0 );

	list_tmp = list_tmp -> next;
    }

    /*     on crée le texte récapilutatif */

    texte_affiche = g_strdup("");

    if ( liste_autorise_et_voulu )
    {
	if ( g_slist_length ( liste_autorise_et_voulu ) == 1 )
	    texte_affiche = g_strdup_printf ( _("balance of account %s is under desired and authorised minima!"),
					      (gchar *) liste_autorise_et_voulu -> data );
	else
	{
	    texte_affiche = g_strdup(_("accounts with the balance under desired and authorised minimal :\n\n"));
	    liste_tmp = liste_autorise_et_voulu;
	    while ( liste_tmp )
	    {
	        gchar* oldstr = texte_affiche;
		texte_affiche = g_strconcat ( oldstr,
					      liste_tmp -> data,
					      "\n",
					      NULL );
		g_free ( oldstr );
		liste_tmp = liste_tmp -> next;
	    }
	}
    }

    if ( liste_autorise )
    {
	if ( strlen (texte_affiche))
	{
	    gchar* oldstr = texte_affiche;
	    texte_affiche = g_strconcat ( oldstr,
					  "\n\n",
					  NULL );
	    g_free ( oldstr );
	}

	if ( g_slist_length ( liste_autorise ) == 1 )
	{
	    gchar* oldstr = texte_affiche;
	    gchar* tmpstr = g_strdup_printf ( _("balance of account %s is under authorised minimum!"),
							    (gchar *) liste_autorise -> data );
	    texte_affiche = g_strconcat ( oldstr, tmpstr , NULL );
	    g_free ( oldstr );
	    g_free ( tmpstr );
	}
	else
	{
	    gchar* oldstr = texte_affiche;
	    texte_affiche = g_strconcat ( oldstr,
					  _("accounts with the balance under authorised minimal :\n\n"),
					  NULL );
	    g_free ( oldstr );
	    liste_tmp = liste_autorise;
	    while ( liste_tmp )
	    {
	        gchar* oldstr = texte_affiche;
		texte_affiche = g_strconcat ( oldstr,
					      liste_tmp -> data,
					      "\n",
					      NULL );
	        g_free ( oldstr );
		liste_tmp = liste_tmp -> next;
	    }
	}
    }

    if ( liste_voulu )
    {
	if ( strlen (texte_affiche))
	{
	    gchar* oldstr = texte_affiche;
	    texte_affiche = g_strconcat ( oldstr,
					  "\n\n",
					  NULL );

	    g_free ( oldstr );
	}

	if ( g_slist_length ( liste_voulu ) == 1 )
	{
	    gchar* oldstr = texte_affiche;
	    gchar* tmpstr = g_strdup_printf ( _("balance of account %s is under desired minimum!"),
							    (gchar *) liste_voulu -> data );
	    texte_affiche = g_strconcat ( texte_affiche, tmpstr , NULL );
	    g_free ( tmpstr );
	    g_free ( oldstr );
	}
	else
	{
	    gchar* oldstr = texte_affiche;
	    texte_affiche = g_strconcat ( oldstr,
					  _("accounts with the balance under desired minimal :\n\n"),
					  NULL );
	    g_free ( oldstr );
	    liste_tmp = liste_voulu;
	    while ( liste_tmp )
	    {
	        gchar* oldstr = texte_affiche;
		texte_affiche = g_strconcat ( oldstr,
					      liste_tmp -> data,
					      "\n",
					      NULL );
	        g_free ( oldstr );
		liste_tmp = liste_tmp -> next;
	    }
	}
    }

    if ( strlen ( texte_affiche ))
	dialog_message ( "minimum-balance-alert", texte_affiche );
    g_free ( texte_affiche );
}
/* ************************************************************************* */

void update_fin_comptes_passifs ( gboolean force )
{
    GtkWidget *vbox;
    GtkWidget *label;
    GSList *liste_tmp;
    GSList *pointeur;
    GSList *list_tmp;

    if ( !force
	 &&
	 !mise_a_jour_fin_comptes_passifs )
	return;

    devel_debug (NULL);

    mise_a_jour_fin_comptes_passifs = 0;

    gtk_notebook_remove_page ( GTK_NOTEBOOK(frame_etat_fin_compte_passif), 0 );
    hide_paddingbox ( frame_etat_fin_compte_passif );

    if ( !etat.show_closed_accounts )
        return;

    list_tmp = gsb_data_account_get_list_accounts ();
    liste_tmp = NULL;

    while ( list_tmp )
    {
	gint i;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( gsb_data_account_get_kind (i) == GSB_TYPE_LIABILITIES
	     &&
	     gsb_data_account_get_current_balance (i).mantissa >= 0 )
	    liste_tmp = g_slist_append ( liste_tmp, gsb_data_account_get_name (i) );

	list_tmp = list_tmp -> next;
    }

    if ( g_slist_length ( liste_tmp ) )
    {
	vbox = gtk_vbox_new ( FALSE, 0 );
	gtk_container_add ( GTK_CONTAINER ( frame_etat_fin_compte_passif ), vbox );
	gtk_widget_show ( vbox );

	pointeur = liste_tmp;

	while ( pointeur )
	{
	    label = gtk_label_new (pointeur -> data );
	    gtk_box_pack_start ( GTK_BOX ( vbox ), label, FALSE, FALSE, 0 );
	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
	    gtk_widget_show ( label );

	    pointeur = pointeur -> next;
	}

	show_paddingbox ( frame_etat_fin_compte_passif );
    }
}




/**
 * update the finished scheduled transactions part in the main page
 * the scheduled transaction in param is finished
 *
 * \param scheduled_number
 *
 * \return FALSE
 * */
gboolean gsb_main_page_update_finished_scheduled_transactions ( gint scheduled_number )
{
    GtkWidget * label, * hbox, * page;
    gint account_number;
    gint currency_number;
	gchar* tmpstr;

    account_number = gsb_data_scheduled_get_account_number (scheduled_number);
    currency_number = gsb_data_scheduled_get_currency_number (scheduled_number);

    /* check if the vbox is already made, and make it if necesssary */
    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (main_page_finished_scheduled_transactions_part), 0);

    if ( !page )
    {
	page = gtk_vbox_new ( FALSE,
			      5 );
	gtk_notebook_append_page  (GTK_NOTEBOOK (main_page_finished_scheduled_transactions_part),
				   page, NULL );
	gtk_widget_show ( page );
    }
    /* append in the page the finished scheduled transaction */

    hbox = gtk_hbox_new ( TRUE, 0 );
    gtk_widget_show (  hbox );

    /* label à gauche */

    tmpstr = g_strconcat ( gsb_format_gdate ( gsb_data_scheduled_get_date (scheduled_number)),
					  " : ",
					  gsb_data_payee_get_name (gsb_data_scheduled_get_party_number (scheduled_number), FALSE),
					  NULL );
    label = gtk_label_new ( tmpstr );
    g_free ( tmpstr );

    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, TRUE, TRUE, 0 );
    gtk_widget_show ( label  );

    /* label à droite */

    if ( gsb_data_scheduled_get_amount (scheduled_number).mantissa >= 0 )
    {
	gchar* tmpstr2 = gsb_real_get_string_with_currency ( gsb_data_scheduled_get_amount (
		scheduled_number), currency_number, TRUE );
        tmpstr = g_strdup_printf ( _("%s credited on %s"),
				  tmpstr2,
				  gsb_data_account_get_name (account_number));
        g_free ( tmpstr2);
    }
    else
    {
	gchar* tmpstr2 = gsb_real_get_string_with_currency ( gsb_real_abs (
		gsb_data_scheduled_get_amount (scheduled_number)), currency_number, TRUE );
	tmpstr = g_strdup_printf ( _("%s debited on %s"),
				  tmpstr2,
				  gsb_data_account_get_name (account_number));
        g_free ( tmpstr2 );
    }

    label = gtk_label_new ( tmpstr );
    g_free ( tmpstr );
    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_RIGHT, MISC_VERT_CENTER );
    gtk_box_pack_end ( GTK_BOX (hbox), label, FALSE, TRUE, 0 );
    gtk_widget_show (  label );

    gtk_box_pack_start ( GTK_BOX (page),
			 hbox,
			 FALSE,
			 TRUE,
			 0 );
    gtk_widget_show (  label );

    show_paddingbox (main_page_finished_scheduled_transactions_part);

    return FALSE;
}


/* *******************************************************************************/
/* page de configuration pour la page d'accueil */
/* *******************************************************************************/
GtkWidget *onglet_accueil (void)
{
    GtkWidget *vbox_pref, *vbox, *paddingbox, *button;
    GtkWidget *hbox, *vbox2, *sw, *treeview;
    GtkListStore *list_store;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;
    GtkTreeSelection *selection;
    GtkTreeDragDestIface * dst_iface;
    GtkTreeDragSourceIface * src_iface;
    static GtkTargetEntry row_targets[] = {
    { "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_WIDGET, 0 }
    };

    vbox_pref = new_vbox_with_title_and_icon ( _("Configuration of the main page"),
                        "title.png" );

    vbox = gtk_vbox_new ( FALSE, 12 );
    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), vbox, TRUE, TRUE, 0 );
    gtk_container_set_border_width ( GTK_CONTAINER ( vbox ), 12 );

    /* pour les francophones ;-) */
    if ( g_strstr_len ( ( g_ascii_strup ( gdk_set_locale ( ), -1 ) ), -1, "FR" ) )
    {
        paddingbox = new_paddingbox_with_title (vbox, FALSE, "Pluriel de final" );

        gtk_box_pack_start ( GTK_BOX ( paddingbox ),
                        gsb_automem_radiobutton_new ( "Soldes finals",
                        "Soldes finaux",
                        &conf.pluriel_final,
                        G_CALLBACK (gsb_gui_navigation_update_home_page), NULL ),
                        FALSE, FALSE, 0 );
    }

    /* Take into account the planned operations in the calculation of balances */
    paddingbox = new_paddingbox_with_title ( vbox, FALSE, _("Calculation of balances") );

    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    button = gsb_automem_checkbutton_new (
                        _("Take into account the scheduled operations "
                          "in the calculation of balances"),
                        &conf.balances_with_scheduled,
                        G_CALLBACK ( gsb_config_scheduler_switch_balances_with_scheduled ),
                        NULL );

    gtk_box_pack_start ( GTK_BOX ( hbox ), button, FALSE, FALSE, 0 );

    /* Data partial balance settings */
    paddingbox = new_paddingbox_with_title (vbox, FALSE, 
                        _("Balances partials of the list of accounts") );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, TRUE, TRUE, 5);

    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                        GTK_SHADOW_ETCHED_IN);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                        GTK_POLICY_AUTOMATIC,
                        GTK_POLICY_ALWAYS);
    gtk_box_pack_start ( GTK_BOX (hbox), sw, TRUE,TRUE, 0 );

    /* Create Add/Edit/Remove buttons */
    vbox2 = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox2, FALSE, FALSE, 0 );

    /* Button "Add" */
    button = gtk_button_new_from_stock (GTK_STOCK_ADD);
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK  ( gsb_partial_balance_add ),
                        vbox_pref );
    gtk_box_pack_start ( GTK_BOX ( vbox2 ), button, FALSE, FALSE, 5 );
    g_object_set_data ( G_OBJECT (vbox_pref), "add_button", button );

    /* Button "Edit" */
    button = gtk_button_new_from_stock (GTK_STOCK_EDIT);
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK  ( gsb_partial_balance_edit ),
                        vbox_pref );
    gtk_box_pack_start ( GTK_BOX ( vbox2 ), button, FALSE, FALSE, 5 );
    gtk_widget_set_sensitive ( button, FALSE );
    g_object_set_data ( G_OBJECT (vbox_pref), "edit_button", button );

    /* Button "Remove" */
    button = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    g_signal_connect ( G_OBJECT ( button ),
                        "clicked",
                        G_CALLBACK ( gsb_partial_balance_delete ),
                        vbox_pref );
    gtk_box_pack_start ( GTK_BOX ( vbox2 ), button, FALSE, FALSE, 5 );
    gtk_widget_set_sensitive ( button, FALSE );
    g_object_set_data ( G_OBJECT (vbox_pref), "remove_button", button );

    /* create the model */
    list_store = gsb_partial_balance_create_model ( );

    /* populate the model if necessary */
    if ( g_slist_length ( gsb_data_partial_balance_get_list ( ) ) > 0 )
        gsb_partial_balance_fill_model ( list_store );

    /* create the treeview */
    treeview = gtk_tree_view_new_with_model (
                        GTK_TREE_MODEL (list_store) );
    g_object_unref ( list_store );

    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
    gtk_widget_set_size_request ( treeview, -1, 150 );

    /* check the keys on the list */
    g_signal_connect ( G_OBJECT ( treeview ),
                        "key_press_event",
                        G_CALLBACK ( gsb_partial_balance_key_press ),
                        NULL );

    /* check the buttons on the list */
    g_signal_connect ( G_OBJECT ( treeview ),
                        "button_press_event",
                        G_CALLBACK ( gsb_partial_balance_button_press ),
                        NULL );

    /* Enable drag & drop */
    gtk_tree_view_enable_model_drag_source ( GTK_TREE_VIEW (treeview),
                        GDK_BUTTON1_MASK, row_targets, 1,
                        GDK_ACTION_MOVE );
    gtk_tree_view_enable_model_drag_dest ( GTK_TREE_VIEW (treeview), row_targets,
                        1, GDK_ACTION_MOVE );
    gtk_tree_view_set_reorderable ( GTK_TREE_VIEW (treeview), TRUE );

    selection = gtk_tree_view_get_selection ( GTK_TREE_VIEW (treeview) );
    gtk_tree_selection_set_select_function ( selection,
                        (GtkTreeSelectionFunc) gsb_partial_balance_select_func,
                        vbox_pref, NULL );
    gtk_container_add (GTK_CONTAINER (sw), treeview);
    gtk_container_set_resize_mode (GTK_CONTAINER (sw), GTK_RESIZE_PARENT);
    g_object_set_data ( G_OBJECT (vbox_pref), "treeview", treeview );

    /* Nom du solde partiel */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Name"),
                        cell, "text", 0, NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_sort_column_id (column, 0);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (treeview), column);

    /* Liste des comptes */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Accounts list"),
                        cell, "text", 1, NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_sort_column_id (column, 1);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (treeview), column);

    /* Colorize */
    cell = gtk_cell_renderer_toggle_new ( );
    g_signal_connect ( cell,
                        "toggled",
                        G_CALLBACK ( gsb_partial_balance_colorise_toggled ),
                        treeview );
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_object_set (cell, "xalign", 0.5, NULL);

    column = gtk_tree_view_column_new_with_attributes ( _("Colorize"),
                        cell,
                        "active", 5,
                        NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(treeview), column);

    /* Type de compte */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Account kind"),
                        cell, "text", 2, NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_sort_column_id (column, 2);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (treeview), column);

    /* Devise */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new_with_attributes ( _("Currency"),
                        cell, "text", 3, NULL);
    gtk_tree_view_column_set_expand ( column, TRUE );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_set_sort_column_id (column, 3);
    gtk_tree_view_append_column ( GTK_TREE_VIEW (treeview), column);

    dst_iface = GTK_TREE_DRAG_DEST_GET_IFACE ( list_store );
    if ( dst_iface )
        dst_iface -> drag_data_received = &gsb_data_partial_balance_drag_data_received;

    src_iface = GTK_TREE_DRAG_SOURCE_GET_IFACE ( list_store );
    if ( src_iface )
    {
        gtk_selection_add_target ( treeview,
                      GDK_SELECTION_PRIMARY,
                      GDK_SELECTION_TYPE_ATOM,
                      1 );
        src_iface -> drag_data_get = &gsb_data_partial_balance_drag_data_get;
    }

    gtk_widget_show_all ( vbox_pref );

    return ( vbox_pref );
}


/**
 * update the title of the main page
 *
 * \param title
 *
 *
 * */
void gsb_main_page_update_homepage_title ( const gchar *title )
{
    gchar * tmp_str;

    /* at the first use of grisbi,label_titre_fichier doesn't still exist */
    if ( !label_titre_fichier || !GTK_IS_LABEL ( label_titre_fichier ) )
        return;

    tmp_str = g_markup_printf_escaped ("<span size=\"x-large\">%s</span>", title );

    gtk_label_set_markup ( GTK_LABEL ( label_titre_fichier ), tmp_str );

    g_free ( tmp_str );
}


gboolean gsb_config_scheduler_switch_balances_with_scheduled ( void )
{
    GSList *list_tmp;

    devel_debug ( NULL );

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
        gint account_number;

        account_number = gsb_data_account_get_no_account ( list_tmp -> data );
        gsb_data_account_set_balances_are_dirty ( account_number );

        /* MAJ HOME_PAGE */
        gsb_gui_navigation_update_home_page ( );

        list_tmp = list_tmp -> next;
    }
    return FALSE;
}


/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
