/*  Fichier qui s'occupe d'afficher les états via une gtktable */
/*      etats_gtktable.c */

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

#include "etats_gtktable.h"

struct struct_etat_affichage gtktable_affichage = {
  gtktable_initialise,
  gtktable_affiche_titre,
  gtktable_affiche_separateur,
  gtktable_affiche_total_categories,
  gtktable_affiche_total_sous_categ,
  gtktable_affiche_total_ib,
  gtktable_affiche_total_sous_ib,
  gtktable_affiche_total_compte,
  gtktable_affiche_total_tiers,
  gtktable_affichage_ligne_ope,
  gtktable_affiche_total_partiel,
  gtktable_affiche_total_general,
  gtktable_affiche_total_periode,
  gtktable_affiche_categ_etat,
  gtktable_affiche_sous_categ_etat,
  gtktable_affiche_ib_etat,
  gtktable_affiche_sous_ib_etat,
  gtktable_affiche_compte_etat,
  gtktable_affiche_tiers_etat,
  gtktable_affiche_titre_revenus_etat,
  gtktable_affiche_titre_depenses_etat,
  gtktable_affiche_totaux_sous_jaccent,
  gtktable_affiche_titres_colonnes,
  gtktable_finish
};

/*****************************************************************************************************/
gint gtktable_initialise (GSList * opes_selectionnees)
{
  /* on peut maintenant créer la table */
  /* pas besoin d'indiquer la hauteur, elle grandit automatiquement */

  table_etat = gtk_table_new ( 0,
			       nb_colonnes,
			       FALSE );
  gtk_table_set_col_spacings ( GTK_TABLE ( table_etat ),
			       5 );
  if ( GTK_BIN ( scrolled_window_etat ) -> child )
    gtk_container_remove ( GTK_CONTAINER ( scrolled_window_etat ),
			   GTK_BIN ( scrolled_window_etat ) -> child );

  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window_etat ),
					  table_etat );
  gtk_widget_show ( table_etat );
  
  return 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gtktable_affiche_titre ( gint ligne )
{
  GtkWidget *label;
  gchar *titre;

  titre = etats_titre () ;

  label = gtk_label_new ( titre );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  return 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gtktable_affiche_separateur ( gint ligne )
{
  GtkWidget * separateur;
  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     separateur,
		     0, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );

  return ligne + 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les catégories sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gtktable_affiche_total_categories ( gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_categ
       &&
       etat_courant -> affiche_sous_total_categ )
    {
      /* si rien n'est affiché en dessous de la catég, on */
      /* met le résultat sur la ligne de la catég */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> afficher_sous_categ
	   ||
	   etat_courant -> utilise_ib
	   ||
	   etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  /* 	  si on affiche les opés, on met les traits entre eux */

	  if ( etat_courant -> afficher_opes
	       &&
	       ligne_debut_partie != -1 )
	    {
	      gint i;
	      gint colonne;

	      colonne = 2;

	      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
		{
		  separateur = gtk_vseparator_new ();
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     separateur,
				     colonne, colonne + 1,
				     ligne_debut_partie, ligne,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( separateur );

		  colonne = colonne + 2;
		}
	      ligne_debut_partie = -1;
	    }


	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols -1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  if ( nom_categ_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( _("Total %s (%d transactions)"),
							  nom_categ_en_cours,
							  nb_ope_categ_etat ));
	      else
		label = gtk_label_new ( g_strconcat ( POSTSPACIFY(_("Total")),
						      nom_categ_en_cours,
						      NULL ));
	    }
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( COLON(_("Category total (%d transactions)")),
							    nb_ope_categ_etat));
	      else
		label = gtk_label_new ( COLON(_("Category total")) );
	    }
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_categ_etat,
						    devise_categ_etat -> code_devise ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   1,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols -1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  if ( etat_courant -> afficher_nb_opes )
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s ( %d opérations )",
						      montant_categ_etat,
						      devise_categ_etat -> code_devise,
						      nb_ope_categ_etat ));
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						      montant_categ_etat,
						      devise_categ_etat -> code_devise ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}

    }

  montant_categ_etat = 0;
  nom_categ_en_cours = NULL;
  titres_affiches = 0;
  nb_ope_categ_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_categ sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gtktable_affiche_total_sous_categ ( gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_categ
       &&
       etat_courant -> afficher_sous_categ
       &&
       etat_courant -> affiche_sous_total_sous_categ )
    {
      /* si rien n'est affiché en dessous de la sous_categ, on */
      /* met le résultat sur la ligne de la ss categ */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> utilise_ib
	   ||
	   etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  /* 	  si on affiche les opés, on met les traits entre eux */

	  if ( etat_courant -> afficher_opes
	       &&
	       ligne_debut_partie != -1 )
	    {
	      gint i;
	      gint colonne;

	      colonne = 2;

	      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
		{
		  separateur = gtk_vseparator_new ();
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     separateur,
				     colonne, colonne + 1,
				     ligne_debut_partie, ligne,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( separateur );

		  colonne = colonne + 2;
		}
	      ligne_debut_partie = -1;
	    }


	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  if ( nom_categ_en_cours
	       &&
	       nom_ss_categ_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new (  g_strdup_printf ( COLON(_("Total %s: %s (%d transactions)")),
							   nom_categ_en_cours,
							   nom_ss_categ_en_cours,
							   nb_ope_sous_categ_etat ));
	      else
		label = gtk_label_new ( g_strdup_printf ( _("Total %s: %s"),
							  nom_categ_en_cours,
							  nom_ss_categ_en_cours ));
	    }
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( COLON(_("Sub-categories total (%d transactions)")),
							  nb_ope_sous_categ_etat ));
	      else
		label = gtk_label_new ( COLON(_("Sub-categories total")) );
	    }
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_sous_categ_etat,
						    devise_categ_etat -> code_devise ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   1,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  if ( etat_courant -> afficher_nb_opes )
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s ( %d opérations )",
						      montant_sous_categ_etat,
						      devise_categ_etat -> code_devise,
						      nb_ope_sous_categ_etat ));
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						      montant_sous_categ_etat,
						      devise_categ_etat -> code_devise ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  montant_sous_categ_etat = 0;
  nom_ss_categ_en_cours = NULL;
  titres_affiches = 0;
  nb_ope_sous_categ_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gtktable_affiche_total_ib ( gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_ib
       &&
       etat_courant -> affiche_sous_total_ib )
    {
      /* si rien n'est affiché en dessous de la ib, on */
      /* met le résultat sur la ligne de l'ib */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> afficher_sous_ib
	   ||
	   etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  /* 	  si on affiche les opés, on met les traits entre eux */

	  if ( etat_courant -> afficher_opes
	       &&
	       ligne_debut_partie != -1 )
	    {
	      gint i;
	      gint colonne;

	      colonne = 2;

	      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
		{
		  separateur = gtk_vseparator_new ();
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     separateur,
				     colonne, colonne + 1,
				     ligne_debut_partie, ligne,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( separateur );

		  colonne = colonne + 2;
		}
	      ligne_debut_partie = -1;
	    }


	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  if ( nom_ib_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( _("Total %s (%d transactions)"),
							  nom_ib_en_cours,
							  nb_ope_ib_etat ));
	      else
		label = gtk_label_new ( g_strconcat ( POSTSPACIFY(_("Total")),
						      nom_ib_en_cours,
						      NULL ));
		}
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( COLON(_("Budgetary lines total: (%d transactions)")),
							  nb_ope_ib_etat ));
	      else
		label = gtk_label_new ( COLON(_("Budgetary lines total")) );
	    }
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_ib_etat,
						    devise_ib_etat -> code_devise ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   1,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  if ( etat_courant -> afficher_nb_opes )
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s ( %d opérations )",
						      montant_ib_etat,
						      devise_ib_etat -> code_devise,
						      nb_ope_ib_etat ));
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						      montant_ib_etat,
						      devise_ib_etat -> code_devise ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  montant_ib_etat = 0;
  nom_ib_en_cours = NULL;
  titres_affiches = 0;
  nb_ope_ib_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gtktable_affiche_total_sous_ib ( gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_ib
       &&
       etat_courant -> afficher_sous_ib
       &&
       etat_courant -> affiche_sous_total_sous_ib )
    {
      /* si rien n'est affiché en dessous de la sous ib, on */
      /* met le résultat sur la ligne de la sous ib */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> regroupe_ope_par_compte
	   ||
	   etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  /* 	  si on affiche les opés, on met les traits entre eux */

	  if ( etat_courant -> afficher_opes
	       &&
	       ligne_debut_partie != -1 )
	    {
	      gint i;
	      gint colonne;

	      colonne = 2;

	      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
		{
		  separateur = gtk_vseparator_new ();
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     separateur,
				     colonne, colonne + 1,
				     ligne_debut_partie, ligne,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( separateur );

		  colonne = colonne + 2;
		}
	      ligne_debut_partie = -1;
	    }


	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  if ( nom_ib_en_cours
	       &&
	       nom_ss_ib_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( COLON(_("Total %s: %s (%d transactions)")),
						      nom_ib_en_cours,
						      nom_ss_ib_en_cours,
						      nb_ope_sous_ib_etat ));
	      else
		label = gtk_label_new ( g_strdup_printf ( _("Total %s: %s"),
						      nom_ib_en_cours, nom_ss_ib_en_cours ));
	    }
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( COLON(_("Sub-budgetary lines total: (%d transactions)")),
							  nb_ope_sous_ib_etat ));
	      else
		label = gtk_label_new ( COLON(_("Sub-budgetary lines total")) );
	    }
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_sous_ib_etat,
						    devise_ib_etat -> code_devise ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   1,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  if ( etat_courant -> afficher_nb_opes )
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s ( %d opérations )",
						      montant_sous_ib_etat,
						      devise_ib_etat -> code_devise,
						      nb_ope_sous_ib_etat ));
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						      montant_sous_ib_etat,
						      devise_ib_etat -> code_devise ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  montant_sous_ib_etat = 0;
  nom_ss_ib_en_cours = NULL;
  titres_affiches = 0;
  nb_ope_sous_ib_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les compte sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gtktable_affiche_total_compte ( gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> regroupe_ope_par_compte
       &&
       etat_courant -> affiche_sous_total_compte )
    {
      /* si rien n'est affiché en dessous du compte, on */
      /* met le résultat sur la ligne du compte */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> utilise_tiers
	   ||
	   etat_courant -> afficher_opes )
	{
	  /* 	  si on affiche les opés, on met les traits entre eux */

	  if ( etat_courant -> afficher_opes
	       &&
	       ligne_debut_partie != -1 )
	    {
	      gint i;
	      gint colonne;

	      colonne = 2;

	      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
		{
		  separateur = gtk_vseparator_new ();
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     separateur,
				     colonne, colonne + 1,
				     ligne_debut_partie, ligne,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( separateur );

		  colonne = colonne + 2;
		}
	      ligne_debut_partie = -1;
	    }


	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  if ( nom_compte_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( _("Total %s (%d transactions)"),
						      nom_compte_en_cours,
						      nb_ope_compte_etat ));
	      else
		label = gtk_label_new ( g_strconcat ( POSTSPACIFY(_("Total")),
						      nom_compte_en_cours,
						      NULL ));
	    }
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( COLON(_("Account total: (%d transactions)")),
							    nb_ope_compte_etat ) );
	      else
		label = gtk_label_new ( COLON(_("Account total")) );
	    }
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_compte_etat,
						    devise_compte_en_cours_etat -> code_devise ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   1,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  if ( etat_courant -> afficher_nb_opes )
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s ( %d opérations )",
						      montant_compte_etat,
						      devise_compte_en_cours_etat -> code_devise,
						      nb_ope_compte_etat ));
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						      montant_compte_etat,
						      devise_compte_en_cours_etat -> code_devise ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  montant_compte_etat = 0;
  nom_compte_en_cours = NULL;
  titres_affiches = 0;
  nb_ope_compte_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les tiers sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gtktable_affiche_total_tiers ( gint ligne )
{
  GtkWidget *separateur;
  GtkWidget *label;

  if ( etat_courant -> utilise_tiers
       &&
       etat_courant -> affiche_sous_total_tiers )
    {
      /* si rien n'est affiché en dessous du tiers, on */
      /* met le résultat sur la ligne du tiers */
      /* sinon on fait une barre et on met le résultat */

      if ( etat_courant -> afficher_opes )
	{
	  /* 	  si on affiche les opés, on met les traits entre eux */

	  if ( etat_courant -> afficher_opes
	       &&
	       ligne_debut_partie != -1 )
	    {
	      gint i;
	      gint colonne;

	      colonne = 2;

	      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
		{
		  separateur = gtk_vseparator_new ();
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     separateur,
				     colonne, colonne + 1,
				     ligne_debut_partie, ligne,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( separateur );

		  colonne = colonne + 2;
		}
	      ligne_debut_partie = -1;
	    }


	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  separateur = gtk_hseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  ligne++;

	  if ( nom_tiers_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( _("Total %s (%d transactions)"),
							  nom_tiers_en_cours,
							  nb_ope_tiers_etat ));
	      else
		label = gtk_label_new ( g_strdup_printf ( _("Total %s"),
							  nom_tiers_en_cours ));
	    }
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		label = gtk_label_new ( g_strdup_printf ( COLON(_("Third party total: (%d transactions)")),
							  nb_ope_tiers_etat ));
	      else
		label = gtk_label_new ( COLON(_("Third party total")) );
	    }
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						    montant_tiers_etat,
						    devise_tiers_etat -> code_devise ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   1,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
      else
	{
	  ligne--;

	  if ( etat_courant -> afficher_nb_opes )
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s ( %d opérations )",
						      montant_tiers_etat,
						      devise_tiers_etat -> code_devise,
						      nb_ope_tiers_etat ));
	  else
	    label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						      montant_tiers_etat,
						      devise_tiers_etat -> code_devise ));
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
    }

  montant_tiers_etat = 0;
  nom_tiers_en_cours = NULL;
  titres_affiches = 0;
  nb_ope_tiers_etat = 0;

  return (ligne );
}
/*****************************************************************************************************/






/*****************************************************************************************************/
/* affiche le total de la période à l'endroit donné de la table */
/* retourne le ligne suivante de la table */
/* si force = 0, vérifie les dates et affiche si nécessaire */
/*   si force = 1, affiche le total (chgt de categ, ib ...) */
/*****************************************************************************************************/
gint gtktable_affiche_total_periode ( struct structure_operation *operation,
				      gint ligne,
				      gint force )
{
  GtkWidget *label;
  GtkWidget *separateur;

  if ( etat_courant -> separation_par_plage )
    {
      gchar *message;

      message = NULL;

      /* si la date de début de période est nulle, on la met au début de la période la date de l'opération */

      if ( !date_debut_periode )
	{
	  if ( operation )
	    {
	      /*  il y a une opération, on va rechercher le début de la période qui la contient */
	      /* ça peut être le début de la semaine, du mois ou de l'année de l'opé */

	      switch ( etat_courant -> type_separation_plage )
		{
		case 0:
		  /* séparation par semaine : on recherche le début de la semaine qui contient l'opé */

		  date_debut_periode = g_date_new_dmy ( g_date_day ( operation -> date ),
							g_date_month ( operation -> date ),
							g_date_year ( operation -> date ));

		  if ( g_date_weekday ( date_debut_periode )  != (etat_courant -> jour_debut_semaine + 1 ))
		    {
		      if ( g_date_weekday ( date_debut_periode ) < (etat_courant -> jour_debut_semaine + 1 ))
			g_date_subtract_days ( date_debut_periode,
					       g_date_weekday ( date_debut_periode ) + etat_courant -> jour_debut_semaine - 2 );
		      else
			g_date_subtract_days ( date_debut_periode,
					       g_date_weekday ( date_debut_periode ) - etat_courant -> jour_debut_semaine - 1 );
		    }
		  break;

		case 1:
		  /* séparation par mois */

		  date_debut_periode = g_date_new_dmy ( 1,
							g_date_month ( operation -> date ),
							g_date_year ( operation -> date ));
		  break;

		case 2:
		  /* séparation par an */

		  date_debut_periode = g_date_new_dmy ( 1,
							1,
							g_date_year ( operation -> date ));
		  break;
		}
	    }
	  else
	    date_debut_periode = NULL;
	  return ( ligne );
	}

      /* on vérifie maintenant s'il faut afficher un total ou pas */

      switch ( etat_courant -> type_separation_plage )
	{
	  gchar buffer[30];
	  GDate *date_tmp;

	case 0:
	  /* séparation par semaine */

	  /* 	  si c'est le même jour que l'opé précédente, on fait rien */

	  if ( !force
	       &&
	       !g_date_compare ( operation -> date,
				date_debut_periode ))
	    return ( ligne );

	  /* 	  si on est en début de semaine, on met un sous total */

	  date_tmp = g_date_new_dmy ( g_date_day ( date_debut_periode ),
				      g_date_month ( date_debut_periode ),
				      g_date_year ( date_debut_periode ));
	  g_date_add_days ( date_tmp,
			    7 );

	  if ( !force
	       &&
	       ( g_date_weekday ( operation -> date )  != (etat_courant -> jour_debut_semaine + 1 )
		 &&
		 g_date_compare ( operation -> date,
				  date_tmp ) < 0 ))
	    return ( ligne );

	  /* on doit retrouver la date du début de semaine et y ajouter 6j pour afficher la période */

	  if ( g_date_weekday ( date_debut_periode )  != (etat_courant -> jour_debut_semaine + 1 ))
	    {
	      if ( g_date_weekday ( date_debut_periode ) < (etat_courant -> jour_debut_semaine + 1 ))
		g_date_subtract_days ( date_debut_periode,
				       g_date_weekday ( date_debut_periode ) + etat_courant -> jour_debut_semaine - 2 );
	      else
		g_date_subtract_days ( date_debut_periode,
				       g_date_weekday ( date_debut_periode ) - etat_courant -> jour_debut_semaine - 1 );
	    }


	  g_date_free ( date_tmp );

	  date_tmp = g_date_new_dmy ( g_date_day ( date_debut_periode ),
				      g_date_month ( date_debut_periode ),
				      g_date_year ( date_debut_periode ));
	  g_date_add_days ( date_tmp,
			    6 );
	  if ( etat_courant -> afficher_nb_opes )
	    message = g_strdup_printf ( _("Result from %02d/%02d/%04d to %02d/%02d/%04d (%d transactions):"),
					g_date_day ( date_debut_periode ),
					g_date_month ( date_debut_periode ),
					g_date_year ( date_debut_periode ),
					g_date_day ( date_tmp ),
					g_date_month ( date_tmp ),
					g_date_year ( date_tmp ),
					nb_ope_periode_etat );
	  else
	    message = g_strdup_printf ( _("Result from %02d/%02d/%04d to %02d/%02d/%04d:"),
					g_date_day ( date_debut_periode ),
					g_date_month ( date_debut_periode ),
					g_date_year ( date_debut_periode ),
					g_date_day ( date_tmp ),
					g_date_month ( date_tmp ),
					g_date_year ( date_tmp ));

	  break;

	case 1:
	  /* séparation par mois */

	  if ( !force
	       &&
	       operation -> mois == g_date_month ( date_debut_periode ) )
	    return ( ligne );

	  g_date_strftime ( buffer,
			    29,
			    "%B %Y",
			    date_debut_periode );
			    
	  if ( etat_courant -> afficher_nb_opes )
	    message = g_strdup_printf ( COLON(_("Result of %s (%d transactions)")),
					buffer, nb_ope_periode_etat );
	  else
	    message = g_strconcat ( COLON(_("Result of %s")),
				    buffer );
				      
	  break;

	case 2:
	  /* séparation par an */

	  if ( !force
	       &&
	       operation -> annee == g_date_year ( date_debut_periode ) )
	    return ( ligne );

	  g_date_strftime ( buffer,
			    29,
			    "%Y",
			    date_debut_periode );
			    
	  if ( etat_courant -> afficher_nb_opes )
	    message = g_strdup_printf ( COLON(_("Result of the year%s (%d transactions)")),
				    buffer, nb_ope_periode_etat );
	  else
	    message = g_strdup_printf ( COLON(_("Result of the year%s")),
					buffer );
	  break;
	}

      /*       si on arrive ici, c'est qu'il y a un chgt de période ou que c'est forcé */
      /* 	  si on affiche les opés, on met les traits entre eux */

      if ( etat_courant -> afficher_opes
	   &&
	   ligne_debut_partie != -1 )
	{
	  gint i;
	  gint colonne;

	  colonne = 2;

	  for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
	    {
	      separateur = gtk_vseparator_new ();
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 separateur,
				 colonne, colonne + 1,
				 ligne_debut_partie, ligne,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( separateur );

	      colonne = colonne + 2;
	    }
	  ligne_debut_partie = -1;
	}


      label = gtk_label_new ( "" );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, GTK_TABLE ( table_etat ) -> ncols - 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;

      separateur = gtk_hseparator_new ();
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 separateur,
			 1, GTK_TABLE ( table_etat ) -> ncols,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( separateur );

      ligne++;

      label = gtk_label_new ( message );
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       0,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 1, GTK_TABLE ( table_etat ) -> ncols - 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
						montant_periode_etat,
						devise_generale_etat -> code_devise ));
      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       1,
			       0.5 );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 GTK_TABLE ( table_etat ) -> ncols - 1, GTK_TABLE ( table_etat ) -> ncols,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      ligne++;

	  label = gtk_label_new ( "" );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     1, GTK_TABLE ( table_etat ) -> ncols - 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;


      montant_periode_etat = 0;
      nb_ope_periode_etat = 0;

      /* comme il y a un changement de période, on remet la date_debut_periode à celle du début de la période */
      /* de l'opération  en cours */


	  if ( operation )
	    {
	      /*  il y a une opération, on va rechercher le début de la période qui la contient */
	      /* ça peut être le début de la semaine, du mois ou de l'année de l'opé */

	      switch ( etat_courant -> type_separation_plage )
		{
		case 0:
		  /* séparation par semaine : on recherche le début de la semaine qui contient l'opé */

		  date_debut_periode = g_date_new_dmy ( g_date_day ( operation -> date ),
							g_date_month ( operation -> date ),
							g_date_year ( operation -> date ));

		  if ( g_date_weekday ( date_debut_periode )  != (etat_courant -> jour_debut_semaine + 1 ))
		    {
		      if ( g_date_weekday ( date_debut_periode ) < (etat_courant -> jour_debut_semaine + 1 ))
			g_date_subtract_days ( date_debut_periode,
					       g_date_weekday ( date_debut_periode ) + etat_courant -> jour_debut_semaine - 2 );
		      else
			g_date_subtract_days ( date_debut_periode,
					       g_date_weekday ( date_debut_periode ) - etat_courant -> jour_debut_semaine - 1 );
		    }
		  break;

		case 1:
		  /* séparation par mois */

		  date_debut_periode = g_date_new_dmy ( 1,
							g_date_month ( operation -> date ),
							g_date_year ( operation -> date ));
		  break;

		case 2:
		  /* séparation par an */

		  date_debut_periode = g_date_new_dmy ( 1,
							1,
							g_date_year ( operation -> date ));
		  break;
		}
	    }
	  else
	    date_debut_periode = NULL;
    }

  return (ligne );

}
/*****************************************************************************************************/

/*****************************************************************************************************/
gint gtktable_affichage_ligne_ope ( struct structure_operation *operation,
				    gint ligne )
{
  gint colonne;
  GtkWidget *label;

  /* on met tous les labels dans un event_box pour aller directement sur l'opé si elle est clickée */


  if ( etat_courant -> afficher_opes )
    {
      /* on affiche ce qui est demandé pour les opés */


      /* si les titres ne sont pas affichés et qu'il faut le faire, c'est ici */

      if ( !titres_affiches
	   &&
	   etat_courant -> afficher_titre_colonnes
	   &&
	   etat_courant -> type_affichage_titres )
	ligne = gtktable_affichage . affiche_titres_colonnes ( ligne );

      colonne = 1;

      /*       pour chaque info, on vérifie si on l'opé doit être clickable */
      /* si c'est le cas, on place le label dans un event_box */

      if ( etat_courant -> afficher_no_ope )
	{
	  label = gtk_label_new ( itoa ( operation -> no_operation ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );

	  if ( etat_courant -> ope_clickables )
	    {
	      GtkWidget *event_box;

	      event_box = gtk_event_box_new ();
	      gtk_signal_connect ( GTK_OBJECT ( event_box ),
				   "enter_notify_event",
				   GTK_SIGNAL_FUNC ( met_en_prelight ),
				   NULL );
	      gtk_signal_connect ( GTK_OBJECT ( event_box ),
				   "leave_notify_event",
				   GTK_SIGNAL_FUNC ( met_en_normal ),
				   NULL );
	      gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					  "button_press_event",
					  GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					  (GtkObject *) operation );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 event_box,
				 colonne, colonne + 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( event_box );

	      gtk_widget_set_style ( label,
				     style_label_nom_compte );
	      gtk_container_add ( GTK_CONTAINER ( event_box ),
				  label );
	    }
	  else
	    gtk_table_attach ( GTK_TABLE ( table_etat ),
			       label,
			       colonne, colonne + 1,
			       ligne, ligne + 1,
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
			       0, 0 );

	  gtk_widget_show ( label );

	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_date_ope )
	{
	  label = gtk_label_new ( g_strdup_printf  ( "%.2d/%.2d/%d",
						     operation -> jour,
						     operation -> mois,
						     operation -> annee ));
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );

	  if ( etat_courant -> ope_clickables )
	    {
	      GtkWidget *event_box;

	      event_box = gtk_event_box_new ();
	      gtk_signal_connect ( GTK_OBJECT ( event_box ),
				   "enter_notify_event",
				   GTK_SIGNAL_FUNC ( met_en_prelight ),
				   NULL );
	      gtk_signal_connect ( GTK_OBJECT ( event_box ),
				   "leave_notify_event",
				   GTK_SIGNAL_FUNC ( met_en_normal ),
				   NULL );
	      gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					  "button_press_event",
					  GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					  (GtkObject *) operation );
	      gtk_table_attach ( GTK_TABLE ( table_etat ),
				 event_box,
				 colonne, colonne + 1,
				 ligne, ligne + 1,
				 GTK_SHRINK | GTK_FILL,
				 GTK_SHRINK | GTK_FILL,
				 0, 0 );
	      gtk_widget_show ( event_box );

	      gtk_widget_set_style ( label,
				     style_label_nom_compte );
	      gtk_container_add ( GTK_CONTAINER ( event_box ),
				  label );
	    }
	  else
	    gtk_table_attach ( GTK_TABLE ( table_etat ),
			       label,
			       colonne, colonne + 1,
			       ligne, ligne + 1,
			       GTK_SHRINK | GTK_FILL,
			       GTK_SHRINK | GTK_FILL,
			       0, 0 );


	  gtk_widget_show ( label );

	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_exo_ope )
	{
	  if ( operation -> no_exercice )
	    {
	      label = gtk_label_new ( ((struct struct_exercice *)(g_slist_find_custom ( liste_struct_exercices,
											GINT_TO_POINTER ( operation -> no_exercice ),
											(GCompareFunc) recherche_exercice_par_no )->data)) -> nom_exercice );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_tiers_ope )
	{
	  if ( operation -> tiers )
	    {
	      label = gtk_label_new ( ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
										     GINT_TO_POINTER ( operation -> tiers ),
										     (GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers );
		      
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }

	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_categ_ope )
	{
	  gchar *pointeur;

	  pointeur = NULL;

	  if ( operation -> categorie )
	    {
	      struct struct_categ *categ;

	      categ = g_slist_find_custom ( liste_struct_categories,
					    GINT_TO_POINTER ( operation -> categorie ),
					    (GCompareFunc) recherche_categorie_par_no ) -> data;
	      pointeur = categ -> nom_categ;

	      if ( operation -> sous_categorie
		   &&
		   etat_courant -> afficher_sous_categ_ope )
		pointeur = g_strconcat ( pointeur,
					 " : ",
					 ((struct struct_sous_categ *)(g_slist_find_custom ( categ -> liste_sous_categ,
											     GINT_TO_POINTER ( operation -> sous_categorie ),
											     (GCompareFunc) recherche_sous_categorie_par_no ) -> data )) -> nom_sous_categ,
					 NULL );
	    }
	  else
	    {
	      /* si c'est un virement, on le marque, sinon c'est qu'il n'y a pas de categ */
	      /* ou que c'est une opé ventilée, et on marque rien */

	      if ( operation -> relation_no_operation )
		{
		  /* c'est un virement */

		  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> relation_no_compte;

		  if ( operation -> montant < 0 )
		    pointeur = g_strdup_printf ( _("Transfer to %s"),
						 NOM_DU_COMPTE );
		  else
		    pointeur = g_strdup_printf ( _("Transfer from %s"),
						 NOM_DU_COMPTE );
		}
	    }

	  if ( pointeur )
	    {
	      label = gtk_label_new ( pointeur );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}
		


      if ( etat_courant -> afficher_ib_ope )
	{
	  if ( operation -> imputation )
	    {
	      struct struct_imputation *ib;
	      gchar *pointeur;

	      ib = g_slist_find_custom ( liste_struct_imputation,
					 GINT_TO_POINTER ( operation -> imputation ),
					 (GCompareFunc) recherche_imputation_par_no ) -> data;
	      pointeur = ib -> nom_imputation;

	      if ( operation -> sous_imputation
		   &&
		   etat_courant -> afficher_sous_ib_ope )
		pointeur = g_strconcat ( pointeur,
					 " : ",
					 ((struct struct_sous_imputation *)(g_slist_find_custom ( ib -> liste_sous_imputation,
												  GINT_TO_POINTER ( operation -> sous_imputation ),
												  (GCompareFunc) recherche_sous_imputation_par_no ) -> data )) -> nom_sous_imputation,
					 NULL );

	      label = gtk_label_new ( pointeur );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_notes_ope )
	{
	  if ( operation -> notes )
	    {
	      label = gtk_label_new ( operation -> notes );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_type_ope )
	{
	  GSList *pointeur;

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	  pointeur = g_slist_find_custom ( TYPES_OPES,
					   GINT_TO_POINTER ( operation -> type_ope ),
					   (GCompareFunc) recherche_type_ope_par_no );

	  if ( pointeur )
	    {
	      struct struct_type_ope *type;

	      type = pointeur -> data;

	      label = gtk_label_new ( type -> nom_type );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_cheque_ope )
	{
	  if ( operation -> contenu_type )
	    {
	      label = gtk_label_new ( operation -> contenu_type );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_pc_ope )
	{
	  if ( operation -> no_piece_comptable )
	    {
	      label = gtk_label_new ( operation -> no_piece_comptable );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}



      if ( etat_courant -> afficher_infobd_ope )
	{
	  if ( operation -> info_banque_guichet )
	    {
	      label = gtk_label_new ( operation -> info_banque_guichet );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_rappr_ope )
	{
	  GSList *pointeur;

	  pointeur = g_slist_find_custom ( liste_no_rapprochements,
					   GINT_TO_POINTER ( operation -> no_rapprochement ),
					   (GCompareFunc) recherche_no_rapprochement_par_no );

	  if ( pointeur )
	    {
	      struct struct_no_rapprochement *rapprochement;

	      rapprochement = pointeur -> data;

	      label = gtk_label_new ( rapprochement -> nom_rapprochement );
	      gtk_misc_set_alignment ( GTK_MISC ( label ),
				       0,
				       0.5 );

	      if ( etat_courant -> ope_clickables )
		{
		  GtkWidget *event_box;

		  event_box = gtk_event_box_new ();
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "enter_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_prelight ),
				       NULL );
		  gtk_signal_connect ( GTK_OBJECT ( event_box ),
				       "leave_notify_event",
				       GTK_SIGNAL_FUNC ( met_en_normal ),
				       NULL );
		  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
					      "button_press_event",
					      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
					      (GtkObject *) operation );
		  gtk_table_attach ( GTK_TABLE ( table_etat ),
				     event_box,
				     colonne, colonne + 1,
				     ligne, ligne + 1,
				     GTK_SHRINK | GTK_FILL,
				     GTK_SHRINK | GTK_FILL,
				     0, 0 );
		  gtk_widget_show ( event_box );

		  gtk_widget_set_style ( label,
					 style_label_nom_compte );
		  gtk_container_add ( GTK_CONTAINER ( event_box ),
				      label );
		}
	      else
		gtk_table_attach ( GTK_TABLE ( table_etat ),
				   label,
				   colonne, colonne + 1,
				   ligne, ligne + 1,
				   GTK_SHRINK | GTK_FILL,
				   GTK_SHRINK | GTK_FILL,
				   0, 0 );

	      gtk_widget_show ( label );
	    }
	  colonne = colonne + 2;
	}



      /* on affiche le montant au bout de la ligne */

      if ( devise_compte_en_cours_etat
	   &&
	   operation -> devise == devise_compte_en_cours_etat -> no_devise )
	label = gtk_label_new ( g_strdup_printf  ("%4.2f %s",
						  operation -> montant,
						  devise_compte_en_cours_etat -> code_devise ));
      else
	{
	  struct struct_devise *devise_operation;

	  devise_operation = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( operation -> devise ),
						   ( GCompareFunc ) recherche_devise_par_no ) -> data;
	  label = gtk_label_new ( g_strdup_printf  ("%4.2f %s",
						    operation -> montant,
						    devise_operation -> code_devise ));
	}

      gtk_misc_set_alignment ( GTK_MISC ( label ),
			       1,
			       0.5 );

      if ( etat_courant -> ope_clickables )
	{
	  GtkWidget *event_box;

	  event_box = gtk_event_box_new ();
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "enter_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_prelight ),
			       NULL );
	  gtk_signal_connect ( GTK_OBJECT ( event_box ),
			       "leave_notify_event",
			       GTK_SIGNAL_FUNC ( met_en_normal ),
			       NULL );
	  gtk_signal_connect_object ( GTK_OBJECT ( event_box ),
				      "button_press_event",
				      GTK_SIGNAL_FUNC ( gtktable_click_sur_ope_etat ),
				      (GtkObject *) operation );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     event_box,
			     GTK_TABLE ( table_etat ) -> ncols - 1, GTK_TABLE ( table_etat ) -> ncols,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( event_box );

	  gtk_widget_set_style ( label,
				 style_label_nom_compte );
	  gtk_container_add ( GTK_CONTAINER ( event_box ),
			      label );
	}
      else
	gtk_table_attach ( GTK_TABLE ( table_etat ),
			   label,
			   GTK_TABLE ( table_etat ) -> ncols - 1, GTK_TABLE ( table_etat ) -> ncols,
			   ligne, ligne + 1,
			   GTK_SHRINK | GTK_FILL,
			   GTK_SHRINK | GTK_FILL,
			   0, 0 );

      gtk_widget_show ( label );

      if ( ligne_debut_partie == -1 )
	ligne_debut_partie = ligne;

      ligne++;
    }
  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gtktable_affiche_total_partiel ( gdouble total_partie,
				      gint ligne,
				      gint type )
{
  GtkWidget *label;
  GtkWidget *separateur;

  /* 	  si on affiche les opés, on met les traits entre eux */

  if ( etat_courant -> afficher_opes
       &&
       ligne_debut_partie != -1 )
    {
      gint i;
      gint colonne;

      colonne = 2;

      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
	{
	  separateur = gtk_vseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     colonne, colonne + 1,
			     ligne_debut_partie, ligne,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  colonne = colonne + 2;
	}
      ligne_debut_partie = -1;
    }


  label = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     1, GTK_TABLE ( table_etat ) -> ncols - 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     separateur,
		     0, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );

  ligne++;

  if ( type )
    {
      if ( etat_courant -> afficher_nb_opes )
	label = gtk_label_new ( g_strdup_printf ( COLON(_("Outgoings total (%d transactions)")),
						  nb_ope_partie_etat ));
      else
	label = gtk_label_new ( COLON(_("Outgoing total")) );
    }
  else
    {
      if ( etat_courant -> afficher_nb_opes )
	label = gtk_label_new ( g_strdup_printf ( COLON(_("Incomes total (%d transactions)")),
						  nb_ope_partie_etat ));
      else
	label = gtk_label_new ( COLON(_("Income total")) );
    }
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     1, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
					    total_partie,
					    devise_generale_etat -> code_devise ));
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   1,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     GTK_TABLE ( table_etat ) -> ncols - 1, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     separateur,
		     0, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );

  ligne++;

  label = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     1, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  nom_categ_en_cours = NULL;
  nom_ss_categ_en_cours = NULL;
  nom_ib_en_cours = NULL;
  nom_ss_ib_en_cours = NULL;
  nom_compte_en_cours = NULL;
  nom_tiers_en_cours = NULL;


  return ( ligne );
}
/*****************************************************************************************************/

/*****************************************************************************************************/
gint gtktable_affiche_total_general ( gdouble total_general,
				      gint ligne )
{
  GtkWidget *label;
  GtkWidget *separateur;

  /* 	  si on affiche les opés, on met les traits entre eux */

  if ( etat_courant -> afficher_opes
       &&
       ligne_debut_partie != -1 )
    {
      gint i;
      gint colonne;

      colonne = 2;

      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ )
	{
	  separateur = gtk_vseparator_new ();
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     separateur,
			     colonne, colonne + 1,
			     ligne_debut_partie, ligne,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( separateur );

	  colonne = colonne + 2;
	}
      ligne_debut_partie = -1;
    }

  label = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     1, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     separateur,
		     0, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );

  ligne++;

  if ( etat_courant -> afficher_nb_opes )
    label = gtk_label_new ( g_strdup_printf ( COLON(_("General total (%d transactions)")),
					      nb_ope_general_etat ));
  else
    label = gtk_label_new ( COLON(_("General total")) );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     1, GTK_TABLE ( table_etat ) -> ncols -1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  label = gtk_label_new ( g_strdup_printf ( "%4.2f %s",
					    total_general,
					    devise_generale_etat -> code_devise ));
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   1,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     GTK_TABLE ( table_etat ) -> ncols -1, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     separateur,
		     0, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );

  ligne++;

  label = gtk_label_new ( "" );
  gtk_misc_set_alignment ( GTK_MISC ( label ),
			   0,
			   0.5 );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     1, GTK_TABLE ( table_etat ) -> ncols,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gtktable_affiche_categ_etat ( struct structure_operation *operation,
				   gchar *decalage_categ,
				   gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* vérifie qu'il y a un changement de catégorie */
  /* ça peut être aussi chgt pour virement, ventilation ou pas de categ */

  if ( etat_courant -> utilise_categ
       &&
       ( operation -> categorie != ancienne_categ_etat
	 ||
	 ( ancienne_categ_speciale_etat == 1
	   &&
	   !operation -> relation_no_operation )
	 ||
	 ( ancienne_categ_speciale_etat == 2
	   &&
	   !operation -> operation_ventilee )
	 ||
	 ( ancienne_categ_speciale_etat == 3
	   &&
	   ( operation -> operation_ventilee
	     ||
	     operation -> relation_no_operation ))))
    {

      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on affiche le total de la période en le forçant */

	  ligne = gtktable_affichage . affiche_total_periode ( operation,
							       ligne,
							       1 );

	  /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

	  ligne = gtktable_affichage . affiche_totaux_sous_jaccent ( 1,
								     ligne );

	  /* on ajoute le total de la categ */

	  ligne = gtktable_affichage . affiche_total_categories ( ligne );
	}

      /*       si on a demandé de ne pas afficher les noms des catég, on saute la partie suivante */

      if ( etat_courant -> afficher_nom_categ )
	{
	  if ( operation -> categorie )
	    {
	      nom_categ_en_cours = ((struct struct_categ *)(g_slist_find_custom ( liste_struct_categories,
										  GINT_TO_POINTER ( operation -> categorie ),
										  (GCompareFunc) recherche_categorie_par_no ) -> data )) -> nom_categ;
	      pointeur_char = g_strconcat ( decalage_categ,
					    nom_categ_en_cours,
					    NULL );
	      ancienne_categ_speciale_etat = 0;
	    }
	  else
	    {
	      if ( operation -> relation_no_operation )
		{
		  pointeur_char = g_strconcat ( decalage_categ,
						_("Transfers"),
						NULL );
		  ancienne_categ_speciale_etat = 1;
		}
	      else
		{
		  if ( operation -> operation_ventilee )
		    {
		      pointeur_char = g_strconcat ( decalage_categ,
						    _("Breakdown of transaction"),
						    NULL );
		      ancienne_categ_speciale_etat = 2;
		    }
		  else
		    {
		      pointeur_char = g_strconcat ( decalage_categ,
						    _("No category"),
						    NULL );
		      ancienne_categ_speciale_etat = 3;
		    }
		}
	    }

	  label = gtk_label_new ( pointeur_char );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     0, 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}

      ligne_debut_partie = ligne;
      denote_struct_sous_jaccentes ( 1 );

      ancienne_categ_etat = operation -> categorie;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gtktable_affiche_sous_categ_etat ( struct structure_operation *operation,
			       			       gchar *decalage_sous_categ,
					gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  if ( etat_courant -> utilise_categ
       &&
       etat_courant -> afficher_sous_categ
       &&
       operation -> categorie
       &&
       operation -> sous_categorie != ancienne_sous_categ_etat )
    {
      struct struct_categ *categ;

     /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on affiche le total de la période en le forçant */

	  ligne = gtktable_affichage . affiche_total_periode ( operation,
						   ligne,
						   1 );

	  /* on ajoute les totaux de tout ce qu'il y a derrière la sous catégorie */

	  ligne = gtktable_affichage . affiche_totaux_sous_jaccent ( 2,
	ligne );

	  /* on ajoute le total de la sous categ */

	  ligne = gtktable_affichage . affiche_total_sous_categ ( ligne );
	}

      /*       si on a demandé de ne pas afficher les noms des ss-catég, on saute la partie suivante */

      if ( etat_courant -> afficher_nom_categ )
	{
	  categ = g_slist_find_custom ( liste_struct_categories,
					GINT_TO_POINTER ( operation -> categorie ),
					(GCompareFunc) recherche_categorie_par_no ) -> data;

	  if ( operation -> sous_categorie )
	    {
	      nom_ss_categ_en_cours = ((struct struct_sous_categ *)(g_slist_find_custom ( categ->liste_sous_categ,
											  GINT_TO_POINTER ( operation -> sous_categorie ),
											  (GCompareFunc) recherche_sous_categorie_par_no ) -> data )) -> nom_sous_categ;
	      pointeur_char = g_strconcat ( decalage_sous_categ,
					    nom_ss_categ_en_cours,
					    NULL );
	    }
	  else
	    {
	      if ( etat_courant -> afficher_pas_de_sous_categ )
		pointeur_char = g_strconcat ( decalage_sous_categ,
					      _("No subcategory"),
					      NULL );
	      else
		pointeur_char = "";
	    }

	  label = gtk_label_new ( pointeur_char );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     0,1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}
 
     ligne_debut_partie = ligne;
     denote_struct_sous_jaccentes ( 2 );

     ancienne_sous_categ_etat = operation -> sous_categorie;

     debut_affichage_etat = 0;
     changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gtktable_affiche_ib_etat ( struct structure_operation *operation,
		       		       gchar *decalage_ib,
		       gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* mise en place de l'ib */


  if ( etat_courant -> utilise_ib
       &&
       operation -> imputation != ancienne_ib_etat )
    {
      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on affiche le total de la période en le forçant */

	  ligne = gtktable_affichage . affiche_total_periode ( operation,
						   ligne,
						   1 );

	  /* on ajoute les totaux de tout ce qu'il y a derrière l'ib */

	  ligne = gtktable_affichage . affiche_totaux_sous_jaccent ( 3,
	ligne );

	  /* on ajoute le total de l'ib */

	  ligne = gtktable_affichage . affiche_total_ib ( ligne );
	}
 
      /*       si on a demandé de ne pas afficher les noms des ib, on saute la partie suivante */

      if ( etat_courant -> afficher_nom_ib )
	{
	  if ( operation -> imputation )
	    {
	      nom_ib_en_cours = ((struct struct_imputation *)(g_slist_find_custom ( liste_struct_imputation,
										    GINT_TO_POINTER ( operation -> imputation ),
										    (GCompareFunc) recherche_imputation_par_no ) -> data )) -> nom_imputation;
	      pointeur_char = g_strconcat ( decalage_ib,
					    nom_ib_en_cours,
					    NULL );
	    }
	  else
	    pointeur_char = g_strconcat ( decalage_ib,
					  _("No budgetary line"),
					  NULL );

	  label = gtk_label_new ( pointeur_char );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     0, 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}

      ligne_debut_partie = ligne;
      denote_struct_sous_jaccentes ( 3 );

      ancienne_ib_etat = operation -> imputation;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gtktable_affiche_sous_ib_etat ( struct structure_operation *operation,
			    			    gchar *decalage_sous_ib,
			    gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;


  /* mise en place de la sous_ib */


  if ( etat_courant -> utilise_ib
       &&
       etat_courant -> afficher_sous_ib
       &&
       operation -> imputation
       &&
       operation -> sous_imputation != ancienne_sous_ib_etat )
    {
      struct struct_imputation *imputation;

      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on affiche le total de la période en le forçant */

	  ligne = gtktable_affichage . affiche_total_periode ( operation,
						   ligne,
						   1 );

	  /* on ajoute les totaux de tout ce qu'il y a derrière la sous ib */

	  ligne = gtktable_affichage . affiche_totaux_sous_jaccent ( 4,
	ligne );

	  /* on ajoute le total de la sous ib */

	  ligne = gtktable_affichage . affiche_total_sous_ib ( ligne );
	}
 
      /*       si on a demandé de ne pas afficher les noms des ss-ib, on saute la partie suivante */

      if ( etat_courant -> afficher_nom_ib )
	{
	  imputation = g_slist_find_custom ( liste_struct_imputation,
					     GINT_TO_POINTER ( operation -> imputation ),
					     (GCompareFunc) recherche_imputation_par_no ) -> data;

	  if ( operation -> sous_imputation )
	    {
	      nom_ss_ib_en_cours = ((struct struct_sous_imputation *)(g_slist_find_custom ( imputation->liste_sous_imputation,
											    GINT_TO_POINTER ( operation -> sous_imputation ),
											    (GCompareFunc) recherche_sous_imputation_par_no ) -> data )) -> nom_sous_imputation;
	      pointeur_char = g_strconcat ( decalage_sous_ib,
					    nom_ss_ib_en_cours,
					    NULL );
	    }
	  else
	    {
	      if ( etat_courant -> afficher_pas_de_sous_ib )
		pointeur_char = g_strconcat ( decalage_sous_ib,
					      _("No sub-budgetary line"),
					      NULL );
	      else
		pointeur_char = "";
	    }

	  label = gtk_label_new ( pointeur_char );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     0, 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}

      ligne_debut_partie = ligne;
      denote_struct_sous_jaccentes ( 4 );

      ancienne_sous_ib_etat = operation -> sous_imputation;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gtktable_affiche_compte_etat ( struct structure_operation *operation,
			   			   gchar *decalage_compte,
			   gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* mise en place du compte */

  if ( etat_courant -> regroupe_ope_par_compte
       &&
       operation -> no_compte != ancien_compte_etat )
    {
     /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
 	  /* on affiche le total de la période en le forçant */

	  ligne = gtktable_affichage . affiche_total_periode ( operation,
						   ligne,
						   1 );

	  /* on ajoute les totaux de tout ce qu'il y a derrière le compte */

	  ligne = gtktable_affichage . affiche_totaux_sous_jaccent ( 5,
	ligne );

	  /* on ajoute le total du compte */

	  ligne = gtktable_affichage . affiche_total_compte ( ligne );
	}
 
      /*       si on a demandé de ne pas afficher les noms des comptes, on saute la partie suivante */

      if ( etat_courant -> afficher_nom_compte )
	{
	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	  pointeur_char = g_strconcat ( decalage_compte,
					NOM_DU_COMPTE,
					NULL );
	  nom_compte_en_cours = NOM_DU_COMPTE;

	  label = gtk_label_new ( pointeur_char );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     0, 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );

	  ligne++;
	}

      ligne_debut_partie = ligne;
      denote_struct_sous_jaccentes ( 5 );

      ancien_compte_etat = operation -> no_compte;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gtktable_affiche_tiers_etat ( struct structure_operation *operation,
			  			  gchar *decalage_tiers,
			  gint ligne )
{
  gchar *pointeur_char;
  GtkWidget *label;

  /* affiche le tiers */

  if ( etat_courant -> utilise_tiers
       &&
       operation -> tiers != ancien_tiers_etat )
    {
      /* lorsqu'on est au début de l'affichage de l'état, on n'affiche pas de totaux */

      if ( !debut_affichage_etat
	   &&
	   !changement_de_groupe_etat )
	{
	  /* on affiche le total de la période en le forçant */

	  ligne = gtktable_affichage . affiche_total_periode ( operation,
							       ligne,
							       1 );

	  /* on ajoute les totaux de tout ce qu'il y a derrière le tiers */

	  ligne = gtktable_affichage . affiche_totaux_sous_jaccent ( 6,
	ligne );

	  /* on ajoute le total du tiers */

	  ligne = gtktable_affichage . affiche_total_tiers ( ligne );
	}

      /*       si on a demandé de ne pas afficher les noms des tiers, on saute la partie suivante */

      if ( etat_courant -> afficher_nom_tiers )
	{
	  if ( operation -> tiers )
	    {
	      nom_tiers_en_cours = ((struct struct_tiers *)(g_slist_find_custom ( liste_struct_tiers,
										  GINT_TO_POINTER ( operation -> tiers ),
										  (GCompareFunc) recherche_tiers_par_no ) -> data )) -> nom_tiers;
	      pointeur_char = g_strconcat ( decalage_tiers,
					    nom_tiers_en_cours,
					    NULL );
	    }
	  else
	    pointeur_char = g_strconcat ( decalage_tiers,
					  _("No third party"),
					  NULL );

	  label = gtk_label_new ( pointeur_char );
	  gtk_misc_set_alignment ( GTK_MISC ( label ),
				   0,
				   0.5 );
	  gtk_table_attach ( GTK_TABLE ( table_etat ),
			     label,
			     0, 1,
			     ligne, ligne + 1,
			     GTK_SHRINK | GTK_FILL,
			     GTK_SHRINK | GTK_FILL,
			     0, 0 );
	  gtk_widget_show ( label );
	

	  ligne++;
	}

      ligne_debut_partie = ligne;
      denote_struct_sous_jaccentes ( 6 );

      ancien_tiers_etat = operation -> tiers;

      debut_affichage_etat = 0;
      changement_de_groupe_etat = 1;
    }
  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gtktable_affiche_titre_revenus_etat ( gint ligne )
{
  GtkWidget *label;

  label = gtk_label_new ( "" );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;
  label = gtk_label_new ( _("Incomes") );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  label = gtk_label_new ( "" );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gtktable_affiche_titre_depenses_etat ( gint ligne )
{
  GtkWidget *label;

  label = gtk_label_new ( "" );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  label = gtk_label_new ( _("Outgoings") );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;

  label = gtk_label_new ( "" );
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     label,
		     0, 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( label );

  ligne++;


  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* appelée lors de l'affichage d'une structure ( catég, ib ... ) */
/* affiche le total de toutes les structures sous jaccentes */
/*****************************************************************************************************/

gint gtktable_affiche_totaux_sous_jaccent ( gint origine,
					    gint ligne )
{
  GList *pointeur_glist;


  /* on doit partir du bout de la liste pour revenir vers la structure demandée */

  pointeur_glist = g_list_last ( etat_courant -> type_classement );


  while ( GPOINTER_TO_INT ( pointeur_glist -> data ) != origine )
    {
      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	case 1:
	  ligne = gtktable_affichage . affiche_total_categories ( ligne );
	  break;

	case 2:
	  ligne = gtktable_affichage . affiche_total_sous_categ ( ligne );
	  break;

	case 3:
	  ligne = gtktable_affichage . affiche_total_ib ( ligne );
	  break;

	case 4:
	  ligne = gtktable_affichage . affiche_total_sous_ib ( ligne );
	  break;

	case 5:
	  ligne = gtktable_affichage . affiche_total_compte ( ligne );
	  break;

	case 6:
	  ligne = gtktable_affichage . affiche_total_tiers ( ligne );
	  break;
	}
      pointeur_glist = pointeur_glist -> prev;
    }

  return ( ligne );

}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gtktable_affiche_titres_colonnes ( gint ligne )
{
  gint colonne;
  GtkWidget *label;
  GtkWidget *separateur;

  colonne = 1;

  if ( etat_courant -> afficher_no_ope )
    {
      label = gtk_label_new ( _("No") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_date_ope )
    {
      label = gtk_label_new ( _("Date") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_exo_ope )
    {
      label = gtk_label_new ( _("Financial year") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_tiers_ope )
    {
      label = gtk_label_new ( _("Third party") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_categ_ope )
    {
      label = gtk_label_new ( _("Category") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_ib_ope )
    {
      label = gtk_label_new ( _("Budgetary line") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_notes_ope )
    {
      label = gtk_label_new ( _("Notes") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_type_ope )
    {
      label = gtk_label_new ( _("Methods of payment") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_cheque_ope )
    {
      label = gtk_label_new ( _("Cheque") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_pc_ope )
    {
      label = gtk_label_new ( _("Voucher") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_infobd_ope )
    {
      label = gtk_label_new ( _("Bank references") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_rappr_ope )
    {
      label = gtk_label_new ( _("Statement") );
      gtk_table_attach ( GTK_TABLE ( table_etat ),
			 label,
			 colonne, colonne + 1,
			 ligne, ligne + 1,
			 GTK_SHRINK | GTK_FILL,
			 GTK_SHRINK | GTK_FILL,
			 0, 0 );
      gtk_widget_show ( label );

      colonne = colonne + 2;
    }


  ligne++;

  separateur = gtk_hseparator_new ();
  gtk_table_attach ( GTK_TABLE ( table_etat ),
		     separateur,
		     1, GTK_TABLE ( table_etat ) -> ncols - 1,
		     ligne, ligne + 1,
		     GTK_SHRINK | GTK_FILL,
		     GTK_SHRINK | GTK_FILL,
		     0, 0 );
  gtk_widget_show ( separateur );

  ligne++;

  titres_affiches = 1;

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gtktable_finish ()
{
  /* Nothing to do in GTK */
  return 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* cette fonction est appelée si on click sur une opé dans un état */
/* elle affiche la liste des opés sur cette opé */
/*****************************************************************************************************/

void gtktable_click_sur_ope_etat ( struct structure_operation *operation )
{
  /* si c'est une opé de ventilation, on affiche l'opération mère */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

  if ( operation -> no_operation_ventilee_associee )
    operation = g_slist_find_custom ( LISTE_OPERATIONS,
				      GINT_TO_POINTER ( operation -> no_operation_ventilee_associee ),
				      (GCompareFunc) recherche_operation_par_no ) -> data;

  /* passage sur le compte concerné */

  changement_compte ( GINT_TO_POINTER ( operation -> no_compte ));

  /* récupération de la ligne de l'opé dans la liste ; affichage de toutes les opé si nécessaire */

  p_tab_nom_de_compte_variable = p_tab_nom_de_compte_courant;

  if ( operation -> pointe == 2 && !AFFICHAGE_R )
    change_aspect_liste ( NULL,
			  2 );

  OPERATION_SELECTIONNEE = operation;

  selectionne_ligne ( compte_courant );
}
/*****************************************************************************************************/
