/* fichier qui barre d'outils */
/*           barre_outils.c */

/*     Copyright (C) 2000-2003  Cédric Auger */
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


#include "./xpm/ope_1.xpm"
#include "./xpm/ope_2.xpm"
#include "./xpm/ope_3.xpm"
#include "./xpm/ope_4.xpm"
#include "./xpm/ope_sans_r.xpm"
#include "./xpm/ope_avec_r.xpm"
#include "./xpm/image_fleche_haut.xpm"
#include "./xpm/image_fleche_bas.xpm"
#include "./xpm/liste_0.xpm"
#include "./xpm/liste_1.xpm"
#include "./xpm/liste_2.xpm"
#include "./xpm/liste_3.xpm"



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


  /* bouton opérations 4 lignes */

  bouton_ope_4_lignes = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_4_lignes ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_ope_4_lignes,
			 _("4 lignes par opération"),
			 _("4 lignes par opération") );

  icone = gnome_pixmap_new_from_xpm_d ( ope_4_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton_ope_4_lignes ),
		      icone );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ope_4_lignes ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 0 ) );
  gtk_widget_set_usize ( bouton_ope_4_lignes,
			 15,
			 15 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_ope_4_lignes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_ope_4_lignes );



  /* bouton opérations 3 lignes */

  bouton_ope_3_lignes = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_3_lignes ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_ope_3_lignes,
			 _("3 lignes par opération"),
			 _("3 lignes par opération") );

  icone = gnome_pixmap_new_from_xpm_d ( ope_3_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton_ope_3_lignes ),
		      icone );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ope_3_lignes ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 4 ) );
  gtk_widget_set_usize ( bouton_ope_3_lignes,
			 15,
			 15 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_ope_3_lignes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_ope_3_lignes );


  /* bouton opérations 2 lignes */

  bouton_ope_2_lignes = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_2_lignes ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_ope_2_lignes,
			 _("2 lignes par opération"),
			 _("2 lignes par opération") );

  icone = gnome_pixmap_new_from_xpm_d ( ope_2_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton_ope_2_lignes ),
		      icone );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ope_2_lignes ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 5 ) );
  gtk_widget_set_usize ( bouton_ope_2_lignes,
			 15,
			 15 );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_ope_2_lignes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_ope_2_lignes );



  /* bouton opérations 1 ligne */

  bouton_ope_1_lignes = gtk_toggle_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton_ope_1_lignes ),
			  GTK_RELIEF_NONE );
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton_ope_1_lignes,
			 _("1 ligne par opération"),
			 _("1 ligne par opération") );

  icone = gnome_pixmap_new_from_xpm_d ( ope_1_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton_ope_1_lignes ),
		      icone );
  gtk_widget_set_usize ( bouton_ope_1_lignes,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton_ope_1_lignes ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( change_aspect_liste ),
		       GINT_TO_POINTER ( 1 ) );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton_ope_1_lignes,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton_ope_1_lignes );

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

      /* ope 4 lignes */
      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_4_lignes ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_1_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_3_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_2_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));

      /*       si retient_affichage_par_compte est mis, on ne change que le compte courant, */
      /* sinon on change tous les comptes */

      if ( NB_LIGNES_OPE != 4 )
	{
	  if ( etat.retient_affichage_par_compte )
	    {
	      NB_LIGNES_OPE = 4;
	      MISE_A_JOUR = 1;
	    }
	  else
	    {
	      gint i;

	      for ( i=0 ; i<nb_comptes ; i++ )
		{
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		  NB_LIGNES_OPE = 4;
		  MISE_A_JOUR = 1;
		}
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	    }
	}

      break;

    case 1 :

      /* ope 1 ligne */
      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_4_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_1_lignes ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_3_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_2_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));

      /*       si retient_affichage_par_compte est mis, on ne change que le compte courant, */
      /* sinon on change tous les comptes */

      if ( NB_LIGNES_OPE != 1 )
	{
	  if ( etat.retient_affichage_par_compte )
	    {
	      NB_LIGNES_OPE = 1;
	      MISE_A_JOUR = 1;
	    }
	  else
	    {
	      gint i;

	      for ( i=0 ; i<nb_comptes ; i++ )
		{
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		  NB_LIGNES_OPE = 1;
		  MISE_A_JOUR = 1;
		}
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	    }
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

      /*       si retient_affichage_par_compte est mis, on ne change que le compte courant, */
      /* sinon on change tous les comptes */

      if ( !AFFICHAGE_R )
	{
	  if ( etat.retient_affichage_par_compte )
	    {
	      AFFICHAGE_R = 1;
	      MISE_A_JOUR = 1;
	    }
	  else
	    {
	      gint i;

	      for ( i=0 ; i<nb_comptes ; i++ )
		{
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		  AFFICHAGE_R = 1;
		  MISE_A_JOUR = 1;
		}
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	    }
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

      /*       si retient_affichage_par_compte est mis, on ne change que le compte courant, */
      /* sinon on change tous les comptes */

      if ( AFFICHAGE_R )
	{
	  if ( etat.retient_affichage_par_compte )
	    {
	      AFFICHAGE_R = 0;
	      MISE_A_JOUR = 1;
	    }
	  else
	    {
	      gint i;

	      for ( i=0 ; i<nb_comptes ; i++ )
		{
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		  AFFICHAGE_R = 0;
		  MISE_A_JOUR = 1;
		}
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	    }
	}
      break;


    case 4 :

      /* ope 3 lignes */

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_3_lignes ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_4_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_1_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_2_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));

      /*       si retient_affichage_par_compte est mis, on ne change que le compte courant, */
      /* sinon on change tous les comptes */

      if ( NB_LIGNES_OPE != 3 )
	{
	  if ( etat.retient_affichage_par_compte )
	    {
	      NB_LIGNES_OPE = 3;
	      MISE_A_JOUR = 1;
	    }
	  else
	    {
	      gint i;

	      for ( i=0 ; i<nb_comptes ; i++ )
		{
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		  NB_LIGNES_OPE = 3;
		  MISE_A_JOUR = 1;
		}
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	    }
	}
      break;

    case 5 :

      /* ope 2 lignes */

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_2_lignes ),
				     TRUE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 5 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_4_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 NULL );

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_3_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_3_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_3_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 4 ));

      gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_1_lignes ),
				     FALSE );
      gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
					 GTK_SIGNAL_FUNC ( change_aspect_liste ),
					 GINT_TO_POINTER ( 1 ));

      /*       si retient_affichage_par_compte est mis, on ne change que le compte courant, */
      /* sinon on change tous les comptes */

      if ( NB_LIGNES_OPE != 2 )
	{
	  if ( etat.retient_affichage_par_compte )
	    {
	      NB_LIGNES_OPE = 2;
	      MISE_A_JOUR = 1;
	    }
	  else
	    {
	      gint i;

	      for ( i=0 ; i<nb_comptes ; i++ )
		{
		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + i;
		  NB_LIGNES_OPE = 2;
		  MISE_A_JOUR = 1;
		}
	      p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;
	    }
	}
      break;


    default :
    }

/*   si un compte était affiché en ce moment, on le met à jour si necessaire */

  if ( gtk_notebook_get_current_page ( GTK_NOTEBOOK ( notebook_general ) ) == 1 )
    {
      verification_mise_a_jour_liste ();
      selectionne_ligne ( compte_courant );
    }

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




/*******************************************************************************************/
GtkWidget *creation_barre_outils_tiers ( void )
{
  GtkWidget *hbox;
  GtkWidget *separateur;
  GtkWidget *icone;
  GtkWidget *bouton;

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


  /* bouton fermeture de l'arbre */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 0 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Fermer l'arbre"),
			 _("Fermer l'arbre") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_0_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  /* bouton ouverture de l'arbre niveau 1 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 1 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les comptes"),
			 _("Afficher les comptes") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_1_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  /* bouton ouverture de l'arbre niveau 2 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 2 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les opérations"),
			 _("Afficher les opérations") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_2_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       NULL );
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

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





/*******************************************************************************************/
GtkWidget *creation_barre_outils_categ ( void )
{
  GtkWidget *hbox;
  GtkWidget *separateur;
  GtkWidget *icone;
  GtkWidget *bouton;

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


  /* bouton fermeture de l'arbre */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 0 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Fermer l'arbre"),
			 _("Fermer l'arbre") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_0_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (1));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  /* bouton ouverture de l'arbre niveau 1 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 1 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les sous-divisions"),
			 _("Afficher les sous-divisions") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_1_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (1));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  /* bouton ouverture de l'arbre niveau 2 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 2 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les comptes"),
			 _("Afficher les comptes") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_2_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (1));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  /* bouton ouverture de l'arbre niveau 3 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 3 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les opérations"),
			 _("Afficher les opérations") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_3_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (1));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

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






/*******************************************************************************************/
GtkWidget *creation_barre_outils_imputation ( void )
{
  GtkWidget *hbox;
  GtkWidget *separateur;
  GtkWidget *icone;
  GtkWidget *bouton;

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


  /* bouton fermeture de l'arbre */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 0 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Fermer l'arbre"),
			 _("Fermer l'arbre") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_0_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (2));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  /* bouton ouverture de l'arbre niveau 1 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 1 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les sous-divisions"),
			 _("Afficher les sous-divisions") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_1_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (2));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  /* bouton ouverture de l'arbre niveau 2 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 2 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les comptes"),
			 _("Afficher les comptes") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_2_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (2));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

  separateur = gtk_vseparator_new ();
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       separateur,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show ( separateur );

  /* bouton ouverture de l'arbre niveau 3 */

  bouton = gtk_button_new ();
  gtk_button_set_relief ( GTK_BUTTON ( bouton ),
			  GTK_RELIEF_NONE );
  gtk_object_set_data ( GTK_OBJECT ( bouton ),
			"profondeur",
			GINT_TO_POINTER ( 3 ));
  gtk_tooltips_set_tip ( GTK_TOOLTIPS ( tooltips ),
			 bouton,
			 _("Afficher les opérations"),
			 _("Afficher les opérations") );
  icone = gnome_pixmap_new_from_xpm_d ( liste_3_xpm );
  gtk_container_add ( GTK_CONTAINER ( bouton ),
		      icone );
  gtk_widget_set_usize ( bouton,
			 15,
			 15 );
  gtk_signal_connect ( GTK_OBJECT ( bouton ),
		       "clicked",
		       GTK_SIGNAL_FUNC ( demande_expand_arbre ),
		       GINT_TO_POINTER (2));
  gtk_box_pack_start ( GTK_BOX ( hbox ),
		       bouton,
		       FALSE,
		       FALSE,
		       0 );
  gtk_widget_show_all ( bouton );

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


/*******************************************************************************************/
/* étend l'arbre donné en argument en fonction du bouton cliqué (profondeur contenue */
/* dans le bouton) */
/*******************************************************************************************/

void demande_expand_arbre ( GtkWidget *bouton,
			    gint *liste )
{
  GtkWidget *ctree;
  gint i;
  GtkCTreeNode *noeud_selectionne;

  if ( liste )
    {
      if ( GPOINTER_TO_INT ( liste ) == 1 )
	ctree = arbre_categ;
      else
	ctree = arbre_imputation;
    }
  else
    ctree = arbre_tiers;

  gtk_clist_freeze ( GTK_CLIST ( ctree ));

  /* on doit faire ça étage par étage car il y a des ajouts à chaque ouverture de noeud */

  /*   récupère le noeud sélectionné, s'il n'y en a aucun, fera tout l'arbre */

  noeud_selectionne = NULL;

  if ( GTK_CLIST ( ctree ) -> selection )
    noeud_selectionne = GTK_CLIST ( ctree ) -> selection -> data;

  gtk_ctree_collapse_recursive ( GTK_CTREE ( ctree ),
				 noeud_selectionne );

  for ( i=0 ; i < GPOINTER_TO_INT ( gtk_object_get_data ( GTK_OBJECT ( bouton ),
							  "profondeur" )) ; i++ )
    gtk_ctree_expand_to_depth ( GTK_CTREE ( ctree ),
				noeud_selectionne,
				i + 1);
  gtk_clist_thaw ( GTK_CLIST ( ctree ));
}
/*******************************************************************************************/


/*******************************************************************************************/
/* cette fonction met les boutons du nb lignes par opé et de l'affichage de R en fonction du compte */
/* envoyé en argument */
 /*******************************************************************************************/

void mise_a_jour_boutons_caract_liste ( gint no_compte )
{
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + no_compte;

  /*   on va mettre les 4 boutons de lignes à false, puis met à true le bon bouton */

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
				     GTK_SIGNAL_FUNC ( change_aspect_liste ),
				     NULL );
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
				     GTK_SIGNAL_FUNC ( change_aspect_liste ),
				     GINT_TO_POINTER ( 4 ));
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
				     GTK_SIGNAL_FUNC ( change_aspect_liste ),
				     GINT_TO_POINTER ( 5 ));
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
				     GTK_SIGNAL_FUNC ( change_aspect_liste ),
				     GINT_TO_POINTER ( 1 ));


  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_1_lignes ),
				 FALSE );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_2_lignes ),
				 FALSE );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_3_lignes ),
				 FALSE );
  gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_4_lignes ),
				 FALSE );

  switch ( NB_LIGNES_OPE )
    {
    case 1:
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_1_lignes ),
				     TRUE );
      break;
    case 2:
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_2_lignes ),
				     TRUE );
      break;
    case 3:
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_3_lignes ),
				     TRUE );
      break;
    case 4:
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_ope_4_lignes ),
				     TRUE );
      break;
    }

  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_1_lignes),
				       GTK_SIGNAL_FUNC ( change_aspect_liste ),
				       GINT_TO_POINTER ( 1 ));
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_2_lignes ),
				       GTK_SIGNAL_FUNC ( change_aspect_liste ),
				       GINT_TO_POINTER ( 5 ));
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_3_lignes ),
				       GTK_SIGNAL_FUNC ( change_aspect_liste ),
				       GINT_TO_POINTER ( 4 ));
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_ope_4_lignes ),
				       GTK_SIGNAL_FUNC ( change_aspect_liste ),
				       NULL );



  /* on met maintenant le bouton r ou pas r */

  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_enleve_r ),
				     GTK_SIGNAL_FUNC ( change_aspect_liste ),
				     GINT_TO_POINTER ( 3 ));
  gtk_signal_handler_block_by_func ( GTK_OBJECT ( bouton_affiche_r ),
				     GTK_SIGNAL_FUNC ( change_aspect_liste ),
				     GINT_TO_POINTER ( 2 ));

  if ( AFFICHAGE_R )
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_enleve_r ),
				     FALSE );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_r ),
				     TRUE );
    }
  else
    {
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_enleve_r ),
				     TRUE );
      gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON ( bouton_affiche_r ),
				     FALSE );
    }


  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_affiche_r ),
				       GTK_SIGNAL_FUNC ( change_aspect_liste ),
				       GINT_TO_POINTER ( 2 ));
  gtk_signal_handler_unblock_by_func ( GTK_OBJECT ( bouton_enleve_r ),
				       GTK_SIGNAL_FUNC ( change_aspect_liste ),
				       GINT_TO_POINTER ( 3 ));

}
/*******************************************************************************************/
