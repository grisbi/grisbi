/* ************************************************************************** */
/* Ce fichier comprend toutes les opérations concernant le traitement	      */
/* des fichiers								      */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "structures.h"
#include "fichiers_gestion.h"


#include "accueil.h"
#include "categories_onglet.h"
#include "comptes_traitements.h"
#include "devises.h"
#include "dialog.h"
#include "fenetre_principale.h"
#include "fichiers_io.h"
#include "menu.h"
#include "operations_comptes.h"
#include "operations_liste.h"
#include "patienter.h"
#include "traitement_variables.h"
#include "fichier_configuration.h"
#include "utils.h"
#include "affichage_liste.h"
#include "echeancier_liste.h"
#include "imputation_budgetaire.h"
#include "tiers_onglet.h"


static void fichier_selectionne ( GtkWidget *selection_fichier);
static gchar *demande_nom_enregistrement ( void );
static gboolean enregistrement_backup ( void );
static void ajoute_nouveau_fichier_liste_ouverture ( gchar *path_fichier );




gchar *nom_fichier_backup;



extern GtkWidget *window_vbox_principale;
extern gint patience_en_cours;
extern GSList *echeances_saisies;
extern GSList *liste_struct_echeances;  
extern GSList *echeances_a_saisir;
extern gint mise_a_jour_liste_comptes_accueil;
extern gint mise_a_jour_soldes_minimaux;
extern gint mise_a_jour_fin_comptes_passifs;
extern gint id_fonction_idle;
extern gboolean block_menu_cb;
extern GtkItemFactory *item_factory_menu_general;
extern GtkWidget *window;
extern gint compte_courant;
extern gchar *dernier_chemin_de_travail;
extern gchar **tab_noms_derniers_fichiers_ouverts;
extern gint compression_fichier;
extern gint compression_backup;
extern gint nb_max_derniers_fichiers_ouverts;
extern gint nb_derniers_fichiers_ouverts;
extern gint nb_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern gchar *nom_fichier_comptes;
extern GSList *ordre_comptes;
extern gchar *titre_fichier;
extern GtkWidget *notebook_general;
extern gint id_temps;
extern GSList *liste_struct_etats;
extern gint rapport_largeur_colonnes[7];


/* ************************************************************************************************************ */
/* cette fonction est appelée par les menus */
/* elle ferme l'ancien fichier, crée un compte vierge */
/* et initialise l'affichage */
/* ************************************************************************************************************ */
void nouveau_fichier ( void )
{
    gint type_de_compte;
    gint no_compte;

    /*   si la fermeture du fichier en cours se passe mal, on se barre */

    if ( !fermer_fichier () )
	return;

    init_variables ();

    type_de_compte = demande_type_nouveau_compte ();
    if ( type_de_compte == -1 )
	return;

    /*     création de la 1ère devise */

    ajout_devise ( NULL );

    no_compte = initialisation_nouveau_compte ( type_de_compte );

    /* si la création s'est mal passée, on se barre */

    if ( no_compte == -1 )
	return;

    initialisation_variables_nouveau_fichier ();
    initialisation_graphiques_nouveau_fichier ();

    /* on se met sur l'onglet de propriétés du compte */

    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_general ),
			    3 );

    modification_fichier ( TRUE );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
/* cette fonction initialise les variables pour faire un nouveau fichier */
/* ************************************************************************************************************ */
void initialisation_variables_nouveau_fichier ( void )
{
    /*   la taille des colonnes est automatique au départ, on y met les rapports de base */

    etat.largeur_auto_colonnes = 1;
    rapport_largeur_colonnes[0] = 11;
    rapport_largeur_colonnes[1] = 13;
    rapport_largeur_colonnes[2] = 30;
    rapport_largeur_colonnes[3] = 3;
    rapport_largeur_colonnes[4] = 11;
    rapport_largeur_colonnes[5] = 11;
    rapport_largeur_colonnes[6] = 11;

    /* création des listes d'origine */

    creation_liste_categories ();
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* cette fonction est appelée lors de la création d'un nouveau fichier, elle s'occupe */
/* de l'initialisation de la partie graphique */
/* ************************************************************************************************************ */
void initialisation_graphiques_nouveau_fichier ( void )
{
    /* dégrise les menus nécessaire */

    menus_sensitifs ( TRUE );

    creation_liste_categ_combofix ();

    /*     récupère l'organisation des colonnes  */
    
    recuperation_noms_colonnes_et_tips ();

    /* on crée le notebook principal */

    gtk_box_pack_start ( GTK_BOX ( window_vbox_principale),
			 creation_fenetre_principale(),
			 TRUE,
			 TRUE,
			 0 );

    p_tab_nom_de_compte_variable = p_tab_nom_de_compte + compte_courant;

    changement_compte ( GINT_TO_POINTER ( compte_courant ) );

    /* affiche le nom du fichier de comptes dans le titre de la fenetre */

    affiche_titre_fenetre();

    gtk_notebook_set_page ( GTK_NOTEBOOK( notebook_general ),
			    0 );

    /*     on se met sur la page d'accueil */

    gtk_widget_show ( notebook_general );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
void ouvrir_fichier ( void )
{
    GtkWidget *selection_fichier;

    selection_fichier = gtk_file_selection_new ( _("Open an accounts file"));
    gtk_window_set_position ( GTK_WINDOW ( selection_fichier ),
			      GTK_WIN_POS_MOUSE);

    gtk_file_selection_set_filename ( GTK_FILE_SELECTION ( selection_fichier ),
				      dernier_chemin_de_travail );

    gtk_signal_connect_object ( GTK_OBJECT ( GTK_FILE_SELECTION ( selection_fichier ) -> cancel_button ),
				"clicked",
				GTK_SIGNAL_FUNC ( gtk_widget_destroy ),
				GTK_OBJECT ( selection_fichier ));
    gtk_signal_connect_object ( GTK_OBJECT ( GTK_FILE_SELECTION ( selection_fichier ) -> ok_button ),
				"clicked",
				GTK_SIGNAL_FUNC ( fichier_selectionne ),
				GTK_OBJECT (selection_fichier) );

    gtk_widget_show ( selection_fichier );


}
/* ************************************************************************************************************ */

/* ************************************************************************************************************ */
void ouverture_fichier_par_menu ( gpointer null,
				  gint no_fichier )
{
    /*   si la fermeture du fichier courant se passe mal, on se barre */

    if ( !fermer_fichier() )
	return;

    nom_fichier_comptes = tab_noms_derniers_fichiers_ouverts[no_fichier];

    ouverture_confirmee ();
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
void fichier_selectionne ( GtkWidget *selection_fichier)
{

    /*   on cache la fenetre de sélection et ferme le fichier en cours */
    /*     si lors de la fermeture, on a voulu enregistrer mais ça n'a pas marché => return */

    gtk_widget_hide ( selection_fichier );

    /*   si la fermeture du fichier se passe mal, on se barre */

    if ( !fermer_fichier() )
    {
	gtk_widget_hide ( selection_fichier );
	return;
    }


    /* on prend le nouveau nom du fichier */

    nom_fichier_comptes = g_strdup ( gtk_file_selection_get_filename ( GTK_FILE_SELECTION ( selection_fichier)) );

    gtk_widget_hide ( selection_fichier );

    /* on met le répertoire courant dans la variable correspondante */

    dernier_chemin_de_travail = g_strconcat ( GTK_LABEL ( GTK_BIN ( GTK_OPTION_MENU ( GTK_FILE_SELECTION ( selection_fichier ) -> history_pulldown )) -> child ) -> label,
					      C_DIRECTORY_SEPARATOR,
					      NULL );

    ouverture_confirmee ();
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction ouverture_confirmee */
/* ouvre le fichier dont le nom est dans nom_fichier_comptes */
/* ne se préocupe pas d'un ancien fichier ou d'une initialisation de variables */
/* ************************************************************************************************************ */

void ouverture_confirmee ( void )
{
    gint i;
    GtkWidget * widget;
    gchar * item_name = NULL;
    
    if ( DEBUG )
	printf ( "ouverture_confirmee\n" );

    mise_en_route_attente ( _("Load an accounts file") );

    /*  si charge opérations renvoie FALSE, c'est qu'il y a eu un pb et un message est déjà affiché */

    if ( !charge_operations ( nom_fichier_comptes ) )
    {
	/* 	  le chargement du fichier a planté, si l'option sauvegarde à l'ouverture est activée, on */
	/* propose de charger l'ancien fichier */

	annulation_attente ();

	if ( etat.sauvegarde_demarrage )
	{
	    gchar *nom;
	    gint result;
	    gchar **parametres;

	    /* on crée le nom de la sauvegarde */

	    nom = g_strdup ( nom_fichier_comptes );
	    i=0;

	    parametres = g_strsplit ( nom, C_DIRECTORY_SEPARATOR, 0);
	    while ( parametres[i] )
		i++;

	    nom = g_strconcat ( my_get_gsb_file_default_dir(),
				C_DIRECTORY_SEPARATOR,
				parametres [i-1],
				".bak",
				NULL );
	    g_strfreev ( parametres );

	    result = open ( nom, O_RDONLY);
	    if (result == -1)
		return;
	    else
		close (result);

	    mise_en_route_attente ( _("Loading backup") );

	    if ( charge_operations ( nom ) )
	    {
		/* on a réussi a charger la sauvegarde */
		dialogue ( _("Grisbi was unable to load file.  However, Grisbi loaded a backup file instead.\nHowever, all changes made since this backup were possibly lost."));
	    }
	    else
	    {
		/* le chargement de la sauvegarde a échoué */

		annulation_attente ();
		dialogue ( _("Grisbi was unable to load file.  Additionnaly, Grisbi was unable to load a backup file instead."));
		return;
	    }
	}
	else
	{
	    menus_sensitifs ( FALSE );
	    return;
	}
    }
    else
    {
	/* 	    l'ouverture du fichier s'est bien passée */
	/* 	si on veut faire une sauvegarde auto à chaque ouverture, c'est ici */

	if ( etat.sauvegarde_demarrage )
	{
	    gchar *nom;
	    gchar **parametres;

	    update_attente ( _("Autosave") );

	    nom = g_strdup ( nom_fichier_comptes );

	    i=0;

	    /* 	      on récupère uniquement le nom du fichier, pas le chemin */

	    parametres = g_strsplit ( nom,
				      "/",
				      0);

	    while ( parametres[i] )
		i++;

	    nom = g_strconcat ( my_get_gsb_file_default_dir(),
				"/.",
				parametres [i-1],
				".bak",
				NULL );

	    g_strfreev ( parametres );

	    /* on force l'enregistrement */

	    enregistre_fichier ( nom );
	}
    }

    update_attente ( _("Checking schedulers"));

    /*     on vérifie si des échéances sont à récupérer */

    verification_echeances_a_terme ();

    /* affiche le nom du fichier de comptes dans le titre de la fenetre */

    affiche_titre_fenetre();

    /* on save le nom du fichier dans les derniers ouverts */

    if (nom_fichier_comptes)
	ajoute_nouveau_fichier_liste_ouverture ( nom_fichier_comptes );

    /*     récupère l'organisation des colonnes  */

    recuperation_noms_colonnes_et_tips ();

    /*  on calcule les soldes courants */
    /*     important de le faire avant l'affichage de l'accueil */
    /* 	va afficher le message qu'une fois tous les comptes remplis */
    /* 	(donc après l'idle ) */
    
    update_attente ( _("Checking amounts"));

    for ( i=0 ; i<nb_comptes ; i++ )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;

	SOLDE_COURANT = calcule_solde_compte ( i );
	SOLDE_POINTE = calcule_solde_pointe_compte ( i );

	/* 	on met à jour les affichage solde mini et autres */

	MESSAGE_SOUS_MINI = SOLDE_COURANT < SOLDE_MINI;
	MESSAGE_SOUS_MINI_VOULU = SOLDE_COURANT < SOLDE_MINI_VOULU;

	/*     on initialise la fonction de classement  */

	CLASSEMENT_COURANT = recupere_classement_par_no (NO_CLASSEMENT);
    }

    /*     on va afficher la page d'accueil */
    /*     l'appel se fera lors de la création de la fenêtre principale */

    mise_a_jour_liste_comptes_accueil = 1;
    mise_a_jour_soldes_minimaux = 1;
    mise_a_jour_fin_comptes_passifs = 1;

    /*     création de la liste des tiers pour le combofix */
    
    creation_liste_tiers_combofix ();

    /* creation de la liste des categ pour le combofix */

    creation_liste_categ_combofix ();

    /* creation de la liste des imputations pour le combofix */

    creation_liste_imputation_combofix ();

    /* on crée le notebook principal */

    update_attente ( _("Making main window"));

    creation_fenetre_principale();
    
    /*     on dégrise les menus */

    menus_sensitifs ( TRUE );

    /* On met à jour l'affichage des opérations rapprochées */
    block_menu_cb = TRUE;
    widget = gtk_item_factory_get_item ( item_factory_menu_general,
					 menu_name(_("View"), _("Show reconciled transactions"), NULL) );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), AFFICHAGE_R );

    /* On met à jour le contrôle dans le menu du nombre de lignes affichées */
    switch ( NB_LIGNES_OPE )
      {
      case 1 :
	item_name = menu_name ( _("View"), _("Show one line per transaction"), NULL );
	break;
      case 2 :
	item_name = menu_name ( _("View"), _("Show two lines per transaction"), NULL );
	break;
      case 3 :
	item_name = menu_name ( _("View"), _("Show three lines per transaction"), NULL );
	break;
      case 4 :
	item_name = menu_name ( _("View"), _("Show four lines per transaction"), NULL );
	break;
      }

    widget = gtk_item_factory_get_item ( item_factory_menu_general, item_name );
    gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(widget), TRUE );
    block_menu_cb = FALSE;

    /*     on ajoute la fentre principale à la window */

    gtk_box_pack_start ( GTK_BOX ( window_vbox_principale),
			 notebook_general,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( notebook_general );

    annulation_attente ();
}
/* ************************************************************************************************************ */






/* ************************************************************************************************************ */
/* Fonction appelé lorsqu'on veut enregistrer le fichier ( fin de prog, fermeture fichier ... ) */
/* enregistre automatiquement ou demande */
/* si origine = -1 : provient de la fonction fermeture_grisbi */
/* si origine = -2 : provient de enregistrer sous */
/* retour : TRUE si tout va bien, cad on ne veut pas enregistrer ou c'est enregistré */
/* ************************************************************************************************************ */

gboolean enregistrement_fichier ( gint origine )
{
    gint etat_force, result;
    gchar *nouveau_nom_enregistrement;


    etat_force = 0;

    if ( !etat.modification_fichier && origine != -2 )
	return ( TRUE );

    /* si le fichier de comptes n'a pas de nom ou si on enregistre sous un nouveau nom */
    /*     c'est ici */

    if ( !nom_fichier_comptes || origine == -2 )
	nouveau_nom_enregistrement = demande_nom_enregistrement ();
    else
	nouveau_nom_enregistrement = nom_fichier_comptes;

    if ( !nouveau_nom_enregistrement )
	return FALSE;

    /*     on vérifie que le fichier n'est pas locké */

    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement
	 &&
	 origine != -2 )
    {
	dialogue_conditional_hint ( g_strdup_printf( _("Can not save file \"%s\""), nom_fichier_comptes),
				    g_strdup_printf( _("Grisbi was unable to save this file because it is locked.  Please save it with another name or activate the \"%s\" option in setup.  Alternatively, choose the \"%s\" option below."),
						     _("Force saving of locked files"),
						     _("Do not show this message again")), &(etat.force_enregistrement ) );
	return ( FALSE );
    }

    /*   on a maintenant un nom de fichier */
    /*     et on sait qu'on peut sauvegarder */

    mise_en_route_attente ( _("Save file") );

    result = enregistre_fichier ( nouveau_nom_enregistrement );

    if ( result )
    {
	/* 	l'enregistrement s'est bien passé, */
	/* 	on délock le fichier (l'ancien ou le courant) */
	    
	modification_etat_ouverture_fichier ( FALSE );

	nom_fichier_comptes = nouveau_nom_enregistrement;

	/* 	... et locke le nouveau */

	modification_etat_ouverture_fichier ( TRUE );

	/* 	dans tout les cas, le fichier n'était plus ouvert à l'ouverture */

	etat.fichier_deja_ouvert = 0;
	modification_fichier ( FALSE );
	affiche_titre_fenetre ();
	ajoute_nouveau_fichier_liste_ouverture ( nom_fichier_comptes );
    }

    /*     on enregistre la backup si nécessaire */

    enregistrement_backup();

    annulation_attente();

    return ( result );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
gboolean enregistrer_fichier_sous ( void )
{
    return (  enregistrement_fichier ( -2 ) );
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* cette fonction est appelée pour proposer d'enregistrer si le fichier est modifié */
/* elle n'enregistre pas, elle retourne juste le choix de l'utilisateur */
/* retourne : */
/* GTK_RESPONSE_OK : veut enregistrer */
/* GTK_RESPONSE_NO : veut pas enregistrer */
/* autre gint : annuler */
/* ************************************************************************************************************ */
gint question_fermer_sans_enregistrer ( void )
{
    gchar * hint;
    gint result;
    GtkWidget *dialog;

    /*     si le fichier n'est pas modifié on renvoie qu'on ne veut pas enregistrer */

    if ( !etat.modification_fichier )
	return GTK_RESPONSE_NO;
    
    /*     si le fichier était déjà locké et que force enregistrement n'est pas mis, */
    /*     on prévient ici */

    dialog = gtk_message_dialog_new ( GTK_WINDOW (window), 
				      GTK_DIALOG_DESTROY_WITH_PARENT,
				      GTK_MESSAGE_WARNING, 
				      GTK_BUTTONS_NONE,
				      " " );
    if ( etat.fichier_deja_ouvert
	 &&
	 !etat.force_enregistrement )
    {
	hint = g_strdup_printf (_("The document '%s' is locked but modified. If you want to save it, you must cancel and save it with another name or activate the \"%s\" option in setup."),
				  (nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")),
				  _("Force saving of locked files"));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 NULL );
    }
    else
    {
	hint = g_strdup_printf (_("Save changes to document '%s' before closing?"),
				(nom_fichier_comptes ? g_path_get_basename(nom_fichier_comptes) : _("unnamed")));
	gtk_dialog_add_buttons ( GTK_DIALOG(dialog),
				 _("Close without saving"), GTK_RESPONSE_NO,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_SAVE, GTK_RESPONSE_OK,
				 NULL );
    }

    gtk_label_set_markup ( GTK_LABEL ( GTK_MESSAGE_DIALOG(dialog)->label ), 
			   make_hint ( hint, _("If you close without saving, all of your changes will be discarded.")) );

    gtk_window_set_modal ( GTK_WINDOW ( dialog ), TRUE );

    result = gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy ( dialog );

    return result;
}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* cette fonction est appelée lors de l'enregistrement, s'il n'y a pas de nom */
/* ou si on fait un enregistrement sous */
/* elle renvoie le nouveau nom */
/* ************************************************************************************************************ */
gchar *demande_nom_enregistrement ( void )
{
    gchar *nouveau_nom;
    GtkWidget *fenetre_nom;
    gint resultat;
    struct stat test_fichier;

    fenetre_nom = gtk_file_selection_new ( _("Name the accounts file"));
    gtk_window_set_modal ( GTK_WINDOW ( fenetre_nom ),
			   TRUE );
    gtk_file_selection_set_filename ( GTK_FILE_SELECTION ( fenetre_nom ),
				      dernier_chemin_de_travail );
    gtk_entry_set_text ( GTK_ENTRY ( GTK_FILE_SELECTION ( fenetre_nom )->selection_entry),
			 ".gsb" );

    resultat = gtk_dialog_run ( GTK_DIALOG ( fenetre_nom ));

    switch ( resultat )
    {
	case GTK_RESPONSE_OK :
	    nouveau_nom =g_strdup (gtk_file_selection_get_filename ( GTK_FILE_SELECTION ( fenetre_nom )));

	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));

	    /* vérification que qque chose a été entré */

	    if ( !strlen ( nouveau_nom ))
		return NULL;


	    if ( stat ( nouveau_nom,
			&test_fichier ) != -1 )
	    {
		if ( S_ISREG ( test_fichier.st_mode ) )
		{
		    if ( ! question_yes_no_hint (_("File already exists"),
						 g_strdup_printf (_("Do you want to overwrite file \"%s\"?"), nouveau_nom ) ) )
			return NULL;
		}
		else
		{
		    dialogue_error ( g_strdup_printf ( _("Invalid filename: \"%s\"!"),
						       nouveau_nom ));
		    return NULL;
		}
	    }
	    break;

	default :
	    gtk_widget_destroy ( GTK_WIDGET ( fenetre_nom ));
	    return NULL;
    }

    return nouveau_nom;
}
/* ************************************************************************************************************ */





/* ************************************************************************************************************ */
gboolean fermer_fichier ( void )
{
    int i;
    gint result;


    if ( DEBUG)
	printf ( "fermer_fichier\n" );


    if ( !nb_comptes )
	return ( TRUE );


    /*     on propose d'enregistrer */

    result = question_fermer_sans_enregistrer();

    switch ( result )
    {
	case GTK_RESPONSE_OK:

	    /* 	    on a choisi d'enregistrer, si va pas, on s'en va */

	    if ( !enregistrement_fichier (-1) )
		return ( FALSE );

	case GTK_RESPONSE_NO :

	    /* 	    l'enregistrement s'est bien passé ou on a choisi de ne pas enregistrer */

	    /*     on enregistre la config */

	    sauve_configuration ();

	    /*     s'il y a de l'idle, on le retire */

	    termine_idle ();

	    /* si le fichier n'était pas déjà ouvert, met à 0 l'ouverture */

	    if ( !etat.fichier_deja_ouvert
		 &&
		 nb_comptes
		 &&
		 nom_fichier_comptes )
		modification_etat_ouverture_fichier ( FALSE );

	    /*       stoppe le timer */

	    if ( id_temps )
	    {
		gtk_timeout_remove ( id_temps );
		id_temps = 0;
	    }

	    /* libère les opérations de tous les comptes */

	    p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

	    for ( i=0 ; i< nb_comptes ; i++ )
	    {
		if ( LISTE_OPERATIONS )
		    g_slist_free ( LISTE_OPERATIONS );

		free ( *p_tab_nom_de_compte_variable );
		p_tab_nom_de_compte_variable++;
	    };


	    free ( p_tab_nom_de_compte );

	    /* libère les échéances */

	    g_slist_free ( liste_struct_echeances );
	    g_slist_free ( echeances_a_saisir );
	    g_slist_free ( echeances_saisies );
	    g_slist_free ( ordre_comptes );
	    g_slist_free ( liste_struct_etats );

	    gtk_signal_disconnect_by_func ( GTK_OBJECT ( notebook_general ),
					    GTK_SIGNAL_FUNC ( change_page_notebook),
					    NULL );

	    gtk_widget_destroy ( notebook_general );

	    init_variables ();

	    affiche_titre_fenetre();

	    menus_sensitifs ( FALSE );

	    return ( TRUE );
	    break;

	default :
	    return FALSE;
    }
}
/* ************************************************************************************************************ */




/* ************************************************************************************************************ */
/* Fonction appelée une fois qu'on a nommé le fichier de compte */
/* met juste le titre dans la fenetre principale */
/* ************************************************************************************************************ */
void affiche_titre_fenetre ( void )
{
    gchar **parametres;
    gchar *titre = NULL;
    gint i=0;

    if ( DEBUG )
	printf ( "affiche_titre_fenetre\n" );

    if ( titre_fichier && strlen(titre_fichier) )
      titre = titre_fichier;
    else if ( nom_fichier_comptes )
    {
	parametres = g_strsplit ( nom_fichier_comptes, C_DIRECTORY_SEPARATOR, 0);
	while ( parametres[i] )
	  i++;
	titre = parametres [i-1];
	g_strfreev ( parametres );
    }
    else
    {
      titre = g_strconcat ( "<", _("unnamed"), ">", NULL );
    }

    titre = g_strconcat ( titre, " - ", _("Grisbi"), NULL );
    gtk_window_set_title ( GTK_WINDOW ( window ), titre );

}
/* ************************************************************************************************************ */



/* ************************************************************************************************************ */
/* Fonction enregistrement_backup */
/* appelée si necessaire au début de l'enregistrement */
/* ************************************************************************************************************ */

gboolean enregistrement_backup ( void )
{
    gboolean retour;

    if ( !nom_fichier_backup || !strlen(nom_fichier_backup) )
	return FALSE;

    update_attente ( _("Saving backup") );

    mise_en_route_attente ( _("Saving backup") );

    xmlSetCompressMode ( compression_backup );

    retour = enregistre_fichier( nom_fichier_backup );

    xmlSetCompressMode ( compression_fichier );

    annulation_attente();

    return ( retour );
}
/* ************************************************************************************************************ */


/* ************************************************************************************************************ */
void ajoute_nouveau_fichier_liste_ouverture ( gchar *path_fichier )
{
    gint i;
    gint position;
    gchar *dernier;

    if ( DEBUG )
	printf ( "ajoute_nouveau_fichier_liste_ouverture : %s\n", path_fichier );

    if ( !nb_max_derniers_fichiers_ouverts ||
	 ! path_fichier)
	return;

    if ( nb_derniers_fichiers_ouverts < 0 )
	nb_derniers_fichiers_ouverts = 0;

    /* ALAIN-FIXME */
    /* si on n'a pas un chemin absolu, on n'enregistre pas ce fichier
       dans la liste. Du moins jusqu'à ce que quelqu'un trouve un moyen
       pour récupérer le chemein absolu */

    if ( !g_path_is_absolute ( nom_fichier_comptes ) )
    {
	/*     gchar *tmp_name, *tmp_name2;
	       tmp_name = realpath(nom_fichier_comptes, tmp_name2);

	       dialogue(tmp_name);
	       free(tmp_name);
	       dialogue(tmp_name2);
	       free(tmp_name2);*/

	return;
    }

    /* on commence par vérifier si ce fichier n'est pas dans les nb_derniers_fichiers_ouverts noms */

    position = 0;

    for ( i=0 ; i<nb_derniers_fichiers_ouverts ; i++ )
	if ( !strcmp ( path_fichier,
		       tab_noms_derniers_fichiers_ouverts[i] ))
	{
	    /* 	si ce fichier est déjà le dernier ouvert, on laisse tomber */

	    if ( !i )
		return;

	    position = i;
	}

    efface_derniers_fichiers_ouverts();

    if ( position )
    {
	/*       le fichier a été trouvé, on fait juste une rotation */

	for ( i=position ; i>0 ; i-- )
	    tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];
	if ( path_fichier )
	    tab_noms_derniers_fichiers_ouverts[0] = g_strdup ( path_fichier );
	else
	    tab_noms_derniers_fichiers_ouverts[0] = g_strdup ( "<no file>" );

	affiche_derniers_fichiers_ouverts();

	return;
    }

    /*   le fichier est nouveau, on décale tout d'un cran et on met le nouveau à 0 */

    /*   si on est déjà au max, c'est juste un décalage avec perte du dernier */
    /* on garde le ptit dernier dans le cas contraire */

    if ( nb_derniers_fichiers_ouverts )
	dernier = tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1];
    else
	dernier = NULL;

    for ( i= nb_derniers_fichiers_ouverts - 1 ; i>0 ; i-- )
	tab_noms_derniers_fichiers_ouverts[i] = tab_noms_derniers_fichiers_ouverts[i-1];

    if ( nb_derniers_fichiers_ouverts < nb_max_derniers_fichiers_ouverts )
    {
	tab_noms_derniers_fichiers_ouverts = realloc ( tab_noms_derniers_fichiers_ouverts,
						       ( ++nb_derniers_fichiers_ouverts ) * sizeof ( gpointer ));
	tab_noms_derniers_fichiers_ouverts[nb_derniers_fichiers_ouverts-1] = dernier;
    }

    tab_noms_derniers_fichiers_ouverts[0] = g_strdup ( path_fichier );


    affiche_derniers_fichiers_ouverts();
}
/* ************************************************************************************************************ */


/****************************************************************************/
void remove_file_from_last_opened_files_list ( gchar * nom_fichier )
{
    gint i, j;

    efface_derniers_fichiers_ouverts();

    for ( i = 0 ; i < nb_derniers_fichiers_ouverts ; i++ )
    {
	if ( ! strcmp (nom_fichier_comptes, tab_noms_derniers_fichiers_ouverts[i]) )
	{
	    for ( j = i; j < nb_derniers_fichiers_ouverts-1; j++ )
	    {
		tab_noms_derniers_fichiers_ouverts[j] = tab_noms_derniers_fichiers_ouverts[j+1];

	    }
	    break;
	}
    }
    nb_derniers_fichiers_ouverts--;
    affiche_derniers_fichiers_ouverts();
}
/****************************************************************************/


