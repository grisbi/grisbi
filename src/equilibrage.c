/* ************************************************************************** */
/* Fichier qui permet l'équilibrage des comptes                               */
/* 			equilibrage.c                                         */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003 Benjamin Drieu (bdrieu@april.org)		      */
/*			2004 Alain Portal (aportal@univ-montp2.fr) 	      */
/*			http://www.grisbi.org   			      */
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
#include "equilibrage_constants.h"





/*START_INCLUDE*/
#include "equilibrage.h"
#include "utils_devises.h"
#include "utils_montants.h"
#include "operations_liste.h"
#include "dialog.h"
#include "utils_dates.h"
#include "data_account.h"
#include "calendar.h"
#include "utils_str.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_rapprochements.h"
#include "utils_types.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean annuler_equilibrage ( void );
static gboolean clavier_equilibrage ( GtkWidget *widget,
			       GdkEventKey *event );
static void deplacement_type_tri_bas ( void );
static void deplacement_type_tri_haut ( GtkWidget * button, gpointer data );
static void fill_reconciliation_tree ();
static gboolean fin_equilibrage ( GtkWidget *bouton_ok,
		       gpointer data );
static gboolean modif_entree_solde_final_equilibrage ( void );
static gboolean modif_entree_solde_init_equilibrage ( void );
static void reconcile_by_date_toggled ( GtkCellRendererToggle *cell, 
				 gchar *path_str, gpointer data );
static void reconcile_include_neutral_toggled ( GtkCellRendererToggle *cell, 
					 gchar *path_str, gpointer data );
static void select_reconciliation_entry ( GtkTreeSelection * tselection, 
				   GtkTreeModel * model );
static gboolean sortie_entree_date_equilibrage ( GtkWidget *entree );
static gboolean souris_equilibrage ( GtkWidget *entree,
			      GdkEventButton *event );
/*END_STATIC*/






GtkWidget * reconcile_treeview;
GtkTreeStore *reconcile_model;
GtkWidget * button_move_up, * button_move_down;
GtkTreeSelection * reconcile_selection;


GtkWidget *entree_no_rapprochement;
GtkWidget *label_ancienne_date_equilibrage;
GtkWidget *entree_ancien_solde_equilibrage;
GtkWidget *entree_nouvelle_date_equilibrage;
GtkWidget *entree_nouveau_montant_equilibrage;
gdouble solde_initial;
gdouble solde_final;
gdouble operations_pointees;
GtkWidget *label_equilibrage_compte;
GtkWidget *label_equilibrage_initial;
GtkWidget *label_equilibrage_final;
GtkWidget *label_equilibrage_pointe;
GtkWidget *label_equilibrage_ecart;
GtkWidget *bouton_ok_equilibrage;
GSList *liste_struct_rapprochements;            /* contient la liste des structures de no_rapprochement */

/* ces 3 variables retiennent les données de l'utilisateur avant rapprochement */

gint ancien_nb_lignes_ope;
gint ancien_r_modifiable;
gint ancien_retient_affichage_par_compte;


/*START_EXTERN*/
extern gint compte_courant;
extern GtkWidget *label_proprietes_operations_compte;
extern GtkWidget *label_releve;
extern gint mise_a_jour_liste_comptes_accueil;
extern GtkTreeStore *model;
extern gint nb_comptes;
extern GtkWidget *notebook_comptes_equilibrage;
extern GSList *ordre_comptes;
extern FILE * out;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GtkWidget *treeview;
extern GtkWidget *vbox_fleches_tri;
/*END_EXTERN*/



/******************************************************************************/
GtkWidget *creation_fenetre_equilibrage ( void )
{
    GtkWidget *fenetre_equilibrage;
    GtkWidget *label;
    GtkWidget *table;
    GtkWidget *hbox;
    GtkWidget *bouton;
    GtkWidget *separateur;
    GtkTooltips *tips;


    /* la fenetre est une vbox */

    fenetre_equilibrage = gtk_vbox_new ( FALSE, 5 );
    gtk_container_set_border_width ( GTK_CONTAINER ( fenetre_equilibrage ),
				     10 );
    gtk_signal_connect ( GTK_OBJECT ( fenetre_equilibrage ),
			 "key_press_event",
			 GTK_SIGNAL_FUNC ( traitement_clavier_liste ),
			 NULL );
    gtk_widget_show ( fenetre_equilibrage );


    /* on met le nom du compte à équilibrer en haut */

    label_equilibrage_compte = gtk_label_new ( "" );
    gtk_label_set_justify ( GTK_LABEL ( label_equilibrage_compte ),
			    GTK_JUSTIFY_CENTER );
    gtk_box_pack_start ( GTK_BOX ( fenetre_equilibrage ),
			 label_equilibrage_compte,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( label_equilibrage_compte );


    separateur = gtk_hseparator_new();
    gtk_box_pack_start ( GTK_BOX ( fenetre_equilibrage ),
			 separateur,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( separateur );


    /* on crée le tooltips */

    tips = gtk_tooltips_new ();

    /* sous le nom, on met le no de rapprochement, c'est une entrée car il est modifiable */

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( fenetre_equilibrage ),
			 hbox,
			 FALSE,
			 FALSE,
			 10);
    gtk_widget_show ( hbox );

    label = gtk_label_new ( COLON(_("Reconciliation reference")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 label,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( label );

    entree_no_rapprochement = gtk_entry_new ();
    /*   gtk_widget_set_usize ( entree_no_rapprochement, */
    /* 			 100, FALSE ); */
    gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			   entree_no_rapprochement,
			   _("If reconciliation reference ends in a digit, it is automatically incremented at each reconciliation."),
			   _("Reconciliation reference") );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 entree_no_rapprochement,
			 TRUE,
			 TRUE,
			 0);
    gtk_widget_show ( entree_no_rapprochement );


    separateur = gtk_hseparator_new();
    gtk_box_pack_start ( GTK_BOX ( fenetre_equilibrage ),
			 separateur,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( separateur );

    /* on met un premier tab en haut contenant dates et soldes des relevés
       avec possibilité de modif du courant */

    table = gtk_table_new ( 3,
			    5,
			    FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ),
				 5 );
    gtk_box_pack_start ( GTK_BOX ( fenetre_equilibrage ),
			 table,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( table );


    separateur = gtk_hseparator_new();
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				separateur,
				0, 3,
				1, 2 );
    gtk_widget_show ( separateur );


    separateur = gtk_hseparator_new();
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				separateur,
				0, 3,
				3, 4 );
    gtk_widget_show ( separateur );


    separateur = gtk_vseparator_new ();
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				separateur,
				1, 2,
				0, 5 );
    gtk_widget_show ( separateur );



    label = gtk_label_new ( _("Date") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label,
				0, 1,
				0, 1);
    gtk_widget_show ( label );


    label = gtk_label_new ( _("Balance") );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label,
				2, 3,
				0, 1);
    gtk_widget_show ( label );



    label_ancienne_date_equilibrage = gtk_label_new ( "" );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label_ancienne_date_equilibrage,
				0, 1,
				2, 3 );
    gtk_widget_show ( label_ancienne_date_equilibrage );


    entree_ancien_solde_equilibrage = gtk_entry_new ( );
    gtk_widget_set_usize ( entree_ancien_solde_equilibrage,
			   50, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_ancien_solde_equilibrage ),
			 "changed",
			 GTK_SIGNAL_FUNC ( modif_entree_solde_init_equilibrage ),
			 NULL );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				entree_ancien_solde_equilibrage,
				2, 3,
				2, 3 );
    gtk_widget_show ( entree_ancien_solde_equilibrage );



    entree_nouvelle_date_equilibrage = gtk_entry_new ();
    gtk_widget_set_usize ( entree_nouvelle_date_equilibrage,
			   50, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_nouvelle_date_equilibrage ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( souris_equilibrage ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( entree_nouvelle_date_equilibrage ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( clavier_equilibrage ),
			 NULL );
    gtk_signal_connect_after ( GTK_OBJECT ( entree_nouvelle_date_equilibrage ),
			       "focus-out-event",
			       GTK_SIGNAL_FUNC ( sortie_entree_date_equilibrage ),
			       NULL );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				entree_nouvelle_date_equilibrage,
				0, 1,
				4, 5 );
    gtk_widget_show ( entree_nouvelle_date_equilibrage );


    entree_nouveau_montant_equilibrage = gtk_entry_new ();
    gtk_widget_set_usize ( entree_nouveau_montant_equilibrage,
			   50, FALSE );
    gtk_signal_connect ( GTK_OBJECT ( entree_nouveau_montant_equilibrage ),
			 "changed",
			 GTK_SIGNAL_FUNC ( modif_entree_solde_final_equilibrage ),
			 NULL );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				entree_nouveau_montant_equilibrage,
				2, 3,
				4, 5 );
    gtk_widget_show ( entree_nouveau_montant_equilibrage );




    separateur = gtk_hseparator_new();
    gtk_box_pack_start ( GTK_BOX ( fenetre_equilibrage ),
			 separateur,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( separateur );


    /* la 2ème table contient le solde init, final, du pointage et l'écart */

    table = gtk_table_new ( 5,
			    2,
			    FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ),
				 5 );
    gtk_box_pack_start ( GTK_BOX ( fenetre_equilibrage ),
			 table,
			 FALSE,
			 FALSE,
			 15);
    gtk_widget_show ( table );



    label = gtk_label_new ( COLON(_("Initial balance")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label,
				0, 1,
				0, 1);
    gtk_widget_show ( label );


    label_equilibrage_initial = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_equilibrage_initial ),
			     1,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label_equilibrage_initial,
				1, 2,
				0, 1);
    gtk_widget_show ( label_equilibrage_initial );


    label = gtk_label_new ( COLON(_("Final balance")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label,
				0, 1,
				1, 2);
    gtk_widget_show ( label );


    label_equilibrage_final = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_equilibrage_final ),
			     1,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label_equilibrage_final,
				1, 2,
				1, 2);
    gtk_widget_show ( label_equilibrage_final );


    label = gtk_label_new ( COLON(_("Checking")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label,
				0, 1,
				2, 3);
    gtk_widget_show ( label );

    label_equilibrage_pointe = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_equilibrage_pointe ),
			     1,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label_equilibrage_pointe,
				1, 2,
				2, 3);
    gtk_widget_show ( label_equilibrage_pointe );


    separateur = gtk_hseparator_new();
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				separateur,
				0, 2,
				3, 4);
    gtk_widget_show ( separateur );


    label = gtk_label_new ( COLON(_("Variance")) );
    gtk_misc_set_alignment ( GTK_MISC ( label ),
			     0,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label,
				0, 1,
				4, 5);
    gtk_widget_show ( label );


    label_equilibrage_ecart = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_equilibrage_ecart ),
			     1,
			     0.5 );
    gtk_table_attach_defaults ( GTK_TABLE ( table ),
				label_equilibrage_ecart,
				1, 2,
				4, 5);

    /* on met les boutons */


    hbox = gtk_hbox_new ( TRUE,
			  5);
    gtk_box_pack_end ( GTK_BOX ( fenetre_equilibrage ),
		       hbox,
		       FALSE,
		       FALSE,
		       0);
    gtk_widget_show ( hbox );

    bouton = gtk_button_new_from_stock ( GTK_STOCK_CANCEL );
    gtk_button_set_relief ( GTK_BUTTON ( bouton),
			    GTK_RELIEF_NONE);
    gtk_signal_connect ( GTK_OBJECT (bouton),
			 "clicked",
			 ( GtkSignalFunc ) annuler_equilibrage,
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( bouton );

    bouton_ok_equilibrage = gtk_button_new_from_stock ( GTK_STOCK_OK );
    gtk_button_set_relief ( GTK_BUTTON ( bouton_ok_equilibrage),
			    GTK_RELIEF_NONE);
    gtk_signal_connect ( GTK_OBJECT ( bouton_ok_equilibrage ),
			 "clicked",
			 ( GtkSignalFunc ) fin_equilibrage,
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 bouton_ok_equilibrage,
			 FALSE,
			 FALSE,
			 0);
    gtk_widget_show ( bouton_ok_equilibrage );

    separateur = gtk_hseparator_new();
    gtk_box_pack_end ( GTK_BOX ( fenetre_equilibrage ),
		       separateur,
		       FALSE,
		       FALSE,
		       0);
    gtk_widget_show ( separateur );


    return ( fenetre_equilibrage );
}
/******************************************************************************/

/******************************************************************************/
void equilibrage ( void )
{
    GDate *date;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    /* efface le label propriétés du compte */

    gtk_widget_hide ( label_proprietes_operations_compte );

    /* calcule le montant des opérations pointées */
    /* FIXME : à vérifie mais normalement, pas besoin de ça vu que c'est en temps réel... */
    /*     calcule_total_pointe_compte ( compte_courant ); */


    /* récupère l'ancien no de rapprochement et essaie d'incrémenter la partie
       numérique. Si ne réussit pas, remet juste le nom de l'ancien */

    if ( gsb_account_get_reconcile_last_number (compte_courant) )
    {
	gchar *new_rap;

	new_rap = rapprochement_name_by_no ( gsb_account_get_reconcile_last_number (compte_courant) );

	if ( new_rap )
	{
	    gchar *pointeur_mobile;
	    gchar *pointeur_fin;

	    /* on va créer une nouvelle chaine contenant la partie numérique */

	    pointeur_fin = new_rap + (strlen ( new_rap ) - 1) * sizeof (gchar);
	    pointeur_mobile = pointeur_fin;

	    while ( pointeur_mobile[0] > 47 && pointeur_mobile[0] < 58 && pointeur_mobile >= new_rap )
		pointeur_mobile--;

	    if ( pointeur_mobile != pointeur_fin )
	    {
		/* la fin du no de rapprochement est numérique */

		gchar *rempl_zero;
		gchar *partie_num;
		gint longueur;
		gint nouvelle_longueur;

		pointeur_mobile++;

		partie_num = g_strdup ( pointeur_mobile );
		pointeur_mobile[0] = 0;

		longueur = strlen ( partie_num );

		/* on incrémente la partie numérique */

		partie_num = itoa ( my_atoi ( partie_num ) + 1 );

		/* si la nouvelle partie numérique est plus petite que l'ancienne, */
		/* c'est que des 0 ont été shuntés, on va les rajouter ici */

		nouvelle_longueur = strlen ( partie_num );

		if ( nouvelle_longueur < longueur )
		{
		    gint i;

		    rempl_zero = malloc ((longueur-nouvelle_longueur+1)*sizeof (gchar));

		    for ( i=0 ; i<longueur-nouvelle_longueur ; i++ )
			rempl_zero[i]=48;

		    rempl_zero[longueur-nouvelle_longueur] = 0;
		}
		else
		    rempl_zero = "";

		/* on  remet le tout ensemble */

		new_rap = g_strconcat ( new_rap,
					rempl_zero,
					partie_num,
					NULL );
	    }

	    gtk_entry_set_text ( GTK_ENTRY ( entree_no_rapprochement ),
				 new_rap );

	}
    }
    else
	gtk_entry_set_text ( GTK_ENTRY ( entree_no_rapprochement ),
			     "" );

    /* récupère l'ancienne date et l'augmente d'1 mois et le met dans entree_nouvelle_date_equilibrage */

    if ( gsb_account_get_current_reconcile_date (compte_courant) )
    {
	date = g_date_new_dmy ( g_date_day ( gsb_account_get_current_reconcile_date (compte_courant) ),
				g_date_month ( gsb_account_get_current_reconcile_date (compte_courant) ),
				g_date_year ( gsb_account_get_current_reconcile_date (compte_courant) ));

	gtk_label_set_text ( GTK_LABEL ( label_ancienne_date_equilibrage ),
			     g_strdup_printf ( "%02d/%02d/%d",
					       g_date_day ( date ),
					       g_date_month ( date ),
					       g_date_year ( date ) ));
	g_date_add_months ( date, 1 );

	if ( g_date_compare ( date, gdate_today()) > 0 )
	    date = gdate_today();

	gtk_entry_set_text ( GTK_ENTRY ( entree_ancien_solde_equilibrage ),
			     g_strdup_printf ("%4.2f", gsb_account_get_reconcile_balance (compte_courant )));
	gtk_widget_set_sensitive ( GTK_WIDGET ( entree_ancien_solde_equilibrage ),
				   FALSE );
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_ancienne_date_equilibrage ),
			     _("None") );

	date = gdate_today();

	gtk_entry_set_text ( GTK_ENTRY ( entree_ancien_solde_equilibrage ),
			     g_strdup_printf ("%4.2f", gsb_account_get_init_balance (compte_courant) ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( entree_ancien_solde_equilibrage ),
				   TRUE );
    }



    gtk_entry_set_text ( GTK_ENTRY ( entree_nouvelle_date_equilibrage ),
			 g_strdup_printf ( "%02d/%02d/%d",
					   g_date_day ( date ),
					   g_date_month ( date ),
					   g_date_year ( date ) ));



    gtk_entry_set_text ( GTK_ENTRY ( entree_nouveau_montant_equilibrage ),
			 "" );

    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_compte ),
			 gsb_account_get_name (compte_courant) );


    etat.equilibrage = 1;

    if ( solde_final - solde_initial - operations_pointees )
	gtk_widget_set_sensitive ( bouton_ok_equilibrage,
				   FALSE );
    else
	gtk_widget_set_sensitive ( bouton_ok_equilibrage,
				   TRUE );


    /* affiche la liste en opé simplifiées sans R*/

    /*     déjà on passe en choix par compte pour éviter de refaire toutes les listes */

    ancien_retient_affichage_par_compte = etat.retient_affichage_par_compte;
    etat.retient_affichage_par_compte = 1;

    /*     on passe en non affichage des R */

    ancien_r_modifiable = gsb_account_get_r (compte_courant);
    gsb_account_set_r (compte_courant,
		       FALSE );
    mise_a_jour_affichage_r ( 0 );

    /*     on affiche les opés sur 1 ligne */

    ancien_nb_lignes_ope = gsb_account_get_nb_rows ( compte_courant );
    gsb_account_set_nb_rows ( compte_courant, 
			      1 );
    mise_a_jour_affichage_lignes ( 1 );

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			    2 );

}
/******************************************************************************/

/******************************************************************************/
gboolean sortie_entree_date_equilibrage ( GtkWidget *entree )
{
    /* si l'entrée contenant la date est vide, alors on met la date du jour */

    if ( strlen ( g_strstrip ( (gchar*) gtk_entry_get_text ( GTK_ENTRY ( entree )))) == 0  )
	gtk_entry_set_text ( GTK_ENTRY ( entree ), gsb_today() );

    format_date ( entree );
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean modif_entree_solde_init_equilibrage ( void )
{

    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_initial ),
			 (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_ancien_solde_equilibrage )) );

    solde_initial = my_strtod ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_ancien_solde_equilibrage )),
				NULL );


    if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       0.0 ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   TRUE );
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       solde_final - solde_initial - operations_pointees ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   FALSE );
    }

    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean modif_entree_solde_final_equilibrage ( void )
{
    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_final ),
			 (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_nouveau_montant_equilibrage )) );

    /*     s'il n'y a rien dans l'entrée du montant de l'eq, on efface, l'écart */

    if ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( entree_nouveau_montant_equilibrage ))))
	gtk_widget_show ( label_equilibrage_ecart );
    else
	gtk_widget_hide ( label_equilibrage_ecart );

    solde_final = my_strtod ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_nouveau_montant_equilibrage )),
			      NULL );

    if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       0.0 ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   TRUE );
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       solde_final - solde_initial - operations_pointees ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   FALSE );
    }

    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
/* on annule l'équilibrage */
/******************************************************************************/
gboolean annuler_equilibrage ( void )
{
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			    0 );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    etat.equilibrage = 0;

    /*     on restaure la config de l'utilisateur */

    gsb_account_set_nb_rows ( compte_courant, 
			      ancien_nb_lignes_ope );
    mise_a_jour_affichage_lignes ( ancien_nb_lignes_ope );

    gsb_account_set_r (compte_courant,
		       ancien_r_modifiable );
    mise_a_jour_affichage_r ( ancien_r_modifiable );

    etat.retient_affichage_par_compte = ancien_retient_affichage_par_compte;


    gtk_widget_show ( label_proprietes_operations_compte );
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
/* fonction appelée quand il y a un click dans la colonne des P, et si l'équilibrage */
/* est en cours */
/******************************************************************************/
void pointe_equilibrage ( int p_ligne )
{
    struct structure_operation *operation;
    gdouble montant;
    GtkTreeIter iter;
    gint col;

    col = find_p_r_col ();
    if ( col == -1 )
	return;

    operation = cherche_operation_from_ligne ( p_ligne,
					       compte_courant );

    if ( operation == GINT_TO_POINTER ( -1 )
	 ||
	 operation -> pointe == 3 )
	return;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    montant = calcule_montant_devise_renvoi ( operation -> montant,
					      gsb_account_get_currency (compte_courant),
					      operation -> devise,
					      operation -> une_devise_compte_egale_x_devise_ope,
					      operation -> taux_change,
					      operation -> frais_change );

    gtk_tree_model_get_iter_from_string ( GTK_TREE_MODEL ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view (compte_courant) ))),
					  &iter,
					  itoa ( p_ligne ));

    if ( operation -> pointe )
    {
	operations_pointees = operations_pointees - montant;
	gsb_account_set_marked_balance ( compte_courant,
					 gsb_account_get_marked_balance (compte_courant) - montant );

	operation -> pointe = 0;

	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view (compte_courant) ))),
			     &iter,
			     col, NULL,
			     -1 );
    }
    else
    {
	operations_pointees = operations_pointees + montant;
	gsb_account_set_marked_balance ( compte_courant,
					 gsb_account_get_marked_balance (compte_courant) + montant );

	operation -> pointe = 1;
	
	gtk_list_store_set ( GTK_LIST_STORE ( gtk_tree_view_get_model ( GTK_TREE_VIEW ( gsb_account_get_tree_view (compte_courant) ))),
			     &iter,
			     col, _("P"),
			     -1 );
    }


    /* si c'est une opération ventilée, on recherche les opérations filles
       pour leur mettre le même pointage que la mère */

    if ( operation -> operation_ventilee )
    {
	/* p_tab est déjà pointé sur le compte courant */

	GSList *liste_tmp;

	liste_tmp = gsb_account_get_transactions_list (compte_courant);

	while ( liste_tmp )
	{
	    struct structure_operation *ope_fille;

	    ope_fille = liste_tmp -> data;

	    if ( ope_fille -> no_operation_ventilee_associee == operation -> no_operation )
		ope_fille -> pointe = operation -> pointe;

	    liste_tmp = liste_tmp -> next;
	}
    }


    gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
			 g_strdup_printf ("%4.2f",
					  operations_pointees ));

    if ( fabs ( solde_final - solde_initial - operations_pointees ) < 0.01 )
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       0.0 ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   TRUE );
    }
    else
    {
	gtk_label_set_text ( GTK_LABEL ( label_equilibrage_ecart ),
			     g_strdup_printf ( "%4.2f",
					       solde_final - solde_initial - operations_pointees ));
	gtk_widget_set_sensitive ( GTK_WIDGET ( bouton_ok_equilibrage ),
				   FALSE );
    }

    mise_a_jour_labels_soldes ();
    modification_fichier( TRUE );
}
/******************************************************************************/

/******************************************************************************/
gboolean fin_equilibrage ( GtkWidget *bouton_ok,
		       gpointer data )
{
    GSList *pointeur_liste_ope;
    gchar *text;
    gint nb_parametres;
    GDate *date;
    gint date_releve_jour;
    gint date_releve_mois;
    gint date_releve_annee;


    if ( fabs ( solde_final - solde_initial - operations_pointees ) >= 0.01 )
    {
	dialogue_warning_hint ( _("There is a variance in balances, check that both final balance and initial balance minus marked transactions are equal."),
				_("Reconciliation can't be completed.") );
	return FALSE;
    }


    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;


    /* récupération de la date */

    text = (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_nouvelle_date_equilibrage ) );

    if ( ( nb_parametres = sscanf ( text,
				    "%d/%d/%d",
				    &date_releve_jour,
				    &date_releve_mois,
				    &date_releve_annee))
	 != 3 )
    {
	if ( !nb_parametres || nb_parametres == -1 )
	{
	    dialogue_error_hint ( _("Grisbi can't parse date.  It should be of the form 'dd/mm/yyyy'."),
				  g_strdup_printf ( _("Invalid date '%s'"), text ) );
	    return FALSE;
	}

	date = g_date_new ();
	g_date_set_time ( date,
			  time(NULL));

	if ( nb_parametres == 1 )
	    date_releve_mois = g_date_month( date );

	date_releve_annee = g_date_year( date );

    }

    if ( !g_date_valid_dmy ( date_releve_jour,
			     date_releve_mois,
			     date_releve_annee))
    {
	dialogue_error_hint ( _("Date format looks valid but date is valid according to calendar."),
			      g_strdup_printf ( _("Invalid date '%s'"), text ) );
	return FALSE;
	return FALSE;
    }
    gsb_account_set_current_reconcile_date ( compte_courant,
					     g_date_new_dmy ( date_releve_jour,
							      date_releve_mois,
							      date_releve_annee ));

    gtk_label_set_text ( GTK_LABEL ( label_releve ),
			 g_strdup_printf ( _("Last statement: %02d/%02d/%d"), 
					   date_releve_jour,
					   date_releve_mois,
					   date_releve_annee ));


    /*   récupération du no de rapprochement, */
    /*     s'il n'existe pas,on le crée */

    if ( strlen ( g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_no_rapprochement )))))
    {
	struct struct_no_rapprochement *rapprochement;
	gchar *rap_txt;

	rap_txt = g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_no_rapprochement )));
	
	rapprochement = rapprochement_par_nom ( rap_txt );

	if ( rapprochement )
	{
	    /* le rapprochement existe déjà */

	    gsb_account_set_reconcile_last_number ( compte_courant,
						    rapprochement -> no_rapprochement);
	}
	else
	{
	    /* le rapprochement n'existe pas */

	    rapprochement = malloc ( sizeof ( struct struct_no_rapprochement ));
	    rapprochement -> no_rapprochement = g_slist_length ( liste_struct_rapprochements ) + 1;
	    rapprochement -> nom_rapprochement = g_strdup ( rap_txt );

	    liste_struct_rapprochements = g_slist_append ( liste_struct_rapprochements,
							   rapprochement );

	    gsb_account_set_reconcile_last_number ( compte_courant,
						    rapprochement -> no_rapprochement);
	}
    }
    else
	gsb_account_set_reconcile_last_number ( compte_courant,
						0 );

    /* met tous les P à R */

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    pointeur_liste_ope = gsb_account_get_transactions_list (compte_courant);

    while ( pointeur_liste_ope )
    {
	struct structure_operation *operation;

	operation = pointeur_liste_ope -> data;

	if ( operation -> pointe == OPERATION_POINTEE ||
	     operation -> pointe == OPERATION_TELERAPPROCHEE )
	{
	    operation -> pointe = OPERATION_RAPPROCHEE;
	    operation -> no_rapprochement = gsb_account_get_reconcile_last_number (compte_courant);
	}

	pointeur_liste_ope = pointeur_liste_ope -> next;
    }



    /* on réaffiche la liste */

    modification_fichier( TRUE );
    gsb_account_set_reconcile_balance ( compte_courant,
					solde_final );

    /*     on remet tout normal pour les opérations */

    annuler_equilibrage ();

    /* Update account list */
    mise_a_jour_liste_comptes_accueil = 1;
    return FALSE;
}
/******************************************************************************/


/******************************************************************************/
gboolean souris_equilibrage ( GtkWidget *entree,
			      GdkEventButton *event )
{
    GtkWidget *popup_cal;

    if ( event -> type == GDK_2BUTTON_PRESS )
	popup_cal = gsb_calendar_new ( entree );
    return FALSE;
}
/******************************************************************************/

/******************************************************************************/
gboolean clavier_equilibrage ( GtkWidget *widget,
			       GdkEventKey *event )
{
    GtkWidget *popup_cal;

    switch ( event -> keyval )
    {
	case GDK_Return :		/* touches entrée */
	case GDK_KP_Enter :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( event -> state & GDK_CONTROL_MASK ) == GDK_CONTROL_MASK )
		popup_cal = gsb_calendar_new ( widget );
	    break;

	case GDK_plus:		/* touches + */
	case GDK_KP_Add:

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( event -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK || event -> keyval != GDK_KP_Add )
		inc_dec_date ( widget, ONE_DAY );
	    else
		inc_dec_date ( widget, ONE_WEEK );
	    return TRUE;
	    break;

	case GDK_minus:		/* touches - */
	case GDK_KP_Subtract:

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( event -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK || event -> keyval != GDK_KP_Subtract )
		inc_dec_date ( widget, - ONE_DAY );
	    else
		inc_dec_date ( widget, - ONE_WEEK );
	    return TRUE;
	    break;

	case GDK_Page_Up :		/* touche PgUp */
	case GDK_KP_Page_Up :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( event -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		inc_dec_date ( widget, ONE_MONTH );
	    else
		inc_dec_date ( widget, ONE_YEAR );
	    return TRUE;
	    break;

	case GDK_Page_Down :		/* touche PgDown */
	case GDK_KP_Page_Down :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					   "key-press-event");
	    if ( ( event -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
		inc_dec_date ( widget, - ONE_MONTH );
	    else
		inc_dec_date ( widget, - ONE_YEAR );
	    return TRUE;

	    break;

	default:
	    /* Reverting to default handler */
	    return FALSE;
	    break;
    }
    return TRUE;
}
/******************************************************************************/


/** 
 * TODO: document this + move
 */
void fill_reconciliation_tree ()
{
    GtkTreeIter account_iter, payment_method_iter;
    GSList *pUserAccountsList = NULL;

    pUserAccountsList = g_slist_copy ( ordre_comptes );

    do
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + GPOINTER_TO_INT ( pUserAccountsList -> data );

	GSList * liste_tmp;

	gtk_tree_store_append (reconcile_model, &account_iter, NULL);
	gtk_tree_store_set (reconcile_model, &account_iter,
			    RECONCILIATION_NAME_COLUMN, gsb_account_get_name (GPOINTER_TO_INT ( pUserAccountsList -> data )),
			    RECONCILIATION_VISIBLE_COLUMN, TRUE,
			    RECONCILIATION_SORT_COLUMN, ! TRI,
			    RECONCILIATION_SPLIT_NEUTRAL_COLUMN, NEUTRES_INCLUS,
			    RECONCILIATION_ACCOUNT_COLUMN, p_tab_nom_de_compte_variable,
			    RECONCILIATION_TYPE_COLUMN, -1,
			    -1 );

	liste_tmp = LISTE_TRI;

	while ( liste_tmp )
	{
	    struct struct_type_ope * type_ope;

	    type_ope = type_ope_par_no ( abs(GPOINTER_TO_INT(liste_tmp -> data)),
					 GPOINTER_TO_INT ( pUserAccountsList -> data ) );
	    if ( type_ope )
	    {
		gchar * nom;

		gtk_tree_store_append (reconcile_model, &payment_method_iter, 
				       &account_iter);

		if ( (type_ope -> signe_type == 1
		      ||
		      !type_ope -> signe_type)
		     &&
		     NEUTRES_INCLUS 
		     &&
		     GPOINTER_TO_INT(liste_tmp->data) < 0 )
		    nom = g_strconcat ( type_ope -> nom_type, " ( - )", NULL );
		else 
		    if ((type_ope -> signe_type == 2
			 ||
			 ! type_ope -> signe_type)
			&&
			NEUTRES_INCLUS
			&&
			GPOINTER_TO_INT(liste_tmp->data) > 0 )
			nom = g_strconcat ( type_ope -> nom_type, " ( + )", NULL );
		    else
			nom = type_ope -> nom_type;

		gtk_tree_store_set (reconcile_model, &payment_method_iter,
				    RECONCILIATION_NAME_COLUMN, nom,
				    RECONCILIATION_VISIBLE_COLUMN, FALSE,
				    RECONCILIATION_SORT_COLUMN, FALSE,
				    RECONCILIATION_SPLIT_NEUTRAL_COLUMN, FALSE,
				    RECONCILIATION_ACCOUNT_COLUMN, p_tab_nom_de_compte_variable,
				    RECONCILIATION_TYPE_COLUMN, type_ope -> no_type,
				    -1 );
	    }
	    liste_tmp = liste_tmp -> next;
	}

	if ( gtk_tree_model_iter_has_child( GTK_TREE_MODEL(reconcile_model), &account_iter) &&
	     gsb_account_get_sort_type (GPOINTER_TO_INT ( pUserAccountsList -> data )) )
	{
	    GtkTreePath * treepath;
	    treepath = gtk_tree_model_get_path (GTK_TREE_MODEL(reconcile_model), &account_iter);
	    if ( treepath )
	    {
		gtk_tree_view_expand_row ( GTK_TREE_VIEW(reconcile_treeview), treepath, TRUE );
		gtk_tree_path_free ( treepath );
	    }
	}
    }
    while ( (  pUserAccountsList = pUserAccountsList -> next ) );
    g_slist_free ( pUserAccountsList );
}



/**
 * TODO: move + document this
 *
 */
void select_reconciliation_entry ( GtkTreeSelection * tselection, 
				   GtkTreeModel * model )
{
    GtkTreeIter iter;
    GtkTreePath * treepath;
    GValue value_visible = {0, };
    gboolean good;

    good = gtk_tree_selection_get_selected (tselection, NULL, &iter);
    if (good)
	gtk_tree_model_get_value (model, &iter, 
				  RECONCILIATION_VISIBLE_COLUMN, &value_visible);

    if ( good && ! g_value_get_boolean(&value_visible) )
    {
	/* Is there something before? */
	treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(model), &iter );
	gtk_widget_set_sensitive ( button_move_up, 
				   gtk_tree_path_prev(treepath) );
	gtk_tree_path_free ( treepath );

	/* Is there something after? */
	gtk_widget_set_sensitive ( button_move_down, 
				   gtk_tree_model_iter_next (model, &iter) );
    }
    else 
    {
	gtk_widget_set_sensitive ( button_move_up, FALSE );
	gtk_widget_set_sensitive ( button_move_down, FALSE );
    }
}



/** 
 * TODO: document this
 */
void deplacement_type_tri_haut ( GtkWidget * button, gpointer data )
{
    GtkTreePath * treepath;
    gboolean good, visible;
    GtkTreeIter iter, other;
    GSList * elt;
    gint no_type;

    good = gtk_tree_selection_get_selected (reconcile_selection, NULL, &iter);
    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(reconcile_model), &iter, 
			     RECONCILIATION_VISIBLE_COLUMN, &visible,
			     RECONCILIATION_ACCOUNT_COLUMN, &p_tab_nom_de_compte_variable,
			     RECONCILIATION_TYPE_COLUMN, &no_type,
			     -1 );

    if ( good && ! visible )
    {
	treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(reconcile_model), 
					     &iter );

	if ( gtk_tree_path_prev ( treepath ) &&
	     gtk_tree_model_get_iter ( GTK_TREE_MODEL(reconcile_model), 
				       &other, treepath ) )
	{
	    gtk_tree_store_move_before ( GTK_TREE_STORE(reconcile_model), 
					 &iter, &other );
	}
    }

    select_reconciliation_entry ( reconcile_selection, 
				  GTK_TREE_MODEL(reconcile_model) );

    for ( elt = LISTE_TRI ; elt -> next ; elt = elt -> next )
    {
	if ( elt -> next &&
	     GPOINTER_TO_INT(elt -> next -> data) == no_type )
	{
	    LISTE_TRI = g_slist_remove ( LISTE_TRI, (gpointer) no_type );
	    LISTE_TRI = g_slist_insert_before ( LISTE_TRI, elt, (gpointer) no_type );
	    break;
	}
    }  
}



/** 
 * TODO: document this
 */
void deplacement_type_tri_bas ( void )
{
    GtkTreePath * treepath;
    gboolean good, visible;
    GtkTreeIter iter, other;
    GSList * elt;
    gint no_type;

    good = gtk_tree_selection_get_selected (reconcile_selection, NULL, &iter);
    if (good)
	gtk_tree_model_get ( GTK_TREE_MODEL(reconcile_model), &iter, 
			     RECONCILIATION_VISIBLE_COLUMN, &visible,
			     RECONCILIATION_ACCOUNT_COLUMN, &p_tab_nom_de_compte_variable,
			     RECONCILIATION_TYPE_COLUMN, &no_type,
			     -1 );

    if ( good && ! visible )
    {
	treepath = gtk_tree_model_get_path ( GTK_TREE_MODEL(reconcile_model), 
					     &iter );

	gtk_tree_path_next ( treepath ) ;
	if ( gtk_tree_model_get_iter ( GTK_TREE_MODEL(reconcile_model), 
				       &other, treepath ) )
	{
	    gtk_tree_store_move_after ( GTK_TREE_STORE(reconcile_model), 
					&iter, &other );
	}
    }

    select_reconciliation_entry ( reconcile_selection, 
				  GTK_TREE_MODEL(reconcile_model) );

    for ( elt = LISTE_TRI ; elt -> next ; elt = elt -> next )
    {
	if ( elt -> next && ((gint) elt -> data) == no_type )
	{
	    gint ref = ((gint) elt -> next -> data);
	    LISTE_TRI = g_slist_remove ( LISTE_TRI, (gpointer) ref );
	    LISTE_TRI = g_slist_insert_before ( LISTE_TRI, elt, (gpointer) ref );
	    break;
	}
    }  
}




/**
 * TODO: document this
 *
 */
void reconcile_by_date_toggled ( GtkCellRendererToggle *cell, 
				 gchar *path_str, gpointer data )
{
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean toggle;
    gpointer adr_compte;

    treepath = gtk_tree_path_new_from_string ( path_str );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (reconcile_model),
			      &iter, treepath );

    gtk_tree_model_get (GTK_TREE_MODEL(reconcile_model), &iter, 
			RECONCILIATION_SORT_COLUMN, &toggle, 
			RECONCILIATION_ACCOUNT_COLUMN, &adr_compte,
			-1);

    toggle ^= 1;

    /* set new value */
    gtk_tree_store_set (GTK_TREE_STORE (reconcile_model), &iter, 
			RECONCILIATION_SORT_COLUMN, toggle, 
			-1);

    /* Set to 1 (sort by types) if toggle is not selected */

    /* FIXME : remplacer adr compte par le no du compte, voir au niveau du tri_model */
    
    gsb_account_set_sort_type ( adr_compte,
				! toggle );

    if (toggle)
    {
	gtk_tree_view_collapse_row ( GTK_TREE_VIEW(reconcile_treeview), treepath );
    }
    else
    {
	gtk_tree_view_expand_row ( GTK_TREE_VIEW(reconcile_treeview), treepath, FALSE );
    }

    gtk_tree_path_free ( treepath );
}



/**
 * TODO: document this
 *
 */
void reconcile_include_neutral_toggled ( GtkCellRendererToggle *cell, 
					 gchar *path_str, gpointer data )
{
    GSList * liste_tmp;
    GtkTreePath * treepath;
    GtkTreeIter iter;
    gboolean toggle, clear_tree = 0;

    treepath = gtk_tree_path_new_from_string ( path_str );
    gtk_tree_model_get_iter ( GTK_TREE_MODEL (reconcile_model),
			      &iter, treepath );

    gtk_tree_model_get (GTK_TREE_MODEL(reconcile_model), &iter, 
			RECONCILIATION_SPLIT_NEUTRAL_COLUMN, &toggle, 
			RECONCILIATION_ACCOUNT_COLUMN, &p_tab_nom_de_compte_variable,
			-1);

    toggle ^= 1;
    NEUTRES_INCLUS = toggle;

    /* set new value */
    gtk_tree_store_set (GTK_TREE_STORE (reconcile_model), &iter, 
			RECONCILIATION_SPLIT_NEUTRAL_COLUMN, toggle, 
			-1);

    if ( toggle )
    {
	liste_tmp = LISTE_TRI;

	while ( liste_tmp )
	{
	    struct struct_type_ope *type_ope = NULL;

	    if ( GPOINTER_TO_INT ( liste_tmp->data ) > 0 )
	    {
		type_ope = type_ope_par_no ( GPOINTER_TO_INT ( liste_tmp -> data ),
					     compte_courant );

		if ( type_ope && !type_ope->signe_type )
		{
		    LISTE_TRI = g_slist_append ( LISTE_TRI,
						 GINT_TO_POINTER ( - GPOINTER_TO_INT ( liste_tmp->data )));

		    clear_tree = 1;
		}
	    }
	    liste_tmp = liste_tmp -> next;
	}
    }
    else
    {
	/* on efface tous les nombres négatifs de la liste */

	liste_tmp = LISTE_TRI;

	while ( liste_tmp )
	{
	    if ( GPOINTER_TO_INT ( liste_tmp->data ) < 0 )
	    {
		LISTE_TRI = g_slist_remove ( LISTE_TRI, liste_tmp -> data );
		liste_tmp = LISTE_TRI;
		clear_tree = 1;
	    }
	    else
		liste_tmp = liste_tmp -> next;
	}
    }

    if ( clear_tree )
    {
	gtk_tree_store_clear ( GTK_TREE_STORE(reconcile_model) );
	fill_reconciliation_tree ( );
    } 
}



/**
 * TODO: document this
 *
 */
GtkWidget * tab_display_reconciliation ( void )
{
    GtkWidget *hbox, *scrolled_window;
    GtkWidget *vbox_pref, *paddingbox;
    GtkTreeViewColumn *column;
    GtkCellRenderer *cell;

    vbox_pref = new_vbox_with_title_and_icon ( _("Reconciliation"),
					       "reconciliation.png" );

    paddingbox = new_paddingbox_with_title ( vbox_pref, TRUE,
					     COLON(_("Reconciliation: sort transactions") ) );

    /* la partie du milieu est une hbox avec les types */
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 TRUE, TRUE, 0 );

    /* mise en place de la liste qui contient les types classés */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    reconcile_model = gtk_tree_store_new ( NUM_RECONCILIATION_COLUMNS,
					   G_TYPE_STRING, /* Name */
					   G_TYPE_BOOLEAN, /* Visible */
					   G_TYPE_BOOLEAN, /* Sort by date */
					   G_TYPE_BOOLEAN, /* Split neutrals */
					   G_TYPE_POINTER, /* Account pointer */
					   G_TYPE_INT ); /* type_ope -> no_type */
    reconcile_treeview = gtk_tree_view_new_with_model ( GTK_TREE_MODEL (reconcile_model) );
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (reconcile_treeview), TRUE);
    gtk_tree_selection_set_mode ( gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview)),
				  GTK_SELECTION_SINGLE );
    reconcile_selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (reconcile_treeview));
    g_signal_connect (reconcile_selection, "changed", 
		      G_CALLBACK (select_reconciliation_entry), reconcile_model);

    /* Name */
    cell = gtk_cell_renderer_text_new ( );
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Payment method") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "text", RECONCILIATION_NAME_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Sort by date */
    cell = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_signal_connect (cell, "toggled", 
		      G_CALLBACK (reconcile_by_date_toggled), reconcile_model);
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Sort by date") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", RECONCILIATION_SORT_COLUMN,
					 "activatable", RECONCILIATION_VISIBLE_COLUMN,
					 "visible", RECONCILIATION_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Split neutral payment methods */
    cell = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_toggle_set_radio ( GTK_CELL_RENDERER_TOGGLE(cell), FALSE );
    g_signal_connect (cell, "toggled", 
		      G_CALLBACK (reconcile_include_neutral_toggled), reconcile_model);
    g_object_set (cell, "xalign", 0.5, NULL);
    column = gtk_tree_view_column_new ( );
    gtk_tree_view_column_set_alignment ( column, 0.5 );
    gtk_tree_view_column_pack_end ( column, cell, TRUE );
    gtk_tree_view_column_set_title ( column, _("Split neutral payment methods") );
    gtk_tree_view_column_set_attributes (column, cell,
					 "active", RECONCILIATION_SPLIT_NEUTRAL_COLUMN,
					 "activatable", RECONCILIATION_VISIBLE_COLUMN,
					 "visible", RECONCILIATION_VISIBLE_COLUMN,
					 NULL);
    gtk_tree_view_append_column ( GTK_TREE_VIEW(reconcile_treeview), column);

    /* Various remaining settings */
    /*   g_signal_connect (treeview, "realize", G_CALLBACK (gtk_tree_view_expand_all),  */
    /* 		    NULL); */
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					  GTK_SHADOW_IN);
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), reconcile_treeview );

    fill_reconciliation_tree();

    /* on place ici les flèches sur le côté de la liste */
    vbox_fleches_tri = gtk_vbutton_box_new ();
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox_fleches_tri,
			 FALSE, FALSE, 0);

    button_move_up = gtk_button_new_from_stock (GTK_STOCK_GO_UP);
    gtk_button_set_relief ( GTK_BUTTON ( button_move_up ), GTK_RELIEF_NONE );
    g_signal_connect ( GTK_OBJECT ( button_move_up ), "clicked",
		       (GCallback) deplacement_type_tri_haut, NULL );
    gtk_container_add ( GTK_CONTAINER ( vbox_fleches_tri ), button_move_up );
    gtk_widget_set_sensitive ( button_move_up, FALSE );

    button_move_down = gtk_button_new_from_stock (GTK_STOCK_GO_DOWN);
    gtk_button_set_relief ( GTK_BUTTON ( button_move_down ), GTK_RELIEF_NONE );
    g_signal_connect ( GTK_OBJECT ( button_move_down ), "clicked",
		       (GCallback) deplacement_type_tri_bas, NULL);
    gtk_container_add ( GTK_CONTAINER ( vbox_fleches_tri ), button_move_down );
    gtk_widget_set_sensitive ( button_move_down, FALSE );

    if ( !nb_comptes )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
