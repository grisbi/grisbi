/* Ce fichier s'occupe des différents paramètres d'affichage réglés
   dans les paramètres */
/* affichage.c */

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
#include "structures.h"
#include "variables-extern.c"
#include "en_tete.h"



GtkWidget * list_font_name_label, * list_font_size_label;
GtkWidget * general_font_name_label, * general_font_size_label;

/** Button used to store a nice preview of the homepage logo */
GtkWidget *logo_button;

/** GtkImage containing the preview  */
GtkWidget *preview;



/**
 * Updates transaction form according to state "etat"
 *
 * \param checkbox Widget that triggers this event.  Normally not
 * needed there.
 * \param data A pointer to some random data passed to this hook.  Not
 * used there.
 */
gboolean
update_transaction_form ( GtkWidget * checkbox,
			  gpointer data )
{
  if ( nb_comptes && gtk_widget_get_style ( widget_formulaire_operations[1] ) == 
       style_entree_formulaire[0] )
    {
      if ( etat.affiche_no_operation )
	gtk_widget_show ( widget_formulaire_operations[0] );
      else
	gtk_widget_hide ( widget_formulaire_operations[0] );

      gtk_widget_set_sensitive ( widget_formulaire_operations[7],
				 etat.affiche_date_bancaire );

      gtk_widget_set_sensitive ( widget_formulaire_operations[11],
				 etat.utilise_exercice );
      gtk_widget_set_sensitive ( widget_formulaire_ventilation[6],
				 etat.utilise_exercice );
      gtk_widget_set_sensitive ( widget_formulaire_echeancier[9],
				 etat.utilise_exercice );

      gtk_widget_set_sensitive ( widget_formulaire_operations[12],
				 etat.utilise_imputation_budgetaire );
      gtk_widget_set_sensitive ( page_imputations,
				 etat.utilise_imputation_budgetaire );
      gtk_widget_set_sensitive ( widget_formulaire_ventilation[4],
				 etat.utilise_imputation_budgetaire );
      gtk_widget_set_sensitive ( widget_formulaire_echeancier[10],
				 etat.utilise_imputation_budgetaire );

      gtk_widget_set_sensitive ( widget_formulaire_operations[14],
				 etat.utilise_piece_comptable );
      gtk_widget_set_sensitive ( widget_formulaire_ventilation[7],
				 etat.utilise_piece_comptable );
      gtk_widget_set_sensitive ( widget_formulaire_echeancier[12],
				 etat.utilise_piece_comptable );

      gtk_widget_set_sensitive ( widget_formulaire_operations[17],
				 etat.utilise_info_banque_guichet );
      gtk_widget_set_sensitive ( widget_formulaire_echeancier[11],
				 etat.utilise_info_banque_guichet );

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
}


GtkWidget *
onglet_display_transaction_form ( void )
{
  GtkWidget *hbox, *vbox_pref;
  GtkWidget *label, *paddingbox;
  GtkWidget *table, *bouton, *radiogroup;

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
  GtkWidget *table, *font_button;
  GtkWidget *hbox_font, *init_button;
  GdkPixbuf * pixbuf;

  vbox_pref = new_vbox_with_title_and_icon ( _("Fonts & logo"),
					     "fonts.png" );


  /* Change Grisbi Logo */
  paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					   _("Grisbi logo") );

  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), hbox,
		       FALSE, FALSE, 0 );

  logo_button = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( logo_button ),
			  GTK_RELIEF_NONE );

  pixbuf = gdk_pixbuf_new_from_file (chemin_logo, NULL);
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

  logo_button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER(logo_button), preview);
  gtk_signal_connect ( GTK_OBJECT ( logo_button ), "clicked",
		       GTK_SIGNAL_FUNC ( modification_logo_accueil ), NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ), logo_button,
		       FALSE, FALSE, 0 );

  label = gtk_label_new ( _("Click on preview to change homepage logo") );
  gtk_box_pack_start ( GTK_BOX ( hbox ), label,
		       FALSE, FALSE, 0 );

  /* Change fonts */
  paddingbox = new_paddingbox_with_title ( vbox_pref, FALSE,
					   _("Fonts") );

  /* Create table */
  table = gtk_table_new ( 2, 2, FALSE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table ), 5 );
  gtk_table_set_row_spacings ( GTK_TABLE ( table ), 5 );
  gtk_box_pack_start ( GTK_BOX ( paddingbox ), table,
		       FALSE, FALSE, 0 );

  /* Change general font */
  label = gtk_label_new (COLON(_("General font")));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 0, 1,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );

  /* Create font button */
  font_button = gtk_button_new ();
  hbox_font = gtk_hbox_new ( FALSE, 0 );
  general_font_name_label = gtk_label_new (general_font_name);
  if (fonte_general)
    {
      gtk_widget_modify_font (general_font_name_label,
			      pango_font_description_from_string (fonte_general));
    }
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       general_font_name_label,
		       TRUE, TRUE, 5 );
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       gtk_vseparator_new (),
		       FALSE, FALSE, 0 );
  general_font_size_label = gtk_label_new (general_font_size);
  gtk_box_pack_start ( GTK_BOX ( hbox_font ), 
		       general_font_size_label,
		       FALSE, FALSE, 5 );
  gtk_container_add (GTK_CONTAINER ( font_button ), hbox_font);
  gtk_signal_connect ( GTK_OBJECT ( font_button ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( choix_fonte_general ),
		       NULL );
  gtk_table_attach ( GTK_TABLE ( table ),
		     font_button, 1, 2, 0, 1,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );
  update_font_button(general_font_name_label,
		     general_font_size_label,
		     fonte_general);

  /* Change list font */
  label = gtk_label_new (COLON(_("Transaction list font")));
  gtk_misc_set_alignment (GTK_MISC (label), 0, 1);
  gtk_label_set_justify ( GTK_LABEL (label), GTK_JUSTIFY_RIGHT );
  gtk_table_attach ( GTK_TABLE ( table ),
		     label, 0, 1, 1, 2,
		     GTK_SHRINK | GTK_FILL, 0,
		     0, 0 );

  /* Create font button */
  font_button = gtk_button_new ();
  hbox_font = gtk_hbox_new ( FALSE, 0 );
  list_font_name_label = gtk_label_new (list_font_name);
  if ( fonte_liste )
    {
      gtk_widget_modify_font (list_font_name_label,
			      pango_font_description_from_string (fonte_liste));
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
  gtk_table_attach ( GTK_TABLE ( table ),
		     font_button, 1, 2, 1, 2,
		     GTK_EXPAND | GTK_FILL, 0,
		     0, 0 );
  update_font_button(list_font_name_label,
		     list_font_size_label,
		     fonte_liste);

  hbox = gtk_hbox_new ( FALSE, 5 );
  gtk_box_pack_end ( GTK_BOX ( vbox_pref ), hbox,
		     FALSE, FALSE, 0 );
  init_button = gtk_button_new_with_label ( SPACIFY(_("Restore defaults fonts")) );
  gtk_box_pack_end ( GTK_BOX ( hbox ), init_button,
		     FALSE, FALSE, 0 );
  g_signal_connect (init_button, "clicked", 
		    G_CALLBACK (init_fonts), NULL);

  gtk_widget_show_all ( vbox_pref );

  return vbox_pref;
}


/**
 * Creates the "Titles & Addresses" tab.  This tab is mainly composed
 * of text entries and editables.
 *
 * \returns A newly allocated vbox
 */
GtkWidget *onglet_display_addresses ( void )
{
  GtkWidget *hbox, *vbox_pref, *separateur, *scrolled_window, *label, *frame;
  GtkWidget *vbox2, *fleche, *hbox2, *bouton, *onglet, *paddingbox;
  GSList *liste_tmp;

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



/* FIXME remove as it is deprecated */
GtkWidget *onglet_affichage ( void )
{
}


/* FIXME remove as it is deprecated */
void selection_choix_ordre_comptes ( GtkWidget *box )
{
/*   gtk_widget_set_sensitive ( box, TRUE ); */
}


/* FIXME remove as it is deprecated */
void deselection_choix_ordre_comptes ( GtkWidget *box )
{
/*   gtk_widget_set_sensitive ( box, FALSE ); */
}
/* ************************************************************************************************************** */



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
 * font name
 * \param size_label a widget which is to receive the size part of the
 * font name
 * \param fontname a font name in the form "name, size" or "name,
 * attr, size"
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
  while (isdigit(*tmp) ||
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
  fonte_liste = "Sans 10";
  fonte_general = "Sans 10";
  update_font_button (list_font_name_label, 
		      list_font_size_label, 
		      fonte_liste);
  update_font_button (general_font_name_label, 
		      general_font_size_label, 
		      fonte_general);
}



/* **************************************************************************************************************************** */
void choix_fonte ( GtkWidget *bouton,
		   gchar *fonte,
		   gpointer null )
{
  GdkFont *font;
  gchar * fontname;
  GtkWidget * dialog;
  gint i, size;

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
			    fontname);
	break;
      default:
	gtk_widget_destroy (dialog);
	return;
    }

  fonte_liste = fontname;
  
  if ( nb_comptes )
    {
      font = gdk_font_load ( fonte_liste );

      /* Use font */
      /* FIXME FONTS ... this does not work !!! */
      gtk_style_set_font (style_couleur [0], font);
      gtk_style_set_font (style_couleur [1], font);
      gtk_style_set_font (style_rouge_couleur [0], font);
      gtk_style_set_font (style_rouge_couleur [1], font);

      /* Find font size */
      p_tab_nom_de_compte_variable = p_tab_nom_de_compte;
      for ( i = 0 ; i < nb_comptes ; i++ )
	{
	  gint size = pango_font_description_get_size
	    (pango_font_description_from_string(fontname));
	  gtk_clist_set_row_height ( GTK_CLIST ( CLIST_OPERATIONS ),
				     (size/PANGO_SCALE) + 2 );
	  gtk_clist_set_row_height ( GTK_CLIST ( liste_echeances ),
				     (size/PANGO_SCALE) + 2 );
	  p_tab_nom_de_compte_variable++;
	}

    }
}
/* **************************************************************************************************************************** */





/* **************************************************************************************************************************** */
void choix_fonte_general ( GtkWidget *bouton,
			   gchar *fonte,
			   gpointer null )
{
  GdkFont *font;
  gchar * fontname;
  GtkWidget * dialog;
  GtkStyle * style_general;
  gint i;

  dialog = gtk_font_selection_dialog_new (COLON(_("General font")));
  if (fonte_general)
    gtk_font_selection_dialog_set_font_name (GTK_FONT_SELECTION_DIALOG(dialog), 
					     fonte_general);
  gtk_window_set_modal ( GTK_WINDOW ( dialog ), 
			 TRUE );

  switch ( gtk_dialog_run ( GTK_DIALOG ( dialog ) ) )
    {
    case GTK_RESPONSE_OK:
      fontname = gtk_font_selection_dialog_get_font_name (GTK_FONT_SELECTION_DIALOG(dialog));
      gtk_widget_destroy (dialog);  
      update_font_button (general_font_name_label, general_font_size_label, 
			  fontname);

      gtk_widget_destroy (dialog);
      break;
    default:
      gtk_widget_destroy (dialog);
      return;
    }

  fonte_general = fontname;

  style_general = gtk_widget_get_style ( window );
  gtk_style_set_font (style_general, gdk_font_load ( fonte_general ));
  style_general = gtk_widget_get_default_style ();
  gtk_style_set_font (style_general, gdk_font_load ( fonte_general ));

  if ( nb_comptes )
    {
/*       gtk_notebook_set_page ( GTK_NOTEBOOK ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> notebook ), */
/* 			      0 ); */
/*       gtk_notebook_set_page ( GTK_NOTEBOOK ( GNOME_PROPERTY_BOX ( fenetre_preferences ) -> notebook ), */
/* 			      3 ); */

/*       gtk_widget_destroy ( notebook_general ); */
/*       gtk_widget_destroy ( GNOME_APP ( window ) -> menubar ); */
/*       GNOME_APP ( window ) -> menubar = NULL; */
/*       gnome_app_create_menus ( GNOME_APP ( window ), */
/* 			       menu_principal ); */

/*       ouverture_confirmee (); */
    }
}
/* **************************************************************************************************************************** */



/* **************************************************************************************************************************** */
void modification_logo_accueil ( void )
{
  GtkWidget *dialog, *choix, *bouton;
  GdkPixbuf * pixbuf;
  gint resultat;

  dialog = gnome_dialog_new ( _("Select a new logo"),
			      GNOME_STOCK_BUTTON_OK,
			      GNOME_STOCK_BUTTON_CANCEL,
			      NULL );
/*   gtk_window_set_transient_for ( GTK_WINDOW ( dialog ), */
/* 				 GTK_WINDOW ( fenetre_preferences )); */

  choix = gnome_pixmap_entry_new ( "path_logo_grisbi",
				   _("Select a new logo"),
				   TRUE );
  gtk_widget_set_usize ( choix, 300, 300 );

  if ( chemin_logo )
    gtk_entry_set_text ( GTK_ENTRY ( gnome_pixmap_entry_gtk_entry ( GNOME_PIXMAP_ENTRY ( choix ))),
			 chemin_logo );
  gnome_pixmap_entry_set_preview ( GNOME_PIXMAP_ENTRY ( choix ), TRUE );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ), choix,
		       TRUE, TRUE, 0 );

  bouton = gtk_button_new_with_label ( _("Reset to default logo") );
  gtk_button_set_relief ( GTK_BUTTON ( bouton ), GTK_RELIEF_NONE );
  gtk_signal_connect ( GTK_OBJECT ( bouton ), "clicked",
		       GTK_SIGNAL_FUNC ( remise_a_zero_logo ), choix );
  gtk_box_pack_start ( GTK_BOX ( GNOME_DIALOG ( dialog ) -> vbox ), bouton,
		       TRUE, TRUE, 0 );
  gtk_widget_show_all ( dialog );

  resultat = gnome_dialog_run ( GNOME_DIALOG ( dialog ));

  if ( resultat )
    {
      gnome_dialog_close ( GNOME_DIALOG ( dialog ));
      return;
    }
  if ( nb_comptes )
    {
      /* on change le logo */
      chemin_logo = gnome_pixmap_entry_get_filename ( GNOME_PIXMAP_ENTRY(choix) );

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
	  logo_accueil =  gnome_pixmap_new_from_file ( chemin_logo );
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

  gnome_dialog_close ( GNOME_DIALOG ( dialog ));

  /* on sauvegarde le chemin */

  gnome_config_set_string ( "/Grisbi/Affichage/Chemin_du_logo",
			    chemin_logo );
  gnome_config_sync();
  modification_fichier ( TRUE );

}
/* **************************************************************************************************************************** */


/* **************************************************************************************************************************** */
void remise_a_zero_logo ( GtkWidget *bouton,
			  GtkWidget *pixmap_entry )
{
  chemin_logo = CHEMIN_LOGO;

  gtk_entry_set_text ( GTK_ENTRY ( gnome_pixmap_entry_gtk_entry ( GNOME_PIXMAP_ENTRY ( pixmap_entry ))),
		       chemin_logo );

  gnome_pixmap_entry_set_preview ( GNOME_PIXMAP_ENTRY ( pixmap_entry ),
				   TRUE );
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
gboolean
update_homepage_title (GtkEntry *entry, gchar *value, 
		       gint length, gint * position)
{
  gtk_label_set_text ( GTK_LABEL(label_titre_fichier), 
		       (gchar *) gtk_entry_get_text (GTK_ENTRY (entry)) );

  return FALSE;
}
