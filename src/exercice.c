/* Ce fichier s'occupe de la gestion des exercices */
/* exercice.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003 Benjamin Drieu (bdrieu@april.org) */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#include "include.h"
#include "operations_formulaire_constants.h"
#include "echeancier_formulaire_constants.h"
#include "ventilation_constants.h"


/*START_INCLUDE*/
#include "exercice.h"
#include "utils_buttons.h"
#include "utils_exercices.h"
#include "utils_dates.h"
#include "operations_liste.h"
#include "utils_editables.h"
#include "traitement_variables.h"
#include "utils.h"
#include "dialog.h"
#include "operations_formulaire.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void ajout_exercice ( GtkWidget *bouton,
		      GtkWidget *clist );
static void association_automatique ( void );
static void deselection_ligne_exercice ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev);
static void selection_ligne_exercice ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev );
static void supprime_exercice ( GtkWidget *bouton, GtkWidget *liste );
static gboolean update_financial_year_list ( GtkEntry *entry, gchar *value, 
				      gint length, gint * position );
static gboolean update_financial_year_menus ();
/*END_STATIC*/



GtkWidget *paddingbox_details;	/** Widget handling financial year details */
GSList *liste_struct_exercices;
gint no_derniere_exercice;
gint nb_exercices;
GtkWidget *clist_exercices_parametres;
GtkWidget *bouton_supprimer_exercice;
GtkWidget *nom_exercice;
GtkWidget *debut_exercice;
GtkWidget *fin_exercice;
GtkWidget *affichage_exercice;
gint ligne_selection_exercice;




/*START_EXTERN*/
extern GtkWidget *fenetre_preferences;
extern GtkWidget *formulaire;
extern gint nb_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_TOTAL_WIDGET];
/*END_EXTERN*/




/** 
 * Update financial year name in list.  Normally called as a signal
 * handler.
 *
 * \param entry Widget that triggered signal
 * \param value Handler parameter, not used
 * \param length Handler parameter, not used
 * \param position Handler parameter, not used
 */
gboolean update_financial_year_list ( GtkEntry *entry, gchar *value, 
				      gint length, gint * position )
{
    struct struct_exercice *exercice;

    exercice = gtk_clist_get_row_data ( GTK_CLIST ( clist_exercices_parametres ),
					ligne_selection_exercice );
    /*   exercice -> nom_exercice = gtk_entry_get_text ( GTK_ENTRY (nom_exercice) ); */
    gtk_clist_set_text ( GTK_CLIST(clist_exercices_parametres), 
			 ligne_selection_exercice, 0, 
			 gtk_entry_get_text ( GTK_ENTRY(nom_exercice)) );

    /* TODO: perhaps this should not be hooked there */
    update_financial_year_menus ();

    return FALSE;
}



/**
 * Rebuild financial years menus in various forms
 */
gboolean update_financial_year_menus ()
{
    if ( widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE) &&
	 GTK_OPTION_MENU(widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE)) -> menu )
    {
	gtk_widget_destroy ( GTK_OPTION_MENU(widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE)) -> menu );
	gtk_option_menu_set_menu ( GTK_OPTION_MENU (widget_formulaire_par_element (TRANSACTION_FORM_EXERCICE)),
				   creation_menu_exercices (0) );
    }

    if ( widget_formulaire_par_element (TRANSACTION_FORM_DEVISE) &&
	 GTK_OPTION_MENU(widget_formulaire_par_element (TRANSACTION_FORM_DEVISE)) -> menu )
    {
	gtk_widget_destroy ( GTK_OPTION_MENU(widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA]) -> menu );
	gtk_option_menu_set_menu ( GTK_OPTION_MENU(widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_CONTRA]),
				   creation_menu_exercices (0) );
    }

    if ( widget_formulaire_par_element (TRANSACTION_FORM_TYPE) &&
	 GTK_OPTION_MENU(widget_formulaire_par_element (TRANSACTION_FORM_TYPE)) -> menu )
    {
	gtk_widget_destroy ( GTK_OPTION_MENU(widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]) -> menu );
	gtk_option_menu_set_menu ( GTK_OPTION_MENU(widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE]),
				   creation_menu_exercices (1) );
    }

    return FALSE;
}



/**
 * Creates the "Financial years" tab.  It creates a financial years
 * list and then a form that allows to edit selected financial year.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_exercices ( void )
{
    GtkWidget *vbox_pref, *label;
    GtkWidget *scrolled_window, *vbox, *bouton, *hbox;
    GtkWidget *paddingbox, *table;
    GSList *liste_tmp;
    gchar *titres[]={_("Name")};

    vbox_pref = new_vbox_with_title_and_icon ( _("Financial years"),
					       "financial-years.png" );
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Known financial years") );
    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    /* Create financial years list */
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			 TRUE, TRUE, 0);
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_widget_set_usize ( GTK_WIDGET ( scrolled_window ), FALSE, 120 );
    clist_exercices_parametres = gtk_clist_new_with_titles ( 1, titres );
    gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_exercices_parametres ),
				       0, TRUE );
    gtk_clist_column_titles_passive ( GTK_CLIST ( clist_exercices_parametres ));
    gtk_signal_connect_object  ( GTK_OBJECT ( fenetre_preferences ),
				 "close",
				 GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				 GTK_OBJECT ( clist_exercices_parametres ));
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			clist_exercices_parametres );


    /* Do not activate unless an account is opened */
    if ( !nb_comptes )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }
    else
    {
	liste_tmp = liste_struct_exercices;

	while ( liste_tmp )
	{
	    struct struct_exercice *exercice;
	    gchar *ligne[1];
	    gint ligne_insert;

	    exercice = liste_tmp -> data;

	    ligne[0] = exercice -> nom_exercice;

	    ligne_insert = gtk_clist_append ( GTK_CLIST (clist_exercices_parametres),
					      ligne );

	    /* on associe à la ligne la struct de la exercice */
	    gtk_clist_set_row_data ( GTK_CLIST ( clist_exercices_parametres ),
				     ligne_insert,
				     exercice );

	    liste_tmp = liste_tmp -> next;
	}
    }

    /* Order buttons in a vbox */
    vbox = gtk_vbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			 FALSE, FALSE, 0 );

    /* Add button */
    bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
    gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
			 GTK_SIGNAL_FUNC  ( ajout_exercice ),
			 clist_exercices_parametres );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
			 FALSE, FALSE, 5 );
    /* Remove button */
    bouton_supprimer_exercice = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
    gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_exercice ), "clicked",
			 GTK_SIGNAL_FUNC  ( supprime_exercice ),
			 clist_exercices_parametres );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_exercice,
			 FALSE, FALSE, 5 );
    /* Associate operations */
    bouton = gtk_button_new_with_label ( _("Associate operations without financial years") );
    gtk_signal_connect ( GTK_OBJECT ( bouton ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( association_automatique ),
			 NULL );
    gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
			 FALSE, FALSE, 5 );
    gtk_widget_show ( bouton );

    /* Handle clist selections */
    gtk_signal_connect ( GTK_OBJECT ( clist_exercices_parametres ), "select-row",
			 GTK_SIGNAL_FUNC ( selection_ligne_exercice ),
			 NULL );
    gtk_signal_connect ( GTK_OBJECT ( clist_exercices_parametres ), "unselect-row",
			 GTK_SIGNAL_FUNC ( deselection_ligne_exercice ),
			 NULL );


    /* Financial year details */
    paddingbox_details = new_paddingbox_with_title ( vbox_pref, FALSE,
						     _("Financial year details") );

    /* Put stuff in a table */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 6 );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox_details ), table,
			 FALSE, FALSE, 0 );

    /* Financial year name */
    label = gtk_label_new ( COLON(_("Name")) );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    nom_exercice = new_text_entry ( NULL, (GCallback) update_financial_year_list );
    gtk_table_attach ( GTK_TABLE ( table ),
		       nom_exercice, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Start */
    label = gtk_label_new ( COLON(_("Start")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    debut_exercice = new_date_entry ( NULL, NULL );
    gtk_table_attach ( GTK_TABLE ( table ),
		       debut_exercice, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* End */
    label = gtk_label_new ( COLON(_("End")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    fin_exercice = new_date_entry ( NULL, NULL );
    gtk_table_attach ( GTK_TABLE ( table ),
		       fin_exercice, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Activate in transaction form? */
    affichage_exercice = new_checkbox_with_title (_("Activate in transaction form"),
						  NULL, 
						  G_CALLBACK(update_financial_year_menus));
    gtk_box_pack_start ( GTK_BOX ( paddingbox_details ), affichage_exercice,
			 FALSE, FALSE, 0 );

    gtk_widget_set_sensitive ( paddingbox_details, FALSE );
    gtk_widget_set_sensitive ( bouton_supprimer_exercice, FALSE );

    /* Select first entry if applicable */
    gtk_clist_select_row ( GTK_CLIST(clist_exercices_parametres), 0, 0 );

    return ( vbox_pref );
}
/* ************************************************************************************************************** */



/* ***************************************************************************************************** */
/* Fonction ajout_exercice */
/* appelée par le bouton ajouter de l'onglet exercices des paramètres */
/* ***************************************************************************************************** */

void ajout_exercice ( GtkWidget *bouton,
		      GtkWidget *clist )
{
    struct struct_exercice *exercice;
    gchar *ligne[1];
    gint ligne_insert;

    /* Enlève la sélection de la liste ( ce qui nettoie les entrées ) */
    gtk_clist_unselect_all ( GTK_CLIST ( clist ));

    /* crée un nouvelle exercice au nom de "nouvel exercice" en mettant
       tous les paramètres à 0 et le no à -1 */
    exercice = calloc ( 1, sizeof ( struct struct_exercice ));
    exercice -> no_exercice = -1;
    exercice -> nom_exercice = g_strdup ( _("New financial year") );
    liste_struct_exercices = g_slist_append ( liste_struct_exercices, exercice );

    exercice -> no_exercice = ++no_derniere_exercice;
    nb_exercices++;

    ligne[0] = exercice -> nom_exercice;

    ligne_insert = gtk_clist_append ( GTK_CLIST ( clist ), ligne );

    /* on associe à la ligne la struct de la exercice */
    gtk_clist_set_row_data ( GTK_CLIST ( clist ), ligne_insert, exercice );

    /* on sélectionne le nouveau venu */
    gtk_clist_select_row ( GTK_CLIST ( clist ), ligne_insert, 0 );

    /* on sélectionne le "nouvel exercice" et lui donne le focus */
    gtk_entry_select_region ( GTK_ENTRY ( nom_exercice ), 0, -1 );
    gtk_widget_grab_focus ( nom_exercice );

    /* Update various menus */
    update_financial_year_menus ();
}


/* **************************************************************************************************************************** */
/* Fonction supprime_exercice */
/* appelée lorsqu'on clicke sur le bouton annuler dans les paramètres */
/* **************************************************************************************************************************** */

void supprime_exercice ( GtkWidget *bouton, GtkWidget *liste )
{
    struct struct_exercice *exercice;

    exercice = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
					ligne_selection_exercice );

    gtk_clist_remove ( GTK_CLIST(liste), ligne_selection_exercice );
    liste_struct_exercices = g_slist_remove ( liste_struct_exercices,
					      exercice );
    nb_exercices--;
    free ( exercice );
    if ( ligne_selection_exercice > 0 )
	gtk_clist_select_row ( GTK_CLIST(liste), ligne_selection_exercice-1, 0 );
    else
	gtk_clist_select_row ( GTK_CLIST(liste), 0, 0 );

    /* Update various menus */
    update_financial_year_menus ();
}
/* **************************************************************************************************************************** */



/**
 * Called when a line is selected in the financial years list.
 *
 * \param ligne Line number of selected line
 * \param colonne Column number of selected cell.  Not used.
 * \param ev GdkEventButton that triggered this handler.  Not used
 */
void selection_ligne_exercice ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev )
{
    struct struct_exercice *exercice;

    ligne_selection_exercice = ligne;

    exercice = gtk_clist_get_row_data ( GTK_CLIST ( liste ), ligne );

    entry_set_value ( nom_exercice, &(exercice -> nom_exercice) );
    date_set_value ( debut_exercice, &(exercice -> date_debut), TRUE );
    date_set_value ( fin_exercice, &(exercice -> date_fin), TRUE );
    checkbox_set_value ( affichage_exercice, &(exercice->affiche_dans_formulaire), 
			 TRUE );

    gtk_widget_set_sensitive ( paddingbox_details, TRUE );
    gtk_widget_set_sensitive ( bouton_supprimer_exercice, TRUE );
}



/**
 * Called when a line is deselected in the financial years list.
 *
 * \param ligne Line number of last selected line
 * \param colonne Column number of last selected cell.  Not used.
 * \param ev GdkEventButton that triggered this handler.  Not used
 */
void deselection_ligne_exercice ( GtkWidget *liste,
				  gint ligne,
				  gint colonne,
				  GdkEventButton *ev)
{
    /* Blank all entries */

    entry_set_value ( nom_exercice, NULL );
    entry_set_value ( get_entry_from_date_entry(debut_exercice), NULL );
    entry_set_value ( get_entry_from_date_entry(fin_exercice), NULL );
    checkbox_set_value ( affichage_exercice, FALSE, TRUE );

    gtk_widget_set_sensitive ( paddingbox_details, FALSE );
    gtk_widget_set_sensitive ( bouton_supprimer_exercice, FALSE );
}



//* ************************************************************************************************************ */
/* Fonction creation_menu_exercices */
/* crée un menu qui contient les noms des exercices associés à leur no et adr */
/* et le renvoie */
/* origine = 0 si ça vient des opérations */
/* origine = 1 si ça vient de l'échéancier ; dans ce cas on rajoute automatique */
/* ************************************************************************************************************ */

GtkWidget *creation_menu_exercices ( gint origine )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    GSList *pointeur;

    menu = gtk_menu_new ();
    gtk_widget_show ( menu );


    /* si ça vient de l'échéancier, le 1er est automatique */
    /* on lui associe -2 */

    if ( origine )
    {
	menu_item = gtk_menu_item_new_with_label ( _("Automatic") );
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "adr_exercice",
			      GINT_TO_POINTER (-2));
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "no_exercice",
			      GINT_TO_POINTER (-2));
	gtk_widget_show ( menu_item );
    }

    /* le premier nom est Aucun */
    /* on lui associe 0 */

    menu_item = gtk_menu_item_new_with_label ( _("None") );
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "adr_exercice",
			  NULL );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_exercice",
			  NULL );
    gtk_widget_show ( menu_item );


    /* le second est non affiché */
    /* on lui associe -1 */

    menu_item = gtk_menu_item_new_with_label ( _("Not displayed") );
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "adr_exercice",
			  GINT_TO_POINTER (-1));
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_exercice",
			  GINT_TO_POINTER (-1));
    gtk_widget_show ( menu_item );


    pointeur = liste_struct_exercices;

    while ( pointeur )
    {
	struct struct_exercice *exercice;

	exercice = pointeur -> data;

	if ( exercice->affiche_dans_formulaire )
	{
	    menu_item = gtk_menu_item_new_with_label ( exercice -> nom_exercice );
	    gtk_menu_append ( GTK_MENU ( menu ),
			      menu_item );
	    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				  "adr_exercice",
				  exercice );
	    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
				  "no_exercice",
				  GINT_TO_POINTER ( exercice -> no_exercice ));
	    gtk_widget_show ( menu_item );
	}

	pointeur = pointeur -> next;
    }

    return ( menu );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************** */
/* fonction affiche_exercice_par_date */
/* met automatiquement l'option menu de l'exercice par rapport */
/* à la date dans le formulaire */
/* ************************************************************************************************************** */

void affiche_exercice_par_date ( GtkWidget *entree_date,
				 GtkWidget *option_menu_exercice )
{
    GDate *date_courante;
    gint jour, mois, an;
    GSList *liste_tmp;
    gint trouve;

    if ( !etat.affichage_exercice_automatique )
	return;

    /*     si on n'utilise pas l'exo, on se barre */

    if ( !option_menu_exercice )
	return;

    sscanf ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( entree_date )),
	     "%02d/%02d/%04d",
	     &jour,
	     &mois,
	     &an );

    if ( !g_date_valid_dmy ( jour,
			     mois,
			     an ))
	return;

    date_courante = g_date_new_dmy ( jour,
				     mois,
				     an );

    /* on fait le tour des exercices pour trouver le premier qui inclut la date */

    liste_tmp = liste_struct_exercices;
    trouve = 0;
    ;
    while ( liste_tmp )
    {
	struct struct_exercice *exercice;

	exercice = liste_tmp -> data;

	if ( exercice->date_debut
	     &&
	     exercice->date_fin
	     &&
	     exercice->affiche_dans_formulaire
	     &&
	     (g_date_compare ( date_courante,
			       exercice->date_debut ) >= 0 )
	     &&
	     (g_date_compare ( date_courante,
			       exercice->date_fin ) <= 0 ))
	{
	    gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_exercice ),
					  cherche_no_menu_exercice ( exercice->no_exercice,
								     option_menu_exercice ));
	    trouve = 1;
	}

	liste_tmp = liste_tmp -> next;
    }

    /*       si rien trouvé, met à aucun */

    if ( !trouve )
	gtk_option_menu_set_history ( GTK_OPTION_MENU ( option_menu_exercice ),
				      0 );
}
/* ************************************************************************************************************** */



/* ************************************************************************************************************** */
/* fonction association_automatique */
/* recherche les opés sans exercice, et les associe si possible avec un des exercice créé */
/* ************************************************************************************************************** */

void association_automatique ( void )
{
    gint resultat, i;

    resultat = question_yes_no_hint ( _("Automatic association of financial years?"),
				      _("This function assigns each transaction without a financial year to the one related to its transaction date.  If no financial year matches, the transaction will not be changed.") );

    if ( resultat )
	return;

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	GSList *pointeur_tmp;

	pointeur_tmp = LISTE_OPERATIONS;

	while ( pointeur_tmp )
	{
	    struct structure_operation *operation;

	    operation = pointeur_tmp -> data;

	    if ( !operation -> no_exercice )
	    {
		GSList *pointeur_exo;

		pointeur_exo = liste_struct_exercices;

		while ( pointeur_exo )
		{
		    struct struct_exercice *exo;

		    exo = pointeur_exo -> data;

		    if ( g_date_compare ( exo -> date_debut,
					  operation -> date ) <= 0
			 &&
			 g_date_compare ( exo -> date_fin,
					  operation -> date ) >= 0 )
		    {
			operation -> no_exercice = exo -> no_exercice;
		    }

		    pointeur_exo = pointeur_exo -> next;
		}
	    }
	    pointeur_tmp = pointeur_tmp -> next;
	}
	p_tab_nom_de_compte_variable++;
    }

    demande_mise_a_jour_tous_comptes ();

    modification_fichier ( TRUE );
}
/* ************************************************************************************************************** */


