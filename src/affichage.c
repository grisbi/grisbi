/* Ce fichier s'occupe des différents paramètres d'affichage réglés
   dans les paramètres */
/* affichage.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org) */
/*			2003-2004 Benjamin Drieu (bdrieu@april.org) */
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
#include "structures.h"
#include "variables-extern.c"
#include "affichage.h"
#include "constants.h"

#include "banque.h"
#include "operations_formulaire.h"
#include "search_glist.h"
#include "traitement_variables.h"
#include "utils.h"
#include "utils_file_selection.h"


GtkWidget * list_font_name_label, * list_font_size_label;
GtkWidget * general_font_name_label, * general_font_size_label;

/** Button used to store a nice preview of the homepage logo */
GtkWidget *logo_button;

/** GtkImage containing the preview  */
GtkWidget *preview;

GtkWidget *anim_button;
GtkWidget *anim_preview;

extern GtkWidget *widget_formulaire_echeancier[SCHEDULER_FORM_TOTAL_WIDGET];
extern GtkWidget *separateur_formulaire_echeancier;
extern GtkWidget *hbox_valider_annuler_echeance;
extern GtkWidget *liste_echeances;


/**
 * Updates transaction form according to state "etat"
 *
 * \param checkbox Widget that triggers this event.  Normally not
 * needed there.
 * \param data A pointer to some random data passed to this hook.  Not
 * used there.
 */
gboolean update_transaction_form ( GtkWidget * checkbox, gpointer data )
{
    gboolean selected = gtk_widget_get_style ( widget_formulaire_operations[TRANSACTION_FORM_DATE] ) == style_entree_formulaire[ENCLAIR];
    if ( nb_comptes )
    {
	if ( etat.affiche_no_operation )
	    gtk_widget_show ( widget_formulaire_operations[TRANSACTION_FORM_OP_NB] );
	else
	    gtk_widget_hide ( widget_formulaire_operations[TRANSACTION_FORM_OP_NB] );

	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_VALUE_DATE],
				   etat.affiche_date_bancaire );

	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_EXERCICE],
				   etat.utilise_exercice && selected );
	gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_EXERCICE],
				   etat.utilise_exercice && selected  );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_EXERCICE],
				   etat.utilise_exercice && selected  );

	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_BUDGET],
				   etat.utilise_imputation_budgetaire && selected  );
	gtk_widget_set_sensitive ( page_imputations,
				   etat.utilise_imputation_budgetaire );
	gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_BUDGETARY],
				   etat.utilise_imputation_budgetaire && selected  );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_BUDGETARY],
				   etat.utilise_imputation_budgetaire && selected  );

	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_VOUCHER],
				   etat.utilise_piece_comptable && selected  );
	gtk_widget_set_sensitive ( widget_formulaire_ventilation[TRANSACTION_BREAKDOWN_FORM_VOUCHER],
				   etat.utilise_piece_comptable && selected  );
	gtk_widget_set_sensitive ( widget_formulaire_echeancier[SCHEDULER_FORM_VOUCHER],
				   etat.utilise_piece_comptable && selected  );

	gtk_widget_set_sensitive ( widget_formulaire_operations[TRANSACTION_FORM_BANK],
				   etat.utilise_info_banque_guichet && selected  );

	if ( etat.affiche_boutons_valider_annuler )
	{
	    gtk_widget_show ( separateur_formulaire_operations );
	    gtk_widget_show ( hbox_valider_annuler_ope );
	    gtk_widget_show ( separateur_formulaire_echeancier );
	    gtk_widget_show ( hbox_valider_annuler_echeance );
	}
	else
	{
	    gtk_widget_hide ( separateur_formulaire_operations );
	    gtk_widget_hide ( hbox_valider_annuler_ope );
	    gtk_widget_hide ( separateur_formulaire_echeancier );
	    gtk_widget_hide ( hbox_valider_annuler_echeance );

	    affiche_cache_le_formulaire ();
	    affiche_cache_le_formulaire ();
	}
    }
    return ( FALSE );
}


GtkWidget *onglet_display_transaction_form ( void )
{
    GtkWidget *vbox_pref;
    GtkWidget *paddingbox;
    GtkWidget *table, *radiogroup;

    vbox_pref = new_vbox_with_title_and_icon ( _("Transaction form"),
					       "form.png" );

    /* What to do if RETURN is pressed into transaction form */
    radiogroup = new_radiogroup_with_title (vbox_pref,
					    _("Pressing RETURN in transaction form"),
					    _("selects next field"),
					    _("terminates transaction"),
					    &etat.entree, NULL);

    /* Displayed fields */
    paddingbox = new_paddingbox_with_title (vbox_pref, FALSE, 
					    COLON(_("Displayed fields")));

    table = gtk_table_new ( 0, 2, TRUE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 table,
			 FALSE, FALSE, 0 );

    gtk_table_attach ( GTK_TABLE ( table ),
		       new_checkbox_with_title (_("Transaction number"),
						&etat.affiche_no_operation,
						((GCallback) update_transaction_form)),
		       0, 1, 0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    gtk_table_attach ( GTK_TABLE ( table ),
		       new_checkbox_with_title (_("Value date"),
						&etat.affiche_date_bancaire,
						((GCallback) update_transaction_form)),
		       0, 1, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    gtk_table_attach ( GTK_TABLE ( table ),
		       new_checkbox_with_title (_("Financial year"),
						&etat.utilise_exercice,
						((GCallback) update_transaction_form)),
		       0, 1, 2, 3,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    gtk_table_attach ( GTK_TABLE ( table ),
		       new_checkbox_with_title (_("Budgetary information"),
						&etat.utilise_imputation_budgetaire,
						((GCallback) update_transaction_form)),
		       0, 1, 3, 4,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    gtk_table_attach ( GTK_TABLE ( table ),
		       new_checkbox_with_title (_("Voucher number"),
						&etat.utilise_piece_comptable,
						((GCallback) update_transaction_form)),
		       1, 2, 0, 1,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    gtk_table_attach ( GTK_TABLE ( table ),
		       new_checkbox_with_title (_("Bank reference"),
						&etat.utilise_info_banque_guichet,
						((GCallback) update_transaction_form)),
		       1, 2, 1, 2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    gtk_table_attach ( GTK_TABLE ( table ),
		       new_checkbox_with_title (_("'Accept' and 'Cancel' buttons"),
						&etat.affiche_boutons_valider_annuler,
						((GCallback) update_transaction_form)),
		       1, 2, 2, 3,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );

    /* How to display financial year */
    radiogroup = new_radiogroup_with_title (vbox_pref,
					    _("By default, use financial year"),
					    _("last selected financial year"),
					    _("according to transaction date"),
					    &etat.affichage_exercice_automatique, 
					    NULL);

    if ( !nb_comptes )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    return vbox_pref;
}


/**
 * Creates the "Fonts & logo" tab.  This function creates some buttons
 * that are borrowed from applications like gedit.
 *
 * \returns A newly allocated vbox
 */
GtkWidget * onglet_display_fonts ( void )
{
    GtkWidget *hbox, *vbox_pref;
    GtkWidget *label, *paddingbox;
    GtkWidget *font_button;
    GtkWidget *hbox_font, *init_button;
    GdkPixbuf * pixbuf = NULL;
    GtkWidget *check_button;
    GtkWidget *vbox;
    GdkPixbufAnimation *pixbuf_anim=NULL;

    vbox_pref = new_vbox_with_title_and_icon ( _("Fonts & logo"),
					       "fonts.png" );


    /* Change Grisbi Logo */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Grisbi logo") );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0 );

    check_button = gtk_check_button_new_with_label ( _("Use the logo"));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 check_button,
			 FALSE,
			 FALSE,
			 0 );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( check_button ),
				   etat.utilise_logo );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0 );

    /*     le logo est grisé ou non suivant qu'on l'utilise ou pas */

    gtk_widget_set_sensitive ( hbox,
			       etat.utilise_logo );
    g_signal_connect ( G_OBJECT ( check_button ),
		       "toggled",
		       G_CALLBACK ( change_choix_utilise_logo ),
		       hbox );

    logo_button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( logo_button ),
			    GTK_RELIEF_NONE );

    if ( chemin_logo )
    {
	pixbuf = gdk_pixbuf_new_from_file (chemin_logo, NULL);
    }

    if (!pixbuf)
    {
	preview = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE, 
					     GTK_ICON_SIZE_BUTTON );
    }
    else
    {
	if ( gdk_pixbuf_get_width(pixbuf) > 64 ||
	     gdk_pixbuf_get_height(pixbuf) > 64)
	{
	    GdkPixbuf * tmp;
	    tmp = gdk_pixbuf_new ( GDK_COLORSPACE_RGB, TRUE, 8, 
				   gdk_pixbuf_get_width(pixbuf)/2, 
				   gdk_pixbuf_get_height(pixbuf)/2 );
	    gdk_pixbuf_scale ( pixbuf, tmp, 0, 0, 
			       gdk_pixbuf_get_width(pixbuf)/2, 
			       gdk_pixbuf_get_height(pixbuf)/2,
			       0, 0, 0.5, 0.5, GDK_INTERP_HYPER );
	    pixbuf = tmp;
	}
	preview = gtk_image_new_from_pixbuf (pixbuf);
    }

    gtk_container_add (GTK_CONTAINER(logo_button), preview);
    g_signal_connect_swapped ( G_OBJECT ( logo_button ), "clicked",
			       GTK_SIGNAL_FUNC ( modification_logo_accueil ), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), logo_button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Click on preview to change homepage logo") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0 );



    /*     mise en place du choix de l'animation */

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0 );

    /*     le logo est grisé ou non suivant qu'on l'utilise ou pas */

    anim_button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( anim_button ),
			    GTK_RELIEF_NONE );

    if ( etat.fichier_animation_attente )
	pixbuf_anim = gdk_pixbuf_animation_new_from_file (etat.fichier_animation_attente, NULL);

    if (pixbuf_anim)
    {
	/* 	si l'animation est trop grande, compliqué pour la réduire de taille */
	/* 	    donc on affiche juste la première image en pixbuf */

	if ( gdk_pixbuf_animation_get_width (pixbuf_anim) > 64
	     ||
	     gdk_pixbuf_animation_get_height (pixbuf_anim) > 64 )
	{
	    GdkPixbuf * tmp;
	    tmp = gdk_pixbuf_new ( GDK_COLORSPACE_RGB, TRUE, 8, 
				   gdk_pixbuf_animation_get_width(pixbuf_anim)/2, 
				   gdk_pixbuf_animation_get_height(pixbuf_anim)/2 );
	    gdk_pixbuf_scale ( gdk_pixbuf_animation_get_static_image ( pixbuf_anim ), tmp, 0, 0, 
			       gdk_pixbuf_animation_get_width(pixbuf_anim)/2, 
			       gdk_pixbuf_animation_get_height(pixbuf_anim)/2,
			       0, 0, 0.5, 0.5, GDK_INTERP_HYPER );
	    anim_preview = gtk_image_new_from_pixbuf ( tmp );
	}
	else
	    anim_preview = gtk_image_new_from_animation (pixbuf_anim);
    }
    else
	anim_preview = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE, 
						  GTK_ICON_SIZE_BUTTON );

    gtk_container_add (GTK_CONTAINER(anim_button), 
		       anim_preview);
    g_signal_connect_swapped ( G_OBJECT ( anim_button ),
			       "clicked",
			       GTK_SIGNAL_FUNC ( modification_logo_accueil ), 
			       GINT_TO_POINTER (1) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), anim_button,
			 FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Click on preview to change animation") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0 );


    /* Change fonts */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Fonts") );

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );

    check_button = gtk_check_button_new_with_label ( _("Use a custom font for the transactions"));
    gtk_box_pack_start ( GTK_BOX ( hbox ),
			 check_button,
			 FALSE,
			 FALSE,
			 0 );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( check_button ),
				   etat.utilise_fonte_listes );

    /*     on crée la vbox qui contiendra la font button et le raz */

    vbox = gtk_vbox_new ( FALSE,
			  10 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ),
			 vbox,
			 FALSE,
			 FALSE,
			 0 );

    gtk_widget_set_sensitive ( vbox,
			       etat.utilise_fonte_listes );
    g_signal_connect ( G_OBJECT ( check_button ),
		       "toggled",
		       G_CALLBACK ( change_choix_utilise_fonte_liste ),
		       vbox );

    /* mise en place de la ligne du font button */

    hbox = gtk_hbox_new ( TRUE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );

    /* Create font button */

    font_button = gtk_button_new ();
    hbox_font = gtk_hbox_new ( FALSE, 0 );
    list_font_name_label = gtk_label_new (latin2utf8 (list_font_name));
    if ( fonte_liste )
    {
	gtk_widget_modify_font (list_font_name_label,
				pango_font_description_from_string (latin2utf8 (fonte_liste)));
    }
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
			 list_font_name_label,
			 TRUE, TRUE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
			 gtk_vseparator_new (),
			 FALSE, FALSE, 0 );
    list_font_size_label = gtk_label_new ("");
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
			 list_font_size_label,
			 FALSE, FALSE, 5 );
    gtk_container_add (GTK_CONTAINER(font_button), hbox_font);
    gtk_signal_connect ( GTK_OBJECT ( font_button ),
			 "clicked",
			 GTK_SIGNAL_FUNC ( choix_fonte ),
			 NULL );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
			 font_button,
			 FALSE,
			 FALSE,
			 0 );

    update_font_button(list_font_name_label,
		       list_font_size_label,
		       latin2utf8 (fonte_liste));

    /*     mise en place du raz de la fonte */

    hbox = gtk_hbox_new ( FALSE,
			  0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ),
			 hbox,
			 FALSE,
			 FALSE,
			 0 );

    init_button = gtk_button_new_with_label ( SPACIFY(_("Revert to default font")) );
    gtk_box_pack_end ( GTK_BOX ( hbox ),
			 init_button,
			 TRUE,
			 FALSE,
			 0 );

    g_signal_connect (init_button, "clicked", 
		      G_CALLBACK (init_fonts), NULL);

    if ( !nb_comptes )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }


    return vbox_pref;
}
/* ********************************************************************** */




/* ********************************************************************** */
gboolean change_choix_utilise_logo ( GtkWidget *check_button,
				     GtkWidget *hbox )
{

    etat.utilise_logo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button ));
    gtk_widget_set_sensitive ( hbox,
			       etat.utilise_logo );

    if ( etat.utilise_logo )
    {
	/* 	on recharge l'ancien logo */

	if ( !chemin_logo ||
	     !strlen ( g_strstrip ( chemin_logo )) )
	{
	    chemin_logo = NULL;
	    if ( logo_accueil && GTK_IS_WIDGET ( logo_accueil ))
		gtk_widget_hide ( logo_accueil );
	}
	else
	{
	    /* Update homepage logo */

	    logo_accueil =  gtk_image_new_from_file ( chemin_logo );
	    gtk_box_pack_start ( GTK_BOX ( page_accueil ), logo_accueil,
				 FALSE, FALSE, 0 );
	    gtk_widget_show ( logo_accueil );
	}
    }
    else
	gtk_widget_destroy ( logo_accueil ); 

    modification_fichier ( TRUE );

    return ( FALSE );
}
/* ********************************************************************** */


/* ********************************************************************** */
gboolean change_choix_utilise_fonte_liste ( GtkWidget *check_button,
					    GtkWidget *vbox )
{
    gint i;
    PangoFontDescription *fonte_desc;
    gint size;

    etat.utilise_fonte_listes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button ));
    gtk_widget_set_sensitive ( vbox,
			       etat.utilise_fonte_listes );

    /*     pour reprendre la fonte de base, on récupère celle de la liste des tiers, qui n'est pas modifiée */

    if ( etat.utilise_fonte_listes )
	fonte_desc = pango_font_description_from_string(fonte_liste);
    else
	fonte_desc = pango_font_description_copy ( arbre_tiers -> style -> font_desc );

    style_couleur[0] -> font_desc = fonte_desc;
    style_couleur[1] -> font_desc =  fonte_desc;
    style_rouge_couleur[0] -> font_desc = fonte_desc;
    style_rouge_couleur[1] -> font_desc = fonte_desc;
    style_gris -> font_desc = fonte_desc;

    gtk_style_set_font (style_couleur [0], NULL);
    gtk_style_set_font (style_couleur [1], NULL);
    gtk_style_set_font (style_rouge_couleur [0], NULL);
    gtk_style_set_font (style_rouge_couleur [1], NULL);
    gtk_style_set_font (style_gris, NULL);

    /* Find font size */

    size = pango_font_description_get_size ( fonte_desc);

    if ( nb_comptes )
    {
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    gtk_clist_set_row_height ( GTK_CLIST ( CLIST_OPERATIONS ),
				       (size/PANGO_SCALE) + 7 );

	    p_tab_nom_de_compte_variable++;
	}
	gtk_clist_set_row_height ( GTK_CLIST ( liste_echeances ),
				   (size/PANGO_SCALE) + 7 );
    }	
    return ( FALSE );
}
/* ********************************************************************** */



/**
 * Creates the "Titles & Addresses" tab.  This tab is mainly composed
 * of text entries and editables.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_display_addresses ( void )
{
    GtkWidget *hbox, *vbox_pref, *scrolled_window, *label;
    GtkWidget *paddingbox;

    vbox_pref = new_vbox_with_title_and_icon ( _("Addresses & titles"),
					       "addresses.png" );

    /* Account file title */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Titles") );
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( COLON(_("Accounts file title")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    entree_titre_fichier = new_text_entry (&titre_fichier,
					   ((GCallback)update_homepage_title));
    gtk_box_pack_start ( GTK_BOX ( hbox ), entree_titre_fichier,
			 TRUE, TRUE, 0);

    /* Addresses */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Addresses") );

    /* Common address */
    label = gtk_label_new ( COLON(_("Common address")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), label,
			 TRUE, TRUE, 0);
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
			 FALSE, FALSE, 0);
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    entree_adresse_commune = new_text_area ( &adresse_commune, NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			entree_adresse_commune );

    /* Secondary address */
    /** \note This is not implemented yet */
    label = gtk_label_new ( COLON(_("Secondary address")) );
    gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
    gtk_label_set_justify ( GTK_LABEL(label), GTK_JUSTIFY_RIGHT );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), label,
			 TRUE, TRUE, 0);
    scrolled_window = gtk_scrolled_window_new ( NULL, NULL );
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( scrolled_window ),
				     GTK_POLICY_AUTOMATIC,
				     GTK_POLICY_AUTOMATIC );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW(scrolled_window), 
					  GTK_SHADOW_IN );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), scrolled_window,
			 FALSE, FALSE, 0);
    entree_adresse_secondaire = new_text_area ( &adresse_secondaire, NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			entree_adresse_secondaire );

    if ( !nb_comptes )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );

    return ( vbox_pref );
}
/* **************************************************************************************************************************** */




/* **************************************************************************************************************************** */
void deplacement_haut ( void )
{
    if ( !GPOINTER_TO_INT (GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ))
	return;

    gtk_clist_swap_rows ( GTK_CLIST ( liste_choix_ordre_comptes ),
			  GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ),
			  GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) - 1 );

}
/* **************************************************************************************************************************** */





/* **************************************************************************************************************************** */
void deplacement_bas ( void )
{
    if ( GPOINTER_TO_INT (GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) == ( nb_comptes - 1 ) )
	return;

    gtk_clist_swap_rows ( GTK_CLIST ( liste_choix_ordre_comptes ),
			  GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ),
			  GPOINTER_TO_INT ( GTK_CLIST ( liste_choix_ordre_comptes ) -> selection -> data ) + 1 );
}
/* **************************************************************************************************************************** */



/**
 * Update two labels according to font name, which is parsed to
 * separate both name and size
 *
 * \param name_label a widget which is to receive the name part of the
 *                   font name
 * \param size_label a widget which is to receive the size part of the
 *                   font name
 * \param fontname a font name in the form "name, size" or "name,
 *                 attr, size"
 */
void update_font_button(GtkWidget * name_label,
			GtkWidget * size_label,
			gchar * fontname)
{
    gchar * font_name, *font_size, *tmp;

    if (!fontname)
	fontname = "Sans 10";

    gtk_widget_modify_font (name_label,
			    pango_font_description_from_string(fontname));

    font_name = g_strdup ( fontname );
    tmp = font_name + strlen(font_name) - 1;
    while (g_ascii_isdigit(*tmp) ||
	   (*tmp) == '.')
	tmp --;
    font_size = tmp+1;

    while (*tmp == ' ' ||
	   *tmp == ',')
    {
	*tmp=0;
	tmp--;
    }

    gtk_label_set_text (GTK_LABEL(name_label), font_name);
    gtk_label_set_text (GTK_LABEL(size_label), font_size);
}



/**
 * Reset fonts to their initial value, that is "Sans 10".  Update
 * fonts buttons as well.
 *
 * \param button Button that was clicked
 * \param user_data Not used but required by signal
 */
gboolean init_fonts ( GtkWidget * button,
		      gpointer user_data)
{
    GdkFont *font;
    gint i, size;

    fonte_liste = g_strdup (_("Sans 10"));
    update_font_button (list_font_name_label, 
			list_font_size_label, 
			fonte_liste);
    if ( nb_comptes )
    {
	font = gdk_font_load ( fonte_liste );

	/* Use font */
	style_couleur[0] -> font_desc = pango_font_description_from_string(fonte_liste);
	style_couleur[1] -> font_desc = pango_font_description_from_string(fonte_liste);
	style_rouge_couleur[0] -> font_desc = pango_font_description_from_string(fonte_liste);
	style_rouge_couleur[1] -> font_desc = pango_font_description_from_string(fonte_liste);
	style_gris -> font_desc = pango_font_description_from_string(fonte_liste);

	gtk_style_set_font (style_couleur [0], NULL);
	gtk_style_set_font (style_couleur [1], NULL);
	gtk_style_set_font (style_rouge_couleur [0], NULL);
	gtk_style_set_font (style_rouge_couleur [1], NULL);
	gtk_style_set_font (style_gris, NULL);

	/* Find font size */
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
	size = pango_font_description_get_size (pango_font_description_from_string(fonte_liste));

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    gtk_clist_set_row_height ( GTK_CLIST ( CLIST_OPERATIONS ),
				       (size/PANGO_SCALE) + 7 );
	    p_tab_nom_de_compte_variable++;
	}	

	gtk_clist_set_row_height ( GTK_CLIST ( liste_echeances ),
				   (size/PANGO_SCALE) + 7 );
    }
    return ( FALSE );
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void choix_fonte ( GtkWidget *bouton,
		   gchar *fonte,
		   gpointer null )
{
    GdkFont *font;
    gchar * fontname;
    GtkWidget * dialog;
    gint i,size;

    dialog = gtk_font_selection_dialog_new (COLON(_("Transaction list font")));
    if (fonte_liste)
	gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG(dialog), 
						 fonte_liste);
    gtk_window_set_modal ( GTK_WINDOW ( dialog ), 
			   TRUE );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
	case GTK_RESPONSE_OK:
	    fontname = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG(dialog));
	    gtk_widget_destroy (dialog);  
	    update_font_button (list_font_name_label, list_font_size_label, 
				latin2utf8 (fontname));
	    break;
	default:
	    gtk_widget_destroy (dialog);
	    return;
    }

    fonte_liste = latin2utf8 (fontname);

    if ( nb_comptes )
    {
	font = gdk_font_load ( fonte_liste );

	/* Use font */
	style_couleur[0] -> font_desc = pango_font_description_from_string(fontname);
	style_couleur[1] -> font_desc = pango_font_description_from_string(fontname);
	style_rouge_couleur[0] -> font_desc = pango_font_description_from_string(fontname);
	style_rouge_couleur[1] -> font_desc = pango_font_description_from_string(fontname);
	style_gris -> font_desc = pango_font_description_from_string(fontname);
	gtk_style_set_font (style_couleur [0], NULL);
	gtk_style_set_font (style_couleur [1], NULL);
	gtk_style_set_font (style_rouge_couleur [0], NULL);
	gtk_style_set_font (style_rouge_couleur [1], NULL);
	gtk_style_set_font (style_gris, NULL);

	/* Find font size */
	p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
	size = pango_font_description_get_size (pango_font_description_from_string(fontname));

	for ( i = 0 ; i < nb_comptes ; i++ )
	{
	    gtk_clist_set_row_height ( GTK_CLIST ( CLIST_OPERATIONS ),
				       (size/PANGO_SCALE) + 7 );
	    p_tab_nom_de_compte_variable++;
	}	

	gtk_clist_set_row_height ( GTK_CLIST ( liste_echeances ),
				   (size/PANGO_SCALE) + 7 );
    }
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void change_logo_accueil ( GtkWidget *widget, gpointer user_data )
{
    GtkWidget *file_selector = (GtkWidget *)user_data;
    GdkPixbuf * pixbuf;
    const gchar *selected_filename;

    selected_filename = file_selection_get_filename (GTK_FILE_SELECTION (file_selector));

    if ( nb_comptes )
    {
	/* on change le logo */
        chemin_logo = g_strdup ( (gchar *) selected_filename );

	if ( !chemin_logo ||
	     !strlen ( g_strstrip ( chemin_logo )) )
	{
	    chemin_logo = NULL;
	    if ( logo_accueil && GTK_IS_WIDGET ( logo_accueil ))
		gtk_widget_hide ( logo_accueil );
	}
	else
	{
	    /* Update homepage logo */
	    gtk_widget_destroy ( logo_accueil ); 

	    logo_accueil =  gtk_image_new_from_file ( chemin_logo );
	    gtk_box_pack_start ( GTK_BOX ( page_accueil ), logo_accueil,
				 FALSE, FALSE, 0 );
	    gtk_widget_show ( logo_accueil );
	}

	/* Update preview */
	pixbuf = gdk_pixbuf_new_from_file (chemin_logo, NULL);
	gtk_container_remove (GTK_CONTAINER(logo_button), preview);
	if (!pixbuf)
	{
	    preview = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE, 
						 GTK_ICON_SIZE_BUTTON );
	}
	else
	{
	    if ( gdk_pixbuf_get_width(pixbuf) > 64 ||
		 gdk_pixbuf_get_height(pixbuf) > 64)
	    {
		GdkPixbuf * tmp;
		tmp = gdk_pixbuf_new ( GDK_COLORSPACE_RGB, TRUE, 8, 
				       gdk_pixbuf_get_width(pixbuf)/2, 
				       gdk_pixbuf_get_height(pixbuf)/2 );
		gdk_pixbuf_scale ( pixbuf, tmp, 0, 0, 
				   gdk_pixbuf_get_width(pixbuf)/2, 
				   gdk_pixbuf_get_height(pixbuf)/2,
				   0, 0, 0.5, 0.5, GDK_INTERP_HYPER );
		pixbuf = tmp;
	    }
	    preview = gtk_image_new_from_pixbuf (pixbuf);
	}
	gtk_widget_show ( preview );
	gtk_container_add ( GTK_CONTAINER(logo_button), preview );
    }
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
void change_animation ( GtkWidget *widget,
			gpointer user_data )
{
    GtkWidget *file_selector = (GtkWidget *)user_data;
    GdkPixbufAnimation *pixbuf=NULL;

    etat.fichier_animation_attente = file_selection_get_filename (GTK_FILE_SELECTION (file_selector));


    if ( !etat.fichier_animation_attente
	 ||
	 !strlen ( g_strstrip (etat.fichier_animation_attente )) )
	etat.fichier_animation_attente = NULL;

    /* Update preview */
    if ( etat.fichier_animation_attente )
	pixbuf = gdk_pixbuf_animation_new_from_file (etat.fichier_animation_attente, NULL);

    gtk_container_remove (GTK_CONTAINER(anim_button),
			  anim_preview);

    if (pixbuf)
    {
	/* 	si l'animation est trop grande, compliqué pour la réduire de taille */
	/* 	    donc on affiche juste la première image en pixbuf */

	if ( gdk_pixbuf_animation_get_width (pixbuf) > 64
	     ||
	     gdk_pixbuf_animation_get_height (pixbuf) > 64 )
	{
	    GdkPixbuf * tmp;
	    tmp = gdk_pixbuf_new ( GDK_COLORSPACE_RGB, TRUE, 8, 
				   gdk_pixbuf_animation_get_width(pixbuf)/2, 
				   gdk_pixbuf_animation_get_height(pixbuf)/2 );
	    gdk_pixbuf_scale ( gdk_pixbuf_animation_get_static_image ( pixbuf ), tmp, 0, 0, 
			       gdk_pixbuf_animation_get_width(pixbuf)/2, 
			       gdk_pixbuf_animation_get_height(pixbuf)/2,
			       0, 0, 0.5, 0.5, GDK_INTERP_HYPER );
	    anim_preview = gtk_image_new_from_pixbuf ( tmp );
	}
	else
	    anim_preview = gtk_image_new_from_animation (pixbuf);
    }
    else
	anim_preview = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE, 
						  GTK_ICON_SIZE_BUTTON );
    gtk_widget_show ( anim_preview );
    gtk_container_add ( GTK_CONTAINER(anim_button),
			anim_preview );
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
gboolean modification_logo_accueil ( gint origine )
{
    GtkWidget *file_selector;

    /*     origine = 0 si c'est pour une modif du logo d'accueil */
    /* 	=1 pour une modif de l'animation d'attente */

    if ( origine )
    {
	file_selector = gtk_file_selection_new (_("Select a new animation"));
	g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
			  "clicked",
			  G_CALLBACK (change_animation),
			  (gpointer) file_selector);
    }
    else
    {
	file_selector = gtk_file_selection_new (_("Select a new logo"));

	g_signal_connect (G_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
			  "clicked",
			  G_CALLBACK (change_logo_accueil),
			  (gpointer) file_selector);
    }

    gtk_window_set_transient_for ( GTK_WINDOW ( file_selector ),
				   GTK_WINDOW ( fenetre_preferences ));
    gtk_window_set_modal ( GTK_WINDOW ( file_selector ), TRUE );

    /* Ensure that the dialog box is destroyed when the user clicks a button. */

    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
			      "clicked",
			      G_CALLBACK (gtk_widget_destroy), 
			      (gpointer) file_selector); 

    g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
			      "clicked",
			      G_CALLBACK (gtk_widget_destroy),
			      (gpointer) file_selector); 

    /* Display that dialog */

    gtk_widget_show (file_selector);
    return ( FALSE );
}
/* **************************************************************************************************************************** */



/**
 * Update the label that contain main title in homepage.
 *
 * \param entry Widget that triggered this handled.  Not used.
 * \param value Not used handler parameter.
 * \param length Not used handler parameter.
 * \param position Not used handler parameter.
 */
gboolean update_homepage_title (GtkEntry *entry, gchar *value, 
				gint length, gint * position)
{
    gtk_label_set_text ( GTK_LABEL(label_titre_fichier), 
			 (gchar *) gtk_entry_get_text (GTK_ENTRY (entry)) );

    gtk_label_set_markup ( GTK_LABEL ( label_titre_fichier ), 
			   g_strconcat ("<span size=\"x-large\">",
					(gchar *) gtk_entry_get_text (GTK_ENTRY (entry)),
					"</span>", NULL ) );

    return FALSE;
}
