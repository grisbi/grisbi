/* ************************************************************************** */
/* Fichier s'occupe de tout ce qui concerne les banques                       */
/*                                                                            */
/*                                banque.c                                    */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2004 Alain Portal (aportal@univ-monpt2.fr)	      */
/*			http://www.grisbi.org				      */
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
#include "banque.h"
#include "comptes_gestion.h"
#include "dialog.h"
#include "search_glist.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_editables.h"
/*END_INCLUDE*/

/*START_STATIC*/
static struct struct_banque * ajout_banque ( GtkWidget *bouton, GtkWidget *clist );
static GtkWidget * bank_form ( GtkWidget * parent );
static void deselection_ligne_banque ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev,
				GtkWidget *frame );
static void selection_ligne_banque ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
			      GtkWidget *frame );
static void supprime_banque ( GtkWidget *bouton,
		       GtkWidget *liste );
static void update_bank_form ( struct struct_banque * bank, GtkWidget * frame );
static gboolean update_bank_list ( GtkEntry *entry, gchar *value, 
			    gint length, gint * position );
static gboolean update_bank_menu ();
/*END_STATIC*/



GSList *liste_struct_banques;
gint nb_banques;
gint no_derniere_banque;

GtkWidget *clist_banques_parametres;
GtkWidget *bouton_supprimer_banque;
GtkWidget *nom_banque;
GtkWidget *code_banque;
GtkWidget *tel_banque;
GtkWidget *adr_banque;
GtkWidget *email_banque;
GtkWidget *web_banque;
GtkWidget *nom_correspondant;
GtkWidget *tel_correspondant;
GtkWidget *email_correspondant;
GtkWidget *fax_correspondant;
GtkWidget *remarque_banque;
gint ligne_selection_banque;




/*START_EXTERN*/
extern gint compte_courant_onglet;
extern GtkWidget *detail_option_menu_banque;
extern GtkWidget *fenetre_preferences;
extern GtkWidget *hbox_boutons_modif;
extern gint nb_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern GtkTreeSelection * selection;
extern GtkWidget *window;
/*END_EXTERN*/





/** 
 * Update bank name in list.  Normally called as a signal handler.
 *
 * \param entry Widget that triggered signal
 * \param value Handler parameter, not used
 * \param length Handler parameter, not used
 * \param position Handler parameter, not used
 */
gboolean update_bank_list ( GtkEntry *entry, gchar *value, 
			    gint length, gint * position )
{
    struct struct_banque * bank;

    if ( GTK_IS_CLIST ( clist_banques_parametres ) )
    {
	bank = gtk_clist_get_row_data ( GTK_CLIST ( clist_banques_parametres ),
					ligne_selection_banque );
	if ( gtk_object_get_data ( GTK_OBJECT(nom_banque), "pointer" ))
	{
	    if (gtk_entry_get_text (GTK_ENTRY(nom_banque)))
		gtk_clist_set_text ( GTK_CLIST(clist_banques_parametres),
				     ligne_selection_banque, 0,
				     gtk_entry_get_text ( GTK_ENTRY(nom_banque)) );
	    if (gtk_entry_get_text (GTK_ENTRY(nom_correspondant)))
		gtk_clist_set_text ( GTK_CLIST(clist_banques_parametres),
				     ligne_selection_banque, 1, 
				     gtk_entry_get_text (GTK_ENTRY(nom_correspondant)));
	}
    }

    update_bank_menu();
    return FALSE;
}



/**
 * Rebuild bank menus in various forms
 */
gboolean update_bank_menu ()
{
    gtk_widget_destroy ( GTK_OPTION_MENU ( detail_option_menu_banque ) -> menu );
    gtk_option_menu_set_menu ( GTK_OPTION_MENU ( detail_option_menu_banque ),
			       creation_menu_banques () );
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_option_menu_banque  ) -> menu ),
				"selection-done",
				GTK_SIGNAL_FUNC ( modif_detail_compte ),
				GTK_OBJECT ( hbox_boutons_modif ) );

    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + compte_courant_onglet;
    gtk_option_menu_set_history ( GTK_OPTION_MENU ( detail_option_menu_banque ),
				  g_slist_index ( liste_struct_banques,
						  banque_par_no ( BANQUE )) +1 );

    return FALSE;
}


/* ***************************************************************************************************** */
/* Fonction ajout_banque */
/* appelée par le bouton ajouter de l'onglet banques des paramètres */
/* ***************************************************************************************************** */

struct struct_banque * ajout_banque ( GtkWidget *bouton, GtkWidget *clist )
{
    struct struct_banque *banque;
    gchar *ligne[1];
    gint ligne_insert;

    if ( clist && GTK_IS_CLIST(clist) )
    {
	/* enlève la sélection de la liste ( ce qui nettoie les entrées ) */
	gtk_clist_unselect_all ( GTK_CLIST ( clist ));
    }

    /* crée une nouvelle banque au nom de "nouvelle banque" en mettant
       tous les paramètres à 0 */
    banque = calloc ( 1, sizeof ( struct struct_banque ));
    if (!banque)
    {
	dialogue_error ( _("Cannot allocate memory, bad things will happen soon") );
	return NULL;
    }
    banque -> no_banque = ++nb_banques;
    banque -> nom_banque = g_strdup ( _("New bank") );
    banque -> nom_correspondant = "";
    liste_struct_banques = g_slist_append ( liste_struct_banques, banque );
    ligne[0] = banque -> nom_banque;

    if ( clist && GTK_IS_CLIST(clist) )
    {
	ligne_insert = gtk_clist_append ( GTK_CLIST ( clist ), ligne );

	/* on associe à la ligne la struct de la banque */
	gtk_clist_set_row_data ( GTK_CLIST ( clist ), ligne_insert, banque );

	/* on sélectionne le nouveau venu */
	gtk_clist_select_row ( GTK_CLIST ( clist ), ligne_insert, 0 );

	/* on sélectionne le "nouvelle banque" et lui donne le focus */
	gtk_entry_select_region ( GTK_ENTRY ( nom_banque ), 0, -1 );
	gtk_widget_grab_focus ( nom_banque );
    }

    update_bank_menu ();

    return banque;
}





/* **************************************************************************************************************************** */
/* Fonction supprime_banque */
/* appelée lorsqu'on clicke sur le bouton enlever */
/* **************************************************************************************************************************** */
void supprime_banque ( GtkWidget *bouton,
		       GtkWidget *liste )
{
    struct struct_banque *banque;
    gboolean resultat;
    gpointer **save;
    gint i;
    gboolean bank_is_used=FALSE;
    gint bank_nb_to_remove;

    banque = gtk_clist_get_row_data ( GTK_CLIST ( liste ),
				      ligne_selection_banque );

    bank_nb_to_remove = banque -> no_banque;

    save = p_tab_nom_de_compte_variable;
    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

    for ( i=0 ; i < nb_comptes ; i++ )
    {
	if ( BANQUE == bank_nb_to_remove )
	    bank_is_used = TRUE;
	p_tab_nom_de_compte_variable++;
    }

    if ( bank_is_used )
	resultat = question_yes_no_hint ( _("Confirmation of bank removal"),
					  g_strdup_printf ( _("Bank \"%s\" is used by one or several accounts.\nDo you really want to remove it?"),
							    banque -> nom_banque));
    else
	resultat = question_yes_no_hint ( _("Confirmation of bank removal"),
					  g_strdup_printf ( _("Are you sure you want to remove bank \"%s\"?\n"),
							    banque -> nom_banque));

    if ( resultat )
    {
	/* La suppression de la banque est confirmée, On fait le tour des
	   comptes pour en modifier le numéro de banque associée */
	
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	for ( i=0 ; i < nb_comptes ; i++ )
	{
	    if ( BANQUE == bank_nb_to_remove )
		BANQUE = 0;
	    if ( BANQUE > bank_nb_to_remove )
		BANQUE--;
	    p_tab_nom_de_compte_variable++;
	}

	/* On retire la banque de la liste */

	gtk_clist_remove ( GTK_CLIST ( liste ),
			   ligne_selection_banque );
	liste_struct_banques = g_slist_remove ( liste_struct_banques,
						banque );
	free ( banque );

	/* Puis on fait le tour des banques pour recaler leurs numéros
	   et ne pas créer de trous dans la suite des numéros */

	for ( i = bank_nb_to_remove+1 ; i <= nb_banques ; i++ )
	{
	    GSList *pList;

	    pList = g_slist_find_custom ( liste_struct_banques,
					  GINT_TO_POINTER ( i ),
					 ( GCompareFunc ) recherche_banque_par_no );

	    if ( pList )
	    {
		struct struct_banque *pBank;
	    
		pBank = pList -> data;
		pBank -> no_banque--;
	    }
	}
	nb_banques--;
	modification_fichier ( TRUE );
    }

    p_tab_nom_de_compte_variable = save;
    update_bank_menu ();
}



/* ************************************************************************************************************ */
/* Fonction creation_menu_banques */
/* crée un menu qui contient les noms des banques associés à leur no et adr */
/* et le renvoie */
/* ************************************************************************************************************ */

GtkWidget *creation_menu_banques ( void )
{
    GtkWidget *menu;
    GtkWidget *menu_item;
    GSList *pointeur;

    menu = gtk_menu_new ();
    gtk_widget_show ( menu );


    /* le premier nom est Aucune */

    menu_item = gtk_menu_item_new_with_label ( _("None") );
    gtk_menu_append ( GTK_MENU ( menu ),
		      menu_item );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "adr_banque",
			  NULL );
    gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			  "no_banque",
			  NULL );
    gtk_widget_show ( menu_item );


    pointeur = liste_struct_banques;

    while ( pointeur )
    {
	struct struct_banque *banque;

	banque = pointeur -> data;

	menu_item = gtk_menu_item_new_with_label ( banque -> nom_banque );
	gtk_menu_append ( GTK_MENU ( menu ),
			  menu_item );
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "adr_banque",
			      banque );
	gtk_object_set_data ( GTK_OBJECT ( menu_item ),
			      "no_banque",
			      GINT_TO_POINTER ( banque -> no_banque ));
	g_signal_connect ( G_OBJECT ( menu_item ),
			   "activate",
			   GTK_SIGNAL_FUNC ( changement_de_banque ),
			   NULL );
	gtk_widget_show ( menu_item );

	pointeur = pointeur -> next;
    }

    return ( menu );
}
/* ************************************************************************************************************ */



/**
 * Creates the "Banks" tab.  It creates a bank list and then a form
 * that allows to edit selected bank.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_banques ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *scrolled_window, *vbox, *vbox2;
    GtkWidget *bouton, *hbox, *paddingbox;

    GSList *liste_tmp;
    gchar *bank_cols_titles [2] = {_("Bank"),
	_("Contact name") };

	vbox_pref = new_vbox_with_title_and_icon ( _("Banks"), "banks.png" );

	/* Create bank list */
	paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
						 _("Known banks") );
	scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					 GTK_POLICY_NEVER,
					 GTK_POLICY_AUTOMATIC);
	clist_banques_parametres = gtk_clist_new_with_titles ( 2,
							       bank_cols_titles);
	gtk_widget_set_usize ( clist_banques_parametres,
			       FALSE, 100 );
	gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_banques_parametres ) ,
					   0, TRUE );
	gtk_clist_set_column_auto_resize ( GTK_CLIST ( clist_banques_parametres ) ,
					   1, TRUE );
	gtk_signal_connect_object  ( GTK_OBJECT ( fenetre_preferences ),
				     "close",
				     GTK_SIGNAL_FUNC ( gtk_clist_unselect_all ),
				     GTK_OBJECT ( clist_banques_parametres ));
	gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			    clist_banques_parametres );

	hbox = gtk_hbox_new ( FALSE, 10 );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			     FALSE, FALSE, 0);
	gtk_box_pack_start ( GTK_BOX ( hbox ), scrolled_window,
			     TRUE, TRUE, 0);

	/* Do not activate unless an account is opened */
	if ( !nb_comptes )
	    gtk_widget_set_sensitive ( vbox_pref, FALSE );
	else
	{
	    liste_tmp = liste_struct_banques;

	    while ( liste_tmp )
	    {
		struct struct_banque *banque;
		gchar *ligne[2];
		gint ligne_insert;

		banque = liste_tmp -> data;

		ligne[0] = banque -> nom_banque;
		ligne[1] = banque -> nom_correspondant;

		ligne_insert = gtk_clist_append ( GTK_CLIST ( clist_banques_parametres ),
						  ligne );

		/* on associe à la ligne la struct de la banque */

		gtk_clist_set_row_data ( GTK_CLIST ( clist_banques_parametres ),
					 ligne_insert,
					 banque );

		liste_tmp = liste_tmp -> next;
	    }
	}

	/* Handle Add & Remove buttons */
	vbox = gtk_vbox_new ( FALSE, 5 );
	gtk_box_pack_start ( GTK_BOX ( hbox ), vbox,
			     FALSE, FALSE, 0 );
	/* Add button */
	bouton = gtk_button_new_from_stock (GTK_STOCK_ADD);
	gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
			     GTK_SIGNAL_FUNC  ( ajout_banque ),
			     clist_banques_parametres );
	gtk_box_pack_start ( GTK_BOX ( vbox ), bouton,
			     FALSE, FALSE, 5 );
	/* Remove button */
	bouton_supprimer_banque = gtk_button_new_from_stock (GTK_STOCK_REMOVE);
	gtk_widget_set_sensitive ( bouton_supprimer_banque, FALSE );
	gtk_signal_connect ( GTK_OBJECT ( bouton_supprimer_banque ), "clicked",
			     GTK_SIGNAL_FUNC  ( supprime_banque ),
			     clist_banques_parametres );
	gtk_box_pack_start ( GTK_BOX ( vbox ), bouton_supprimer_banque,
			     FALSE, FALSE, 5 );


	/* Add a scroll because bank details are huge */
	scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					 GTK_POLICY_NEVER,
					 GTK_POLICY_AUTOMATIC );
	gtk_box_pack_start ( GTK_BOX ( vbox_pref ), scrolled_window,
			     TRUE, TRUE, 5 );
	vbox2 = gtk_vbox_new ( FALSE, 0 );
	gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
						vbox2 );
	gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					      GTK_SHADOW_NONE );
	gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( scrolled_window ) -> child ),
				       GTK_SHADOW_NONE );
	gtk_widget_set_sensitive ( vbox2, FALSE );

	bank_form ( vbox2 );

	/* Active only if a bank is selected */
	gtk_signal_connect ( GTK_OBJECT ( clist_banques_parametres ),
			     "select-row",
			     GTK_SIGNAL_FUNC ( selection_ligne_banque ),
			     vbox2 );
	gtk_signal_connect ( GTK_OBJECT ( clist_banques_parametres ),
			     "unselect-row",
			     GTK_SIGNAL_FUNC ( deselection_ligne_banque ),
			     vbox2 );

	/* Select first entry if applicable */
	gtk_clist_select_row ( GTK_CLIST(clist_banques_parametres), 0, 0 );

	return ( vbox_pref );
}


/**
 * Fills in bank details widgets with corresponding stuff.  Make them
 * active as well.
 *
 * \param liste		gtkclist containing banks
 * \param ligne		selected bank
 * \param colonne	not used, for handler's sake only
 * \param ev button	event, not used, for handler's sake only
 * \param frame		widget or widget group to activate
 */
void selection_ligne_banque ( GtkWidget *liste,
			      gint ligne,
			      gint colonne,
			      GdkEventButton *ev,
			      GtkWidget *frame )
{
    struct struct_banque *banque;

    ligne_selection_banque = ligne;
    banque = gtk_clist_get_row_data ( GTK_CLIST ( liste ), ligne );

    update_bank_form ( banque, frame );
}



/**
 * Clear bank details widgets.  Make them unactive as well.
 *
 * \param liste		gtkclist containing banks
 * \param ligne		previously selected bank
 * \param colonne	not used, for handler's sake only
 * \param ev button	event, not used, for handler's sake only
 * \param frame		widget or widget group to deactivate
 */
void deselection_ligne_banque ( GtkWidget *liste,
				gint ligne,
				gint colonne,
				GdkEventButton *ev,
				GtkWidget *frame )
{
    entry_set_value ( nom_banque, NULL );
    entry_set_value ( code_banque, NULL );
    entry_set_value ( tel_banque, NULL );
    entry_set_value ( email_banque, NULL );
    entry_set_value ( web_banque, NULL );
    entry_set_value ( nom_correspondant, NULL );
    entry_set_value ( tel_correspondant, NULL );
    entry_set_value ( fax_correspondant, NULL );
    entry_set_value ( email_correspondant, NULL );

    text_area_set_value ( adr_banque, NULL );
    text_area_set_value ( remarque_banque, NULL );

    gtk_widget_set_sensitive ( frame, FALSE );
    gtk_widget_set_sensitive ( bouton_supprimer_banque, FALSE );
}



GtkWidget * bank_form ( GtkWidget * parent )
{
    GtkWidget * paddingbox, *table, *label, *scrolled_window;
    GtkSizeGroup * size_group;

    /* Bank details */
    paddingbox = new_paddingbox_with_title ( parent, FALSE,
					     _("Bank details") );

    /* Create a table to align things nicely */
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 TRUE, TRUE, 5 );
    size_group = gtk_size_group_new ( GTK_SIZE_GROUP_HORIZONTAL );

    /* Bank name item */
    label = gtk_label_new ( COLON(_("Name")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    nom_banque = new_text_entry ( NULL, G_CALLBACK(update_bank_list) );
    gtk_size_group_add_widget ( size_group, nom_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       nom_banque, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Bank Sort code item */
    label = gtk_label_new ( COLON(_("Sort code")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    code_banque = new_text_entry ( NULL, NULL );
    gtk_size_group_add_widget ( size_group, code_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       code_banque, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Bank address */
    label = gtk_label_new ( COLON(_("Address")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    /* Create the text view */
    adr_banque = new_text_area ( NULL, NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), adr_banque );
    gtk_size_group_add_widget ( size_group, adr_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       scrolled_window, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Phone number */
    label = gtk_label_new ( COLON(_("Phone")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 4, 5,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    tel_banque = new_text_entry ( NULL, NULL );
    gtk_size_group_add_widget ( size_group, tel_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       tel_banque, 1, 2, 4, 5,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* E-mail */
    label = gtk_label_new ( COLON(_("E-Mail")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 3, 4,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    email_banque = new_text_entry ( NULL, NULL );
    gtk_size_group_add_widget ( size_group, email_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       email_banque, 1, 2, 3, 4,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Website */
    label = gtk_label_new ( COLON(_("Website")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 5, 6,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    web_banque = new_text_entry ( NULL, NULL );
    gtk_size_group_add_widget ( size_group, web_banque );
    gtk_table_attach ( GTK_TABLE ( table ),
		       web_banque, 1, 2, 5, 6,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Contact */
    paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Contact") );
    table = gtk_table_new ( 2, 2, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
			 TRUE, TRUE, 5 );

    /* Name */
    label = gtk_label_new ( COLON(_("Name")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    nom_correspondant = new_text_entry ( NULL, G_CALLBACK(update_bank_list) );
    gtk_size_group_add_widget ( size_group, nom_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       nom_correspondant, 1, 2, 0, 1,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Phone number */
    label = gtk_label_new ( COLON(_("Phone")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    tel_correspondant = new_text_entry ( NULL, NULL );
    gtk_size_group_add_widget ( size_group, tel_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       tel_correspondant, 1, 2, 1, 2,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Fax */
    label = gtk_label_new ( COLON(_("Fax")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 3, 4,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    fax_correspondant = new_text_entry ( NULL, NULL );
    gtk_size_group_add_widget ( size_group, fax_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       fax_correspondant, 1, 2, 3, 4,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* E-Mail */
    label = gtk_label_new ( COLON(_("E-Mail")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_table_attach ( GTK_TABLE ( table ),
		       label, 0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL, 0,
		       0, 0 );
    email_correspondant = new_text_entry ( NULL, NULL );
    gtk_size_group_add_widget ( size_group, email_correspondant );
    gtk_table_attach ( GTK_TABLE ( table ),
		       email_correspondant, 1, 2, 2, 3,
		       GTK_EXPAND | GTK_FILL, 0,
		       0, 0 );

    /* Notes */
    paddingbox = new_paddingbox_with_title ( parent, FALSE, _("Notes") );
    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
			 TRUE, TRUE, 5 );
    remarque_banque = new_text_area ( NULL, NULL );
    gtk_widget_set_usize ( remarque_banque, FALSE, 100 );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ), remarque_banque );

    return parent;
}



void update_bank_form ( struct struct_banque * bank, GtkWidget * frame )
{
    entry_set_value ( nom_banque, &(bank -> nom_banque) );
    entry_set_value ( code_banque, &(bank -> code_banque) );
    entry_set_value ( tel_banque, &(bank -> tel_banque) );
    entry_set_value ( email_banque, &(bank -> email_banque) );
    entry_set_value ( web_banque, &(bank -> web_banque) );

    entry_set_value ( nom_correspondant, &(bank -> nom_correspondant) );
    entry_set_value ( tel_correspondant, &(bank -> tel_correspondant) );
    entry_set_value ( email_correspondant, &(bank -> email_correspondant) );
    entry_set_value ( fax_correspondant, &(bank -> fax_correspondant) );

    text_area_set_value ( adr_banque, &(bank -> adr_banque) );
    text_area_set_value ( remarque_banque, &(bank -> remarque_banque) );

    gtk_widget_set_sensitive ( frame, TRUE );
    if ( GTK_IS_WIDGET(bouton_supprimer_banque))
	gtk_widget_set_sensitive ( bouton_supprimer_banque, TRUE );
}



void view_bank ( GtkWidget * button, gpointer data )
{
    struct struct_banque * bank;

    bank = gtk_object_get_data ( GTK_OBJECT ( GTK_OPTION_MENU ( detail_option_menu_banque ) -> menu_item ),
				 "adr_banque" ); 

    if ( !bank )
	return;

    edit_bank ( button, bank );
}



void edit_bank ( GtkWidget * button, struct struct_banque * bank )
{
    GtkWidget * dialog, *form, * scrolled_window, *vbox;

    dialog = gtk_dialog_new_with_buttons ( _("Edit bank"),
					   GTK_WINDOW(window),
					   GTK_DIALOG_DESTROY_WITH_PARENT,
					   GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					   NULL );

    scrolled_window = gtk_scrolled_window_new ( FALSE, FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_NONE );
    gtk_container_add ( GTK_CONTAINER ( GTK_DIALOG(dialog)->vbox ), scrolled_window );
    gtk_widget_set_usize ( scrolled_window, 600, 400 );

    vbox = gtk_vbox_new ( FALSE, 6 );
    gtk_container_set_border_width ( GTK_CONTAINER (vbox), 12 );
    form = bank_form ( vbox );
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT (GTK_BIN(scrolled_window) -> child ),
				   GTK_SHADOW_NONE );

    gtk_widget_show_all ( GTK_DIALOG(dialog)->vbox );

    if (! bank )
    {
	bank = ajout_banque ( NULL, NULL );
    }
    update_bank_form ( bank, GTK_DIALOG(dialog)->vbox );

    gtk_dialog_run (GTK_DIALOG(dialog));
    gtk_widget_destroy ( dialog );
}






/* **************************************************************************************************** */
/* cette fonction renvoie l'adr de la banque demandée en argument */
/* et NULL si pas trouvée */
/* **************************************************************************************************** */
struct struct_banque *banque_par_no ( gint no_banque )
{
    if ( no_banque )
    {
	GSList *liste_tmp;

	liste_tmp = g_slist_find_custom ( liste_struct_banques,
					  GINT_TO_POINTER ( no_banque ),
					  (GCompareFunc) recherche_banque_par_no );

	if ( liste_tmp )
	    return ( liste_tmp -> data );
    }
    return NULL;
}
/* **************************************************************************************************** */



/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
