/*  Fichier qui s'occupe d'afficher les états via gnomeprint */
/*      etats_gnomeprint.c */

/*     Copyright (C) 2002  Benjamin Drieu */
/* 			bdrieu@april.org */
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

#include "etats_gnomeprint.h"

GtkWidget * table_etat;

struct struct_etat_affichage gnomeprint_affichage = {
  gnomeprint_initialise,
  gnomeprint_affiche_titre,
  gnomeprint_affiche_separateur,
  gnomeprint_affiche_total_categories,
  gnomeprint_affiche_total_sous_categ,
  gnomeprint_affiche_total_ib,
  gnomeprint_affiche_total_sous_ib,
  gnomeprint_affiche_total_compte,
  gnomeprint_affiche_total_tiers,
  gnomeprint_affichage_ligne_ope,
  gnomeprint_affiche_total_partiel,
  gnomeprint_affiche_total_general,
  gnomeprint_affiche_categ_etat,
  gnomeprint_affiche_sous_categ_etat,
  gnomeprint_affiche_ib_etat,
  gnomeprint_affiche_sous_ib_etat,
  gnomeprint_affiche_compte_etat,
  gnomeprint_affiche_tiers_etat,
  gnomeprint_affiche_titre_revenus_etat,
  gnomeprint_affiche_titre_depenses_etat,
  gnomeprint_affiche_totaux_sous_jaccent,
  gnomeprint_affiche_titres_colonnes,
  gnomeprint_finish
};


GnomePrintContext *pc = NULL;
GnomePrintMaster *gpm = NULL;
int do_preview=0;
GnomeFont *title_font, *subtitle_font, *text_font;
int point_x, point_y, tmp_x, tmp_y;
gfloat red=0, green=0, blue=0;


gint gnomeprint_initialise ()
{
  GnomePrintDialog *gpd;
  static int copies=1, collate;
  
  gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new("Impression de Grisbi", 
						   GNOME_PRINT_DIALOG_COPIES));
  gnome_print_dialog_set_copies(gpd, copies, collate);

  switch (gnome_dialog_run(GNOME_DIALOG(gpd))) {
  case GNOME_PRINT_PRINT:
    do_preview = 0;
    break;
  case GNOME_PRINT_PREVIEW:
    do_preview = 1;
    break;
  case GNOME_PRINT_CANCEL:
    gnome_dialog_close (GNOME_DIALOG(gpd));
    return 0;
  }

  gpm = gnome_print_master_new();
  gnome_print_dialog_get_copies(gpd, &copies, &collate);
  gnome_print_master_set_copies(gpm, copies, collate);
  gnome_print_master_set_printer(gpm, gnome_print_dialog_get_printer(gpd));

  gnome_dialog_close (GNOME_DIALOG(gpd));
  pc = gnome_print_master_get_context(gpm);

  title_font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 1, 36);
  subtitle_font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 1, 20);
  text_font = gnome_font_new_closest ("Times", GNOME_FONT_BOOK, 1, 14);

  tmp_x = point_x = 10;
  tmp_y = point_y = gnome_paper_psheight(gnome_print_master_get_paper(gpm));

  return 1;
}


gint gnomeprint_affiche_titre ( gint ligne )
{
  gint i;

  /* TODO: utiliser un compteur de page ? */
  gnome_print_beginpage (pc, "1");

  /* FIXME: crado */
  p_tab_nom_de_compte_variable = p_tab_nom_de_compte;

  gnomeprint_set_color ( 1, 0, 0 );
  gnomeprint_affiche_texte (etat_courant -> nom_etat, title_font);
  gnomeprint_commit_point ();

  gnomeprint_set_color ( 0, 0, 0 );
  gnomeprint_move_point ( 0, -10 );

  for ( i=0 ; i < nb_comptes ; i++ )
    {
      gnomeprint_affiche_texte(NOM_DU_COMPTE, subtitle_font);
      gnomeprint_commit_point();
      p_tab_nom_de_compte_variable++;
    }

  return 1;
}


void gnomeprint_set_color ( gfloat tred, gfloat tgreen, gfloat tblue )
{
  red = tred;
  green = tgreen;
  blue = tblue;
}


void gnomeprint_move_point ( gfloat x, gfloat y )
{
  point_x = tmp_x + gnome_paper_convert_to_points(x, gnome_unit_with_name ("Millimeter"));
  point_y = tmp_y + gnome_paper_convert_to_points(y, gnome_unit_with_name ("Millimeter"));
}


void gnomeprint_affiche_texte ( char * texte, GnomeFont * font)
{
  gint font_height = gnome_font_get_size(font);

  gnomeprint_update_point ( );

  gnome_print_gsave (pc);
  gnome_print_setfont (pc, font);
  gnome_print_setrgbcolor (pc, red, green, blue); 
  gnome_print_moveto (pc, point_x, point_y-font_height);
  gnome_print_show (pc, latin2utf8(texte));
  gnome_print_grestore (pc);

  tmp_y -= font_height;
}


void gnomeprint_commit_point ( )
{
  gnomeprint_commit_x ( );
  gnomeprint_commit_y ( );
}


void gnomeprint_update_point ( )
{
  tmp_x = point_x;
  tmp_y = point_y;
}


void gnomeprint_commit_x ( )
{
  point_x = tmp_x;
}


void gnomeprint_commit_y ( )
{
  point_y = tmp_y;
}


gint gnomeprint_affiche_separateur ( gint ligne )
{

  return ligne + 1;
}


/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les catégories sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_categories ( gint ligne )
{

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_categ sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_sous_categ ( gint ligne )
{

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_ib ( gint ligne )
{

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les sous_ib sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_sous_ib ( gint ligne )
{

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les compte sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_compte ( gint ligne )
{

  return (ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les tiers sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_tiers ( gint ligne )
{

  return (ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gnomeprint_affichage_ligne_ope ( struct structure_operation *operation,
			   gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gnomeprint_affiche_total_partiel ( gdouble total_partie,
			     gint ligne,
			     gint type )
{

  return ( ligne );
}
/*****************************************************************************************************/

/*****************************************************************************************************/
gint gnomeprint_affiche_total_general ( gdouble total_general,
			     gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gnomeprint_affiche_categ_etat ( struct structure_operation *operation,
			  			  gchar *decalage_categ,
			  gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gnomeprint_affiche_sous_categ_etat ( struct structure_operation *operation,
			       			       gchar *decalage_sous_categ,
			       gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gnomeprint_affiche_ib_etat ( struct structure_operation *operation,
		       		       gchar *decalage_ib,
		       gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gnomeprint_affiche_sous_ib_etat ( struct structure_operation *operation,
			    			    gchar *decalage_sous_ib,
			    gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gnomeprint_affiche_compte_etat ( struct structure_operation *operation,
			   			   gchar *decalage_compte,
			   gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gnomeprint_affiche_tiers_etat ( struct structure_operation *operation,
			  			  gchar *decalage_tiers,
			  gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gnomeprint_affiche_titre_revenus_etat ( gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gnomeprint_affiche_titre_depenses_etat ( gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* appelée lors de l'affichage d'une structure ( catég, ib ... ) */
/* affiche le total de toutes les structures sous jaccentes */
/*****************************************************************************************************/

gint gnomeprint_affiche_totaux_sous_jaccent ( gint origine,
				   gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gnomeprint_affiche_titres_colonnes ( gint ligne )
{

  return ( ligne );
}
/*****************************************************************************************************/


gint gnomeprint_finish ( )
{
  gnome_print_showpage (pc);

  gnome_print_master_close(gpm);
  if (do_preview)
    {
      GnomePrintMasterPreview * pmp;
      pmp = gnome_print_master_preview_new(gpm, 
					   "Prévisualisation de l'impression de Grisbi");
      gtk_widget_show(GTK_WIDGET(pmp));
    }
  else
    {
      gnome_print_master_print(gpm);
    }

  return 1;

  // FIXME: quand est-ce qu'on le détruit ?
  //   gtk_object_unref (GTK_OBJECT (pc));
}
