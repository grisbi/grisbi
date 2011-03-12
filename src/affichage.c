/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger	(cedric@grisbi.org)           */
/*          2006-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"

/*START_INCLUDE*/
#include "affichage.h"
#include "utils_file_selection.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_file.h"
#include "navigation.h"
#include "fenetre_principale.h"
#include "barre_outils.h"
#include "accueil.h"
#include "gsb_scheduler_list.h"
#include "gsb_select_icon.h"
#include "main.h"
#include "traitement_variables.h"
#include "utils_str.h"
#include "utils.h"
#include "parametres.h"
#include "transaction_list.h"
#include "utils_font.h"
#include "structures.h"
#include "custom_list.h"
#include "utils_buttons.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static gboolean change_choix_utilise_fonte_liste ( GtkWidget *check_button,
                        GtkWidget *vbox );
static gboolean change_choix_utilise_logo ( GtkWidget *check_button,
                        GtkWidget *hbox );
static gboolean change_grisbi_title_type ( GtkRadioButton *button, GtkWidget *entry );
static void change_logo_accueil ( GtkWidget * file_selector );
static gboolean change_toolbar_display_mode ( GtkRadioButton *button );
static gboolean modification_logo_accueil ( );
static gboolean preferences_switch_headings_bar ( GtkWidget *toggle_button,
                        gpointer null );
static gboolean preferences_view_color_changed ( GtkWidget *color_button,
                        GtkWidget *combobox );
static gboolean preferences_view_color_combobox_changed ( GtkWidget *combobox,
                        GtkWidget *color_button );
static gboolean preferences_view_color_default ( GtkWidget *button,
                        GtkWidget *combobox );
static GtkWidget *preferences_view_create_color_combobox (void);
static gboolean preferences_view_update_preview_logo ( GtkFileChooser *file_chooser,
                        GtkWidget *preview );
static void update_fonte_listes ( gchar *fontname,
                        gpointer null);
/*END_STATIC*/


/*START_EXTERN*/
extern gchar *adresse_commune;
extern gchar *adresse_secondaire;
extern GdkColor archive_background_color;
extern GdkColor calendar_entry_color;
extern GdkColor couleur_bet_division;
extern GdkColor couleur_bet_future;
extern GdkColor couleur_bet_solde;
extern GdkColor couleur_bet_transfert;
extern GdkColor couleur_fond[2];
extern GdkColor couleur_grise;
extern GdkColor couleur_jour;
extern GdkColor couleur_selection;
extern GdkColor default_archive_background_color;
extern GdkColor default_calendar_entry_color;
extern GdkColor default_couleur_bet_division;
extern GdkColor default_couleur_bet_future;
extern GdkColor default_couleur_bet_solde;
extern GdkColor default_couleur_bet_transfert;
extern GdkColor default_couleur_fond[2];
extern GdkColor default_couleur_grise;
extern GdkColor default_couleur_jour;
extern GdkColor default_couleur_selection;
extern GdkColor default_split_background;
extern GdkColor default_text_color[2];
extern GtkWidget *fenetre_preferences;
extern GtkWidget *hbox_title;
extern GtkWidget *label_titre_fichier;
extern GtkWidget *logo_accueil;
extern GdkColor split_background;
extern GdkColor text_color[2];
extern gchar *titre_fichier;
extern GtkWidget *window;
/*END_EXTERN*/




/** Button used to store a nice preview of the homepage logo */
static GtkWidget *logo_button = NULL;

/** GtkImage containing the preview  */
static GtkWidget *preview = NULL;



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
    GtkWidget *button;
    GtkWidget *color_combobox;
    GtkWidget *color_button;

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
    g_signal_connect ( G_OBJECT ( check_button ),
                        "toggled",
                        G_CALLBACK ( change_choix_utilise_logo ),
                        hbox );

    logo_button = gtk_button_new ();
    gtk_button_set_relief ( GTK_BUTTON ( logo_button ), GTK_RELIEF_NONE );

	pixbuf = gsb_select_icon_get_logo_pixbuf ( );

    if (!pixbuf)
    {
        preview = gtk_image_new_from_pixbuf ( gsb_select_icon_get_default_logo_pixbuf ( ) );
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

    gtk_container_add (GTK_CONTAINER(logo_button), preview);
    g_signal_connect_swapped ( G_OBJECT ( logo_button ), "clicked",
			       G_CALLBACK ( modification_logo_accueil ), NULL );
    gtk_box_pack_start ( GTK_BOX ( hbox ), logo_button, FALSE, FALSE, 0 );

    label = gtk_label_new ( _("Click on preview to change logo") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label, FALSE, FALSE, 0 );

    /* Change fonts */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Fonts") );

    hbox = gtk_hbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0 );

    check_button = gtk_check_button_new_with_label ( 
                        _("Use a custom font for the transactions: "));
    gtk_box_pack_start ( GTK_BOX ( hbox ), check_button, FALSE, FALSE, 0 );
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( check_button ),
				        conf.utilise_fonte_listes );

    /*     on crée la vbox qui contiendra la font button et le raz */
    vbox = gtk_vbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX ( hbox ), vbox, FALSE, FALSE, 0 );

    gtk_widget_set_sensitive ( vbox, conf.utilise_fonte_listes );
    g_signal_connect ( G_OBJECT ( check_button ), "toggled",
		       G_CALLBACK ( change_choix_utilise_fonte_liste ), vbox );


    /* Create font button */
    font_button = utils_font_create_button ( &conf.font_string,
					    G_CALLBACK (update_fonte_listes), NULL);
    gtk_box_pack_start ( GTK_BOX (vbox), font_button, FALSE, FALSE, 0 );

    if ( !gsb_data_account_get_accounts_amount () )
    {
	gtk_widget_set_sensitive ( vbox_pref, FALSE );
    }

    /* change colors */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Colors") );

    vbox = gtk_vbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), vbox, FALSE, FALSE, 10 );

    hbox = gtk_hbox_new ( FALSE, 10 );
    gtk_box_pack_start ( GTK_BOX ( vbox ), hbox, FALSE, FALSE, 10 );

    color_combobox = preferences_view_create_color_combobox ();
    gtk_box_pack_start ( GTK_BOX (hbox),
			 color_combobox,
			 FALSE, FALSE, 0);

    color_button = gtk_color_button_new ();
    gtk_color_button_set_title ( GTK_COLOR_BUTTON(color_button), _("Choosing color") );
    g_signal_connect ( G_OBJECT (color_button),
		       "color-set",
		       G_CALLBACK (preferences_view_color_changed),
		       G_OBJECT (color_combobox));
    gtk_box_pack_start ( GTK_BOX (hbox),
			 color_button,
			 FALSE, FALSE, 0);

    /* connect the color button to the combobox if changed */
    g_signal_connect ( G_OBJECT (color_combobox),
		       "changed",
		       G_CALLBACK (preferences_view_color_combobox_changed),
		       G_OBJECT (color_button));

    button = gtk_button_new_with_label (_("Back to default"));
    g_signal_connect ( G_OBJECT (button),
		       "clicked",
		       G_CALLBACK (preferences_view_color_default),
		       G_OBJECT (color_combobox));
    gtk_box_pack_start ( GTK_BOX (vbox),
			 button,
			 FALSE, FALSE, 0);

    gtk_combo_box_set_active ( GTK_COMBO_BOX (color_combobox), 0);

    return vbox_pref;
}




/* ********************************************************************** */
gboolean change_choix_utilise_logo ( GtkWidget *check_button,
                        GtkWidget *hbox )
{

    etat.utilise_logo = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON 
                            ( check_button ));
    gtk_widget_set_sensitive ( hbox, etat.utilise_logo );

    if ( etat.utilise_logo )
    {
        /* 	on recharge l'ancien logo */

        if ( GTK_IS_WIDGET ( logo_accueil ) )
            gtk_widget_hide ( logo_accueil );
        else
        {
            GdkPixbuf *pixbuf = NULL;

            /* Update homepage logo */
            pixbuf = gsb_select_icon_get_logo_pixbuf ( );
            if ( pixbuf == NULL )
            {
                pixbuf = gsb_select_icon_get_default_logo_pixbuf ( );
                etat.is_pixmaps_dir = TRUE;
            }
            logo_accueil =  gtk_image_new_from_pixbuf ( pixbuf );
            if ( logo_accueil )
            {
                gtk_box_pack_start ( GTK_BOX ( hbox_title ), logo_accueil, FALSE, FALSE, 0 );
                gtk_widget_set_size_request ( hbox_title, -1, -1 );
                gtk_widget_show ( logo_accueil );
            }
        }
    }
    else
    {
        gtk_widget_destroy ( logo_accueil );
        gtk_widget_set_size_request ( hbox_title, -1, -1 );
        if ( etat.name_logo && strlen ( etat.name_logo ) )
            g_free ( etat.name_logo );
        etat.name_logo = NULL;
        etat.is_pixmaps_dir = 0;
    }

    if ( etat.modification_fichier == 0 )
        modification_fichier ( TRUE );

    return ( FALSE );
}
/* ********************************************************************** */


/* ********************************************************************** */
gboolean change_choix_utilise_fonte_liste ( GtkWidget *check_button,
                        GtkWidget *vbox )
{
    conf.utilise_fonte_listes = gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( check_button ));
    gtk_widget_set_sensitive ( vbox,
			       conf.utilise_fonte_listes );

    update_fonte_listes ( conf.font_string, NULL );

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

    if ( conf.utilise_fonte_listes )
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
    GtkWidget *entry;
    GtkWidget *radio, *radiogroup;

    vbox_pref = new_vbox_with_title_and_icon ( _("Addresses & titles"),
					       "addresses.png" );

    /* Account file title */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE, _("Titles") );

    /* It first creates the entry of title */
    entry = gsb_automem_entry_new ( &titre_fichier,
                        ( GCallback ) update_homepage_title, NULL );

    /* Choice of title type */
    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox, FALSE, FALSE, 0);

    radiogroup = gtk_radio_button_new_with_label ( NULL, _("Accounts file title") );
    g_object_set_data ( G_OBJECT ( radiogroup ), "display", GINT_TO_POINTER ( GSB_ACCOUNTS_TITLE ) );
    if ( conf.display_grisbi_title == GSB_ACCOUNTS_TITLE )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radiogroup ), TRUE );
    g_signal_connect ( G_OBJECT ( radiogroup ), 
                        "toggled",
                        G_CALLBACK ( change_grisbi_title_type ), 
                        entry );
    gtk_box_pack_start ( GTK_BOX( hbox ), radiogroup, FALSE, FALSE, 0 );

    radio = gtk_radio_button_new_with_label_from_widget ( 
                        GTK_RADIO_BUTTON ( radiogroup ),
                        _("Account owner name") );
    g_object_set_data ( G_OBJECT ( radio ), "display", GINT_TO_POINTER ( GSB_ACCOUNT_HOLDER ) );
    if ( conf.display_grisbi_title == GSB_ACCOUNT_HOLDER )
            gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio ), TRUE );
    g_signal_connect ( G_OBJECT ( radio ), 
                    "toggled",
                    G_CALLBACK ( change_grisbi_title_type ), 
                    entry );
    gtk_box_pack_start ( GTK_BOX( hbox ), radio, FALSE, FALSE, 0 );

    radio = gtk_radio_button_new_with_label_from_widget ( 
                        GTK_RADIO_BUTTON ( radiogroup ),
                        _("Filename") );
    g_object_set_data ( G_OBJECT ( radio ), "display", GINT_TO_POINTER ( GSB_ACCOUNTS_FILE ) );
    if ( conf.display_grisbi_title == GSB_ACCOUNTS_FILE )
        gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( radio ), TRUE );
    g_signal_connect ( G_OBJECT ( radio ), 
                        "toggled",
                        G_CALLBACK ( change_grisbi_title_type ), 
                        entry );
    gtk_box_pack_start ( GTK_BOX( hbox ), radio, FALSE, FALSE, 0 );

    hbox = gtk_hbox_new ( FALSE, 6 );
    gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
			 FALSE, FALSE, 0);

    label = gtk_label_new ( _("Accounts file title: ") );
    gtk_box_pack_start ( GTK_BOX ( hbox ), label,
			 FALSE, FALSE, 0);

    if ( conf.display_grisbi_title == GSB_ACCOUNTS_TITLE )
        gtk_widget_set_sensitive ( entry, TRUE);
    else
        gtk_widget_set_sensitive ( entry, FALSE);
    gtk_box_pack_start ( GTK_BOX ( hbox ), entry,
			 TRUE, TRUE, 0);

    /* Addresses */
    paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					     _("Addresses") );

    /* Common address */
    label = gtk_label_new ( _("Common address: ") );
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
    entry = gsb_automem_textview_new ( &adresse_commune, NULL, NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			entry );

    /* Secondary address */
    /** \note This is not implemented yet */
    label = gtk_label_new ( _("Secondary address: ") );
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
    entry = gsb_automem_textview_new ( &adresse_secondaire, NULL, NULL );
    gtk_container_add ( GTK_CONTAINER ( scrolled_window ),
			entry );

    if ( !gsb_data_account_get_accounts_amount () )
	gtk_widget_set_sensitive ( vbox_pref, FALSE );

    return ( vbox_pref );
}


/* **************************************************************************************************************************** */
void change_logo_accueil ( GtkWidget * file_selector )
{
    GdkPixbuf * pixbuf;

    const gchar *selected_filename;

    selected_filename = file_selection_get_filename ( GTK_FILE_CHOOSER ( file_selector ) );

    if ( gsb_data_account_get_accounts_amount () )
    {
        /* on change le logo */
        gchar * chemin_logo;

        gtk_container_remove ( GTK_CONTAINER ( logo_button ), preview );
        chemin_logo = g_strstrip ( g_strdup ( selected_filename ) );
        if ( !strlen ( chemin_logo ) )
        {
            if ( logo_accueil && GTK_IS_WIDGET ( logo_accueil ) )
                gtk_widget_hide ( logo_accueil );
            preview = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE,
                             GTK_ICON_SIZE_BUTTON );
        }
        else
        {
            /* Update preview */
            pixbuf = gdk_pixbuf_new_from_file ( chemin_logo, NULL );
            if (!pixbuf)
            {
                if ( logo_accueil && GTK_IS_WIDGET ( logo_accueil ))
                    gtk_widget_hide ( logo_accueil );
                preview = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE,
                                    GTK_ICON_SIZE_BUTTON );
            }
            else
            {
                if ( g_strcmp0 ( g_path_get_dirname ( chemin_logo ), GRISBI_PIXMAPS_DIR ) == 0 )
                {
                    gchar *name_logo;

                    etat.is_pixmaps_dir = TRUE;

                    name_logo = g_path_get_basename ( chemin_logo );
                    if ( g_strcmp0 ( name_logo, "grisbi-logo.png" ) != 0 )
                        etat.name_logo = name_logo;
                    else
                        etat.name_logo = NULL;
                }
                else
                {
                    etat.is_pixmaps_dir = FALSE;
                    if ( etat.name_logo && strlen ( etat.name_logo ) )
                        g_free ( etat.name_logo );
                    etat.name_logo = NULL;
                }

                gsb_select_icon_set_logo_pixbuf ( pixbuf );
                preview = gtk_image_new_from_pixbuf ( 
                        gdk_pixbuf_scale_simple ( 
                        pixbuf, 48, 48, GDK_INTERP_BILINEAR ) );
                
                /* Update homepage logo */
                gtk_widget_destroy ( logo_accueil );
                
                logo_accueil =  gtk_image_new_from_pixbuf ( 
                                    gsb_select_icon_get_logo_pixbuf ( ) );
                gtk_box_pack_start ( GTK_BOX ( hbox_title ), logo_accueil, FALSE, FALSE, 0 );
                gtk_widget_show ( logo_accueil );
                /* modify the icon of grisbi (set in the panel of gnome or other) */
                gtk_window_set_default_icon ( 
                            gsb_select_icon_get_logo_pixbuf ( ) );
            }

            g_free ( chemin_logo );
        }
        gtk_widget_show ( preview );
        gtk_container_add ( GTK_CONTAINER ( logo_button ), preview );
        
        /* Mark file as modified */
        if ( etat.modification_fichier == 0 )
            modification_fichier ( TRUE );
    }
}
/* **************************************************************************************************************************** */

/* **************************************************************************************************************************** */
gboolean modification_logo_accueil ( )
{
    GtkWidget *file_selector;
    GtkWidget *preview;

    file_selector = gtk_file_chooser_dialog_new ( _("Select a new logo"),
					   GTK_WINDOW ( fenetre_preferences ),
					   GTK_FILE_CHOOSER_ACTION_OPEN,
					   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					   GTK_STOCK_OPEN, GTK_RESPONSE_OK,
					   NULL);

    if ( etat.is_pixmaps_dir )
        gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (
                        file_selector ), GRISBI_PIXMAPS_DIR );
    else
        gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER (
                        file_selector ), gsb_file_get_last_path () );
        
    gtk_window_set_position ( GTK_WINDOW ( file_selector ), GTK_WIN_POS_CENTER_ON_PARENT );

    /* create the preview */
    preview = gtk_image_new ();
    gtk_file_chooser_set_preview_widget ( GTK_FILE_CHOOSER ( file_selector ), preview );
    g_signal_connect (G_OBJECT (file_selector),
                        "update-preview",
                        G_CALLBACK ( preferences_view_update_preview_logo ),
                        preview );

    switch ( gtk_dialog_run ( GTK_DIALOG ( file_selector ) ) )
    {
	case GTK_RESPONSE_OK:
	    change_logo_accueil ( file_selector );
	    gsb_file_update_last_path ( file_selection_get_last_directory (
                        GTK_FILE_CHOOSER ( file_selector ), TRUE ) );

	default:
	    gtk_widget_destroy ( file_selector );
	    break;
    }

    return ( FALSE );
}


/**
 * update the preview of the log file chooser
 *
 * \param file_chooser
 * \param preview
 *
 * \return FALSE
 * */
static gboolean preferences_view_update_preview_logo ( GtkFileChooser *file_chooser,
                        GtkWidget *preview )
{
  char *filename;
  GdkPixbuf *pixbuf;
  gboolean have_preview;

  filename = gtk_file_chooser_get_preview_filename (file_chooser);
  if (!filename)
      return FALSE;

  pixbuf = gdk_pixbuf_new_from_file_at_size ( filename, 
                        LOGO_WIDTH, LOGO_HEIGHT, NULL );
  have_preview = ( pixbuf != NULL );
  g_free (filename);

  gtk_image_set_from_pixbuf ( GTK_IMAGE ( preview ), pixbuf );
  if ( pixbuf )
    g_object_unref ( pixbuf );

  gtk_file_chooser_set_preview_widget_active ( file_chooser, have_preview );
  return FALSE;
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
    if ( titre_fichier && strlen ( titre_fichier ) )
        g_free ( titre_fichier );

    titre_fichier = my_strdup ( gtk_entry_get_text ( GTK_ENTRY ( entry ) ) );

    /* set Grisbi title */
    gsb_main_set_grisbi_title ( -1 );

    /* Mark file as modified */
    if ( etat.modification_fichier == 0 )
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
gboolean change_toolbar_display_mode ( GtkRadioButton *button )
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
						       G_CALLBACK (preferences_switch_headings_bar), NULL ),
			 FALSE, FALSE, 0 );

    return ( vbox_pref );

}


/**
 * called when switch the preferences heading bar button
 * to show/hide the headings bar
 *
 * \param toggle button
 * \param null
 *
 * \return FALSE
 * */
gboolean preferences_switch_headings_bar ( GtkWidget *toggle_button,
                        gpointer null )
{
    gsb_gui_update_show_headings ();
    return FALSE;
}



/**
 * create a list of customable colors
 *
 * \param
 *
 * \return a GtkComboBox
 * */
static GtkWidget *preferences_view_create_color_combobox (void)
{
    GtkWidget *combobox;
    GtkListStore *store;
    gint i;
    GtkCellRenderer *renderer;

    struct config_color {
    gchar *name;
    GdkColor *color;
    GdkColor *default_color;

    } config_colors[] = {
    { N_("Transaction list background 1"), &couleur_fond[0], &default_couleur_fond[0]},
    { N_("Transaction list background 2"), &couleur_fond[1], &default_couleur_fond[1]},
    { N_("Color for the operation that gives the balance today"), &couleur_jour, &default_couleur_jour},
    { N_("Color of transaction's text"), &text_color[0], &default_text_color[0]},
    { N_("Text of unfinished split transaction"), &text_color[1], &default_text_color[1]},
    { N_("Children of split transaction"), &split_background, &default_split_background},
    { N_("Selection color"), &couleur_selection, &default_couleur_selection},
    { N_("Background of non selectable scheduled transactions"), &couleur_grise, &default_couleur_grise},
    { N_("Archive color"), &archive_background_color, &default_archive_background_color},
    { N_("Background of invalid date entry"), &calendar_entry_color, &default_calendar_entry_color },
    { N_("Background of bet division"), &couleur_bet_division, &default_couleur_bet_division },
    { N_("Background of bet futur"), &couleur_bet_future, &default_couleur_bet_future },
    { N_("Background of bet solde"), &couleur_bet_solde, &default_couleur_bet_solde },
    { N_("Background of bet transfer"), &couleur_bet_transfert, &default_couleur_bet_transfert },
    { NULL, 0, 0},
    };

    /* the store contains the name of the color we can modify and
     * a pointer to the corresponding variable */
    store = gtk_list_store_new ( 3,
                        G_TYPE_STRING,
                        G_TYPE_POINTER,
                        G_TYPE_POINTER );
    /* fill the store */
    for ( i = 0 ; config_colors[i].name != NULL ; i++ )
    {
    GtkTreeIter iter;

    gtk_list_store_append ( GTK_LIST_STORE (store),
                        &iter );
    gtk_list_store_set ( GTK_LIST_STORE (store),
                        &iter,
                        0, _(config_colors[i].name),
                        1, config_colors[i].color,
                        2, config_colors[i].default_color,
                        -1);
    }

    /* create the combobox */
    combobox = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer,
                        "text", 0,
                        NULL);
    return combobox;
}


/**
 * called when the color combobox changed,
 * update the GtkColorButton with the color of the combobox
 *
 * \param combobox
 * \param color_button
 *
 * \return FALSE
 * */
static gboolean preferences_view_color_combobox_changed ( GtkWidget *combobox,
                        GtkWidget *color_button )
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
	GtkTreeModel *model;
	GdkColor *color;

	model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
	gtk_tree_model_get ( GTK_TREE_MODEL (model),
			     &iter,
			     1, &color,
			     -1 );
	if (color)
	    gtk_color_button_set_color ( GTK_COLOR_BUTTON (color_button),
					 color );
    }
    return FALSE;
}


/**
 * called when a color is chosen in the GtkColorButton,
 * update the color selected
 *
 * \param color_button
 * \param combobox
 *
 * \return FALSE
 * */
static gboolean preferences_view_color_changed ( GtkWidget *color_button,
                        GtkWidget *combobox )
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
	GtkTreeModel *model;
	GdkColor *color;

	model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
	gtk_tree_model_get ( GTK_TREE_MODEL (model),
			     &iter,
			     1, &color,
			     -1 );
	if (color)
	{
	    gtk_color_button_get_color ( GTK_COLOR_BUTTON (color_button),
				       color );

	    /* update the colors in the transactions list */
	    transaction_list_redraw ();

	    /* update scheduled list */
	    gsb_scheduler_list_fill_list (gsb_scheduler_list_get_tree_view ());
	    gsb_scheduler_list_set_background_color (gsb_scheduler_list_get_tree_view ());
	    gsb_scheduler_list_select (-1);
	}
    }
    return FALSE;
}


/**
 * revert to default the selected color into the combobox
 *
 * \param button
 * \param combobox
 *
 * \return FALSE
 * */
static gboolean preferences_view_color_default ( GtkWidget *button,
                        GtkWidget *combobox )
{
    GtkTreeIter iter;

    if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter))
    {
	GtkTreeModel *model;
	GdkColor *color;
	GdkColor *default_color;

	model = gtk_combo_box_get_model (GTK_COMBO_BOX (combobox));
	gtk_tree_model_get ( GTK_TREE_MODEL (model),
			     &iter,
			     1, &color,
			     2, &default_color,
			     -1 );
	if (color && default_color)
	{
	    gboolean return_val;

	    color -> pixel = default_color -> pixel;
	    color -> red = default_color -> red;
	    color -> green = default_color -> green;
	    color -> blue = default_color -> blue;

	    g_signal_emit_by_name (combobox,
				   "changed",
				   &return_val);

	    /* update the colors in the list */
	    transaction_list_redraw ();

	    /* update scheduled list */
	    gsb_scheduler_list_redraw ();
	}
    }
    return FALSE;
}

/**
 * Signal triggered when user configure display grisbi title
 *
 * \param button	Radio button that triggered event.
 *
 * \return FALSE
 */
gboolean change_grisbi_title_type ( GtkRadioButton *button, GtkWidget *entry )
{
    if ( gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON ( button ) ) )
    {
        conf.display_grisbi_title = GPOINTER_TO_INT ( g_object_get_data 
                        ( G_OBJECT ( button ), "display" ) );
    }

    switch ( conf.display_grisbi_title )
    {
        case GSB_ACCOUNTS_TITLE:
            gtk_widget_set_sensitive ( entry, TRUE );
            if ( titre_fichier && strlen ( titre_fichier ) )
                gtk_entry_set_text ( GTK_ENTRY ( entry ), titre_fichier );
            else
            {
                gtk_entry_set_text ( GTK_ENTRY ( entry ), "" );
                titre_fichier = NULL;
            }
        break;
        case GSB_ACCOUNT_HOLDER:
            gtk_widget_set_sensitive ( entry, FALSE);
        break;
        case GSB_ACCOUNTS_FILE:
            gtk_widget_set_sensitive ( entry, FALSE);
        break;
    }

    /* set Grisbi title */
    gsb_main_set_grisbi_title ( gsb_gui_navigation_get_current_account ( ) );

    return FALSE;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
