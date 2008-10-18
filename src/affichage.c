/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)	2000-2007 Cédric Auger	(cedric@grisbi.org)	      */
/*			2006-2008 Benjamin Drieu (bdrieu@april.org)	      */
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
#include "./barre_outils.h"
#include "./main.h"
#include "./traitement_variables.h"
#include "./utils_str.h"
#include "./utils.h"
#include "./transaction_list.h"
#include "./utils_font.h"
#include "./structures.h"
#include "./custom_list.h"
#include "./utils_file_selection.h"
#include "./utils_buttons.h"
#include "./include.h"
#include "./erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean change_choix_utilise_fonte_liste ( GtkWidget *check_button,
					    GtkWidget *vbox );
static gboolean change_choix_utilise_logo ( GtkWidget *check_button,
				     GtkWidget *hbox );
static void change_logo_accueil ( GtkWidget * file_selector );
static gboolean change_toolbar_display_mode ( GtkRadioButton * button );
static gboolean modification_logo_accueil ( );
static void update_fonte_listes ( gchar *fontname,
			   gpointer null);
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *adresse_commune ;
extern gchar *adresse_secondaire ;
extern gchar *chemin_logo ;
extern GtkWidget *entree_adresse_commune ;
extern GtkWidget *entree_adresse_secondaire ;
extern GtkWidget *entree_titre_fichier ;
extern GtkWidget *fenetre_preferences ;
extern GtkWidget *hbox_title ;
extern GtkWidget *label_titre_fichier ;
extern GtkWidget *logo_accueil ;
extern gchar *titre_fichier ;
extern GtkWidget *window ;
/*END_EXTERN*/




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
    GtkWidget *check_button, *vbox;
    GdkPixbuf * pixbuf = NULL;

    vbox_pref = new_vbox_with_title_and_icon ( _("Fonts & logo"), "fonts.png" );

    /* Change Grisbi Logo */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Grisbi logo") );

    hbox = gtk_hbox_new ( FALSE, 5 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    check_button = gtk_check_button_new_with_label ( _("Display a logo"));
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


    /* Create font button */
    font_button = utils_font_create_button (&etat.font_string,
					    G_CALLBACK (update_fonte_listes), NULL);
    gtk_box_pack_start ( GTK_BOX (paddingbox), font_button, FALSE, FALSE, 0 );

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    /* change colors */

    return vbox_pref;
}




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

    update_fonte_listes (etat.font_string, NULL);

    return ( FALSE );
}
/* ********************************************************************** */


/**
 * update the font in all the transactions in the list
 *
 * \param
 *
 * \return
 * */
void update_fonte_listes ( gchar *fontname,
			   gpointer null)
{
    GValue value = {0,};
    gchar *font;

    devel_debug (NULL);

    if ( etat.utilise_fonte_listes )
	font = fontname;
    else
	font = NULL;

    g_value_init (&value, G_TYPE_STRING);
    g_value_set_string (&value, font);
    transaction_list_update_column (CUSTOM_MODEL_FONT, &value);
}



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
    /* at the first use of grisbi,label_titre_fichier doesn't still exist */
    if (label_titre_fichier)
    {
	gtk_label_set_text ( GTK_LABEL(label_titre_fichier), 
			     (gchar *) gtk_entry_get_text (GTK_ENTRY (entry)) );

	gchar* tmpstr = g_strconcat ("<span size=\"x-large\">",
				     (gchar *) gtk_entry_get_text (GTK_ENTRY (entry)),
				     "</span>", NULL );
	gtk_label_set_markup ( GTK_LABEL ( label_titre_fichier ), tmpstr );
	g_free ( tmpstr );
    }
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
