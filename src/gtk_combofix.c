/* ComboFix Widget
 *
 *     Copyright (C)	2001 Cédric Auger (cedric@grisbi.org) 
 *			2003 Benjamin Drieu (bdrieu@april.org) 
 * 			http://www.grisbi.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "gtkcombofix.h"
#include <gdk/gdkkeysyms.h>
#include <ctype.h>

/* Liste des fonctions statiques */

static void gtk_combofix_class_init ( GtkComboFixClass *klass );
static void gtk_combofix_init ( GtkComboFix *combofix );
static gint classe_alphabetique ( gchar *string_1,
				  gchar *string_2 );
static gboolean change_arrow ( GtkWidget *bouton,
			       GtkComboFix *combofix );
static gboolean affiche_proposition ( GtkWidget *entree,
				      gchar *texte,
				      gint longueur,
				      gint *position,
				      GtkComboFix *combofix );
static gboolean realize_scrolled_window ( GtkWidget *vbox,
					  GtkAllocation *allocation,
					  GtkComboFix *combofix );
static gboolean gtk_combofix_button_press ( GtkWidget *widget,
					    GdkEventButton *ev,
					    GtkComboFix *combofix );
static gboolean met_en_prelight ( GtkWidget *event_box,
				  GdkEventMotion *ev,
				  GtkComboFix *combofix );
static gboolean click_sur_label ( GtkWidget *event_box,
				  GdkEventButton *ev,
				  GtkComboFix *combofix );
static gboolean  focus_out_combofix ( GtkWidget *widget,
				      GdkEvent *ev,
				      GtkComboFix *combofix );
static gboolean met_selection ( GtkWidget *entry,
				GdkRectangle *area,
				GtkComboFix *combofix );
static gboolean verifie_efface_texte ( GtkWidget *entree,
				       gint start,
				       gint end,
				       GtkComboFix *combofix );
static gboolean efface_texte ( GtkWidget *entree,
			       gint start,
			       gint end,
			       GtkComboFix *combofix );
static gboolean touche_pressee ( GtkWidget *entry,
				 GdkEventKey *ev,
				 GtkComboFix *combofix );
static GSList *classe_combofix ( GSList *liste );
static gboolean touche_pressee_dans_popup ( GtkWidget *popup,
					    GdkEventKey *event,
					    GtkComboFix *combofix );


/* Variables globales */

static gint rafraichir_selection = 0;
static gint bloque_proposition = 0;
static gint case_sensitive = 0;



/* **************************************************************************************************** */
guint gtk_combofix_get_type ( void )
{
    static guint gtk_combofix_type = 0;

    if ( !gtk_combofix_type )
    {
	static const GtkTypeInfo gtk_combofix_info = {
	    "GtkComboFix",
	    sizeof (GtkComboFix),
	    sizeof (GtkComboFixClass),
	    (GtkClassInitFunc) gtk_combofix_class_init,
	    (GtkObjectInitFunc) gtk_combofix_init,
	    NULL,
	    NULL,
	    (GtkClassInitFunc) NULL
	};

	gtk_combofix_type = gtk_type_unique ( gtk_hbox_get_type(),
					      &gtk_combofix_info );
    }

    return ( gtk_combofix_type );

}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
static void gtk_combofix_class_init ( GtkComboFixClass *klass )
{
    /*   GtkWidgetClass *widget_class; */
    /*   GtkObjectClass *object_class; */

    /*   rien pour le moment */



}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
static void gtk_combofix_init ( GtkComboFix *combofix )
{
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *frame;

    /*   création de la vbox ( permet d'éviter que le combofix s'agrandisse si on agrandit la fenetre )*/

    vbox = gtk_vbox_new ( FALSE,
			  0 );

    gtk_container_add ( GTK_CONTAINER ( combofix ),
			vbox );

    gtk_widget_show ( vbox );


    /*   création de la hbox */

    hbox = gtk_hbox_new ( FALSE,
			  0 );

    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 TRUE,
			 FALSE,
			 0 );
    gtk_widget_show ( hbox );


    /* création de l'entrée */

    combofix->entry = gtk_entry_new();

    gtk_signal_connect ( GTK_OBJECT ( combofix -> entry ),
			 "key-press-event",
			 GTK_SIGNAL_FUNC ( touche_pressee ),
			 combofix );
    gtk_signal_connect_after ( GTK_OBJECT ( combofix->entry ),
			       "insert-text",
			       GTK_SIGNAL_FUNC ( affiche_proposition ),
			       combofix );
    gtk_signal_connect ( GTK_OBJECT ( combofix->entry ),
			 "delete-text",
			 GTK_SIGNAL_FUNC ( verifie_efface_texte  ),
			 combofix );
    gtk_signal_connect_after ( GTK_OBJECT ( combofix->entry ),
			       "delete-text",
			       GTK_SIGNAL_FUNC ( efface_texte  ),
			       combofix );
    gtk_signal_connect_after ( GTK_OBJECT ( combofix->entry ),
			       "focus-out-event",
			       GTK_SIGNAL_FUNC ( focus_out_combofix ),
			       combofix );
    gtk_signal_connect ( GTK_OBJECT ( combofix->entry ),
			 "expose-event",
			 GTK_SIGNAL_FUNC ( met_selection ),
			 combofix );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 combofix->entry,
			 TRUE,
			 TRUE,
			 0 );
    gtk_widget_show ( combofix->entry );

    /*   création de l'arrow */

    combofix -> arrow = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( combofix -> arrow ),
			    GTK_RELIEF_NONE );

    gtk_container_add ( GTK_CONTAINER ( combofix -> arrow ),
			gtk_arrow_new ( GTK_ARROW_DOWN,
					GTK_SHADOW_ETCHED_OUT) );
    combofix -> affiche_liste = 1;

    gtk_signal_connect ( GTK_OBJECT ( combofix -> arrow ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( change_arrow ),
			 combofix );
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 combofix->arrow,
			 FALSE,
			 FALSE,
			 0 );
    gtk_widget_show_all ( combofix->arrow);


    /* création du menu attaché au widget */
    /* on ne crée que pour l'instant la window popup et la boite qui contiendra la liste, une scrolledWindow */

    combofix->popup = gtk_window_new ( GTK_WINDOW_POPUP );
    gtk_window_set_policy ( GTK_WINDOW ( combofix->popup ),
			    FALSE,
			    FALSE,
			    TRUE );
    gtk_signal_connect ( GTK_OBJECT ( combofix->popup ),
			 "button-press-event",
			 GTK_SIGNAL_FUNC ( gtk_combofix_button_press ),
			 combofix );
    gtk_signal_connect ( GTK_OBJECT ( combofix -> popup ),
			 "key-press-event", 
			 GTK_SIGNAL_FUNC ( touche_pressee_dans_popup ),
			 combofix );


    frame = gtk_frame_new ( NULL );
    gtk_container_add ( GTK_CONTAINER ( combofix -> popup ),
			frame );
    gtk_widget_show ( frame );

    combofix -> event_box = NULL;
    combofix -> label_selectionne = -1;


    /* pour l'instant, met la liste à 0 */

    combofix -> liste = NULL;


}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* gtk_combofix_new : */
/* arguments :             */
/* liste :                      liste de noms ( \t en début de chaine pour une sous catégorie ) */
/* force_text :            TRUE ( le texte doit correspondre à la liste ) / FALSE */
/* affiche_liste :         TRUE ( la liste sera affichée en tapant le mot ) / FALSE */
/* classement_auto : TRUE ( la liste est toujours classée par ordre alphabétique ) / FALSE  */
/* lignes_max : contient le nb maximal de lignes affichées de la liste, si 0=les affiche toutes, */
/* s'il est dépassé, n'affiche rien */
/*                                                                                                               */
/* retour : le widget gtk_combofix ( une hbox contenant l'entrée et d'une arrow ) */
/* **************************************************************************************************** */

GtkWidget *gtk_combofix_new ( GSList *liste,
			      gint force_text,
			      gint affiche_liste,
			      gint classement_auto,
			      gint lignes_max )
{
    GSList *pointeur;
    gchar *derniere_categ;
    GSList *nouvelle_liste;

    GtkComboFix *combofix = GTK_COMBOFIX ( gtk_type_new ( gtk_combofix_get_type () ) );


    /* recopie la liste originale */

    nouvelle_liste = NULL;
    pointeur = liste;

    while ( pointeur )
    {
	nouvelle_liste = g_slist_append ( nouvelle_liste,
					  g_strdup ( pointeur -> data ) );
	pointeur = pointeur -> next;
    }



    /* remplit les champs de la combofix */

    combofix -> force_text = force_text;
    combofix -> liste = nouvelle_liste;
    combofix -> affiche_liste = affiche_liste;
    combofix -> complex = 0;
    combofix -> auto_sort = classement_auto;
    combofix -> lignes_max = lignes_max;


    if ( affiche_liste )
	gtk_arrow_set ( GTK_ARROW ( GTK_BIN ( combofix -> arrow ) -> child ),
			GTK_ARROW_DOWN,
			GTK_SHADOW_ETCHED_OUT);
    else
	gtk_arrow_set ( GTK_ARROW ( GTK_BIN ( combofix -> arrow ) -> child ),
			GTK_ARROW_UP,
			GTK_SHADOW_ETCHED_OUT);


    /* création de la liste des complétions : système de catégories - sous catégories */
    /* toutes catégorie est mise dans la liste */
    /*   les sous-catég sont mises sous la forme catég : sous-catég */
    /* on trie à la fin la liste par ordre alphabétique pour que lors d'une complétion, c'est le 1er qui sorte */

    /* protège de l'erreur si on met une sous categ en 1er */

    derniere_categ = "";

    combofix -> liste_completion = NULL;
    pointeur = liste;

    while ( pointeur )
    {
	gchar *string;

	string = pointeur -> data;

	if ( string[0] == '\t' )
	    string = g_strconcat ( derniere_categ,
				   " : ",
				   string + 1,
				   NULL );
	else
	    derniere_categ = string;

	combofix -> liste_completion = g_slist_append ( combofix -> liste_completion,
							string );

	pointeur = pointeur -> next;
    }

    return ( GTK_WIDGET ( combofix ) );
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* gtk_combofix_new_complex : */
/* arguments :                             */
/* liste :                      liste de liste de noms ( \t en début de chaine pour une sous catégorie ) */
/*                               chaque liste sera séparée d'une ligne */
/* force_text :            TRUE ( le texte doit correspondre à la liste ) / FALSE */
/* affiche_liste :         TRUE ( la liste sera affichée en tapant le mot ) / FALSE */
/* classement_auto : TRUE ( la liste est toujours classée par ordre alphabétique ) / FALSE  */
/* lignes_max : contient le nb maximal de lignes affichées de la liste, si 0=les affiche toutes, */
/* s'il est dépassé, n'affiche rien */
/*                                                                                                                                    */
/* retour : le widget gtk_combofix ( une hbox contenant l'entrée et d'une arrow ) */
/* **************************************************************************************************** */

GtkWidget *gtk_combofix_new_complex ( GSList *liste,
				      gint force_text,
				      gint affiche_liste,
				      gint classement_auto,
				      gint lignes_max )
{
    GSList *pointeur;
    gchar *derniere_categ;
    GSList *nouvelle_liste;

    GtkComboFix *combofix = GTK_COMBOFIX ( gtk_type_new ( gtk_combofix_get_type () ) );

    /* recopie la liste originale */

    nouvelle_liste = NULL;
    pointeur = liste;

    while ( pointeur )
    {
	GSList *sous_pointeur;
	GSList *sous_liste;

	sous_pointeur = pointeur -> data;
	sous_liste = NULL;

	while ( sous_pointeur )
	{
	    sous_liste = g_slist_append ( sous_liste,
					  g_strdup ( sous_pointeur -> data ) );
	    sous_pointeur = sous_pointeur -> next;
	}

	nouvelle_liste = g_slist_append (nouvelle_liste,
					 sous_liste );

	pointeur = pointeur -> next;
    }

    /* remplit les champs de la combofix */

    combofix -> force_text = force_text;
    combofix -> liste = nouvelle_liste;
    combofix -> affiche_liste = affiche_liste;
    combofix -> complex = 1;
    combofix -> auto_sort = classement_auto;
    combofix -> lignes_max = lignes_max;

    if ( affiche_liste )
	gtk_arrow_set ( GTK_ARROW ( GTK_BIN ( combofix -> arrow ) -> child ),
			GTK_ARROW_DOWN,
			GTK_SHADOW_ETCHED_OUT);
    else
	gtk_arrow_set ( GTK_ARROW ( GTK_BIN ( combofix -> arrow ) -> child ),
			GTK_ARROW_UP,
			GTK_SHADOW_ETCHED_OUT);

    /* création de la liste des complétions : système de catégories - sous catégories */
    /* toutes catégorie est mise dans la liste */
    /*   les sous-catég sont mises sous la forme catég : sous-catég */

    /* protège de l'erreur si on met une sous categ en 1er */

    derniere_categ = "";

    combofix -> liste_completion = NULL;
    pointeur = liste;

    while ( pointeur )
    {
	gchar *string;
	GSList *sous_pointeur;

	sous_pointeur = pointeur -> data;

	while ( sous_pointeur )
	{
	    string = sous_pointeur -> data;

	    if ( string[0] == '\t' )
		string = g_strconcat ( derniere_categ,
				       " : ",
				       string + 1,
				       NULL );
	    else
		derniere_categ = string;

	    combofix -> liste_completion = g_slist_append ( combofix -> liste_completion,
							    string );

	    sous_pointeur = sous_pointeur -> next;
	}
	pointeur = pointeur -> next;
    }

    return ( GTK_WIDGET ( combofix ) );
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* Fonction classe_combofix */
/* **************************************************************************************************** */

static gint classe_alphabetique ( gchar *string_1,
				  gchar *string_2 )
{
    /*   comme la fonction g_strcasecmp met les accents derrière, on magouille pour les prendre en compte */

    string_1 = g_strdup ( string_1 );
    string_1 = g_strdelimit ( string_1,
			      "éÉèÈêÊ",
			      'e' );
    string_1 = g_strdelimit ( string_1,
			      "çÇ",
			      'c' );
    string_1 = g_strdelimit ( string_1,
			      "àÀ",
			      'a' );
    string_1 = g_strdelimit ( string_1,
			      "ùûÙÛ",
			      'u' );
    string_1 = g_strdelimit ( string_1,
			      "ôÔ",
			      'o' );
    string_1 = g_strdelimit ( string_1,
			      "îÎ",
			      'i' );

    string_2 = g_strdup ( string_2 );
    string_2 = g_strdelimit ( string_2,
			      "éÉèÈêÊ",
			      'e' );
    string_2 = g_strdelimit ( string_2,
			      "çÇ",
			      'c' );
    string_2 = g_strdelimit ( string_2,
			      "àÀ",
			      'a' );
    string_2 = g_strdelimit ( string_2,
			      "ùûÙÛ",
			      'u' );
    string_2 = g_strdelimit ( string_2,
			      "ôÔ",
			      'o' );
    string_2 = g_strdelimit ( string_2,
			      "îÎ",
			      'i' );



    return ( g_strcasecmp ( string_1,
			    string_2 ));

}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* Fonction : change_arrow : */
/* appelée lors de click sur la flèche */
/* **************************************************************************************************** */

static gboolean change_arrow ( GtkWidget *bouton,
			       GtkComboFix *combofix )
{
    /* nécessaire de mettre le grab_focus en premier pour le fonctionnement avec grisbi */

    gtk_widget_grab_focus ( combofix -> entry );

    if ( combofix -> affiche_liste && GTK_WIDGET_VISIBLE ( combofix -> popup ) )
    {
	combofix -> affiche_liste = 0;
	gtk_arrow_set ( GTK_ARROW ( GTK_BIN ( combofix -> arrow ) -> child ),
			GTK_ARROW_UP,
			GTK_SHADOW_ETCHED_OUT);

	gtk_grab_remove ( combofix -> popup );
	gdk_pointer_ungrab ( GDK_CURRENT_TIME );
	gtk_widget_hide ( combofix->popup );
    }
    else
    {
	combofix -> affiche_liste = 1;
	combofix -> label_selectionne = -1;
	gtk_arrow_set ( GTK_ARROW ( GTK_BIN ( combofix -> arrow ) -> child ),
			GTK_ARROW_DOWN,
			GTK_SHADOW_ETCHED_OUT);
	affiche_proposition ( GINT_TO_POINTER (-1), "", 0, 0, combofix );
    }

    return TRUE;
} 
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* fonction appelée à la fin lorsqu'une lettre est tapée */
/* ou lorsque du texte est effacé : dans ce cas entree = NULL */
/* **************************************************************************************************** */

static gboolean affiche_proposition ( GtkWidget *entree,
				      gchar *texte,
				      gint longueur,
				      gint *position,
				      GtkComboFix *combofix )
{
    GSList *liste_tmp, *liste_affichee;
    GtkStyle *style_label;
    GdkColor couleur_bleue;
    GtkWidget *liste, *scrolled_window, *label, *event_box;
    gint menu_rempli, menu_rempli_ok, ligne_en_cours, i;
    gchar *chaine, *completion, *categorie;

    if (bloque_proposition)
	return(FALSE);

#define COULEUR_RED  40000
#define COULEUR_GREEN  40000
#define COULEUR_BLUE  65535

    /* Initialisation des couleurs */

    couleur_bleue.red = COULEUR_RED ;
    couleur_bleue.green = COULEUR_GREEN;
    couleur_bleue.blue = COULEUR_BLUE;

    /*   pour éviter un warning lors de la compil : */

    categorie = "";


    /*   si la liste de mot est nulle, soit force_text n'est pas mis, et on vire, soit il est mis, */
    /* et on vide l'entrée avant de virer */

    if ( !combofix->liste  )
    {
	if ( combofix -> force_text )
	{
	    gtk_signal_handler_block_by_func ( GTK_OBJECT ( combofix->entry ),
					       GTK_SIGNAL_FUNC ( efface_texte ),
					       combofix );
	    gtk_editable_delete_text ( GTK_EDITABLE ( combofix->entry ),
				       0,
				       -1 );
	    gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( combofix->entry ),
						 GTK_SIGNAL_FUNC ( efface_texte ),
						 combofix );
	    if ( position )
		*position = 0;
	}

	return TRUE;
    }

    /* recherche de la complétion */

recherche_completion:

    for ( i=0; texte && i < longueur; i++)
    {
	if ( isupper(texte[i]) )
	{
	    case_sensitive = 1;
	    break;
	}
    }

    chaine = (gchar *) gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ));
    completion = NULL;

    if ( chaine && strlen ( chaine ) )
    {
	liste_tmp = combofix-> liste_completion;

	while ( liste_tmp && !completion )
	{
	    if ( !case_sensitive )
	    {
		if ( !g_strncasecmp ( chaine, liste_tmp->data, strlen ( chaine ) ) )
		    completion = liste_tmp -> data;
	    }
	    else
	    {
		if ( !strncmp ( chaine, liste_tmp->data, strlen ( chaine ) ) )
		    completion = liste_tmp -> data;
	    }

	    liste_tmp = liste_tmp -> next;
	}
    }
    else
	completion = "";

    /* commence par vérifier l'entrée si force_text = 1 */
    /* si le mot n'existe pas dans la liste autorisée, efface ce qui a été ajouté */

    if ( combofix -> force_text
	 &&
	 !completion
	 &&
	 entree )
    {
	gtk_signal_handler_block_by_func ( GTK_OBJECT ( combofix->entry ),
					   GTK_SIGNAL_FUNC ( efface_texte ),
					   combofix );
	if ( position )
	{
	    gtk_editable_delete_text ( GTK_EDITABLE ( combofix->entry ),
				       *position - longueur,
				       *position );
	    (*position) = *position - longueur;
	}
	else
	    gtk_editable_delete_text ( GTK_EDITABLE ( combofix->entry ),
				       0,
				       -1 );

	gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( combofix->entry ),
					     GTK_SIGNAL_FUNC ( efface_texte ),
					     combofix );

	goto recherche_completion;
    }


    /*   si on voulait juste une complétion sans popup, on la fait et on s'en va */
    /* ne fait pas la complétion si c'est du texte effacé */

    if ( entree && !combofix -> affiche_liste && completion )
    {
	if ( completion )
	{
	    gtk_combofix_set_text ( combofix,
				    completion );

	    /*  l'affichage de la sélection se fera plus tard ( une fois l'objet dessiné ) */

	    rafraichir_selection = 1;
	}
	return TRUE;
    }


    /* on commence à récupérer toutes les lignes qu'on va inclure dans la box */
    /* si le nb de lignes est > à lignes_max, on n'affiche pas */


    /* si on a clické sur l'arrow, on affiche toute la liste */

    if ( entree == GINT_TO_POINTER ( -1 ) )
	chaine = texte;
    else
	chaine = (char *) gtk_entry_get_text ( GTK_ENTRY ( combofix->entry ));


    liste_tmp = combofix->liste;
    liste_affichee = NULL;
    ligne_en_cours = 0;

    if ( combofix -> complex )
	while ( liste_tmp )
	{
	    GSList *sous_liste_tmp;
	    GSList *sous_liste_affichee;

	    /* protection si pas de catég à une sous-categ */

	    categorie = "";

	    sous_liste_tmp = liste_tmp -> data;
	    sous_liste_affichee = NULL;

	    while ( sous_liste_tmp )
	    {
		gchar *string;

		string = g_strdup ( sous_liste_tmp -> data );

		/* la comparaison est différente selon que ce soit une catég ou une sous catég : */
		/*  pour une catég, on la compare directement avec l'entrée */
		/* pour une sous-catég, on compare categ : sous-categ avec l'entrée */

		if ( string[0] != '\t' )
		{
		    gchar *string_tot;

		    /* c'est une catégorie */
		    /* sauvegarde de la catégorie pour tester les sous-catégories */

		    categorie = string;
		    string_tot = g_strconcat ( string,
					       " : ",
					       NULL );
		    if ( !g_strncasecmp ( chaine,
					  string_tot,
					  strlen ( chaine )) 
			 ||
			 !g_strncasecmp ( chaine,
					  string_tot,
					  strlen ( string_tot )))

		    {
			/* cette catégorie devra être affichée */

			sous_liste_affichee = g_slist_append ( sous_liste_affichee,
							       string );
			ligne_en_cours++;
		    }
		}
		else
		{
		    if ( !g_strncasecmp ( chaine,
					  g_strconcat ( categorie,
							" : ",
							string + 1,
							NULL ),
					  strlen ( chaine )))
		    {
			/* cette sous-catég devra être affichée */

			sous_liste_affichee = g_slist_append ( sous_liste_affichee,
							       string );
			ligne_en_cours++;
		    }
		}

		if ( combofix -> lignes_max
		     &&
		     ligne_en_cours > combofix -> lignes_max )
		{
		    if ( GTK_WIDGET_VISIBLE ( combofix->popup ))
		    {
			gtk_widget_hide ( combofix->popup );

			gtk_grab_remove ( combofix -> popup );
			gdk_pointer_ungrab ( GDK_CURRENT_TIME );
		    }
		    return TRUE;
		}
		sous_liste_tmp = sous_liste_tmp -> next;
	    }

	    /* on a fait le tour de la sous-liste du combofix complex */
	    /* on ajoute la sous-liste des textes à afficher dans liste_affichee */

	    liste_affichee = g_slist_append ( liste_affichee,
					      sous_liste_affichee );

	    liste_tmp = liste_tmp -> next;
	}
    else
	while ( liste_tmp )
	{
	    gchar *string;

	    /* protection si pas de catég à une sous-categ */

	    categorie = "";

	    string = liste_tmp -> data;

	    /* la comparaison est différente selon que ce soit une catég ou une sous catég : */
	    /*  pour une catég, on la compare directement avec l'entrée */
	    /* pour une sous-catég, on compare categ : sous-categ avec l'entrée */

	    if ( string[0] != '\t' )
	    {
		gchar *string_tot;

		/* sauvegarde de la catégorie pour tester les sous-catégories */

		categorie = string;
		string_tot = g_strconcat ( string,
					   " : ",
					   NULL );
		if ( !g_strncasecmp ( chaine,
				      string_tot,
				      strlen ( chaine ) ) 
		     ||
		     !g_strncasecmp ( chaine,
				      string_tot,
				      strlen ( string_tot )))

		{
		    liste_affichee = g_slist_append ( liste_affichee,
						      string );
		    ligne_en_cours++;
		}
	    }
	    else
	    {
		if ( !g_strncasecmp ( chaine,
				      g_strconcat ( categorie,
						    " : ",
						    string + 1,
						    NULL ),
				      strlen ( chaine ) ) )
		{
		    liste_affichee = g_slist_append ( liste_affichee,
						      string );
		    ligne_en_cours++;
		}
	    }

	    if ( combofix -> lignes_max
		 &&
		 ligne_en_cours > combofix -> lignes_max )
	    {
		if ( GTK_WIDGET_VISIBLE ( combofix->popup ))
		{
		    gtk_widget_hide ( combofix->popup );

		    gtk_grab_remove ( combofix -> popup );
		    gdk_pointer_ungrab ( GDK_CURRENT_TIME );
		}
		return TRUE;
	    }

	    liste_tmp = liste_tmp -> next;
	}


    /*   si on est censé trier, c'est ici, où on ne trie que les lignes qu'on va afficher */


    if ( combofix -> auto_sort )
    {
	if ( combofix -> complex )
	{
	    liste_tmp = liste_affichee;

	    while ( liste_tmp )
	    {
		liste_tmp -> data = classe_combofix ( liste_tmp -> data );
		liste_tmp = liste_tmp -> next;
	    }
	}
	else
	{
	    liste_affichee = classe_combofix ( liste_affichee );
	    /* 	  combofix -> liste_completion = g_slist_sort ( combofix -> liste_completion, */
	    /* 							(GCompareFunc) classe_alphabetique ); */
	}
    }


    /* on crée la nouvelle box si necessaire en fonction du contenu de l'entrée */
    /* et la complétion */

    scrolled_window = gtk_scrolled_window_new ( FALSE,
						FALSE );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );

    liste = gtk_vbox_new ( TRUE,
			   0 );
    gtk_signal_connect_after ( GTK_OBJECT ( liste ),
			       "size-allocate",
			       GTK_SIGNAL_FUNC ( realize_scrolled_window ),
			       combofix );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window ),
					    liste );

    if ( combofix->event_box )
	g_slist_free ( combofix->event_box );

    combofix->event_box = NULL;

    style_label = gtk_style_copy ( gtk_widget_get_style (  combofix -> arrow ));

    style_label->bg[GTK_STATE_PRELIGHT] = couleur_bleue;




    liste_tmp = liste_affichee;

    menu_rempli = 0;
    menu_rempli_ok = 0;

    if ( combofix -> complex )
	while ( liste_tmp )
	{
	    GSList *sous_liste_tmp;

	    sous_liste_tmp = liste_tmp -> data;

	    while ( sous_liste_tmp )
	    {
		gchar *string;

		string = g_strdup ( sous_liste_tmp -> data );

		if ( string[0] != '\t' )
		{
		    /* c'est une catégorie, on la met dans la liste */


		    /* on met la catég dans la variable qui sera utilisée dans l'affichage de la sous catég */

		    categorie = string;

		    /* permet d'éviter d'avoir une barre en haut */

		    menu_rempli_ok = 1;

		    /* on met une séparation si nécessaire */

		    if ( menu_rempli )
		    {
			label = gtk_hseparator_new ();
			gtk_box_pack_start ( GTK_BOX ( liste ),
					     label,
					     TRUE,
					     TRUE,
					     0 );
			combofix->event_box = g_slist_append ( combofix->event_box,
							       label );
			menu_rempli = 0;
		    }

		    /* création de l'event_box */

		    event_box = gtk_event_box_new ();
		    gtk_widget_set_style ( event_box,
					   style_label );
		    gtk_object_set_data ( GTK_OBJECT ( event_box ),
					  "texte",
					  string );
		    gtk_signal_connect ( GTK_OBJECT ( event_box ),
					 "enter-notify-event",
					 GTK_SIGNAL_FUNC ( met_en_prelight ),
					 combofix );
		    gtk_signal_connect ( GTK_OBJECT ( event_box ),
					 "button-press-event",
					 GTK_SIGNAL_FUNC ( click_sur_label ),
					 combofix );

		    /* création du label */

		    label = gtk_label_new ( string );

		    gtk_misc_set_alignment ( GTK_MISC ( label ),
					     0,
					     0.5 );
		    gtk_widget_set_style ( label,
					   style_label );
		    gtk_container_add ( GTK_CONTAINER ( event_box ),
					label );

		    gtk_box_pack_start ( GTK_BOX ( liste ),
					 event_box,
					 TRUE,
					 TRUE,
					 0 );
		    combofix->event_box = g_slist_append ( combofix->event_box,
							   event_box );

		}
		else
		{
		    /* c'est une sous-categ, on la met dans la liste avec un espace devant */

		    /* création de l'event_box */

		    event_box = gtk_event_box_new ();
		    gtk_widget_set_style ( event_box,
					   style_label );
		    gtk_object_set_data ( GTK_OBJECT ( event_box ),
					  "texte",
					  g_strconcat ( categorie,
							" : ",
							string + 1,
							NULL ));
		    gtk_signal_connect ( GTK_OBJECT ( event_box ),
					 "enter-notify-event",
					 GTK_SIGNAL_FUNC ( met_en_prelight ),
					 combofix );
		    gtk_signal_connect ( GTK_OBJECT ( event_box ),
					 "button-press-event",
					 GTK_SIGNAL_FUNC ( click_sur_label ),
					 combofix );

		    string = g_strdelimit ( string,
					    "\t",
					    ' ' );
		    string = g_strconcat ( "     ",
					   string,
					   NULL );

		    label = gtk_label_new ( string );

		    gtk_misc_set_alignment ( GTK_MISC ( label ),
					     0,
					     0.5 );
		    gtk_widget_set_style ( label,
					   style_label );
		    gtk_container_add ( GTK_CONTAINER ( event_box ),
					label );

		    gtk_box_pack_start ( GTK_BOX ( liste ),
					 event_box,
					 TRUE,
					 TRUE,
					 0 );
		    combofix->event_box = g_slist_append ( combofix->event_box,
							   event_box );
		}
		sous_liste_tmp = sous_liste_tmp -> next;
	    }

	    if ( menu_rempli_ok )
	    {
		menu_rempli = 1;
		menu_rempli_ok = 0;
	    }

	    liste_tmp = liste_tmp -> next;
	}
    else
	while ( liste_tmp )
	{
	    gchar *string;

	    string = liste_tmp -> data;

	    if ( string[0] != '\t' )
	    {
		/* c'est une categ, on l'affiche */

		/* on met la catég dans la variable qui sera utilisée dans l'affichage de la sous catég */

		categorie = string;

		/* création de l'event_box */

		event_box = gtk_event_box_new ();
		gtk_widget_set_style ( event_box,
				       style_label );
		gtk_object_set_data ( GTK_OBJECT ( event_box ),
				      "texte",
				      string );
		gtk_signal_connect ( GTK_OBJECT ( event_box ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     combofix );
		gtk_signal_connect ( GTK_OBJECT ( event_box ),
				     "button-press-event",
				     GTK_SIGNAL_FUNC ( click_sur_label ),
				     combofix );


		/* création du label */

		label = gtk_label_new ( string );

		gtk_misc_set_alignment ( GTK_MISC ( label ),
					 0,
					 0.5 );
		gtk_widget_set_style ( label,
				       style_label );
		gtk_container_add ( GTK_CONTAINER ( event_box ),
				    label );

		gtk_box_pack_start ( GTK_BOX ( liste ),
				     event_box,
				     TRUE,
				     TRUE,
				     0 );
		combofix->event_box = g_slist_append ( combofix->event_box,
						       event_box );
	    }
	    else
	    {
		/* c'est une sous-categ, on la met dans la liste avec un espace devant */

		/* création de l'event_box */

		event_box = gtk_event_box_new ();
		gtk_widget_set_style ( event_box,
				       style_label );
		gtk_object_set_data ( GTK_OBJECT ( event_box ),
				      "texte",
				      g_strconcat ( categorie,
						    " : ",
						    string + 1,
						    NULL ));
		gtk_signal_connect ( GTK_OBJECT ( event_box ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     combofix );
		gtk_signal_connect ( GTK_OBJECT ( event_box ),
				     "button-press-event",
				     GTK_SIGNAL_FUNC ( click_sur_label ),
				     combofix );

		string = g_strconcat ( "     ",
				       string,
				       NULL );

		label = gtk_label_new ( string );

		gtk_misc_set_alignment ( GTK_MISC ( label ),
					 0,
					 0.5 );
		gtk_widget_set_style ( label,
				       style_label );
		gtk_container_add ( GTK_CONTAINER ( event_box ),
				    label );

		gtk_box_pack_start ( GTK_BOX ( liste ),
				     event_box,
				     TRUE,
				     TRUE,
				     0 );
		combofix->event_box = g_slist_append ( combofix->event_box,
						       event_box );
	    }
	    liste_tmp = liste_tmp -> next;
	}




    if ( entree && completion )
    {
	/* on affiche maintenant la complétion dans le combofix */

	gtk_combofix_set_text ( combofix,
				completion );

	/*  l'affichage de la sélection se fera plus tard ( une fois l'objet dessiné ) */

	rafraichir_selection = 1;
    }


    /*la liste est maintenant créée ; si la popup était déjà faite, on remplace l'ancienne liste, sinon, on crée la popup */
    /* la position et la taille de la popup seront calculées quand la taille sera allouée */

    combofix -> label_selectionne = -1;

    if ( !GTK_WIDGET_REALIZED ( combofix->popup ) )
    {
	if ( !completion || !g_slist_length ( combofix->event_box ))
	    return TRUE;

	gtk_container_add ( GTK_CONTAINER ( GTK_BIN ( combofix -> popup ) -> child  ),
			    scrolled_window );
	combofix -> box = scrolled_window;

	gtk_widget_show_all ( combofix->popup );

	gtk_grab_add ( combofix -> popup );
	gdk_pointer_grab (combofix->popup->window, 
			  TRUE,
			  GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			  GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
			  GDK_POINTER_MOTION_MASK,
			  NULL, 
			  NULL, 
			  GDK_CURRENT_TIME);
    }
    else
	if ( !GTK_WIDGET_VISIBLE ( combofix->popup ) )
	{
	    gfloat value;

	    if ( !completion  || !g_slist_length ( combofix->event_box ))
		return TRUE;

	    value = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW  ( combofix->box ) ) -> value;

	    gtk_widget_destroy ( combofix->box );

	    gtk_container_add ( GTK_CONTAINER ( GTK_BIN ( combofix -> popup ) -> child ),
				scrolled_window );
	    combofix -> box = scrolled_window;

	    gtk_widget_show_all ( combofix->popup );

	    gtk_grab_add ( combofix -> popup );
	    gdk_pointer_grab (combofix->popup->window, 
			      TRUE,
			      GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
			      GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
			      GDK_POINTER_MOTION_MASK,
			      NULL, 
			      NULL, 
			      GDK_CURRENT_TIME);
	    gtk_adjustment_set_value ( gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW  ( scrolled_window ) ),
				       value );
	}
	else
	{

	    if ( !completion  || !g_slist_length ( combofix->event_box ))
	    {
		gtk_widget_hide ( combofix->popup );

		gtk_grab_remove ( combofix -> popup );
		gdk_pointer_ungrab ( GDK_CURRENT_TIME );
		return TRUE;
	    }

	    gtk_widget_destroy ( combofix->box );

	    gtk_container_add ( GTK_CONTAINER ( GTK_BIN ( combofix -> popup ) -> child ),
				scrolled_window );
	    combofix -> box = scrolled_window;
	    gtk_widget_show_all ( combofix->popup );

	}

    return TRUE;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
static gboolean realize_scrolled_window ( GtkWidget *vbox,
					  GtkAllocation *allocation,
					  GtkComboFix *combofix )
{
    gint x, y;
    gint hauteur;

    gtk_signal_disconnect_by_func ( GTK_OBJECT ( vbox ),
				    GTK_SIGNAL_FUNC ( realize_scrolled_window ),
				    combofix );


    /* récupère la position de l'entry pour définir x et y du placement de la fenêtre */

    gdk_window_get_origin ( GTK_WIDGET ( combofix->entry )->window,
			    &x,
			    &y );

    hauteur = vbox -> requisition.height + 10;

    /*   si la largeur de la liste est > à celle du combofix, une scrollbar sera affichée en bas, on augment la hauteur */

    if ( vbox -> requisition.width > GTK_WIDGET ( combofix -> entry ) -> allocation.width )
	hauteur = hauteur + GTK_SCROLLED_WINDOW  ( combofix->box ) -> hscrollbar -> requisition.height + 5;



    /*   si la popup est trop petite pour contenir toute la liste et que la place vers le haut est 2 fois plus grande que vers le bas, */
    /*     on affiche la popup au dessus de l'entrée */

    if ( (( gdk_screen_height () - y - GTK_WIDGET ( combofix ) -> allocation.height ) < hauteur )
	 &&
	 ( ( ( gdk_screen_height () - y ) * 3 ) < y ) )
    {
	/* on met la popup au dessus de l'entrée */

	if ( y > hauteur )
	    y = y - hauteur;
	else
	{
	    hauteur = y;
	    y = 0;
	}
    }
    else
    {
	/* on met la popup en dessous de l'entrée */

	y = y + GTK_WIDGET ( combofix ) -> requisition.height;

	if ( ( gdk_screen_height () - y ) < hauteur )
	    hauteur = gdk_screen_height () - y;

    }


    gtk_widget_set_usize ( GTK_WIDGET ( combofix->popup ),
			   GTK_WIDGET ( combofix ) ->allocation.width,
			   hauteur );


    gtk_widget_set_uposition ( GTK_WIDGET ( combofix->popup ),
			       x,
			       y );

    return TRUE;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* fonction appelée lorsque la popup reçoit un évènement button_press */
/* résultat : on vire la popup */
/* une autre fonction se charge de récupérer le label si c'est un click sur la popup */
/* **************************************************************************************************** */

static gboolean gtk_combofix_button_press ( GtkWidget *widget,
					    GdkEventButton *ev,
					    GtkComboFix *combofix )
{
    /*   si le click est sur la popup ( pour éviter que la popup vire si on click sur les barres de défilement ), on fait rien */

    if ( ( ev -> x_root > ( GTK_WIDGET ( combofix->popup ) -> allocation.x ))
	 &&
	 ( ev -> x_root < ( GTK_WIDGET ( combofix->popup ) -> allocation.x + GTK_WIDGET ( combofix->popup ) -> allocation. width ))
	 &&
	 ( ev -> y_root > ( GTK_WIDGET ( combofix->popup ) -> allocation.y ))
	 &&
	 ( ev -> x_root < ( GTK_WIDGET ( combofix->popup ) -> allocation.y +  GTK_WIDGET ( combofix->popup ) -> allocation. height)))
	return TRUE;

    gtk_widget_hide (combofix->popup);

    gtk_grab_remove ( combofix -> popup );
    gdk_pointer_ungrab ( GDK_CURRENT_TIME );

    return TRUE;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
static gboolean met_en_prelight ( GtkWidget *event_box,
				  GdkEventMotion *ev,
				  GtkComboFix *combofix )
{
    if ( !combofix->event_box )
    {
	combofix -> label_selectionne = 0;
	return TRUE;
    }

    /* efface l'ancienne sélection */

    if ( combofix -> label_selectionne != -1 )
	gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
			       GTK_STATE_NORMAL );

    gtk_widget_set_state ( GTK_WIDGET ( event_box ),
			   GTK_STATE_PRELIGHT );

    /* met à jour la variable label_selectionne */

    combofix -> label_selectionne  = g_slist_index ( combofix -> event_box,
						     event_box );

    return TRUE;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
static gboolean click_sur_label ( GtkWidget *event_box,
				  GdkEventButton *ev,
				  GtkComboFix *combofix )
{
    gtk_combofix_set_text ( combofix,
			    gtk_object_get_data ( GTK_OBJECT ( event_box ), "texte" ) );

    gtk_grab_remove ( combofix -> popup );
    gdk_pointer_ungrab ( GDK_CURRENT_TIME );
    gtk_widget_hide ( combofix->popup );

    return TRUE;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
static gboolean focus_out_combofix ( GtkWidget *widget,
				     GdkEvent *ev,
				     GtkComboFix *combofix )
{
    gint x,y;

    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( widget ),
				   "focus-out-event" );

    case_sensitive = 0;

    gtk_widget_get_pointer ( GTK_WIDGET ( combofix ),
			     &x,
			     &y );

    /*   si la popup était affichée, on la cache et on libère le grab pointer */

    if ( GTK_WIDGET_VISIBLE ( combofix -> popup ) )
    {
	gtk_grab_remove ( combofix -> popup );
	gdk_pointer_ungrab ( GDK_CURRENT_TIME );

	/*       n'efface que si ce n'est pas un click sur le combofix ( cad si c'est pas sur le bouton ) */
	if ( gdk_window_at_pointer ( &x,&y ) != GTK_WIDGET ( combofix->arrow ) -> window )
	    gtk_widget_hide ( combofix->popup );
    }

    /*   si force_text est mis, il faut que le texte corresponde, sinon, met le 1er trouvé */

    if ( combofix -> force_text )
    {
	gchar *chaine;
	GSList *liste_tmp;

	chaine = g_strstrip ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ) ) );

	if ( !chaine || !strlen ( chaine ) )
	    return TRUE;

	liste_tmp = combofix->liste_completion;

	/* recherche de la chaine tapée dans la liste et remplit l'entrée de la 1ère*/

	do
	    if ( !g_strncasecmp ( chaine,
				  liste_tmp->data, 
				  strlen ( chaine ) ) )
	    {
		gtk_combofix_set_text ( combofix,
					liste_tmp->data );
		liste_tmp = NULL;
	    }
	    else
		liste_tmp = liste_tmp -> next;
	while ( liste_tmp );

    }

    /* vire la selection */

    gtk_entry_select_region ( GTK_ENTRY ( combofix -> entry ),
			      0,
			      0 );
    return ( FALSE );
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
static gboolean met_selection ( GtkWidget *entry,
				GdkRectangle *area,
				GtkComboFix *combofix )
{
    if ( GTK_WIDGET_HAS_FOCUS ( combofix->entry ) && rafraichir_selection == 1 )
    {
	gtk_entry_select_region ( GTK_ENTRY ( combofix->entry ),
				  gtk_editable_get_position ( GTK_EDITABLE ( combofix->entry )),
				  -1);
	rafraichir_selection = 0;
    }

    return FALSE;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* vérifie avant d'effacer une partie du texte si force_text est mis, que le résultat fait partit de la liste */
/* **************************************************************************************************** */
static gboolean verifie_efface_texte ( GtkWidget *entree,
				       gint start,
				       gint end,
				       GtkComboFix *combofix )
{
    gchar *chaine;
    gint longueur;
    GSList *liste_tmp;


    /* si force_text n'est pas mis, on s'en va */

    if ( !combofix->force_text )
	return TRUE;

    /*   si la liste est nulle, on s'en va */

    if ( !combofix -> liste )
	return TRUE;

    /* crée la future chaine */

    chaine = g_strdup ( gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry ) ));
    longueur = strlen ( chaine );

    if ( end > longueur )
	end = longueur;

    memmove ( chaine + start, chaine + end, longueur - end );
    chaine [longueur - end + start] = 0;

    /* vérifie que la chaine fait partie de la liste */

    liste_tmp = combofix->liste_completion;

    do
	if ( !g_strncasecmp ( chaine,
			      liste_tmp->data, 
			      strlen ( chaine ) ) )
	    /*       la partie du mot fait partie de la liste -> on s'en va */
	    return TRUE;
    while ( ( liste_tmp = liste_tmp -> next ) );

    /*   pas trouvé dans la liste -> stoppe le signal */

    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( combofix -> entry ),
				   "delete_text" );

    /*   la position du curseur a déjà été modifiée, on le remet à sa place */

    if( gtk_editable_get_position ( GTK_EDITABLE ( combofix -> entry ) ) == start )
	gtk_editable_set_position ( GTK_EDITABLE ( combofix -> entry ),
				    end );
    else
	gtk_editable_set_position ( GTK_EDITABLE ( combofix -> entry ),
				    start );

    return TRUE;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* lorsque du texte est effacé, on recrée la popup mais plus de sélection */
/* en mettant le 1er arg de affiche_proposition à NULL */
/* **************************************************************************************************** */

static gboolean efface_texte ( GtkWidget *entree,
			       gint start,
			       gint end,
			       GtkComboFix *combofix )
{

    if ( ( !GTK_WIDGET_VISIBLE ( combofix -> popup )
	   ||
	   ( strlen ( gtk_entry_get_text ( GTK_ENTRY ( combofix -> entry )))
	     ||
	     ( end - start ) ))
	 &&
	 combofix -> affiche_liste )
	affiche_proposition ( NULL, "", 0, 0, combofix );

    if ( !strlen ( gtk_entry_get_text ( GTK_ENTRY(combofix -> entry) ) ) )
    {
	case_sensitive = 0;
    }

    return TRUE;

}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* fonction gtk_combofix_set_text */
/* fonction externe qui prend comme argument le combofix et le texte à mettre dedans */
/* **************************************************************************************************** */

void gtk_combofix_set_text ( GtkComboFix *combofix,
			     gchar *text )
{

    g_return_if_fail (combofix);
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));
    g_return_if_fail ( text);

    /* FIXME: gruik but I cannot understand how to *completely* inhibit
       this signal ... this happened to work fine with GTK1.2 [benj] */
    bloque_proposition = 1;
    gtk_entry_set_text ( GTK_ENTRY ( combofix -> entry ),
			 text );
    bloque_proposition = 0;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* Fonction gtk_combofix_force_text : */
/* argument : la combofix */
/*                     booleen TRUE ( force ) / FALSE */
/* **************************************************************************************************** */

void gtk_combofix_force_text ( GtkComboFix *combofix,
			       gint valeur )
{

    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    combofix->force_text = valeur;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* Fonction gtk_combofix_view_list : */
/* argument :  la combofix */
/*                      booleen TRUE ( met la liste ) / FALSE */
/* **************************************************************************************************** */

void gtk_combofix_view_list ( GtkComboFix *combofix,
			      gint valeur )
{

    g_return_if_fail (combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));


    /* si pas de changement par rapport à ce qui est affiché -> dégage */

    if ( ( valeur == FALSE && combofix->affiche_liste == 0 ) || ( valeur != FALSE && combofix->affiche_liste == 1 ))
	return;

    change_arrow ( combofix->arrow,
		   combofix );

}
/* **************************************************************************************************** */

/* **************************************************************************************************** */
/* fonction gtk_combofix_get_text */
/* arguments : le combofix  */
/* retourne un pointeur vers le texte  dans le combofix */
/* **************************************************************************************************** */

gchar *gtk_combofix_get_text ( GtkComboFix *combofix )
{

    g_return_val_if_fail (combofix , NULL);
    g_return_val_if_fail (GTK_IS_COMBOFIX (combofix), NULL);


    return ( (gchar *) gtk_entry_get_text ( GTK_ENTRY ( combofix->entry ) ));
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */

static gboolean touche_pressee ( GtkWidget *entry,
				 GdkEventKey *ev,
				 GtkComboFix *combofix )
{
    GtkAdjustment *ajustement;

    if ( !combofix -> liste )
	return FALSE;

    switch ( ev -> keyval )
    {

	/* touche gauche ou droite pressée s'il y a une sélection, vire la
	   sélection et place le curseur à la fin de cette sélection */
	case GDK_Left :
	case GDK_KP_Left:
	case GDK_Right :
	case GDK_KP_Right:
	case GDK_Tab :

	    gtk_grab_remove ( combofix -> popup );
	    gdk_pointer_ungrab ( GDK_CURRENT_TIME );
	    gtk_widget_hide ( combofix->popup );
	    return FALSE;

	    /* touche entrée : prend le label sélectionné puis vire la popup */
	case GDK_KP_Enter :
	case GDK_Return :

	    if ( GTK_WIDGET_VISIBLE ( combofix -> popup ) &&   combofix -> label_selectionne != -1 )
	    {
		gtk_combofix_set_text ( combofix,
					gtk_object_get_data ( GTK_OBJECT ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ), "texte" ) );
		gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entry ),
					       "key-press-event" );
	    }

	    /* touches échap  : vire la popup et  la sélection */
	case GDK_Escape:

	    if ( GTK_WIDGET_VISIBLE ( combofix -> popup ))
	    {
		gtk_grab_remove ( combofix -> popup );
		gdk_pointer_ungrab ( GDK_CURRENT_TIME );
		gtk_widget_hide ( combofix->popup );

		gtk_entry_select_region ( GTK_ENTRY ( combofix -> entry ),
					  0,
					  0 );
		gtk_signal_emit_stop_by_name ( GTK_OBJECT ( entry ),
					       "key-press-event" );
	    }
	    break;

	case GDK_Down :		/* touches flèche bas */
	case GDK_KP_Down :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( combofix -> entry ),
					   "key-press-event" );
	    /*     si la popup n'est pas affichée à cause de l'arrow, descend l'arrow, affiche la liste */
	    if ( combofix -> affiche_liste == 0 || !GTK_WIDGET_VISIBLE ( combofix->popup ) )
	    {
		gtk_widget_grab_focus ( GTK_WIDGET ( combofix -> arrow ));
		change_arrow ( NULL,
			       combofix );
		gtk_widget_grab_focus ( GTK_WIDGET ( combofix -> entry ));
	    }
	    /*       si la popup est affichée, on descend dans la liste, sinon, c'est qu'il n'y avait aucune sélection possible */

	    if ( GTK_WIDGET_VISIBLE ( combofix->popup ) )
	    {
		if ( combofix -> label_selectionne < ( g_slist_length ( combofix->event_box )-1) || combofix -> label_selectionne == -1 )
		{
		    if ( combofix -> label_selectionne != -1 )
			gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
					       GTK_STATE_NORMAL );

		    combofix -> label_selectionne++;

		    if ( GTK_IS_HSEPARATOR( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )))
			combofix -> label_selectionne++;

		    ajustement = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW ( combofix -> box ));

		    /* 	  si on est en dessous de la liste, on la décale vers le bas */

		    while ( ( combofix -> label_selectionne + 1 )* ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )) -> allocation.height > ajustement -> value + ajustement -> page_size )
			gtk_adjustment_set_value ( ajustement,
						   ajustement -> value + ajustement -> step_increment );

		    gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
					   GTK_STATE_PRELIGHT );
		}
	    }
	    break;

	case GDK_Up :		/* touches flèche haut */
	case GDK_KP_Up :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( combofix -> entry ),
					   "key-press-event" );

	    if ( combofix -> label_selectionne > 0 )
	    {

		gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
				       GTK_STATE_NORMAL );

		combofix -> label_selectionne--;

		if ( GTK_IS_HSEPARATOR ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )))
		    combofix -> label_selectionne--;

		ajustement = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW ( combofix -> box ));

		/* 	  si on est au dessus de la liste, on la décale vers le haut */

		while ( combofix -> label_selectionne* ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )) -> allocation.height < ajustement -> value )
		    gtk_adjustment_set_value ( ajustement,
					       ajustement -> value - ajustement -> step_increment );

		gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
				       GTK_STATE_PRELIGHT );
	    }
	    break;

	case GDK_Page_Up :		/* touches PgUp */
	case GDK_KP_Page_Up :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( combofix -> entry ),
					   "key-press-event" );

	    if ( combofix -> label_selectionne > 0 )
	    {
		gint nb_labels_par_page;

		gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne )),
				       GTK_STATE_NORMAL );

		/* on calcule combien il y a de labels dans une page */

		nb_labels_par_page = (combofix -> box) -> allocation.height / GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne )) -> allocation.height;

		combofix -> label_selectionne = combofix -> label_selectionne - nb_labels_par_page;

		if ( combofix -> label_selectionne < 0 )
		    combofix -> label_selectionne = 0;

		if ( GTK_IS_HSEPARATOR ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )))
		    combofix -> label_selectionne--;

		ajustement = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW ( combofix -> box ));

		/* 	  si on est au dessus de la liste, on la décale vers le haut */

		while ( combofix -> label_selectionne* ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )) -> allocation.height < ajustement -> value )
		    gtk_adjustment_set_value ( ajustement,
					       ajustement -> value - ajustement -> step_increment );

		gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
				       GTK_STATE_PRELIGHT );
	    }

	    break;

	case GDK_Page_Down :		/* touches PgDn */
	case GDK_KP_Page_Down :

	    gtk_signal_emit_stop_by_name ( GTK_OBJECT ( combofix -> entry ),
					   "key-press-event" );
	    /*     si la popup n'est pas affichée à cause de l'arrow, descend l'arrow, affiche la liste */
	    if ( combofix -> affiche_liste == 0 || !GTK_WIDGET_VISIBLE ( combofix->popup ) )
	    {
		gtk_widget_grab_focus ( GTK_WIDGET ( combofix -> arrow ));
		change_arrow ( NULL,
			       combofix );
		gtk_widget_grab_focus ( GTK_WIDGET ( combofix -> entry ));
	    }
	    /*       si la popup est affichée, on descend dans la liste, sinon, c'est qu'il n'y avait aucune sélection possible */

	    if ( GTK_WIDGET_VISIBLE ( combofix->popup ) )
	    {
		if ( combofix -> label_selectionne < ( g_slist_length ( combofix->event_box )-1) || combofix -> label_selectionne == -1 )
		{
		    gint nb_labels_par_page;

		    if ( combofix -> label_selectionne != -1 )
		    {
			gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
					       GTK_STATE_NORMAL );

			/* on calcule combien il y a de labels dans une page */

			nb_labels_par_page = (combofix -> box) -> allocation.height / GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne )) -> allocation.height;
		    }
		    else
			nb_labels_par_page = 1;

		    combofix -> label_selectionne = combofix -> label_selectionne + nb_labels_par_page;

		    if ( combofix -> label_selectionne >= g_slist_length ( combofix -> event_box ))
			combofix -> label_selectionne = g_slist_length ( combofix -> event_box ) - 1;

		    if ( GTK_IS_HSEPARATOR( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )))
			combofix -> label_selectionne++;

		    ajustement = gtk_scrolled_window_get_vadjustment ( GTK_SCROLLED_WINDOW ( combofix -> box ));

		    /* 	  si on est en dessous de la liste, on la décale vers le bas */

		    while ( ( combofix -> label_selectionne + 1 )* ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) )) -> allocation.height > ajustement -> value + ajustement -> page_size )
			gtk_adjustment_set_value ( ajustement,
						   ajustement -> value + ajustement -> step_increment );

		    gtk_widget_set_state ( GTK_WIDGET ( g_slist_nth_data ( combofix->event_box, combofix -> label_selectionne ) ),
					   GTK_STATE_PRELIGHT );
		}
	    }
	    break;
    }
    return FALSE;
}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* Fonction gtk_combofix_show_arrow : */
/* affiche ou non la flèche permettant d'afficher la liste */
/* argument : la combofix */
/*                     booleen TRUE ( affiche la flèche ) / FALSE */
/* **************************************************************************************************** */

void gtk_combofix_show_arrow ( GtkComboFix *combofix,
			       gint valeur )
{

    g_return_if_fail (combofix  );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));


    if ( valeur )
	gtk_widget_show ( combofix -> arrow );
    else
	gtk_widget_hide ( combofix -> arrow );

}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* Fonction gtk_combofix_sort : */
/* classe la liste en respectant les différentes listes si c'est un complex, et en respectant les catég / sous-categ  */
/* argument : la combofix */
/* **************************************************************************************************** */

void gtk_combofix_sort ( GtkComboFix *combofix )
{
    GSList *pointeur;

    g_return_if_fail ( combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    if ( combofix -> complex )
    {
	pointeur = combofix -> liste;

	while ( pointeur )
	{
	    pointeur -> data = classe_combofix ( pointeur -> data );
	    pointeur = pointeur -> next;
	}
    }
    else
	combofix -> liste = classe_combofix ( combofix -> liste );


}
/* **************************************************************************************************** */


/* **************************************************************************************************** */
/* Fonction classe_combofix */
/* **************************************************************************************************** */

static GSList *classe_combofix ( GSList *liste )
{
    GSList *pointeur;
    gchar *categ;
    gchar *string;


    /* on recrée d'abord la liste sous la forme categ : sous-categ */

    pointeur = liste;
    categ = "";

    while ( pointeur )
    {
	string = pointeur -> data;

	if ( string[0] != '\t' )
	    categ = string;
	else
	    pointeur -> data = g_strconcat ( categ,
					     string,
					     NULL );

	pointeur = pointeur -> next;
    }

    /* on classe la liste obtenue */

    liste = g_slist_sort ( liste,
			   ( GCompareFunc ) classe_alphabetique );

    /*       on retire les catégories des sous-categ */

    pointeur = liste;

    while ( pointeur )
    {
	string = pointeur -> data;

	if ( ( categ = strpbrk ( string,
				 "\t" )) )
	    pointeur -> data = categ;

	pointeur = pointeur -> next;
    }

    return ( liste );
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
/* Fonction gtk_combofix_set_list : */
/* change la liste du combofix  */
/* arguments :                           */
/*       combofix : la combofix          */
/*       liste : une gslist              */
/*       complex : TRUE : complex / FALSE */
/*       classement_auto : TRUE / FALSE   */
/*       efface_ancienne_liste : TRUE / FALSE */
/* **************************************************************************************************** */

void gtk_combofix_set_list ( GtkComboFix *combofix,
			     GSList *liste,
			     gint complex,
			     gint classement_auto )
{
    GSList *pointeur;
    gchar *derniere_categ;
    GSList *nouvelle_liste;

    g_return_if_fail ( combofix );
    g_return_if_fail (GTK_IS_COMBOFIX (combofix));

    g_slist_free ( combofix -> liste );
    g_slist_free ( combofix -> liste_completion );


    /* recopie la liste originale */

    nouvelle_liste = NULL;
    pointeur = liste;

    if ( complex )
	while ( pointeur )
	{
	    GSList *sous_pointeur;
	    GSList *sous_liste;

	    sous_pointeur = pointeur -> data;
	    sous_liste = NULL;

	    while ( sous_pointeur )
	    {
		sous_liste = g_slist_append ( sous_liste,
					      g_strdup ( sous_pointeur -> data ) );
		sous_pointeur = sous_pointeur -> next;
	    }

	    nouvelle_liste = g_slist_append (nouvelle_liste,
					     sous_liste );

	    pointeur = pointeur -> next;
	}
    else
	while ( pointeur )
	{
	    nouvelle_liste = g_slist_append ( nouvelle_liste,
					      g_strdup ( pointeur -> data ) );
	    pointeur = pointeur -> next;
	}


    /* remplit les champs de la combofix */

    combofix -> liste = nouvelle_liste;
    combofix -> complex = complex;
    combofix -> auto_sort = classement_auto;


    /* création de la liste des complétions : système de catégories - sous catégories */
    /* toutes catégorie est mise dans la liste */
    /*   les sous-catég sont mises sous la forme catég : sous-catég */
    /* on trie à la fin la liste par ordre alphabétique pour que lors d'une complétion, c'est le 1er qui sorte */

    /* protège de l'erreur si on met une sous categ en 1er */

    derniere_categ = "";

    combofix -> liste_completion = NULL;
    pointeur = liste;

    if ( complex )
	while ( pointeur )
	{
	    gchar *string;
	    GSList *sous_pointeur;

	    sous_pointeur = pointeur -> data;

	    while ( sous_pointeur )
	    {
		string = sous_pointeur -> data;

		if ( string[0] == '\t' )
		    string = g_strconcat ( derniere_categ,
					   " : ",
					   string + 1,
					   NULL );
		else
		    derniere_categ = string;

		combofix -> liste_completion = g_slist_append ( combofix -> liste_completion,
								string );

		sous_pointeur = sous_pointeur -> next;
	    }
	    pointeur = pointeur -> next;
	}
    else
	while ( pointeur )
	{
	    gchar *string;

	    string = pointeur -> data;

	    if ( string[0] == '\t' )
		string = g_strconcat ( derniere_categ,
				       " : ",
				       string + 1,
				       NULL );
	    else
		derniere_categ = string;

	    combofix -> liste_completion = g_slist_append ( combofix -> liste_completion,
							    string );

	    pointeur = pointeur -> next;
	}
}
/* **************************************************************************************************** */




/* **************************************************************************************************** */
static gboolean touche_pressee_dans_popup ( GtkWidget *popup,
					    GdkEventKey *event,
					    GtkComboFix *combofix )
{
    gtk_signal_emit_by_name ( GTK_OBJECT ( combofix -> entry ),
			      "key-press-event",
			      event,
			      malloc ( sizeof ( gboolean )) );
    return FALSE;
}
/* **************************************************************************************************** */



/* **************************************************************************************************** */
/* Fonction gtk_combofix_set_lignes_max : */
/* permet de choisir le nombre maximal de lignes affichées dans la liste  */
/* s'il vaut 0, toutes les lignes seront affichées */
/* arguments :                           */
/*       combofix : la combofix          */
/*       liste : une gslist              */
/*       complex : TRUE : complex / FALSE */
/*       classement_auto : TRUE / FALSE   */
/* **************************************************************************************************** */

void gtk_combofix_set_lignes_max ( GtkComboFix *combofix,
				   gint lignes_max )
{
    combofix -> lignes_max = lignes_max;
}
/* **************************************************************************************************** */
