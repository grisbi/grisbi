/* ************************************************************************** */
/* Fichier qui permet l'équilibrage des comptes                               */
/* 			equilibrage.c                                         */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003 Benjamin Drieu (bdrieu@april.org)		      */
/*			2004 Alain Portal (dionysos@grisbi.org) 	      */
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
#include "structures.h"
#include "variables-extern.c"
#include "accueil.h"
#include "barre_outils.h"
#include "devises.h"
#include "equilibrage.h"
#include "erreur.h"
#include "fichiers_io.h"
#include "operations_liste.h"
#include "parametres.h"
#include "traitement_variables.h"
#include "type_operations.h"
#include "operations_classement.h"
#include "calendar.h"
#include "constants.h"


enum reconciliation_columns {
  RECONCILIATION_NAME_COLUMN = 0,
  RECONCILIATION_VISIBLE_COLUMN,
  RECONCILIATION_SORT_COLUMN,
  RECONCILIATION_SPLIT_NEUTRAL_COLUMN,
  RECONCILIATION_ACCOUNT_COLUMN,
  RECONCILIATION_TYPE_COLUMN,
  NUM_RECONCILIATION_COLUMNS,
};


GtkWidget * reconcile_treeview;
GtkTreeStore *reconcile_model;
GtkWidget * button_move_up, * button_move_down;
GtkTreeSelection * reconcile_selection;


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
  gtk_widget_set_usize ( entree_no_rapprochement,
			 100, FALSE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tips ),
			 entree_no_rapprochement,
			 _("If reconciliation reference ends in a digit, it is automatically incremented at each reconciliation."),
			 _("Reconciliation reference") );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       entree_no_rapprochement,
		       FALSE,
		       FALSE,
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
  gtk_widget_show ( label_equilibrage_ecart );



/* on met les boutons */


  hbox = gtk_hbox_new ( TRUE,
			5);
  gtk_box_pack_end ( GTK_BOX ( fenetre_equilibrage ),
		       hbox,
		       FALSE,
		       FALSE,
		       0);
  gtk_widget_show ( hbox );
  
  bouton_ok_equilibrage = gtk_button_new_with_label (_("Valid") );
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

  
  bouton = gtk_button_new_with_label (_("Cancel") );
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

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  if ( !NB_OPE_COMPTE )
    {
      dialogue_error ( _("This account does not contain any transaction") );
      return;
    }


  /* efface le label propriétés du compte */

  gtk_widget_hide ( label_proprietes_operations_compte );

  /* calcule le montant des opérations pointées */

  calcule_total_pointe_compte ( compte_courant );


  /* récupère l'ancien no de rapprochement et essaie d'incrémenter la partie
     numérique. Si ne réussit pas, remet juste le nom de l'ancien */

  if ( DERNIER_NO_RAPPROCHEMENT )
    {
      GSList *liste_tmp;

      liste_tmp = g_slist_find_custom ( liste_no_rapprochements,
					GINT_TO_POINTER ( DERNIER_NO_RAPPROCHEMENT ),
					(GCompareFunc) recherche_no_rapprochement_par_no );

      if ( liste_tmp )
	{
	  struct struct_no_rapprochement *rapprochement;
	  gchar *pointeur_mobile;
	  gchar *pointeur_fin;
	  gchar *new_rap;

	  rapprochement = liste_tmp -> data;


	  /* on va créer une nouvelle chaine contenant la partie numérique */

	  new_rap = g_strdup ( rapprochement -> nom_rapprochement );
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

	      partie_num = itoa ( atoi ( partie_num ) + 1 );

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
	  else
	    new_rap = rapprochement -> nom_rapprochement;

	  gtk_entry_set_text ( GTK_ENTRY ( entree_no_rapprochement ),
			       new_rap );

	}
    }
  else
    gtk_entry_set_text ( GTK_ENTRY ( entree_no_rapprochement ),
			 "" );

  /* récupère l'ancienne date et l'augmente d'1 mois et le met dans entree_nouvelle_date_equilibrage */

  if ( DATE_DERNIER_RELEVE )
    {
      date = g_date_new_dmy ( g_date_day ( DATE_DERNIER_RELEVE ),
			      g_date_month ( DATE_DERNIER_RELEVE ),
			      g_date_year ( DATE_DERNIER_RELEVE ));

      gtk_label_set_text ( GTK_LABEL ( label_ancienne_date_equilibrage ),
			   g_strdup_printf ( "%02d/%02d/%d",
					     g_date_day ( date ),
					     g_date_month ( date ),
					     g_date_year ( date ) ));
      g_date_add_months ( date, 1 );

      gtk_entry_set_text ( GTK_ENTRY ( entree_ancien_solde_equilibrage ),
			   g_strdup_printf ("%4.2f", SOLDE_DERNIER_RELEVE ));
      gtk_widget_set_sensitive ( GTK_WIDGET ( entree_ancien_solde_equilibrage ),
				 FALSE );
    }
  else
    {
      time_t today;

      gtk_label_set_text ( GTK_LABEL ( label_ancienne_date_equilibrage ),
			   _("None") );

      time ( &today );
      date = g_date_new_dmy ( gmtime ( &today ) -> tm_mday,
			      gmtime ( &today ) -> tm_mon + 1,
			      gmtime ( &today ) -> tm_year + 1900 );

      gtk_entry_set_text ( GTK_ENTRY ( entree_ancien_solde_equilibrage ),
			   g_strdup_printf ("%4.2f", SOLDE_INIT ));
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
		       NOM_DU_COMPTE );


  etat.equilibrage = 1;

  if ( solde_final - solde_initial - operations_pointees )
    gtk_widget_set_sensitive ( bouton_ok_equilibrage,
			       FALSE );
  else
    gtk_widget_set_sensitive ( bouton_ok_equilibrage,
			       TRUE );


  /* affiche la liste en opé simplifiées */

  ancien_nb_lignes_ope = NB_LIGNES_OPE;

  if ( NB_LIGNES_OPE != 1 )
    gtk_button_clicked ( GTK_BUTTON ( bouton_ope_1_lignes ));

  /* on vire les opérations rapprochées */

  etat.valeur_r_avant_rapprochement = AFFICHAGE_R;

  change_aspect_liste ( NULL,
			3 );


  /* classe la liste des opés en fonction des types ou non */

  if ( TRI && LISTE_TRI )
    LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				      (GCompareFunc) classement_sliste );


  remplissage_liste_operations ( compte_courant );

  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			  2 );


  /* la liste des opé prend le focus */

  gtk_widget_grab_focus ( GTK_WIDGET ( CLIST_OPERATIONS ));
}
/******************************************************************************/

/******************************************************************************/
void sortie_entree_date_equilibrage ( GtkWidget *entree )
{
  format_date ( entree );
}
/******************************************************************************/

/******************************************************************************/
void modif_entree_solde_init_equilibrage ( void )
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

}
/******************************************************************************/

/******************************************************************************/
void modif_entree_solde_final_equilibrage ( void )
{
  gtk_label_set_text ( GTK_LABEL ( label_equilibrage_final ),
		       (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_nouveau_montant_equilibrage )) );

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

}
/******************************************************************************/

/******************************************************************************/
/* on annule l'équilibrage */
/******************************************************************************/
void annuler_equilibrage ( GtkWidget *bouton_ann,
			   gpointer data)
{
  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			  0 );

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  etat.equilibrage = 0;

  if ( ancien_nb_lignes_ope != NB_LIGNES_OPE )
    {
      if ( ancien_nb_lignes_ope == 4 )
	gtk_button_clicked ( GTK_BUTTON ( bouton_ope_4_lignes ));
      else
	{
	  if ( ancien_nb_lignes_ope == 3 )
	    gtk_button_clicked ( GTK_BUTTON ( bouton_ope_3_lignes ));
	  else
	    gtk_button_clicked ( GTK_BUTTON ( bouton_ope_2_lignes ));
	}
    }
	

  if ( etat.valeur_r_avant_rapprochement )
    change_aspect_liste ( NULL,
			  2 );

  LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				    (GCompareFunc) classement_sliste );


/*   gtk_clist_set_compare_func ( GTK_CLIST ( CLIST_OPERATIONS ), */
/* 			       (GtkCListCompareFunc) classement_liste_par_date ); */

  remplissage_liste_operations ( compte_courant );

  gtk_widget_show ( label_proprietes_operations_compte );

  focus_a_la_liste ();
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

  operation = gtk_clist_get_row_data ( GTK_CLIST ( CLIST_OPERATIONS ),
				       p_ligne );


  if ( operation == GINT_TO_POINTER ( -1 ) )
    return;


  if ( operation -> pointe == 2 )
    return;


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;


  montant = calcule_montant_devise_renvoi ( operation -> montant,
					    DEVISE,
					    operation -> devise,
					    operation -> une_devise_compte_egale_x_devise_ope,
					    operation -> taux_change,
					    operation -> frais_change );

  if ( operation -> pointe )
    {
      operations_pointees = operations_pointees - montant;
      SOLDE_POINTE = SOLDE_POINTE - montant;

      operation -> pointe = 0;

      gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			   p_ligne,
			   3,
			   " ");
    }
  else
    {
      operations_pointees = operations_pointees + montant;
      SOLDE_POINTE = SOLDE_POINTE + montant;

      operation -> pointe = 1;

      gtk_clist_set_text ( GTK_CLIST ( CLIST_OPERATIONS ),
			   p_ligne,
			   3,
			   _("P"));
    }

    
  /* si c'est une opération ventilée, on recherche les opérations filles
     pour leur mettre le même pointage que la mère */

  if ( operation -> operation_ventilee )
    {
      /* p_tab est déjà pointé sur le compte courant */
      
      GSList *liste_tmp;

      liste_tmp = LISTE_OPERATIONS;

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

  /* met le label du solde pointé */

  gtk_label_set_text ( GTK_LABEL ( solde_label_pointe ),
		       g_strdup_printf ( _("Checked balance: %4.2f %s"),
					 SOLDE_POINTE,
					 devise_name ((struct struct_devise *)(g_slist_find_custom ( liste_struct_devises,
											 GINT_TO_POINTER ( DEVISE ),
											 (GCompareFunc) recherche_devise_par_no )-> data ))) );

  modification_fichier( TRUE );
}
/******************************************************************************/

/******************************************************************************/
void fin_equilibrage ( GtkWidget *bouton_ok,
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
      dialogue ( _("There is a variance"));
      return;
    }


  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;


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
	  dialogue_error ( _("Invalid date") );
	  return;
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
      dialogue_error ( _("Invalid date") );
      return;
    }

  DATE_DERNIER_RELEVE = g_date_new_dmy ( date_releve_jour,
					 date_releve_mois,
					 date_releve_annee );

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
      GSList *liste_tmp;
      gchar *rap_txt;

      rap_txt = g_strstrip ( (char *) gtk_entry_get_text ( GTK_ENTRY ( entree_no_rapprochement )));

      liste_tmp = g_slist_find_custom ( liste_no_rapprochements,
					rap_txt,
					(GCompareFunc) recherche_no_rapprochement_par_nom );

      if ( liste_tmp )
	{
	  /* le rapprochement existe déjà */

	  rapprochement = liste_tmp -> data;
	  DERNIER_NO_RAPPROCHEMENT = rapprochement -> no_rapprochement;
	}
      else
	{
	  /* le rapprochement n'existe pas */

	  rapprochement = malloc ( sizeof ( struct struct_no_rapprochement ));
	  rapprochement -> no_rapprochement = g_slist_length ( liste_no_rapprochements ) + 1;
	  rapprochement -> nom_rapprochement = g_strdup ( rap_txt );

	  liste_no_rapprochements = g_slist_append ( liste_no_rapprochements,
						     rapprochement );

	  DERNIER_NO_RAPPROCHEMENT = rapprochement -> no_rapprochement;
	}
    }
  else
    DERNIER_NO_RAPPROCHEMENT = 0;



/* met tous les P à R */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  pointeur_liste_ope = LISTE_OPERATIONS;

  while ( pointeur_liste_ope )
    {
      struct structure_operation *operation;

      operation = pointeur_liste_ope -> data;

      if ( operation -> pointe == 1 )
	{
	  operation -> pointe = 2;
	  operation -> no_rapprochement = DERNIER_NO_RAPPROCHEMENT;
	}

      pointeur_liste_ope = pointeur_liste_ope -> next;
    }



/* on réaffiche la liste */

  modification_fichier( TRUE );

  SOLDE_DERNIER_RELEVE = solde_final;


  gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_comptes_equilibrage ),
			  0 );

  etat.equilibrage = 0;

  if ( ancien_nb_lignes_ope != NB_LIGNES_OPE )
    {
      if ( ancien_nb_lignes_ope == 4 )
	gtk_button_clicked ( GTK_BUTTON ( bouton_ope_4_lignes ));
      else
	{
	  if ( ancien_nb_lignes_ope == 3 )
	    gtk_button_clicked ( GTK_BUTTON ( bouton_ope_3_lignes ));
	  else
	    gtk_button_clicked ( GTK_BUTTON ( bouton_ope_2_lignes ));
	}
    }
	
  if ( etat.valeur_r_avant_rapprochement )
    change_aspect_liste ( NULL,
			  2 );

  LISTE_OPERATIONS = g_slist_sort ( LISTE_OPERATIONS,
				    (GCompareFunc) classement_sliste );

/*   gtk_clist_set_compare_func ( GTK_CLIST ( CLIST_OPERATIONS ), */
/* 			       (GtkCListCompareFunc) classement_liste_par_date ); */


  remplissage_liste_operations ( compte_courant );

  gtk_widget_show ( label_proprietes_operations_compte );

/*   on redonne le focus à la liste */

  gtk_widget_grab_focus ( GTK_WIDGET ( CLIST_OPERATIONS ) );

  /* Update account list */
  update_liste_comptes_accueil ();
}
/******************************************************************************/

/******************************************************************************/
gint recherche_no_rapprochement_par_nom ( struct struct_no_rapprochement *rapprochement,
					  gchar *no_rap )
{
  return ( strcmp ( rapprochement -> nom_rapprochement,
		    no_rap ));
}
/******************************************************************************/

/******************************************************************************/
gint recherche_no_rapprochement_par_no ( struct struct_no_rapprochement *rapprochement,
					 gint *no_rap )
{

  return ( !(rapprochement -> no_rapprochement == GPOINTER_TO_INT ( no_rap )));

}
/******************************************************************************/

/******************************************************************************/
void calcule_total_pointe_compte ( gint no_compte )
{
  GSList *pointeur_liste_ope;

  p_tab_nom_de_compte_variable =  p_tab_nom_de_compte + no_compte;

  pointeur_liste_ope = LISTE_OPERATIONS;
  operations_pointees = 0;

  while ( pointeur_liste_ope )
    {
      struct structure_operation *operation;

      operation = pointeur_liste_ope -> data;

      /* on ne prend en compte l'opé que si c'est pas une opé de ventil */

      if ( operation -> pointe == 1
	   &&
	   !operation -> no_operation_ventilee_associee )
	{
	  gdouble montant;

	  montant = calcule_montant_devise_renvoi ( operation -> montant,
						    DEVISE,
						    operation -> devise,
						    operation -> une_devise_compte_egale_x_devise_ope,
						    operation -> taux_change,
						    operation -> frais_change );

	  operations_pointees = operations_pointees + montant;
	}

      pointeur_liste_ope = pointeur_liste_ope -> next;
    }

  gtk_label_set_text ( GTK_LABEL ( label_equilibrage_pointe ),
		       g_strdup_printf ( "%4.2f", 
					 operations_pointees ));
}
/******************************************************************************/

/******************************************************************************/
void souris_equilibrage ( GtkWidget *entree,
			  GdkEventButton *event )
{
  GtkWidget *popup_cal;
      
  if ( event -> type == GDK_2BUTTON_PRESS )
    popup_cal = gsb_calendar_new ( entree );
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
	  if ( ( event -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
	    inc_dec_date ( widget, ONE_DAY );
	  else
	    inc_dec_date ( widget, ONE_WEEK );
	  return TRUE;
      break;

    case GDK_minus:		/* touches - */
    case GDK_KP_Subtract:

	  gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
					 "key-press-event");
	  if ( ( event -> state & GDK_SHIFT_MASK ) != GDK_SHIFT_MASK )
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
  gint i;

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
  
  for ( i=0 ; i<nb_comptes ; i++ )
    {
      GSList * liste_tmp;

      gtk_tree_store_append (reconcile_model, &account_iter, NULL);
      gtk_tree_store_set (reconcile_model, &account_iter,
			  RECONCILIATION_NAME_COLUMN, NOM_DU_COMPTE,
			  RECONCILIATION_VISIBLE_COLUMN, TRUE,
			  RECONCILIATION_SORT_COLUMN, ! TRI,
			  RECONCILIATION_SPLIT_NEUTRAL_COLUMN, NEUTRES_INCLUS,
			  RECONCILIATION_ACCOUNT_COLUMN, p_tab_nom_de_compte_variable,
			  RECONCILIATION_TYPE_COLUMN, -1,
			  -1 );

      liste_tmp = LISTE_TRI;

      while ( liste_tmp )
	{
	  struct struct_type_ope * type_ope = NULL;
	  GSList * result;
	  
	  if ( TYPES_OPES )
	    result = g_slist_find_custom ( TYPES_OPES,
					   (gpointer) abs(GPOINTER_TO_INT(liste_tmp -> data)),
					   (GCompareFunc) recherche_type_ope_par_no);
	  if ( result )
	    {
	      gchar * nom;

	      type_ope = result -> data;
	      gtk_tree_store_append (reconcile_model, &payment_method_iter, 
				     &account_iter);
	      
	      if ( type_ope -> signe_type == 1 ||
		   ! type_ope -> signe_type && NEUTRES_INCLUS &&
		   GPOINTER_TO_INT(liste_tmp->data) < 0 )
		nom = g_strconcat ( type_ope -> nom_type, " ( - )", NULL );
	      else if (type_ope -> signe_type == 2 ||
		       ! type_ope -> signe_type && NEUTRES_INCLUS &&
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
	   TRI )
	{
	  GtkTreePath * treepath;
	  treepath = gtk_tree_model_get_path (GTK_TREE_MODEL(reconcile_model), &account_iter);
	  if ( treepath )
	    {
	      gtk_tree_view_expand_row ( GTK_TREE_VIEW(reconcile_treeview), treepath, TRUE );
	      gtk_tree_path_free ( treepath );
	    }
	}
  
      p_tab_nom_de_compte_variable++;
    }
}



/**
 * TODO: move + document this
 *
 */
void select_reconciliation_entry ( GtkTreeSelection * tselection, 
				   GtkTreeModel * model )
{
  GtkTreeIter iter, other;
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



/** TODO: remove this  */
void save_ordre_liste_type_tri ( void )
{
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
  gboolean toggle, good;

  treepath = gtk_tree_path_new_from_string ( path_str );
  gtk_tree_model_get_iter ( GTK_TREE_MODEL (reconcile_model),
			    &iter, treepath );

  gtk_tree_model_get (GTK_TREE_MODEL(reconcile_model), &iter, 
		      RECONCILIATION_SORT_COLUMN, &toggle, 
		      RECONCILIATION_ACCOUNT_COLUMN, &p_tab_nom_de_compte_variable,
		      -1);

  toggle ^= 1;

  /* set new value */
  gtk_tree_store_set (GTK_TREE_STORE (reconcile_model), &iter, 
		      RECONCILIATION_SORT_COLUMN, toggle, 
		      -1);
  TRI = ! toggle;  /* Set to 1 (sort by types) if toggle is not selected */

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
  GtkTreeIter iter, operation;
  gboolean toggle, good, clear_tree = 0;
  struct struct_type_ope * type_ope = NULL;
 
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
	  struct struct_type_ope *type_ope;

	  if ( GPOINTER_TO_INT ( liste_tmp->data ) > 0 )
	    {
	      GSList * result = g_slist_find_custom ( TYPES_OPES,
						      liste_tmp->data,
						      (GCompareFunc) recherche_type_ope_par_no );
	      if (result)
		type_ope = result->data;
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
  GtkWidget *onglet, *hbox, *frame, *scrolled_window, *vbox, *hbox2;
  GtkWidget *menu, *item, *label, *vbox_pref, *paddingbox;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;
  gchar *titres[2] = { _("Accounts"),
		       _("Default") };
  gint i;

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

  return vbox_pref;
}



