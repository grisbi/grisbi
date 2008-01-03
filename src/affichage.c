/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger	(cedric@grisbi.org)	      */
/*			2006-2007 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "affichage.h"
#include "./utils_file_selection.h"
#include "./gsb_automem.h"
#include "./gsb_data_account.h"
#include "./gsb_file.h"
#include "./gsb_form.h"
#include "./navigation.h"
#include "./barre_outils.h"
#include "./gsb_transactions_list.h"
#include "./main.h"
#include "./traitement_variables.h"
#include "./utils_str.h"
#include "./utils.h"
#include "./structures.h"
#include "./utils_file_selection.h"
#include "./utils_buttons.h"
#include "./gsb_transactions_list.h"
#include "./include.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean change_choix_utilise_fonte_liste ( GtkWidget *check_button,
					    GtkWidget *vbox );
static gboolean change_choix_utilise_logo ( GtkWidget *check_button,
				     GtkWidget *hbox );
static void change_logo_accueil ( GtkWidget * file_selector );
static gboolean change_toolbar_display_mode ( GtkRadioButton * button );
static void choix_fonte ( GtkWidget *bouton,
		   gchar *fonte,
		   gpointer null );
static gboolean init_fonts ( GtkWidget * button,
		      gpointer user_data);
static gboolean modification_logo_accueil ( );
static void update_font_button(GtkWidget * name_label,
			GtkWidget * size_label,
			gchar * fontname);
static void update_fonte_listes ( void );
static gboolean update_homepage_title (GtkEntry *entry, gchar *value, 
				gint length, gint * position);
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern GtkWidget *arbre_tiers;
extern gchar *chemin_logo;
extern GtkWidget *entree_adresse_commune;
extern GtkWidget *entree_adresse_secondaire;
extern GtkWidget *entree_titre_fichier;
extern GtkWidget *fenetre_preferences;
extern gint hauteur_ligne_liste_opes;
extern GtkWidget *hbox_title;
extern GtkWidget *label_titre_fichier;
extern GtkWidget *logo_accueil;
extern PangoFontDescription *pango_desc_fonte_liste;
extern gchar *titre_fichier;
extern GtkWidget *window;
/*END_EXTERN*/




static GtkWidget* list_font_name_label = NULL;
static GtkWidget* list_font_size_label = NULL;

/** Button used to store a nice preview of the homepage logo */
static GtkWidget *logo_button = NULL;

/** GtkImage containing the preview  */
static GtkWidget *preview = NULL;


/**
 * Update transaction form according to state "etat"
 *
 * \param checkbox Widget that triggers this event.  Normally not
 * needed there.
 * \param data A pointer to some random data passed to this hook.  Not
 * used there.
 */
gboolean update_transaction_form ( GtkWidget * checkbox, gpointer data )
{
    if ( etat.affiche_boutons_valider_annuler )
    {
	gtk_widget_show_all (gsb_form_get_button_part ());
    }
    else
    {
	gtk_widget_hide_all (gsb_form_get_button_part ());
    }
    return ( FALSE );
}



/**
 * Creates the "Fonts & logo" tab.  This function creates some buttons
 * that are borrowed from applications like gedit.
 *
 * \returns A newly allocated vbox
 */
GtkWidget * onglet_display_fonts ( void )
{
    GtkWidget *hbox, *vbox_pref, *label, *paddingbox, *font_button;
    GtkWidget *hbox_font, *init_button, *check_button, *vbox;
    GdkPixbuf * pixbuf = NULL;

    vbox_pref = new_vbox_with_title_and_icon ( _("Fonts & logo"), "fonts.png" );

    /* Change Grisbi Logo */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Grisbi logo") );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    check_button = gtk_check_button_new_with_label ( _("Use the logo"));
    gtk_box_pack_start ( GTK_BOX ( hbox ), check_button, FALSE, FALSE, 0 );

    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( check_button ),
				   etat.utilise_logo );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    /*     le logo est grisé ou non suivant qu'on l'utilise ou pas */ 
    gtk_widget_set_sensitive ( hbox, etat.utilise_logo );
    g_signal_connect ( G_OBJECT ( check_button ), "toggled",
		       G_CALLBACK ( change_choix_utilise_logo ), hbox );

    logo_button = gtk_button_new ();
    g_signal_connect ( G_OBJECT (logo_button ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &logo_button );
    gtk_button_set_relief ( GTK_BUTTON ( logo_button ), GTK_RELIEF_NONE );

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
	     gdk_pixbuf_get_height(pixbuf) > 64 )
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
    g_signal_connect ( G_OBJECT (preview ), "destroy",
			G_CALLBACK ( gtk_widget_destroyed), &preview );

    gtk_container_add (GTK_CONTAINER(logo_button), preview);
    g_signal_connect_swapped ( G_OBJECT ( logo_button ), "clicked",
			       GTK_SIGNAL_FUNC ( modification_logo_accueil ), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), logo_button, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Click on preview to change logo") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* Change fonts */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Fonts") );

    hbox = gtk_hbox_new ( FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    check_button = gtk_check_button_new_with_label ( _("Use a custom font for the transactions"));
    gtk_box_pack_start ( GTK_BOX ( hbox ), check_button, FALSE, FALSE, 0 );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( check_button ),
				   etat.utilise_fonte_listes );

    /*     on crée la vbox qui contiendra la font button et le raz */ 
    vbox = gtk_vbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), vbox, FALSE, FALSE, 0 );

    gtk_widget_set_sensitive ( vbox, etat.utilise_fonte_listes );
    g_signal_connect ( G_OBJECT ( check_button ), "toggled",
		       G_CALLBACK ( change_choix_utilise_fonte_liste ), vbox );

    /* mise en place de la ligne du font button */ 
    hbox = gtk_hbox_new ( TRUE, 0 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 0 );

    /* Create font button */

    font_button = gtk_button_new ();
    hbox_font = gtk_hbox_new ( FALSE, 6 );
    if ( !pango_desc_fonte_liste )
	pango_desc_fonte_liste = pango_font_description_from_string ("Sans 10" );

    list_font_name_label = gtk_label_new (pango_font_description_to_string ( pango_desc_fonte_liste ));
    g_signal_connect ( G_OBJECT (list_font_name_label ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &list_font_name_label );
    gtk_widget_modify_font (list_font_name_label, pango_desc_fonte_liste);
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), list_font_name_label,
			 TRUE, TRUE, 5 );
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), gtk_vseparator_new (),
			 FALSE, FALSE, 0 );
    list_font_size_label = gtk_label_new (NULL);
    g_signal_connect ( G_OBJECT (list_font_size_label ), "destroy",
    		G_CALLBACK ( gtk_widget_destroyed), &list_font_size_label );
    gtk_box_pack_start ( GTK_BOX ( hbox_font ), list_font_size_label,
			 FALSE, FALSE, 5 );
    gtk_container_add (GTK_CONTAINER(font_button), hbox_font);
    gtk_signal_connect ( GTK_OBJECT ( font_button ), "clicked",
			 GTK_SIGNAL_FUNC ( choix_fonte ), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), font_button, TRUE, TRUE, 0 );

    update_font_button(list_font_name_label, list_font_size_label,
		       pango_font_description_to_string ( pango_desc_fonte_liste));

    /*     mise en place du raz de la fonte */
    init_button = gtk_button_new_with_label ( SPACIFY(_("Revert to default font")) );
    gtk_box_pack_start ( GTK_BOX ( hbox ), init_button, FALSE, FALSE, 0 );

    g_signal_connect (init_button, "clicked", G_CALLBACK (init_fonts), NULL);

    if ( !gsb_data_account_get_accounts_amount () )
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
	    gtk_box_pack_start ( GTK_BOX ( hbox_title ), logo_accueil, FALSE, FALSE, 0 );
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

    etat.utilise_fonte_listes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button ));
    gtk_widget_set_sensitive ( vbox,
			       etat.utilise_fonte_listes );

    update_fonte_listes ();

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

    entree_titre_fichier = gsb_automem_entry_new (&titre_fichier,
						 ((GCallback)update_homepage_title), NULL);
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
    entree_adresse_commune = gsb_automem_textview_new ( &adresse_commune, NULL, NULL );
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
    entree_adresse_secondaire = gsb_automem_textview_new ( &adresse_secondaire, NULL, NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			entree_adresse_secondaire );

    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );

    return ( vbox_pref );
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

    font_name = my_strdup ( fontname );
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

    if ( font_name ) g_free ( font_name );
}


/* **************************************************************************************************************************** */
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
    gchar *nom_init = "Sans 10";


    pango_desc_fonte_liste = pango_font_description_from_string ( nom_init );

    update_font_button (list_font_name_label, 
			list_font_size_label, 
			nom_init);

    update_fonte_listes ();


    return ( FALSE );
}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
void update_fonte_listes ( void )
{
    /* met à jour toutes les listes avec la fonte avec pango_desc_fonte_liste ou null */
    /*     on va faire le tour de toutes les listes pour rajouter à la position 11 la fonte ou null  */

    PangoFontDescription *fonte_desc;
    GSList *list_tmp;

    /*     pour reprendre la fonte de base, on récupère celle de la liste des tiers, qui n'est pas modifiée */

    if ( etat.utilise_fonte_listes )
	fonte_desc = pango_desc_fonte_liste;
    else
	/* 	fonte_desc = pango_font_description_copy ( arbre_tiers -> style -> font_desc ); */
	fonte_desc = NULL;

    list_tmp = gsb_data_account_get_list_accounts ();

    while ( list_tmp )
    {
	gint i;
	GtkTreeIter iter;

	i = gsb_data_account_get_no_account ( list_tmp -> data );

	if ( gsb_transactions_list_get_store()
	     &&
	     gtk_tree_model_get_iter_first ( GTK_TREE_MODEL ( gsb_transactions_list_get_store()  ),
					     &iter ))
	    do
	    {
		gtk_tree_store_set ( GTK_TREE_STORE (gsb_transactions_list_get_store() ),
				     &iter,
				     TRANSACTION_COL_NB_FONT, fonte_desc,
				     -1 );
	    }
	    while ( gtk_tree_model_iter_next ( GTK_TREE_MODEL (gsb_transactions_list_get_store() ),
					       &iter ));

	list_tmp = list_tmp -> next;
    }

    /*     on affiche la liste puis change la hauteur des lignes */

    update_ecran ();
    hauteur_ligne_liste_opes = 0;
    gsb_transactions_list_move_to_current_transaction ( gsb_gui_navigation_get_current_account () );

}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
void choix_fonte ( GtkWidget *bouton,
		   gchar *fonte,
		   gpointer null )
{
    gchar * fontname;
    GtkWidget * dialog;

    dialog = gtk_font_selection_dialog_new (COLON(_("Transaction list font")));

    if (pango_desc_fonte_liste)
	gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG(dialog), 
						 pango_font_description_to_string ( pango_desc_fonte_liste));
    gtk_window_set_modal ( GTK_WINDOW ( dialog ), 
			   TRUE );

    switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
	case GTK_RESPONSE_OK:
	    fontname = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG(dialog));
	    gtk_widget_destroy (dialog);  
	    update_font_button (list_font_name_label, list_font_size_label, 
				fontname);
	    break;
	default:
	    gtk_widget_destroy (dialog);
	    return;
    }

    pango_desc_fonte_liste = pango_font_description_from_string ( fontname );

    update_fonte_listes ();

}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void change_logo_accueil ( GtkWidget * file_selector )
{
    GdkPixbuf * pixbuf;
    const gchar *selected_filename;

    selected_filename = file_selection_get_filename (GTK_FILE_CHOOSER (file_selector));

    if ( gsb_data_account_get_accounts_amount () )
    {
	/* on change le logo */
	if ( chemin_logo )
	    g_free ( chemin_logo );
	chemin_logo = my_strdup ( (gchar *) selected_filename );

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
	    gtk_box_pack_start ( GTK_BOX ( hbox_title ), logo_accueil, FALSE, FALSE, 0 );
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

	/* Mark file as modified */
	modification_fichier ( TRUE );
    }
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
gboolean modification_logo_accueil ( )
{
    GtkWidget *file_selector;

    file_selector = file_selection_new (_("Select a new logo"),
					FILE_SELECTION_IS_OPEN_DIALOG|FILE_SELECTION_MUST_EXIST);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (file_selector),
					 gsb_file_get_last_path ());
    /* FIXME: be sure preview will be displayed. */
    gtk_file_chooser_set_preview_widget_active ( GTK_FILE_CHOOSER(file_selector), TRUE );

    gtk_window_set_transient_for ( GTK_WINDOW ( file_selector ),
				   GTK_WINDOW ( fenetre_preferences ));
    gtk_window_set_modal ( GTK_WINDOW ( file_selector ), TRUE );

    switch ( gtk_dialog_run ( GTK_DIALOG ( file_selector ) ) )
    {
	case GTK_RESPONSE_OK:
	    change_logo_accueil ( file_selector );
	    gsb_file_update_last_path (file_selection_get_last_directory (GTK_FILE_CHOOSER (file_selector), TRUE));

	default:
	    gtk_widget_destroy ( file_selector );
	    break;
    }


    return ( FALSE );
}



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

    gchar* tmpstr = g_strconcat ("<span size=\"x-large\">",
					(gchar *) gtk_entry_get_text (GTK_ENTRY (entry)),
					"</span>", NULL );
    gtk_label_set_markup ( GTK_LABEL ( label_titre_fichier ), tmpstr );
    g_free ( tmpstr );

    /* Update window title */
    gsb_file_update_window_title();

    /* Mark file as modified */
    modification_fichier ( TRUE );

    return FALSE;
}



/**
 * Signal triggered when user configure display mode of toolbar
 * buttons.
 *
 * \param button	Radio button that triggered event.
 * 
 * \return FALSE
 */
gboolean change_toolbar_display_mode ( GtkRadioButton * button )
{
    if ( gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(button)) )
    {
	etat.display_toolbar = GPOINTER_TO_INT (g_object_get_data ( G_OBJECT(button), "display" ));
    }

    gsb_gui_update_transaction_toolbar ();

    return FALSE;
}



/**
 * Create a vbox containing widgets allowing to change aspects of
 * toolbar buttons.
 *
 * \return A newly created vbox.
 */
GtkWidget *tab_display_toolbar ( void )
{
    GtkWidget * vbox_pref, * paddingbox, * radio, * radiogroup;

    vbox_pref = new_vbox_with_title_and_icon ( _("Toolbars"), "toolbar.png" );

    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, 
					     _("Display toolbar buttons as") );

    radiogroup = radio = gtk_radio_button_new_with_label ( NULL, _("Text") );
    g_object_set_data ( G_OBJECT(radio), "display", GINT_TO_POINTER(GSB_BUTTON_TEXT) );
    gtk_box_pack_start ( GTK_BOX(paddingbox), radio, FALSE, FALSE, 0 );
    if ( etat.display_toolbar == GSB_BUTTON_TEXT )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (radio), TRUE );
    g_signal_connect ( G_OBJECT(radio), "toggled", 
		       G_CALLBACK(change_toolbar_display_mode), NULL );

    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON(radiogroup), 
							  _("Icons") );
    g_object_set_data ( G_OBJECT(radio), "display", GINT_TO_POINTER(GSB_BUTTON_ICON) );
    gtk_box_pack_start ( GTK_BOX(paddingbox), radio, FALSE, FALSE, 0 );
    if ( etat.display_toolbar == GSB_BUTTON_ICON )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (radio), TRUE );
    g_signal_connect ( G_OBJECT(radio), "toggled", 
		       G_CALLBACK(change_toolbar_display_mode), NULL );

    radio = gtk_radio_button_new_with_label_from_widget ( GTK_RADIO_BUTTON(radiogroup), 
							  _("Both") );
    g_object_set_data ( G_OBJECT(radio), "display", GINT_TO_POINTER(GSB_BUTTON_BOTH) );
    gtk_box_pack_start ( GTK_BOX(paddingbox), radio, FALSE, FALSE, 0 );    
    if ( etat.display_toolbar == GSB_BUTTON_BOTH )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON (radio), TRUE );
    g_signal_connect ( G_OBJECT(radio), "toggled", 
		       G_CALLBACK(change_toolbar_display_mode), NULL );

    gtk_widget_show_all ( vbox_pref );

    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );

    /** TODO: really add option to hide toolbar?  This could save
     * space, but if so, we should add all equivalents to menus.  */
/*     gtk_box_pack_start ( GTK_BOX ( vbox_pref ),  */
/* 			 gsb_automem_checkbutton_new ( _("Display toolbar"), */
/* 						   &(etat.show_toolbar), */
/* 						   NULL ), */
/* 			 FALSE, FALSE, 0 ); */

    gtk_box_pack_start ( GTK_BOX ( vbox_pref ), 
			 gsb_automem_checkbutton_new ( _("Display headings bar"),
						       &(etat.show_headings_bar),
						       NULL, NULL ),
			 FALSE, FALSE, 0 );

    return ( vbox_pref );

}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
