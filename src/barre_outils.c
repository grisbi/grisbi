/* fichier qui barre d'outils */
/*           barre_outils.c */

/*     Copyright (C) 2000-2002  Cédric Auger */
/* 			cedric@grisbi.org */
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


#include "./xpm/ope_simples.xpm"
#include "./xpm/ope_semi_completes.xpm"
#include "./xpm/ope_completes.xpm"
#include "./xpm/ope_sans_r.xpm"
#include "./xpm/ope_avec_r.xpm"
#include "./xpm/image_fleche_haut.xpm"
#include "./xpm/image_fleche_bas.xpm"



/*******************************************************************************************/
GtkWidget *creation_barre_outils ( void )
{
  GtkWidget *hbox;
  GtkWidget *separateur;
  GtkWidget *icone;
  GtkWidget *hbox2;


  tooltips = gtk_tooltips_new ();

  hbox = gtk_hbox_new ( FALSE,
			5 );


  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( separateur );



  /* bouton affiche / cache le formulaire */

  bouton_affiche_cache_formulaire = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_affiche_cache_formulaire ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_affiche_cache_formulaire,
			 _("Affiche / Masque le formulaire"),
			 _("Affiche / Masque le formulaire") );
  gtk_widget_set_usize ( bouton_affiche_cache_formulaire,
			 15,
			 15 );

  hbox2 = gtk_hbox_new ( TRUE,
			 0 );
  gtk_container_add ( GTK_CONTAINER ( bouton_affiche_cache_formulaire ),
		      hbox2 );
  gtk_widget_show ( hbox2 );

  fleche_haut = gnome_pixmap_new_from_xpm_d ( image_fleche_haut_xpm );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       fleche_haut,
		       FALSE,
		       FALSE,
		       0 );

  fleche_bas = gnome_pixmap_new_from_xpm_d ( image_fleche_bas_xpm );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       fleche_bas,
		       FALSE,
		       FALSE,
		       0 );


  if ( etat.formulaire_toujours_affiche )
    gtk_widget_show ( fleche_bas );
  else
    gtk_widget_show ( fleche_haut );

  gtk_signal_connect ( GTK_OBJECT ( bouton_affiche_cache_formulaire ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( affiche_cache_le_formulaire ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_affiche_cache_formulaire,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_cache_formulaire );


  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( separateur );


  /* bouton opérations complètes */

  bouton_ope_completes = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_completes ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_ope_completes,
			 _("Opérations complètes"),
			 _("Opérations complètes") );

  if ( nb_lignes_ope == 4 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_completes ),
				   TRUE );

  icone = gnome_pixmap_new_from_xpm_d ( ope_completes );
  gtk_container_add ( GTK_CONTAINER ( bouton_ope_completes ),
		      icone );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ope_completes ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 0 ) );
  gtk_widget_set_usize ( bouton_ope_completes,
			 15,
			 15 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_ope_completes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_ope_completes );



  /* bouton opérations semi-complètes */

  bouton_ope_semi_completes = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_semi_completes ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_ope_semi_completes,
			 _("Opérations semi-complètes"),
			 _("Opérations semi-complètes") );

  if ( nb_lignes_ope == 3 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_semi_completes ),
				   TRUE );

  icone = gnome_pixmap_new_from_xpm_d ( ope_semi_completes );
  gtk_container_add ( GTK_CONTAINER ( bouton_ope_semi_completes ),
		      icone );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ope_semi_completes ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 4 ) );
  gtk_widget_set_usize ( bouton_ope_semi_completes,
			 15,
			 15 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_ope_semi_completes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_ope_semi_completes );



  /* bouton opérations simplifiées */

  bouton_ope_simples = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_simples ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_ope_simples,
			 _("Opérations simplifiées"),
			 _("Opérations simplifiées") );
  if ( nb_lignes_ope == 1 )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_simples ),
				   TRUE );

  icone = gnome_pixmap_new_from_xpm_d ( ope_simples );
  gtk_container_add ( GTK_CONTAINER ( bouton_ope_simples ),
		      icone );
  gtk_widget_set_usize ( bouton_ope_simples,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ope_simples ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 1 ) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_ope_simples,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_ope_simples );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( separateur );


  /* bouton affiche opérations relevées */

  bouton_affiche_r = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_affiche_r ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_affiche_r,
			 _("Opérations rapprochées non masquées"),
			 _("Opérations rapprochées non masquées") );
      if (  etat.r_affiches == 1 )
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_r ),
				       TRUE );

  gtk_widget_set_usize ( bouton_affiche_r,
			 15,
			 15 );
  icone = gnome_pixmap_new_from_xpm_d ( ope_avec_r );
  gtk_container_add ( GTK_CONTAINER ( bouton_affiche_r ),
		      icone );
  gtk_signal_connect ( GTK_OBJECT ( bouton_affiche_r ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 2 ) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_affiche_r,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_affiche_r );


  /* bouton efface opérations relevées */

  bouton_enleve_r = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_enleve_r ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_enleve_r,
			 _("Opérations rapprochées masquées"),
			 _("Opérations rapprochées masquées") );
  if (  !etat.r_affiches )
    gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_enleve_r ),
				   TRUE );

  icone = gnome_pixmap_new_from_xpm_d ( ope_sans_r );
  gtk_container_add ( GTK_CONTAINER ( bouton_enleve_r ),
		      icone );
  gtk_widget_set_usize ( bouton_enleve_r,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton_enleve_r ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 3 ) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_enleve_r,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_enleve_r );


  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( separateur );



  label_proprietes_operations_compte = gtk_label_new (_("Opérations du compte"));
  gtk_box_pack_end ( GTK_BOX ( hbox ),
		     label_proprietes_operations_compte,
		     FALSE,
		     FALSE,
		     0 );
  gtk_widget_show ( label_proprietes_operations_compte );

  return ( hbox );
}
/*******************************************************************************************/




/****************************************************************************************************/
void change_aspect_liste ( GtkWidget *bouton,
			   gint demande )
{

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  switch ( demande )
    {
    case 0 :

      /* ope complètes */
      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_completes ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_simples),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_simples ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_simples),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_semi_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_semi_completes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_semi_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));

      if ( nb_lignes_ope != 4 )
	{
	  nb_lignes_ope = 4;
	  demande_mise_a_jour_tous_comptes ();
	}

      break;

    case 1 :

      /* ope simples */
      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_completes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_simples),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_simples ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_simples),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_semi_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_semi_completes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_semi_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));

      if ( nb_lignes_ope != 1 )
	{
      nb_lignes_ope = 1;
      demande_mise_a_jour_tous_comptes ();
	}

      break;

    case 2 :

      /* ope avec r */
      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_enleve_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 3 ));
       gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_enleve_r ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_enleve_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 3 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_affiche_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 2 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_r ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_affiche_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 2 ));

      if (  etat.r_affiches != 1 )
	{
	  etat.r_affiches = 1;

	  demande_mise_a_jour_tous_comptes ();
	  selectionne_ligne ( compte_courant );
	}
      break;

    case 3 :

      /* ope sans r */
      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_enleve_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 3 ));
       gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_enleve_r ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_enleve_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 3 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_affiche_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 2 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_r ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_affiche_r ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 2 ));

      if (  etat.r_affiches )
	{
	  etat.r_affiches = 0;

	  demande_mise_a_jour_tous_comptes ();
	  selectionne_ligne ( compte_courant );
	}
      break;


    case 4 :

      /* ope semi-complètes */

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_semi_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_semi_completes ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_semi_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_completes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_completes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_simples),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_simples ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_simples),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));

      if ( nb_lignes_ope != 3 )
	{
	  nb_lignes_ope = 3;

	  demande_mise_a_jour_tous_comptes ();
	}
      break;


    default :
    }

/*   si un compte était affiché en ce moment, on le met à jour si necessaire */

  if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general ) ) == 1 )
       verification_mise_a_jour_liste ();

  focus_a_la_liste ();
}
/* ***************************************************************************************************** */




/*******************************************************************************************/
GtkWidget *creation_barre_outils_echeancier ( void )
{
  GtkWidget *hbox;
  GtkWidget *separateur;
  GtkWidget *hbox2;


  hbox = gtk_hbox_new ( FALSE,
			5 );
  gtk_widget_show ( hbox );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );



  /* bouton affiche / cache le formulaire */

  bouton_affiche_cache_formulaire_echeancier = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_affiche_cache_formulaire_echeancier ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_affiche_cache_formulaire_echeancier,
			 _("Affiche / masque le formulaire"),
			 _("Affiche / masque le formulaire") );
  gtk_widget_set_usize ( bouton_affiche_cache_formulaire_echeancier,
			 15,
			 15 );

  hbox2 = gtk_hbox_new ( TRUE,
			 0 );
  gtk_container_add ( GTK_CONTAINER ( bouton_affiche_cache_formulaire_echeancier ),
		      hbox2 );
  gtk_widget_show ( hbox2 );

  fleche_haut_echeancier = gnome_pixmap_new_from_xpm_d ( image_fleche_haut_xpm );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       fleche_haut_echeancier,
		       FALSE,
		       FALSE,
		       0 );

  fleche_bas_echeancier = gnome_pixmap_new_from_xpm_d ( image_fleche_bas_xpm );
  gtk_box_pack_start ( GTK_BOX ( hbox2 ),
		       fleche_bas_echeancier,
		       FALSE,
		       FALSE,
		       0 );


  if ( etat.formulaire_echeancier_toujours_affiche )
    gtk_widget_show ( fleche_bas_echeancier );
  else
    gtk_widget_show ( fleche_haut_echeancier );

  gtk_signal_connect ( GTK_OBJECT ( bouton_affiche_cache_formulaire_echeancier ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( affiche_cache_le_formulaire_echeancier ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_affiche_cache_formulaire_echeancier,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( bouton_affiche_cache_formulaire_echeancier );


  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  return ( hbox );
}
/*******************************************************************************************/
