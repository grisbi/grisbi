/* ************************************************************************** */
/* Fichier qui s'occupe de la page d'accueil ( de démarrage lors de           */
/* l'ouverture d'un fichier de comptes                                        */
/*                                                                            */
/*                                  accueil.c                                 */
/*                                                                            */
/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org)	      */
/*			2003-2004 Alain Portal (dionysos@grisbi.org)	      */
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
#include "accueil_constants.h"


/*START_INCLUDE*/
#include "accueil.h"
#include "utils_devises.h"
#include "operations_comptes.h"
#include "classement_echeances.h"
#include "echeancier_liste.h"
#include "dialog.h"
#include "operations_liste.h"
#include "echeancier_formulaire.h"
#include "gtk_list_button.h"
#include "utils_str.h"
#include "utils.h"
#include "utils_tiers.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean click_sur_compte_accueil ( gint *no_compte );
static gboolean saisie_echeance_accueil ( GtkWidget *event_box,
				   GdkEventButton *event,
				   struct operation_echeance *echeance );
static gboolean select_expired_scheduled_transaction ( GtkWidget * event_box, GdkEventButton *event,
						struct structure_operation * operation );
static void update_fin_comptes_passifs ( void );
static void update_liste_comptes_accueil ( void );
static void update_liste_echeances_auto_accueil ( void );
static void update_liste_echeances_manuelles_accueil ( void );
static void update_soldes_minimaux ( void );
/*END_STATIC*/



#define show_paddingbox(child) gtk_widget_show_all (gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(child)))))
#define hide_paddingbox(child) gtk_widget_hide_all (gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(child)))))


gint id_temps;
gchar *chemin_logo;
GtkWidget *logo_accueil;
GtkWidget *label_temps;
GtkWidget *label_titre_fichier;
GtkWidget *frame_etat_comptes_accueil;
GtkWidget *frame_etat_fin_compte_passif;
GtkWidget *frame_etat_echeances_manuelles_accueil;
GtkWidget *frame_etat_echeances_auto_accueil;
GtkWidget *frame_etat_echeances_finies;
GtkWidget *frame_etat_soldes_minimaux_autorises;
GtkWidget *frame_etat_soldes_minimaux_voulus;
GtkStyle *style_label_nom_compte;
GtkStyle *style_label;
GtkWidget * label_jour;

/* ces 5 variables sont mises à 1 lorsqu'il est nécessaire de rafraichir cette */
/* partie la prochaine fois qu'on va sur l'accueil */

gint mise_a_jour_liste_comptes_accueil;
gint mise_a_jour_liste_echeances_manuelles_accueil;
gint mise_a_jour_liste_echeances_auto_accueil;
gint mise_a_jour_soldes_minimaux;
gint mise_a_jour_fin_comptes_passifs;


/*START_EXTERN*/
extern struct operation_echeance *echeance_selectionnnee;
extern GSList *echeances_a_saisir;
extern GSList *echeances_saisies;
extern GtkWidget *formulaire;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *formulaire_echeancier;
extern GtkWidget *frame_formulaire_echeancier;
extern GtkWidget *hbox_valider_annuler_echeance;
extern GSList *liste_struct_devises;
extern gint nb_comptes;
extern GtkWidget *notebook_formulaire_echeances;
extern GSList *ordre_comptes;
extern gpointer **p_tab_nom_de_compte;
extern gpointer **p_tab_nom_de_compte_variable;
extern gint patience_en_cours;
extern GtkWidget *separateur_formulaire_echeancier;
extern gchar *titre_fichier;
extern GtkWidget *vbox_liste_comptes;
extern GtkWidget *window;
/*END_EXTERN*/


/* ************************************************************************* */
GtkWidget *creation_onglet_accueil ( void )
{
    GtkWidget *fenetre_accueil, *paddingbox, *base, *base_scroll, *base_box_scroll;
    GtkWidget *hbox, *label;
    gchar *nom_utilisateur;
    struct passwd *utilisateur;

    /*  la première séparation : une hbox : à gauche, le logo, à droite le reste */

    fenetre_accueil = gtk_hbox_new ( FALSE, 15 );
    gtk_widget_show ( fenetre_accueil );

    /* création du logo */

    if ( !chemin_logo || !strlen ( chemin_logo ))
	chemin_logo = LOGO_PATH;

    if ( etat.utilise_logo )
    {
	if ( chemin_logo )
	{
	    logo_accueil =  gtk_image_new_from_file ( chemin_logo );
	    gtk_box_pack_start ( GTK_BOX ( fenetre_accueil ), logo_accueil, FALSE, FALSE, 20 );
	    gtk_widget_show ( logo_accueil );
	}
	else
	    logo_accueil = NULL;
    }

    /* création de la partie droite */

    base = gtk_vbox_new ( FALSE, 15 );
    gtk_box_pack_end ( GTK_BOX ( fenetre_accueil ), base, TRUE, TRUE, 0 );
    gtk_widget_show ( base );


    /* la partie tout en haut affiche la date, le nom et l'heure */

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( base ), hbox, FALSE, FALSE, 10 );
    gtk_widget_show ( hbox );


    /* récupère le nom de l'utilisateur, si nul, met le login */

    utilisateur = getpwuid ( getuid () );

    if ( ! utilisateur )
    {
	utilisateur = getpwuid ( 65534 );
    }

    if ( utilisateur )
    {
	if ( !strlen ( nom_utilisateur = g_strdelimit ( utilisateur->pw_gecos, ",", 0 ) ) )
	    nom_utilisateur = utilisateur->pw_name;
    }
    else
    {
	nom_utilisateur = _("No user");
    }

    label = gtk_label_new ( g_strconcat ( COLON(_("User")), latin2utf8 (nom_utilisateur), NULL) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, TRUE, FALSE, 5 );
    gtk_widget_show ( label );

    label_jour = gtk_label_new ( "" );
    gtk_misc_set_alignment ( GTK_MISC ( label_jour ), MISC_RIGHT, MISC_RIGHT );

    gtk_box_pack_start ( GTK_BOX ( hbox ), label_jour, TRUE, TRUE, 5 );
    gtk_widget_show ( label_jour );

    label_temps = gtk_label_new ( "" );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label_temps, TRUE, FALSE, 5 );
    gtk_widget_show ( label_temps );

    change_temps ( label_temps );

    id_temps = gtk_timeout_add ( 1000,
				 (GtkFunction) change_temps,
				 GTK_WIDGET ( label_temps ));


    /* en dessous, on met le titre du fichier s'il existe */

    if ( titre_fichier )
    {
	label_titre_fichier = gtk_label_new ( titre_fichier );
	gtk_label_set_markup ( GTK_LABEL ( label_titre_fichier ), 
			       g_strconcat ("<span size=\"x-large\">",
					    titre_fichier, "</span>", NULL ) );
    }
    else
    {
	label_titre_fichier = gtk_label_new ( "" );
    }

    gtk_box_pack_start ( GTK_BOX ( base ), label_titre_fichier, FALSE, FALSE, 0 );
    gtk_widget_show ( label_titre_fichier );


    /* on crée à ce niveau base_scroll qui est aussi une vbox mais qui peut
       scroller verticalement */

    base_scroll = gtk_scrolled_window_new ( NULL, NULL);
    gtk_container_set_border_width ( GTK_CONTAINER ( base_scroll ), 5 );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( base_scroll ),
				     GTK_POLICY_NEVER,
				     GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start ( GTK_BOX ( base ), base_scroll, TRUE, TRUE, 0);
    gtk_widget_show ( base_scroll );


    /* on met la nouvelle vbox dans le fenetre scrollable */

    base_box_scroll = gtk_vbox_new ( FALSE, 0 );
    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( base_scroll ), base_box_scroll);
    gtk_viewport_set_shadow_type ( GTK_VIEWPORT ( GTK_BIN ( base_scroll )  -> child ), GTK_SHADOW_NONE );
    gtk_container_set_border_width ( GTK_CONTAINER ( base_box_scroll ), 10 );
    gtk_widget_show (base_box_scroll);


    /* on crée la première frame dans laquelle on met les états des comptes */
    frame_etat_comptes_accueil = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_comptes_accueil), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_comptes_accueil), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_comptes_accueil), 0 );
    gtk_box_pack_start ( GTK_BOX(base_box_scroll), frame_etat_comptes_accueil, FALSE, FALSE, 0 );

    /* on met la liste des comptes et leur état dans la frame */
    mise_a_jour_liste_comptes_accueil = 1;
    gtk_widget_show_all ( frame_etat_comptes_accueil );


    /* mise en place de la partie fin des comptes passif */
    paddingbox = new_paddingbox_with_title ( base_box_scroll, FALSE,
					     _("Closed liabilities accounts") );
    frame_etat_fin_compte_passif = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_fin_compte_passif), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_fin_compte_passif), FALSE );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_fin_compte_passif, FALSE, FALSE, 0 );
    mise_a_jour_fin_comptes_passifs = 1;


    /* mise en place de la partie des échéances manuelles ( non affiché ) */
    /*     sera mis à jour automatiquement si nécessaire */
    paddingbox = new_paddingbox_with_title ( base_box_scroll, FALSE,
					     _("Manual scheduled transactions at maturity date") );
    frame_etat_echeances_manuelles_accueil = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_echeances_manuelles_accueil), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_echeances_manuelles_accueil), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_echeances_manuelles_accueil), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_echeances_manuelles_accueil, FALSE, FALSE, 6 );


    /* mise en place de la partie des échéances auto  ( non affiché )*/
    /*     sera mis à jour automatiquement si nécessaire */
    paddingbox = new_paddingbox_with_title ( base_box_scroll, FALSE,
					     _("Automatic scheduled transactions entered") );
    frame_etat_echeances_auto_accueil = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_echeances_auto_accueil), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_echeances_auto_accueil), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_echeances_auto_accueil), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_echeances_auto_accueil, FALSE, FALSE, 6 );


    /* partie des fin d'échéances */
    paddingbox = new_paddingbox_with_title ( base_box_scroll, FALSE,
					     _("Closed scheduled transactions") );
    frame_etat_echeances_finies = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_echeances_finies), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_echeances_finies), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_echeances_finies), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_echeances_finies, FALSE, FALSE, 6 );


    /* partie des soldes minimaux autorisés */
    paddingbox = new_paddingbox_with_title ( base_box_scroll, FALSE,
					     _("Accounts under authorized balance") );
    frame_etat_soldes_minimaux_autorises = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_autorises), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_autorises), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_soldes_minimaux_autorises), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_soldes_minimaux_autorises, FALSE, FALSE, 6 );


    /* partie des soldes minimaux voulus */
    paddingbox = new_paddingbox_with_title ( base_box_scroll, FALSE,
					     _("Accounts under wanted balance") );
    frame_etat_soldes_minimaux_voulus = gtk_notebook_new ();
    gtk_notebook_set_show_tabs ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_voulus), FALSE );
    gtk_notebook_set_show_border ( GTK_NOTEBOOK(frame_etat_soldes_minimaux_voulus), FALSE );
    gtk_container_set_border_width ( GTK_CONTAINER(frame_etat_soldes_minimaux_voulus), 0 );
    gtk_box_set_spacing ( GTK_BOX(paddingbox), 6 );
    gtk_box_pack_start ( GTK_BOX(paddingbox), frame_etat_soldes_minimaux_voulus, FALSE, FALSE, 6 );

    mise_a_jour_soldes_minimaux = 1;

    return ( fenetre_accueil );
}
/* ************************************************************************* */





/* ************************************************************************* */
/* fonction appelée lors de l'ouverture de nouveau fichier */
/* et quand on passe sur l'onglet accueil */
/* fait le tour des fonctions update qui vont */
/* s'updater si nécessaire */
/* ************************************************************************* */
void mise_a_jour_accueil ( void )
{
    update_liste_comptes_accueil ();
    update_liste_echeances_manuelles_accueil ();
    update_liste_echeances_auto_accueil ();
    update_soldes_minimaux ();
    update_fin_comptes_passifs ();
}
/* ************************************************************************* */


/* ************************************************************************* */
void change_temps ( GtkWidget *label_temps )
{
    gchar tampon_date [50], * tampon;
    time_t date;
    GError *error = NULL;

    time ( &date );
    strftime ( tampon_date,
	       (size_t) 50,
	       "%X",
	       (const struct tm *) localtime ( &date ) );

    /* Convert to UTF-8 */
    tampon = g_convert (tampon_date, strlen(tampon_date), 
			"UTF-8", "ISO-8859-1", 
			NULL, NULL, &error);

    gtk_label_set_text ( GTK_LABEL ( label_temps ), tampon );

    strftime ( tampon_date,
	       (size_t) 50,
	       "%A %d %B %Y",
	       (const struct tm *) localtime ( &date ) );
    /* Convert to UTF-8 */
    tampon = g_convert ( tampon_date, strlen(tampon_date), 
			 "UTF-8", "ISO-8859-1", 
			 NULL, NULL, &error);
    /* Capitalize */
    tampon[0] = toupper ( tampon[0] );

    gtk_label_set_text ( GTK_LABEL ( label_jour ), tampon );
}
/* ************************************************************************* */

/* ************************************************************************* */
/*        Fonction appelée lorsqu'on clicke sur une échéance à saisir        */
/* ************************************************************************* */
gboolean saisie_echeance_accueil ( GtkWidget *event_box,
				   GdkEventButton *event,
				   struct operation_echeance *echeance )
{
    GtkWidget *ancien_parent, *dialog;
    struct operation_echeance *ancienne_selection_echeance;
    gint resultat, width;

    /* on sélectionne l'échéance demandée */
    ancienne_selection_echeance = echeance_selectionnnee;
    echeance_selectionnnee = echeance;

    ancien_parent = formulaire_echeancier -> parent;

    /* crée la boite de dialogue */
    dialog = gtk_dialog_new_with_buttons ( _("Enter a scheduled transaction"),
					   GTK_WINDOW ( window ),
					   GTK_DIALOG_MODAL,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OK, GTK_RESPONSE_OK,
					   NULL );
    gtk_window_set_position ( GTK_WINDOW ( dialog ), GTK_WIN_POS_CENTER );

    /* met le formulaire dans la boite de dialogue */
    width = frame_formulaire_echeancier -> allocation . width;
    if ( width <= 1 )
        width = 700 ;
    gtk_widget_unrealize ( formulaire_echeancier );
    gtk_widget_reparent ( formulaire_echeancier, GTK_DIALOG ( dialog ) -> vbox );
    gtk_widget_set_usize ( GTK_WIDGET ( dialog ), width, FALSE );

    etat.formulaire_echeance_dans_fenetre = 1;

    /* remplit le formulaire */
    click_sur_saisir_echeance();

    gtk_widget_show ( formulaire_echeancier );
    if ( etat.affiche_boutons_valider_annuler )
    {
	gtk_widget_hide ( separateur_formulaire_echeancier );
	gtk_widget_hide ( hbox_valider_annuler_echeance );
    }

    resultat = gtk_dialog_run ( GTK_DIALOG ( dialog ));

    if ( resultat == GTK_RESPONSE_OK )
	fin_edition_echeance ();

    gtk_widget_reparent ( formulaire_echeancier, ancien_parent );

/*     en remettant la fenetre, elle passe en onglet 2, après l'onglet du formulaire de */
/* 	ventilation ... on la remet en 1 */

    gtk_notebook_reorder_child ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
				 formulaire_echeancier,
				 0 );
    gtk_notebook_set_page ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
			    0 );

/*     de plus, on a perdu le nom de l'étiquette, c'est inutilisé pour l'instant mais on */
/* 	sait jamais... */

    gtk_notebook_set_tab_label ( GTK_NOTEBOOK ( notebook_formulaire_echeances ),
				 formulaire_echeancier,
				 gtk_label_new ( _("Form")));


    etat.formulaire_echeance_dans_fenetre = 0;
    gtk_widget_destroy ( dialog );

    /* remet les boutons valider/annuler si necessaire */

    if ( etat.affiche_boutons_valider_annuler )
    {
	gtk_widget_show ( separateur_formulaire_echeancier );
	gtk_widget_show ( hbox_valider_annuler_echeance );
    }

    formulaire_echeancier_a_zero();

    echeance_selectionnnee = ancienne_selection_echeance;

    if ( !etat.formulaire_echeancier_toujours_affiche )
	gtk_widget_hide ( frame_formulaire_echeancier );



    return FALSE;
}
/* ************************************************************************* */



/* ************************************************************************* */
/* Fonction update_liste_comptes_accueil                                     */
/* affiche la liste des comptes et leur solde courant dans la frame qui leur */
/* est réservée dans l'accueil                                               */
/* ************************************************************************* */
void update_liste_comptes_accueil ( void )
{
    GtkWidget *pTable, *pEventBox, *pLabel, *vbox, *paddingbox;
    GdkColor CouleurSoldeAlarmeVerteNormal, CouleurSoldeAlarmeVertePrelight,
    CouleurSoldeAlarmeOrangeNormal, CouleurSoldeAlarmeOrangePrelight,
    CouleurSoldeAlarmeRougeNormal, CouleurSoldeAlarmeRougePrelight,
    CouleurNomCompteNormal, CouleurNomComptePrelight;
    GtkStyle *pStyleLabelNomCompte, *pStyleLabelSoldeCourant, 
    *pStyleLabelSoldePointe;
    GSList *ordre_comptes_variable, *liste_operations_tmp, *devise;
    gdouble montant, solde_global_courant, solde_global_pointe;
    GList *children;
    gint i;
    gint nb_comptes_bancaires=0, nb_comptes_passif=0, nb_comptes_actif=0;
    gdouble solde_pointe_affichage_liste;


    if ( !mise_a_jour_liste_comptes_accueil
	 ||
	 !nb_comptes )
	return;

    if ( DEBUG )
	printf ( "update_liste_comptes_accueil\n" );

    mise_a_jour_liste_comptes_accueil = 0;

    /* Initialisation des couleurs des différents labels */
    /* Pourra être intégré à la configuration générale */
    CouleurSoldeAlarmeVerteNormal.red =     0.00 * 65535 ;
    CouleurSoldeAlarmeVerteNormal.green =   0.50 * 65535 ;
    CouleurSoldeAlarmeVerteNormal.blue =    0.00 * 65535 ;
    CouleurSoldeAlarmeVertePrelight.red =   0.00 * 65535 ;
    CouleurSoldeAlarmeVertePrelight.green = 0.90 * 65535 ;
    CouleurSoldeAlarmeVertePrelight.blue =  0.00 * 65535 ;

    CouleurSoldeAlarmeOrangeNormal.red =     0.90 * 65535 ;
    CouleurSoldeAlarmeOrangeNormal.green =   0.60 * 65535 ;
    CouleurSoldeAlarmeOrangeNormal.blue =    0.00 * 65535 ;
    CouleurSoldeAlarmeOrangePrelight.red =   1.00 * 65535 ;
    CouleurSoldeAlarmeOrangePrelight.green = 0.80 * 65535 ;
    CouleurSoldeAlarmeOrangePrelight.blue =  0.00 * 65535 ;

    CouleurSoldeAlarmeRougeNormal.red =     0.60 * 65535 ;
    CouleurSoldeAlarmeRougeNormal.green =   0.00 * 65535 ;
    CouleurSoldeAlarmeRougeNormal.blue =    0.00 * 65535 ;
    CouleurSoldeAlarmeRougePrelight.red =   1.00 * 65535 ;
    CouleurSoldeAlarmeRougePrelight.green = 0.00 * 65535 ;
    CouleurSoldeAlarmeRougePrelight.blue =  0.00 * 65535 ;

    CouleurNomCompteNormal.red =     0.00 * 65535 ;
    CouleurNomCompteNormal.green =   0.00 * 65535 ;
    CouleurNomCompteNormal.blue =    0.00 * 65535 ;
    CouleurNomComptePrelight.red =   0.61 * 65535 ;
    CouleurNomComptePrelight.green = 0.61 * 65535 ;
    CouleurNomComptePrelight.blue =  0.61 * 65535 ;

    /* Création d'un label juste pour en récupérer le style */
    pLabel = gtk_label_new ("");

    /* Initialisation du style « Nom du compte » */
    pStyleLabelNomCompte = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
    pStyleLabelNomCompte->fg[GTK_STATE_NORMAL] = CouleurNomCompteNormal;
    pStyleLabelNomCompte->fg[GTK_STATE_PRELIGHT] = CouleurNomComptePrelight;

    /* Création du cadre principal */
    /*   gtk_notebook_remove_page ( GTK_NOTEBOOK (frame_etat_comptes_accueil), 0 ); */

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

    p_tab_nom_de_compte_variable=p_tab_nom_de_compte;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	if ( !COMPTE_CLOTURE )
	{
	    if ( TYPE_DE_COMPTE == 3 )
	    {
		nb_comptes_actif++;
	    }
	    else
	    {
		if ( TYPE_DE_COMPTE == 2 )
		{
		    nb_comptes_passif++;
		}
		else
		{
		    nb_comptes_bancaires++;
		}
	    }
	}
	p_tab_nom_de_compte_variable++;	  
    }

    /* Affichage des comptes bancaires et de caisse */

    for ( devise = liste_struct_devises; devise ; devise = devise->next )
    {
	int devise_is_used = 0;

	p_tab_nom_de_compte_variable=p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    if ( DEVISE == ((struct struct_devise *) devise -> data) -> no_devise
		 && ! COMPTE_CLOTURE
		 && ( TYPE_DE_COMPTE == 0
		      || TYPE_DE_COMPTE == 1 ))
		devise_is_used = 1;
	    p_tab_nom_de_compte_variable++;	  
	}

	if ( !devise_is_used )
	    continue;


	/* Création du tableau dans lequel seront stockés les comptes avec leur     */
	/* solde.                                                                   */
	paddingbox = new_paddingbox_with_title ( vbox, FALSE,
						 g_strdup_printf (_("Account balances in %s"),
								  ((struct struct_devise *) devise -> data) -> nom_devise ));
	pTable = gtk_table_new ( nb_comptes_bancaires + 3, 8, FALSE );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), pTable, FALSE, FALSE, 0 );

	/* Création et remplissage de la première ligne du tableau */
	pLabel = gtk_label_new (_("Reconciled balance"));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_HORIZ_CENTER, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 2, 4, 0, 1 );
	gtk_widget_show ( pLabel );
	pLabel = gtk_label_new (_("Current balance"));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_HORIZ_CENTER, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 5, 7, 0, 1 );
	gtk_widget_show ( pLabel );

	/* Affichage des comptes et de leur solde */
	ordre_comptes_variable = ordre_comptes;
	i = 1;
	solde_global_courant = 0 ;
	solde_global_pointe = 0 ;

	/* Pour chaque compte non cloturé (pour chaque ligne), */
	/* créer toutes les colonnes et les remplir            */
	do
	{
	    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );

	    if ( !COMPTE_CLOTURE &&
		 DEVISE == ((struct struct_devise *) devise -> data) -> no_devise
		 && TYPE_DE_COMPTE != 2
		 && TYPE_DE_COMPTE != 3 )
	    {
		/* Première colonne : vide */
		pLabel = gtk_label_new ( g_strconcat ( (gchar *) NOM_DU_COMPTE, " : ", NULL ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_widget_set_style ( pLabel, pStyleLabelNomCompte );

		/* Est-il réellement nécessaire de créer des labels vides pour */
		/* remplir des colonnes vides d'un tableau ?                   */
		pLabel = gtk_label_new ( "" );
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 0, 1, i, i+1 );
		gtk_widget_show ( pLabel );

		/* Deuxième colonne : elle contient le nom du compte */
		pLabel = gtk_label_new ( g_strconcat ( (gchar *) NOM_DU_COMPTE, " : ", NULL ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_widget_set_style ( pLabel, pStyleLabelNomCompte );

		/* Création d'une boite à évènement qui sera rattachée au nom du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( click_sur_compte_accueil ),
					    ordre_comptes_variable->data );
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   1, 2, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   20, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Troisième colonne : elle contient le solde pointé du compte */
		pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", SOLDE_POINTE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

		/* Mise en place du style du label en fonction du solde pointé */
		pStyleLabelSoldePointe = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
		if ( SOLDE_POINTE >= SOLDE_MINI_VOULU )
		{
		    pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeVerteNormal;
		    pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeVertePrelight;
		}
		else
		{
		    if ( SOLDE_POINTE >= SOLDE_MINI )
		    {
			pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeOrangeNormal;
			pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeOrangePrelight;
		    }
		    else
		    {
			pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeRougeNormal;
			pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeRougePrelight;
		    }
		}
		gtk_widget_set_style ( pLabel, pStyleLabelSoldePointe );

		/* Création d'une boite à évènement qui sera rattachée au solde courant du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( changement_compte ),
					    ordre_comptes_variable->data );
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   2, 3, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   0, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Quatrième colonne : elle contient le symbole de la devise du compte */
		pLabel = gtk_label_new ( devise_code_by_no (  DEVISE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
				   3, 4, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   5, 0 );
		gtk_widget_show ( pLabel );

		/* Cinquième colonne : vide */
		pLabel = gtk_label_new ("");
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 4, 5, i, i+1 );
		gtk_widget_show ( pLabel );

		/* Sixième colonne : elle contient le solde courant du compte */
		pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", SOLDE_COURANT ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

		/* Mise en place du style du label en fonction du solde courant */
		pStyleLabelSoldeCourant = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
		if ( SOLDE_COURANT >= SOLDE_MINI_VOULU )
		{
		    pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeVerteNormal;
		    pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeVertePrelight;
		}
		else
		{
		    if ( SOLDE_COURANT >= SOLDE_MINI )
		    {
			pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeOrangeNormal;
			pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeOrangePrelight;
		    }
		    else
		    {
			pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeRougeNormal;
			pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeRougePrelight;
		    }
		}
		gtk_widget_set_style ( pLabel, pStyleLabelSoldeCourant );

		/* Création d'une boite à évènement qui sera rattachée au solde pointé du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( changement_compte ),
					    ordre_comptes_variable->data );
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   5, 6, i, i+1,
				   GTK_FILL| GTK_SHRINK, GTK_FILL| GTK_SHRINK,
				   0, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Septième colonne : elle contient le symbole de la devise du compte */
		pLabel = gtk_label_new ( devise_code_by_no ( DEVISE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
				   6, 7, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   5, 0 );
		gtk_widget_show ( pLabel );

		/* Huitième colonne : vide */
		pLabel = gtk_label_new ("");
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 7, 8, i, i+1 );
		gtk_widget_show ( pLabel );

		/* ATTENTION : les sommes effectuées ici présupposent que
		   TOUS les comptes sont dans la MÊME DEVISE !!!!!        */
		solde_global_courant += SOLDE_COURANT;
		solde_global_pointe += SOLDE_POINTE ;
	    }
	    i++;
	}
	while ( ( ordre_comptes_variable = ordre_comptes_variable->next ) );

	/* Création de la (nb_comptes + 2)ième ligne du tableau : vide */
	pLabel = gtk_label_new ( "" );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 0, 1, i, i+1 );
	gtk_widget_show ( pLabel );

	/* Création et remplissage de la (nb_comptes + 3)ième ligne du tableau :
	   elle contient la somme des soldes de chaque compte */
	/* Deuxième colonne */
	pLabel = gtk_label_new ( COLON(_("Global balances")));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   1, 2, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   20, 0 );
	gtk_widget_show ( pLabel );

	/* Troisième colonne : elle contient le solde total pointé des comptes */
	pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", solde_global_pointe ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   2, 3, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( pLabel );

	/* Quatrième colonne : elle contient le symbole de la devise du compte */
	pLabel = gtk_label_new ( devise_code_by_no (((struct struct_devise *) devise -> data) -> no_devise ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   3, 4, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   5, 0 );
	gtk_widget_show ( pLabel );

	/* Sixième colonne : elle contient le solde total courant des comptes */
	pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", solde_global_courant ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   5, 6, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( pLabel );

	/* Septième colonne : elle contient le symbole de la devise du compte */
	pLabel = gtk_label_new ( devise_code_by_no (((struct struct_devise *) devise -> data) -> no_devise));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   6, 7, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   5, 0 );
	gtk_widget_show ( pLabel );

	gtk_widget_show_all ( paddingbox );
	gtk_widget_show_all ( pTable );

    }


    /* Affichage des comptes de passif */

    for ( devise = liste_struct_devises; devise ; devise = devise->next )
    {
	int devise_is_used = 0;

	p_tab_nom_de_compte_variable=p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    if ( DEVISE == ((struct struct_devise *) devise -> data) -> no_devise
		 && ! COMPTE_CLOTURE
		 && TYPE_DE_COMPTE == 2 )
		devise_is_used = 1;
	    p_tab_nom_de_compte_variable++;	  
	}

	if ( !devise_is_used )
	    continue;


	/* Création du tableau dans lequel seront stockés les comptes avec leur     */
	/* solde.                                                                   */
	paddingbox = new_paddingbox_with_title ( vbox, FALSE,
						 g_strdup_printf (_("Liabilities accounts balances in %s"),
								  ((struct struct_devise *) devise -> data) -> nom_devise ));
	pTable = gtk_table_new ( nb_comptes_passif + 3, 8, FALSE );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), pTable, FALSE, FALSE, 0 );
	
	/* Création et remplissage de la première ligne du tableau */
	pLabel = gtk_label_new (_("Reconciled balance"));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_HORIZ_CENTER, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 2, 4, 0, 1 );
	gtk_widget_show ( pLabel );
	pLabel = gtk_label_new (_("Current balance"));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_HORIZ_CENTER, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 5, 7, 0, 1 );
	gtk_widget_show ( pLabel );

	/* Affichage des comptes et de leur solde */
	ordre_comptes_variable = ordre_comptes;
	i = 1;
	solde_global_courant = 0 ;
	solde_global_pointe = 0 ;

	/* Pour chaque compte non cloturé (pour chaque ligne), */
	/* créer toutes les colonnes et les remplir            */
	do
	{
	    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );

	    if ( !COMPTE_CLOTURE &&
		 DEVISE == ((struct struct_devise *) devise -> data) -> no_devise &&
		 TYPE_DE_COMPTE == 2 )
	    {
		/* Première colonne : vide */
		pLabel = gtk_label_new ( g_strconcat ( (gchar *) NOM_DU_COMPTE, " : ", NULL ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_widget_set_style ( pLabel, pStyleLabelNomCompte );

		/* Est-il réellement nécessaire de créer des labels vides pour */
		/* remplir des colonnes vides d'un tableau ?                   */
		pLabel = gtk_label_new ( "" );
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 0, 1, i, i+1 );
		gtk_widget_show ( pLabel );

		/* Deuxième colonne : elle contient le nom du compte */
		pLabel = gtk_label_new ( g_strconcat ( (gchar *) NOM_DU_COMPTE, " : ", NULL ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_widget_set_style ( pLabel, pStyleLabelNomCompte );

		/* Création d'une boite à évènement qui sera rattachée au nom du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( click_sur_compte_accueil ),
					    GINT_TO_POINTER ( ordre_comptes_variable->data ) );
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   1, 2, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   20, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Calcul du solde pointé : je ne sais plus pourquoi on fait
		   ce calcul, sans doute le solde pointé n'est-il pas disponible
		   en tant que variable globale */
		solde_pointe_affichage_liste = SOLDE_INIT;

		/* on commence la boucle : fait le tour de toutes les opérations */
		/* met à jour les solde_courant_affichage_liste et solde_pointe_affichage_liste */
		/* affiche l'opération à l'écran en fonction de l'affichage de R */
		liste_operations_tmp = LISTE_OPERATIONS;

		while ( liste_operations_tmp )
		{
		    struct structure_operation *operation;

		    operation = liste_operations_tmp -> data;

		    /* si c'est une opé de ventilation, on la saute */
		    if ( !operation -> no_operation_ventilee_associee )
		    {
			/* quelle que soit l'opération (relevée ou non), on calcule les soldes courant */
			montant = calcule_montant_devise_renvoi ( operation -> montant,
								  DEVISE,
								  operation -> devise,
								  operation -> une_devise_compte_egale_x_devise_ope,
								  operation -> taux_change,
								  operation -> frais_change );

			/* si l'opé est pointée ou relevée, on ajoute ce montant au solde pointé */
			if ( operation -> pointe )
			    solde_pointe_affichage_liste = solde_pointe_affichage_liste + montant;
		    }
		    liste_operations_tmp = liste_operations_tmp -> next;
		}

		/* on enregistre le solde final */
		SOLDE_POINTE = solde_pointe_affichage_liste;

		/* Troisième colonne : elle contient le solde pointé du compte */
		pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", SOLDE_POINTE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

		/* Mise en place du style du label en fonction du solde pointé */
		pStyleLabelSoldePointe = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
		if ( SOLDE_POINTE >= SOLDE_MINI_VOULU )
		{
		    pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeVerteNormal;
		    pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeVertePrelight;
		}
		else
		{
		    if ( SOLDE_POINTE >= SOLDE_MINI )
		    {
			pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeOrangeNormal;
			pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeOrangePrelight;
		    }
		    else
		    {
			pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeRougeNormal;
			pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeRougePrelight;
		    }
		}
		gtk_widget_set_style ( pLabel, pStyleLabelSoldePointe );

		/* Création d'une boite à évènement qui sera rattachée au solde courant du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( click_sur_compte_accueil ),
					    GINT_TO_POINTER ( ordre_comptes_variable->data ));
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   2, 3, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   0, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Quatrième colonne : elle contient le symbole de la devise du compte */
		pLabel = gtk_label_new ( devise_code_by_no (  DEVISE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
				   3, 4, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   5, 0 );
		gtk_widget_show ( pLabel );

		/* Cinquième colonne : vide */
		pLabel = gtk_label_new ("");
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 4, 5, i, i+1 );
		gtk_widget_show ( pLabel );

		/* Sixième colonne : elle contient le solde courant du compte */
		pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", SOLDE_COURANT ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

		/* Mise en place du style du label en fonction du solde courant */
		pStyleLabelSoldeCourant = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
		if ( SOLDE_COURANT >= SOLDE_MINI_VOULU )
		{
		    pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeVerteNormal;
		    pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeVertePrelight;
		}
		else
		{
		    if ( SOLDE_COURANT >= SOLDE_MINI )
		    {
			pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeOrangeNormal;
			pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeOrangePrelight;
		    }
		    else
		    {
			pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeRougeNormal;
			pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeRougePrelight;
		    }
		}
		gtk_widget_set_style ( pLabel, pStyleLabelSoldeCourant );

		/* Création d'une boite à évènement qui sera rattachée au solde pointé du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( click_sur_compte_accueil ),
					    GINT_TO_POINTER ( ordre_comptes_variable->data ));
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   5, 6, i, i+1,
				   GTK_FILL| GTK_SHRINK, GTK_FILL| GTK_SHRINK,
				   0, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Septième colonne : elle contient le symbole de la devise du compte */
		pLabel = gtk_label_new ( devise_code_by_no ( DEVISE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
				   6, 7, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   5, 0 );
		gtk_widget_show ( pLabel );

		/* Huitième colonne : vide */
		pLabel = gtk_label_new ("");
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 7, 8, i, i+1 );
		gtk_widget_show ( pLabel );

		/* ATTENTION : les sommes effectuées ici présupposent que
		   TOUS les comptes sont dans la MÊME DEVISE !!!!!        */
		solde_global_courant += SOLDE_COURANT ;
		solde_global_pointe += SOLDE_POINTE ;
	    }
	    i++;
	}
	while ( ( ordre_comptes_variable = ordre_comptes_variable->next ) );

	/* Création de la (nb_comptes + 2)ième ligne du tableau : vide */
	pLabel = gtk_label_new ( "" );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 0, 1, i, i+1 );
	gtk_widget_show ( pLabel );

	/* Création et remplissage de la (nb_comptes + 3)ième ligne du tableau :
	   elle contient la somme des soldes de chaque compte */
	/* Deuxième colonne */
	pLabel = gtk_label_new ( COLON(_("Global balances")));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   1, 2, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   20, 0 );
	gtk_widget_show ( pLabel );

	/* Troisième colonne : elle contient le solde total pointé des comptes */
	pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", solde_global_pointe ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   2, 3, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( pLabel );

	/* Quatrième colonne : elle contient le symbole de la devise du compte */
	pLabel = gtk_label_new ( devise_code_by_no (((struct struct_devise *) devise -> data) -> no_devise ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   3, 4, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   5, 0 );
	gtk_widget_show ( pLabel );

	/* Sixième colonne : elle contient le solde total courant des comptes */
	pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", solde_global_courant ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   5, 6, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( pLabel );

	/* Septième colonne : elle contient le symbole de la devise du compte */
	pLabel = gtk_label_new ( devise_code_by_no (((struct struct_devise *) devise -> data) -> no_devise));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   6, 7, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   5, 0 );
	gtk_widget_show ( pLabel );

	gtk_widget_show_all ( paddingbox );
	gtk_widget_show_all ( pTable );

    }


    /* Affichage des comptes d'actif */

    for ( devise = liste_struct_devises; devise ; devise = devise->next )
    {
	int devise_is_used = 0;

	p_tab_nom_de_compte_variable=p_tab_nom_de_compte;

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    if ( DEVISE == ((struct struct_devise *) devise -> data) -> no_devise &&
		 !COMPTE_CLOTURE &&
		 TYPE_DE_COMPTE == 3 )
		devise_is_used = 1;
	    p_tab_nom_de_compte_variable++;	  
	}

	if ( !devise_is_used )
	    continue;


	/* Création du tableau dans lequel seront stockés les comptes avec leur     */
	/* solde.                                                                   */
	paddingbox = new_paddingbox_with_title ( vbox, FALSE,
						 g_strdup_printf (_("Assets accounts balances in %s"),
								  ((struct struct_devise *) devise -> data) -> nom_devise ));
	pTable = gtk_table_new ( nb_comptes_actif + 3, 8, FALSE );
	gtk_box_pack_start ( GTK_BOX ( paddingbox ), pTable, FALSE, FALSE, 0 );

	/* Création et remplissage de la première ligne du tableau */
	pLabel = gtk_label_new (_("Reconciled balance"));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_HORIZ_CENTER, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 2, 4, 0, 1 );
	gtk_widget_show ( pLabel );
	pLabel = gtk_label_new (_("Current balance"));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_HORIZ_CENTER, MISC_VERT_CENTER );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 5, 7, 0, 1 );
	gtk_widget_show ( pLabel );

	/* Affichage des comptes et de leur solde */
	ordre_comptes_variable = ordre_comptes;
	i = 1;
	solde_global_courant = 0 ;
	solde_global_pointe = 0 ;

	/* Pour chaque compte non cloturé (pour chaque ligne), */
	/* créer toutes les colonnes et les remplir            */
	do
	{
	    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + GPOINTER_TO_INT ( ordre_comptes_variable->data );

	    if ( !COMPTE_CLOTURE &&
		 DEVISE == ((struct struct_devise *) devise -> data) -> no_devise &&
		 TYPE_DE_COMPTE == 3 )
	    {
		/* Première colonne : vide */
		pLabel = gtk_label_new ( g_strconcat ( (gchar *) NOM_DU_COMPTE, " : ", NULL ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_widget_set_style ( pLabel, pStyleLabelNomCompte );

		/* Est-il réellement nécessaire de créer des labels vides pour */
		/* remplir des colonnes vides d'un tableau ?                   */
		pLabel = gtk_label_new ( "" );
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 0, 1, i, i+1 );
		gtk_widget_show ( pLabel );

		/* Deuxième colonne : elle contient le nom du compte */
		pLabel = gtk_label_new ( g_strconcat ( (gchar *) NOM_DU_COMPTE, " : ", NULL ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_widget_set_style ( pLabel, pStyleLabelNomCompte );

		/* Création d'une boite à évènement qui sera rattachée au nom du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( click_sur_compte_accueil ),
					    GINT_TO_POINTER ( ordre_comptes_variable->data ) );
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   1, 2, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   20, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Calcul du solde pointé : je ne sais plus pourquoi on fait
		   ce calcul, sans doute le solde pointé n'est-il pas disponible
		   en tant que variable globale */
		solde_pointe_affichage_liste = SOLDE_INIT;

		/* on commence la boucle : fait le tour de toutes les opérations */
		/* met à jour les solde_courant_affichage_liste et solde_pointe_affichage_liste */
		/* affiche l'opération à l'écran en fonction de l'affichage de R */
		liste_operations_tmp = LISTE_OPERATIONS;

		while ( liste_operations_tmp )
		{
		    struct structure_operation *operation;

		    operation = liste_operations_tmp -> data;

		    /* si c'est une opé de ventilation, on la saute */
		    if ( !operation -> no_operation_ventilee_associee )
		    {
			/* quelle que soit l'opération (relevée ou non), on calcule les soldes courant */
			montant = calcule_montant_devise_renvoi ( operation -> montant,
								  DEVISE,
								  operation -> devise,
								  operation -> une_devise_compte_egale_x_devise_ope,
								  operation -> taux_change,
								  operation -> frais_change );

			/* si l'opé est pointée ou relevée, on ajoute ce montant au solde pointé */
			if ( operation -> pointe )
			    solde_pointe_affichage_liste = solde_pointe_affichage_liste + montant;
		    }
		    liste_operations_tmp = liste_operations_tmp -> next;
		}

		/* on enregistre le solde final */
		SOLDE_POINTE = solde_pointe_affichage_liste;

		/* Troisième colonne : elle contient le solde pointé du compte */
		pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", SOLDE_POINTE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

		/* Mise en place du style du label en fonction du solde pointé */
		pStyleLabelSoldePointe = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
		if ( SOLDE_POINTE >= SOLDE_MINI_VOULU )
		{
		    pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeVerteNormal;
		    pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeVertePrelight;
		}
		else
		{
		    if ( SOLDE_POINTE >= SOLDE_MINI )
		    {
			pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeOrangeNormal;
			pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeOrangePrelight;
		    }
		    else
		    {
			pStyleLabelSoldePointe->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeRougeNormal;
			pStyleLabelSoldePointe->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeRougePrelight;
		    }
		}
		gtk_widget_set_style ( pLabel, pStyleLabelSoldePointe );

		/* Création d'une boite à évènement qui sera rattachée au solde courant du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( click_sur_compte_accueil ),
					    GINT_TO_POINTER ( ordre_comptes_variable->data ));
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   2, 3, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   0, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Quatrième colonne : elle contient le symbole de la devise du compte */
		pLabel = gtk_label_new ( devise_code_by_no (  DEVISE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
				   3, 4, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   5, 0 );
		gtk_widget_show ( pLabel );

		/* Cinquième colonne : vide */
		pLabel = gtk_label_new ("");
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 4, 5, i, i+1 );
		gtk_widget_show ( pLabel );

		/* Sixième colonne : elle contient le solde courant du compte */
		pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", SOLDE_COURANT ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );

		/* Mise en place du style du label en fonction du solde courant */
		pStyleLabelSoldeCourant = gtk_style_copy ( gtk_widget_get_style ( pLabel ));
		if ( SOLDE_COURANT >= SOLDE_MINI_VOULU )
		{
		    pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeVerteNormal;
		    pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeVertePrelight;
		}
		else
		{
		    if ( SOLDE_COURANT >= SOLDE_MINI )
		    {
			pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeOrangeNormal;
			pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeOrangePrelight;
		    }
		    else
		    {
			pStyleLabelSoldeCourant->fg[GTK_STATE_NORMAL] = CouleurSoldeAlarmeRougeNormal;
			pStyleLabelSoldeCourant->fg[GTK_STATE_PRELIGHT] = CouleurSoldeAlarmeRougePrelight;
		    }
		}
		gtk_widget_set_style ( pLabel, pStyleLabelSoldeCourant );

		/* Création d'une boite à évènement qui sera rattachée au solde pointé du compte */
		pEventBox = gtk_event_box_new ();
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "enter-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_prelight ),
				     NULL );
		gtk_signal_connect ( GTK_OBJECT ( pEventBox ),
				     "leave-notify-event",
				     GTK_SIGNAL_FUNC ( met_en_normal ),
				     NULL );
		gtk_signal_connect_object ( GTK_OBJECT ( pEventBox ),
					    "button-press-event",
					    GTK_SIGNAL_FUNC ( click_sur_compte_accueil ),
					    GINT_TO_POINTER ( ordre_comptes_variable->data ));
		gtk_table_attach ( GTK_TABLE ( pTable ), pEventBox,
				   5, 6, i, i+1,
				   GTK_FILL| GTK_SHRINK, GTK_FILL| GTK_SHRINK,
				   0, 0 );
		gtk_widget_show ( pEventBox );
		gtk_container_add ( GTK_CONTAINER ( pEventBox ), pLabel );
		gtk_widget_show ( pLabel );

		/* Septième colonne : elle contient le symbole de la devise du compte */
		pLabel = gtk_label_new ( devise_code_by_no ( DEVISE ));
		gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
		gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
				   6, 7, i, i+1,
				   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
				   5, 0 );
		gtk_widget_show ( pLabel );

		/* Huitième colonne : vide */
		pLabel = gtk_label_new ("");
		gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 7, 8, i, i+1 );
		gtk_widget_show ( pLabel );

		/* ATTENTION : les sommes effectuées ici présupposent que
		   TOUS les comptes sont dans la MÊME DEVISE !!!!!        */
		solde_global_courant += SOLDE_COURANT ;
		solde_global_pointe += SOLDE_POINTE ;
	    }
	    i++;
	}
	while ( ( ordre_comptes_variable = ordre_comptes_variable->next ) );

	/* Création de la (nb_comptes + 2)ième ligne du tableau : vide */
	pLabel = gtk_label_new ( "" );
	gtk_table_attach_defaults ( GTK_TABLE ( pTable ), pLabel, 0, 1, i, i+1 );
	gtk_widget_show ( pLabel );

	/* Création et remplissage de la (nb_comptes + 3)ième ligne du tableau :
	   elle contient la somme des soldes de chaque compte */
	/* Deuxième colonne */
	pLabel = gtk_label_new ( COLON(_("Global balances")));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   1, 2, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   20, 0 );
	gtk_widget_show ( pLabel );

	/* Troisième colonne : elle contient le solde total pointé des comptes */
	pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", solde_global_pointe ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   2, 3, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( pLabel );

	/* Quatrième colonne : elle contient le symbole de la devise du compte */
	pLabel = gtk_label_new ( devise_code_by_no (((struct struct_devise *) devise -> data) -> no_devise ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   3, 4, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   5, 0 );
	gtk_widget_show ( pLabel );

	/* Sixième colonne : elle contient le solde total courant des comptes */
	pLabel = gtk_label_new ( g_strdup_printf ( "%4.2f", solde_global_courant ));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_RIGHT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   5, 6, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   0, 0 );
	gtk_widget_show ( pLabel );

	/* Septième colonne : elle contient le symbole de la devise du compte */
	pLabel = gtk_label_new ( devise_code_by_no (((struct struct_devise *) devise -> data) -> no_devise));
	gtk_misc_set_alignment ( GTK_MISC ( pLabel ), MISC_LEFT, MISC_VERT_CENTER );
	gtk_table_attach ( GTK_TABLE ( pTable ), pLabel,
			   6, 7, i+1, i+2,
			   GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK,
			   5, 0 );
	gtk_widget_show ( pLabel );

	gtk_widget_show_all ( paddingbox );
	gtk_widget_show_all ( pTable );
    }

    gtk_widget_show_all (vbox);
}
/* ************************************************************************* */



/* ************************************************************************* */
/* cette fonction est appelée quand on click sur un compte dans l'accueil */
/* elle fait un click sur le bouton du compte dans l'onglet des opérations */
/* ************************************************************************* */
gboolean click_sur_compte_accueil ( gint *no_compte )
{
    GList *liste_tmp;

    liste_tmp = GTK_BOX ( vbox_liste_comptes ) -> children;

    while ( liste_tmp )
    {
	GtkBoxChild *child;

	child = liste_tmp -> data;

	if ( gtk_list_button_get_data ( GTK_LIST_BUTTON ( child-> widget )) == no_compte )
	{
	    gtk_button_clicked ( GTK_BUTTON ( child -> widget ));
	    liste_tmp = NULL;
	    continue;
	}
	liste_tmp = liste_tmp -> next;
    }
    return FALSE;
}
/* ************************************************************************* */





/* ************************************************************************* */
void update_liste_echeances_manuelles_accueil ( void )
{
    if ( !mise_a_jour_liste_echeances_manuelles_accueil )
	return;

    if ( DEBUG )
	printf ( "update_liste_echeances_manuelles_accueil\n" );

    mise_a_jour_liste_echeances_manuelles_accueil = 0;

    if ( echeances_a_saisir )
    {
	GtkWidget *vbox;
	GtkWidget *label;
	GSList *pointeur_liste;
	GtkWidget *event_box;
	GtkWidget *hbox;
	GdkColor couleur_bleue, couleur_jaune;

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
	label = gtk_label_new ("");
	/*       gtk_box_pack_start ( GTK_BOX (vbox ), label, FALSE, FALSE, 0 );
		 gtk_widget_show ( label ); */

	/* création du style normal -> bleu */
	/* pointeur dessus -> jaune-rouge */

	style_label = gtk_style_copy ( gtk_widget_get_style (label));

	couleur_bleue.red = 500;
	couleur_bleue.green = 500;
	couleur_bleue.blue = 65535;

	couleur_jaune.red =40000;
	couleur_jaune.green =40000;
	couleur_jaune.blue = 0;

	style_label->fg[GTK_STATE_PRELIGHT] = couleur_jaune;
	style_label->fg[GTK_STATE_NORMAL] = couleur_bleue;
	style_label->fg[GTK_STATE_INSENSITIVE] = couleur_bleue;
	style_label->fg[GTK_STATE_SELECTED] = couleur_bleue;
	style_label->fg[GTK_STATE_ACTIVE] = couleur_bleue;


	pointeur_liste = g_slist_sort(echeances_a_saisir,
				      (GCompareFunc) classement_sliste_echeance_par_date );

	while ( pointeur_liste )
	{
	    hbox = gtk_hbox_new ( TRUE, 5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );
	    gtk_widget_show (  hbox );

	    /* bouton à gauche */

	    event_box = gtk_event_box_new ();
	    gtk_signal_connect ( GTK_OBJECT ( event_box ),
				 "enter-notify-event",
				 GTK_SIGNAL_FUNC ( met_en_prelight ),
				 NULL );
	    gtk_signal_connect ( GTK_OBJECT ( event_box ),
				 "leave-notify-event",
				 GTK_SIGNAL_FUNC ( met_en_normal ),
				 NULL );
	    gtk_signal_connect ( GTK_OBJECT ( event_box ),
				 "button-press-event",
				 (GtkSignalFunc) saisie_echeance_accueil,
				 ECHEANCE_COURANTE );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), event_box, TRUE, TRUE, 5 );
	    gtk_widget_show ( event_box  );

	    label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d : %s",
						      ECHEANCE_COURANTE->jour,
						      ECHEANCE_COURANTE->mois,
						      ECHEANCE_COURANTE->annee,
						      tiers_name_by_no ( ECHEANCE_COURANTE->tiers, FALSE )));

	    gtk_widget_set_style ( label, style_label );
	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
	    gtk_container_add ( GTK_CONTAINER ( event_box ), label );
	    gtk_widget_show ( label  );

	    /* label à droite */

	    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + ECHEANCE_COURANTE->compte;

	    if ( ECHEANCE_COURANTE -> montant >= 0 )
		label = gtk_label_new ( g_strdup_printf (_("%4.2f %s credit on %s"),
							 ECHEANCE_COURANTE->montant,
							 devise_code_by_no(ECHEANCE_COURANTE -> devise ),
							 NOM_DU_COMPTE ));
	    else
		label = gtk_label_new ( g_strdup_printf (_("%4.2f %s debit on %s"),
							 -ECHEANCE_COURANTE->montant,
							 devise_code_by_no( ECHEANCE_COURANTE -> devise ),
							 NOM_DU_COMPTE ));


	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, TRUE, 0 );
	    gtk_widget_show (  label );

	    pointeur_liste = pointeur_liste -> next;
	}

	/* on met une ligne vide pour faire joli */

	/*      label = gtk_label_new ("");
		gtk_box_pack_start ( GTK_BOX (vbox ), label, FALSE, FALSE, 0 );
		gtk_widget_show ( label ); */

    }
    else
    {
	hide_paddingbox ( frame_etat_echeances_manuelles_accueil );
    }
}
/* ************************************************************************* */

/* ************************************************************************* */
void update_liste_echeances_auto_accueil ( void )
{
    if ( ! mise_a_jour_liste_echeances_auto_accueil )
	return;

    if ( DEBUG )
	printf ( "update_liste_echeances_auto_accueil\n" );

    mise_a_jour_liste_echeances_auto_accueil = 0;

    if ( echeances_saisies )
    {
	GtkWidget *vbox, *label, *event_box, *hbox;
	GSList *pointeur_liste;
	struct structure_operation *operation;
	GtkStyle *style_selectable;
	GdkColor gray_color;

	/* s'il y avait déjà un fils dans la frame, le détruit */
	gtk_notebook_remove_page ( GTK_NOTEBOOK ( frame_etat_echeances_auto_accueil ), 0 );
	/* on affiche la seconde frame dans laquelle on place les échéances à saisir */
	show_paddingbox ( frame_etat_echeances_auto_accueil );

	gray_color.red =   0.61 * 65535 ;
	gray_color.green = 0.61 * 65535 ;
	gray_color.blue =  0.61 * 65535 ;
	style_selectable = gtk_style_copy ( gtk_widget_get_style ( frame_etat_echeances_auto_accueil ));
	style_selectable->fg[GTK_STATE_PRELIGHT] = gray_color;

	/* on y place la liste des échéances */

	vbox = gtk_vbox_new ( FALSE, 5 );
	gtk_container_add ( GTK_CONTAINER ( frame_etat_echeances_auto_accueil ), vbox);
	gtk_widget_show ( vbox);

	pointeur_liste = echeances_saisies;

	while ( pointeur_liste )
	{
	    operation = pointeur_liste -> data;

	    hbox = gtk_hbox_new ( TRUE, 5 );
	    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );
	    gtk_widget_show (  hbox );

	    event_box = gtk_event_box_new ();
	    gtk_signal_connect ( GTK_OBJECT ( event_box ),
				 "enter-notify-event",
				 GTK_SIGNAL_FUNC ( met_en_prelight ),
				 NULL );
	    gtk_signal_connect ( GTK_OBJECT ( event_box ),
				 "leave-notify-event",
				 GTK_SIGNAL_FUNC ( met_en_normal ),
				 NULL );
	    gtk_signal_connect ( GTK_OBJECT ( event_box ),
				 "button-press-event",
				 (GtkSignalFunc) select_expired_scheduled_transaction,
				 operation );
	    gtk_widget_show ( event_box );

	    /* label à gauche */

	    label = gtk_label_new ( g_strdup_printf ( "%02d/%02d/%d : %s",
						      operation -> jour,
						      operation -> mois,
						      operation -> annee,
						      tiers_name_by_no (operation->tiers, FALSE)));

	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
	    gtk_widget_set_style ( label, style_selectable );
	    gtk_box_pack_start ( GTK_BOX ( hbox ), event_box, TRUE, TRUE, 5 );
	    gtk_container_add ( GTK_CONTAINER ( event_box ), label );
	    gtk_widget_show ( label  );

	    /* label à droite */

	    p_tab_nom_de_compte_variable=p_tab_nom_de_compte + operation->no_compte;

	    if ( operation -> montant >= 0 )
		label = gtk_label_new ( g_strdup_printf (_("%4.2f %s credit on %s"),
							 operation->montant,
							 devise_code_by_no( operation -> devise ),
							 NOM_DU_COMPTE ));
	    else
		label = gtk_label_new ( g_strdup_printf (_("%4.2f %s debit on %s"),
							 -operation->montant,
							 devise_code_by_no( operation -> devise ),
							 NOM_DU_COMPTE ));

	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_VERT_CENTER );
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
/* ************************************************************************* */

/* ************************************************************************* */
/* Fonction update_soldes_minimaux */
/* vérifie les soldes de tous les comptes, affiche un message d'alerte si nécessaire */
/* et ajoute dans l'accueil les comptes sous les soldes minimaux */
/* ************************************************************************* */

void update_soldes_minimaux ( void )
{
    GtkWidget *vbox_1;
    GtkWidget *vbox_2;
    GtkWidget *label;
    gint i;
    GSList *liste_autorise;
    GSList *liste_voulu;
    GSList *liste_autorise_et_voulu;


    if ( !mise_a_jour_soldes_minimaux  )
	return;

    if ( DEBUG )
	printf ( "update_soldes_minimaux\n" );

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



    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	gint solde_courant;
	gint solde_mini;
	gint solde_mini_voulu;

	p_tab_nom_de_compte_variable=p_tab_nom_de_compte + i;

	/* le plus simple est de faire les comparaisons de soldes sur des integer */

	solde_courant = rint ( SOLDE_COURANT * 100 );
	solde_mini = rint ( SOLDE_MINI * 100 );
	solde_mini_voulu = rint ( SOLDE_MINI_VOULU * 100 );


	if ( solde_courant < solde_mini && TYPE_DE_COMPTE != 2 )
	{
	    if ( !vbox_1 )
	    {
		vbox_1 = gtk_vbox_new ( TRUE, 5 );
		gtk_container_add ( GTK_CONTAINER ( frame_etat_soldes_minimaux_autorises ), vbox_1 );
		gtk_widget_show ( vbox_1 );
		show_paddingbox ( frame_etat_soldes_minimaux_autorises );
	    }
	    label = gtk_label_new ( NOM_DU_COMPTE );
	    gtk_box_pack_start ( GTK_BOX ( vbox_1 ), label, FALSE, FALSE, 0 );
	    gtk_misc_set_alignment ( GTK_MISC ( label ), MISC_LEFT, MISC_TOP );
	    gtk_widget_show ( label );

	    show_paddingbox ( frame_etat_soldes_minimaux_autorises );
	}

	if ( solde_courant < solde_mini_voulu && TYPE_DE_COMPTE != 2 &&
	     solde_courant > solde_mini && TYPE_DE_COMPTE != 2)
	{
	    if ( !vbox_2 )
	    {
		vbox_2 = gtk_vbox_new ( TRUE, 5 );
		gtk_container_add ( GTK_CONTAINER ( frame_etat_soldes_minimaux_voulus ), vbox_2 );
		gtk_widget_show ( vbox_2 );
		show_paddingbox ( frame_etat_soldes_minimaux_voulus );
	    }

	    label = gtk_label_new ( NOM_DU_COMPTE );
	    gtk_box_pack_start ( GTK_BOX ( vbox_2 ), label, FALSE, FALSE, 0 );
	    gtk_widget_show ( label );

	    show_paddingbox ( frame_etat_soldes_minimaux_voulus );
	}
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
    gint i;
    GSList *liste_autorise;
    GSList *liste_voulu;
    GSList *liste_autorise_et_voulu;
    GSList *liste_tmp;
    gchar *texte_affiche;


    if ( !mise_a_jour_soldes_minimaux  )
	return;

    if ( DEBUG )
	printf ( "affiche_dialogue_soldes_minimaux\n" );

    liste_autorise = NULL;
    liste_voulu = NULL;
    liste_autorise_et_voulu = NULL;


    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	gint solde_courant;
	gint solde_mini;
	gint solde_mini_voulu;

	p_tab_nom_de_compte_variable=p_tab_nom_de_compte + i;

	/* le plus simple est de faire les comparaisons de soldes sur des integer */

	solde_courant = rint ( SOLDE_COURANT * 100 );
	solde_mini = rint ( SOLDE_MINI * 100 );
	solde_mini_voulu = rint ( SOLDE_MINI_VOULU * 100 );


	if ( solde_courant < solde_mini
	     &&
	     TYPE_DE_COMPTE != 2
	     &&
	     !MESSAGE_SOUS_MINI
	     &&
	     !patience_en_cours )
	{
	    if ( solde_courant  < solde_mini_voulu )
	    {
		liste_autorise_et_voulu = g_slist_append ( liste_autorise_et_voulu,
							   NOM_DU_COMPTE );
		MESSAGE_SOUS_MINI_VOULU = 1;
	    }
	    else
	    {
		liste_autorise = g_slist_append ( liste_autorise,
						  NOM_DU_COMPTE );
	    }
	    MESSAGE_SOUS_MINI = 1;
	}

	if ( solde_courant < solde_mini_voulu
	     &&
	     solde_courant > solde_mini
	     &&
	     TYPE_DE_COMPTE != 2
	     &&
	     !MESSAGE_SOUS_MINI_VOULU
	     &&
	     !patience_en_cours )
	    {
		liste_voulu = g_slist_append ( liste_voulu,
					       NOM_DU_COMPTE );
		MESSAGE_SOUS_MINI_VOULU = 1;
	    }

	/* 	si on repasse au dessus des seuils, c'est comme si on n'avait rien affiché */

	if ( solde_courant > solde_mini )
	    MESSAGE_SOUS_MINI = 0;
	if ( solde_courant > solde_mini_voulu )
	    MESSAGE_SOUS_MINI_VOULU = 0;
    }

    /*     on crée le texte récapilutatif */

    texte_affiche = "";
    
    if ( liste_autorise_et_voulu )
    {
	if ( g_slist_length ( liste_autorise_et_voulu ) == 1 )
	    texte_affiche = g_strdup_printf ( _("balance of account %s is under wanted and authorised minima!"),
					      (gchar *) liste_autorise_et_voulu -> data );
	else
	{
	    texte_affiche = _("accounts with the balance under wanted and authorised minimal :\n\n");
	    liste_tmp = liste_autorise_et_voulu;
	    while ( liste_tmp )
	    {
		texte_affiche = g_strconcat ( texte_affiche,
					      liste_tmp -> data,
					      "\n",
					      NULL );
		liste_tmp = liste_tmp -> next;
	    }
	}
    }

    if ( liste_autorise )
    {
	if ( strlen (texte_affiche))
	    texte_affiche = g_strconcat ( texte_affiche,
					  "\n\n",
					  NULL );
	
	if ( g_slist_length ( liste_autorise ) == 1 )
	    texte_affiche = g_strconcat ( texte_affiche,
					  g_strdup_printf ( _("balance of account %s is under authorised minimum!"),
							    (gchar *) liste_autorise -> data ),
					  NULL );
	else
	{
	    texte_affiche = g_strconcat ( texte_affiche,
					  _("accounts with the balance under authorised minimal :\n\n"),
					  NULL );
	    liste_tmp = liste_autorise;
	    while ( liste_tmp )
	    {
		texte_affiche = g_strconcat ( texte_affiche,
					      liste_tmp -> data,
					      "\n",
					      NULL );
		liste_tmp = liste_tmp -> next;
	    }
	}
    }

    if ( liste_voulu )
    {
	if ( strlen (texte_affiche))
	    texte_affiche = g_strconcat ( texte_affiche,
					  "\n\n",
					  NULL );
	
	if ( g_slist_length ( liste_voulu ) == 1 )
	    texte_affiche = g_strconcat ( texte_affiche,
					  g_strdup_printf ( _("balance of account %s is under wanted minimum!"),
							    (gchar *) liste_voulu -> data ),
					  NULL );
	else
	{
	    texte_affiche = g_strconcat ( texte_affiche,
					  _("accounts with the balance under wanted minimal :\n\n"),
					  NULL );
	    liste_tmp = liste_voulu;
	    while ( liste_tmp )
	    {
		texte_affiche = g_strconcat ( texte_affiche,
					      liste_tmp -> data,
					      "\n",
					      NULL );
		liste_tmp = liste_tmp -> next;
	    }
	}
    }

    if ( strlen ( texte_affiche ))
	dialogue_conditional ( texte_affiche,
			       &(etat.display_message_minimum_alert));
}
/* ************************************************************************* */





/* ************************************************************************* */
void update_fin_comptes_passifs ( void )
{
    gint i;
    GtkWidget *vbox;
    GtkWidget *label;
    GSList *liste_tmp;
    GSList *pointeur;

    if ( !mise_a_jour_fin_comptes_passifs  )
	return;

    if ( DEBUG )
	printf ( "update_fin_comptes_passifs\n" );

    mise_a_jour_fin_comptes_passifs = 0;

    gtk_notebook_remove_page ( GTK_NOTEBOOK(frame_etat_fin_compte_passif), 0 );
    hide_paddingbox ( frame_etat_fin_compte_passif );

    p_tab_nom_de_compte_variable=p_tab_nom_de_compte;
    liste_tmp = NULL;

    for ( i = 0 ; i < nb_comptes ; i++ )
    {
	if ( TYPE_DE_COMPTE == 2 &&
	     SOLDE_COURANT >= 0 )
	    liste_tmp = g_slist_append ( liste_tmp, NOM_DU_COMPTE );

	p_tab_nom_de_compte_variable++;
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
/* ************************************************************************* */



gboolean select_expired_scheduled_transaction ( GtkWidget * event_box, GdkEventButton *event,
						struct structure_operation * operation )
{
    changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));
    edition_operation ();
    return ( FALSE );
}
