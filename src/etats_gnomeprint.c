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
int do_preview=0, page=0, num_ope=0;
GnomeFont *title_font, *subtitle_font, *header_font, *text_font;
float point_x, point_y, tmp_x, tmp_y;
gfloat red=0, green=0, blue=0;


#define HMARGIN 15
#define VMARGIN 15


/*****************************************************************************************************/
/* Initialise les structures nécessaires pour gnome-print */
/*  - gpd : boîte de dialogue pour demander les réglages du print master */
/*  - gpm : le gnome print master */
/*  - pc : le contexte d'impression */
/*  - diverses polices */
/*****************************************************************************************************/
gint gnomeprint_initialise (GSList * opes_selectionnees)
{
  GnomePrintDialog *gpd;
/*   GnomePaper * paper; */
  static int copies=1, collate;

  gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new(_("Impression de Grisbi"), 
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
/*   paper = gnome_paper_with_size (841.88976, 595.27559);  */
/*   gnome_print_master_set_paper ( gpm, paper );  */
       

  gnome_dialog_close (GNOME_DIALOG(gpd));
  pc = gnome_print_master_get_context(gpm);

  title_font = gnome_font_new_closest ("Utopia", GNOME_FONT_BOLD, 0, 36);
  subtitle_font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 0, 20);
  header_font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 0, 12);
  text_font = gnome_font_new_closest ("Times", GNOME_FONT_BOOK, 0, 12);

  tmp_x = point_x = HMARGIN;
  tmp_y = point_y = gnome_paper_psheight(gnome_print_master_get_paper(gpm)) - VMARGIN;

  do_print_text_page(pc, text_font, opes_selectionnees);

  return 1;
}
/*****************************************************************************************************/


gdouble columns_max[32];
gdouble columns_total[32];
gdouble columns_pos[32];
gdouble columns_min[32];
gdouble columns_size[32];

char *columns[256][256];

GSList * text_to_words (guchar *text)
{
  GSList * words;
  const guchar *p;
  guchar *ub, *u;
  guchar * e = text + strlen(text);

  /* Split text into words & convert to utf-8 */
  ub = g_new (guchar, (e - text) * 2 + 1);
  u = ub;
  words = NULL;
  p = text;
  while (p < e) {
    while ((p < e) && (*p <= ' ')) p++;
    if (p < e) {
      words = g_slist_append (words, u);
      while ((p < e) && (*p > ' ')) {
 	u += g_unichar_to_utf8 (*p++, u);
      }
      *u++ = '\0';
    }
  }
  return words;
}


void
show_words(GnomePrintContext *pc, GnomeFont *font, GSList *words, 
	   gdouble x, gdouble y, gdouble mwidth)
{
  gint space;
  ArtPoint spadv;
  gdouble accwidth = 0.0;
  gdouble fontheight;

  fontheight = gnome_font_get_ascender (font) + gnome_font_get_descender (font);

  while (words != NULL)
    {
      gdouble width;
      GnomeGlyphList * gl;

      gl = gnome_glyphlist_from_text_dumb (font, 0x000000ff, 0.0, 0.0, "");
      gnome_font_get_glyph_stdadvance (font, space, &spadv);
      /* FIXME: gnome_font_get_width_string semble être confus à cause
	 des caractères unicode. */
      width = gnome_font_get_width_string (font, (gchar *) words->data);

      if (accwidth && ((accwidth+width) > mwidth))
	{
	  y-=(1.0*fontheight);
	  accwidth=0;
	}

      gnome_glyphlist_moveto (gl, x+accwidth, y);

      if (words->next) accwidth += spadv.x; 
      accwidth += width;

      gnome_glyphlist_advance (gl, TRUE);
      gnome_glyphlist_text_dumb (gl, words->data);
      gnome_print_moveto (pc, 0.0, 0.0);
      gnome_print_glyphlist (pc, gl);
      gnome_glyphlist_unref (gl);
      words = words->next;
    }
}


void
text_get_min_max (GnomePrintContext * pc, GnomeFont * font,
		  GSList * words, 
		  gdouble * min, gdouble * max)
{
  gdouble fontheight;
  gint space;
  gdouble accwidth;
  ArtPoint spadv;
  gdouble maxword=0;

  if (!font) return;
  fontheight = gnome_font_get_ascender (font) + gnome_font_get_descender (font);
  space = gnome_font_face_lookup_default (gnome_font_get_face (font), ' ');

  accwidth = 0.0;
  while (words != NULL)
    {
      gdouble width;
      GnomeGlyphList * gl;

      gl = gnome_glyphlist_from_text_dumb (font, 0x000000ff, 0.0, 0.0, "");

      gnome_font_get_glyph_stdadvance (font, space, &spadv);
      gnome_glyphlist_moveto (gl, 10.0+accwidth, 100.0);

      width = gnome_font_get_width_string (font, (gchar *) words->data);
      if (words->next) accwidth += (spadv.x/2);
      accwidth += width;

      if (width > maxword)
	{
	  maxword = width;
	}

      printf ("%s\n", words->data, width);

/*       gnome_glyphlist_advance (gl, TRUE); */
/*       gnome_glyphlist_text_dumb (gl, words->data); */
/*       gnome_print_moveto (pc, 0.0, 0.0); */
/*       gnome_print_glyphlist (pc, gl); */
      gnome_glyphlist_unref (gl);
      words = words->next;
    }

  *max=accwidth;
  *min=maxword;
}


void
do_print_text_page (GnomePrintContext *pc, GnomeFont *font, GSList * list)
{
  GSList * words;
  int i=0, maxi, column;
  gdouble total_text=0;
  gdouble total_width = 300;
  struct structure_operation *operation;

  gnome_print_beginpage (pc, "Glyph test page");
  gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
  gnome_print_setlinewidth (pc, 1.0);

  maxi=0;
  while (list)
    {
      operation = list -> data;
      columns[maxi][0] = g_strdup_printf ( "%4.2f", operation -> no_compte);
      columns[maxi][1] = g_strdup_printf ( "%4.2f", operation -> imputation);
      columns[maxi][2] = operation -> notes;
      columns[maxi][3] = g_strdup_printf ( "%4.2f", operation -> montant);
      list=list->next;
      maxi++;
    }
  maxi=10;

  for (i=0; i<maxi; i++)
    {
      for (column=0; column<4; column++)
	{
	  gdouble min, max;
	  words = text_to_words(columns[i][column]);
	  text_get_min_max(pc, font, words, &min, &max);
	  if (min > columns_min[column])
	    {
	      columns_min[column] = min;
	    }
	  columns_total[column] += max;
	  g_slist_free (words);	  
	}
    }
  
  gnome_print_setlinewidth (pc, 1.0);
  gnome_print_moveto (pc, 10, 800);
  gnome_print_lineto (pc, 10, 200);
  gnome_print_lineto (pc, total_width+10, 200);
  gnome_print_lineto (pc, total_width+10, 800);
  gnome_print_closepath (pc);
  gnome_print_stroke (pc);

  for (column=0; column<4; column++)
    {
      total_text += columns_total[column];
    }

  for (column=0; column<4; column++)
    {
      if (((columns_total[column]/total_text) * total_width) <
	  columns_min[column])
	{
	  printf (">> %f, %f\n", columns_min[column], ((columns_total[column]/total_text) * total_width));
	  columns_max[column] = columns_min[column];
	  total_width -= columns_min[column];
	  total_text -= columns_total[column];
	}
      else
	{
	  columns_max[column]=0;
	}
    }

  for (column=0; column<4; column++)
    {
      if (! columns_max[column])
	{
	  columns_max[column] = ((columns_total[column]/total_text) * total_width);
	  printf ("++ %d, %f, %f, %f\n", column, columns_max[column], columns_total[column], total_text);
	}
      if (column > 0)
	{
	  columns_pos[column] = columns_pos[column-1] + columns_max[column-1];
	  printf ("** %d, %f, %f\n", column, columns_pos[column-1], columns_pos[column]);
	}
      printf ("-- %d, %f, %f, %f\n", column, columns_max[column], columns_total[column], total_text);
      gnome_print_gsave (pc);
      gnome_print_setlinewidth (pc, 1);
      gnome_print_moveto (pc, columns_pos[column]+10, 800);
      gnome_print_lineto (pc, columns_pos[column]+10, 200);
      gnome_print_stroke (pc);
      gnome_print_grestore (pc);
    }

  for (i=0; i<maxi; i++)
    {
      for (column=0; column<4; column++)
	{
	  words = text_to_words(columns[i][column]);
	  show_words(pc, font, words, 10+columns_pos[column], 750-i*30, columns_max[column]);
	  g_slist_free (words);	  
	}
    }

/*   words = text_to_words("1 2 3 4 5 6"); */
/*   text_get_min_max(pc, font, words, &min, &max); */
/*   printf (">> %.2f, %.2f\n", min, max); */
/*   show_words(pc, font, words, 20, 400, 50); */
/*   g_slist_free (words); */

  gnome_print_showpage (pc);
}

/*****************************************************************************************************/
/* passe à la page suivante si nécessaire */
/*****************************************************************************************************/
void gnomeprint_verifier_nouvelle_page ( void )
{
  if ( tmp_y < 20 )
    {
      gnome_print_showpage ( pc );
      gnome_print_beginpage ( pc, g_strdup_printf ("%d", ++page) );
      tmp_x = point_x = HMARGIN;
      tmp_y = point_y = gnome_paper_psheight(gnome_print_master_get_paper(gpm)) - VMARGIN;
    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* passe à la ligne suivante si nécessaire */
/*****************************************************************************************************/
void gnomeprint_verifier_nouvelle_ligne ( int height, int margin )
{
  if ( tmp_x > (gnome_paper_pswidth(gnome_print_master_get_paper(gpm)) - margin - HMARGIN))
    {
      tmp_y += height;
      tmp_x = HMARGIN;
      gnomeprint_commit_point ( );
      if (num_ope % 2)
	{
	  gnomeprint_rectangle ( HMARGIN, 
				 tmp_y,
				 gnome_paper_pswidth(gnome_print_master_get_paper(gpm))-HMARGIN, 
				 tmp_y + height);
	}
    }
}
/*****************************************************************************************************/

/*****************************************************************************************************/
/* modifie la couleur utilisée pour l'affichage */
/*****************************************************************************************************/
void gnomeprint_set_color ( gfloat tred, gfloat tgreen, gfloat tblue )
{
  red = tred;
  green = tgreen;
  blue = tblue;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* déplace le point de référence d'un certain nombre de millimètres */
/*****************************************************************************************************/
void gnomeprint_move_point ( gfloat x, gfloat y )
{
  point_x = tmp_x + gnome_paper_convert_to_points(x, gnome_unit_with_name ("Millimeter"));
  point_y = tmp_y + gnome_paper_convert_to_points(y, gnome_unit_with_name ("Millimeter"));
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Met à jour le point de référence à partir du point temporaire */
/*****************************************************************************************************/
void gnomeprint_commit_point ( )
{
  gnomeprint_commit_x ( );
  gnomeprint_commit_y ( );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Met à jour le point temporaire à partir du point de référence */
/*****************************************************************************************************/
void gnomeprint_update_point ( )
{
  tmp_x = point_x;
  tmp_y = point_y;
}


/*****************************************************************************************************/
/* Met à jour la coordonnée x du point temporaire à partir du point de référence */
/*****************************************************************************************************/
void gnomeprint_update_x ( )
{
  tmp_x = point_x;
}


/*****************************************************************************************************/
/* Met à jour la coordonnée y du point temporaire à partir du point de référence */
/*****************************************************************************************************/
void gnomeprint_update_y ( )
{
  tmp_y = point_y;
}


/*****************************************************************************************************/
/* Met à jour la composante Y du point de référence */
/*****************************************************************************************************/
void gnomeprint_commit_x ( )
{
  point_x = tmp_x;
}


/*****************************************************************************************************/
/* Met à jour la composante Y du point de référence */
/*****************************************************************************************************/
void gnomeprint_commit_y ( )
{
  point_y = tmp_y;
}


/*****************************************************************************************************/
/* affiche le titre sur la première page du rapport */
/*****************************************************************************************************/
gint gnomeprint_affiche_titre ( gint ligne )
{
  gnome_print_beginpage ( pc, g_strdup_printf ( "%d", ++page ) );

  gnomeprint_set_color ( 1, 0, 0 );
  gnomeprint_affiche_texte (etat_courant -> nom_etat, title_font);
  gnomeprint_commit_point ();

  gnomeprint_set_color ( 0, 0, 0 );

  return 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Affiche le texte passé en argument dans une certaine police */
/* Est sensible à la position du point de référence et à la couleur */
/* Déplace le point temporaire */
/*****************************************************************************************************/
void gnomeprint_affiche_texte ( char * texte, GnomeFont * font)
{
  gint font_height = gnome_font_get_size(font);

  gnomeprint_verifier_nouvelle_page ();

  gnomeprint_update_point ( );

  gnome_print_gsave (pc);
  gnome_print_setfont (pc, font);
  gnome_print_setrgbcolor (pc, red, green, blue); 
  gnome_print_moveto (pc, point_x, point_y-font_height);
  gnome_print_show (pc, latin2utf8(texte));
  gnome_print_grestore (pc);

  tmp_y -= font_height;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* Affiche un séparateur vertical */
/*****************************************************************************************************/
void gnomeprint_barre_verticale ( gint taille )
{
  gnomeprint_verifier_nouvelle_page ();

  gnome_print_gsave (pc);
  gnome_print_setlinewidth (pc, 1);
  gnome_print_moveto (pc, point_x, point_y);
  gnome_print_lineto (pc, 
		      point_x, point_y + taille);
  gnome_print_stroke (pc);
  gnome_print_grestore (pc);
}


/*****************************************************************************************************/
/* Affiche un rectangle */
/*****************************************************************************************************/
void gnomeprint_rectangle ( gfloat x1, gfloat y1, gfloat x2, gfloat y2 )
{
  gnome_print_gsave (pc);
  
  gnome_print_setrgbcolor ( pc, 0.8, 0.8, 0.8 );
  gnome_print_moveto (pc, x1, y1);
  gnome_print_lineto (pc, x1, y2);
  gnome_print_lineto (pc, x2, y2);
  gnome_print_lineto (pc, x2, y1);
  gnome_print_closepath (pc);
  gnome_print_fill (pc);
  gnome_print_grestore (pc);
}


/*****************************************************************************************************/
/* Affiche un séparateur horizontal */
/*****************************************************************************************************/
gint gnomeprint_affiche_separateur ( gint ligne )
{
  gnomeprint_verifier_nouvelle_page ();

  gnomeprint_move_point ( 0, -5 ); 

  gnome_print_gsave (pc);
  gnome_print_setlinewidth (pc, 2);
  gnome_print_moveto (pc, point_x, point_y);
  gnome_print_lineto (pc, 
		      gnome_paper_pswidth(gnome_print_master_get_paper(gpm))-HMARGIN, 
		      point_y); 
  gnome_print_stroke (pc);
  gnome_print_grestore (pc);

  gnomeprint_move_point ( 0, -5 ); 

  return ligne + 1;
}


/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les catégories sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_categories ( gint ligne )
{
  montant_categ_etat = 0;
  nom_categ_en_cours = NULL;
  titres_affiches = 0;
  
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
  montant_sous_categ_etat = 0;
  nom_ss_categ_en_cours = NULL;
  titres_affiches = 0;

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
	      gnomeprint_affiche_separateur ( ligne );
	    }
	      
	  gnomeprint_move_point ( 0, -5 );
	  
	  ligne++;
	  
	  gnomeprint_move_point ( HMARGIN, 0 );

	  ligne++;

	  if ( nom_ib_en_cours )
	    gnomeprint_affiche_texte ( g_strconcat ( _("Total "),
						  nom_ib_en_cours,
						  NULL ),
				       text_font );
	  else
	    gnomeprint_affiche_texte ( _("Total Imputations budgétaires : "),
					text_font);

	  gnomeprint_update_point ( );
	  gnomeprint_move_point ( 100, 0 );

	  gnomeprint_affiche_texte ( g_strdup_printf ( "%4.2f %s",
						       montant_ib_etat,
						       devise_ib_etat -> code_devise ),
				     text_font);
	  gnomeprint_commit_y ( );

	  ligne++;
	}
      else
	{
	  ligne--;

	  gnomeprint_affiche_texte ( g_strdup_printf ( "%4.2f %s",
							montant_ib_etat,
							devise_ib_etat -> code_devise ),
				      text_font);
	  ligne++;
	}
    }

  tmp_x = HMARGIN;
  gnomeprint_commit_point ( );

  montant_ib_etat = 0;
  nom_ib_en_cours = NULL;
  titres_affiches = 0;

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

  montant_sous_ib_etat = 0;
  nom_ss_ib_en_cours = NULL;
  titres_affiches = 0;

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

  montant_compte_etat = 0;
  nom_compte_en_cours = NULL;
  titres_affiches = 0;

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

  montant_tiers_etat = 0;
  nom_tiers_en_cours = NULL;
  titres_affiches = 0;

  return (ligne );
}
/*****************************************************************************************************/




/*****************************************************************************************************/
gint gnomeprint_affichage_ligne_ope ( struct structure_operation *operation,
			   gint ligne )
{
  int colonne, origine_y = tmp_y;
  int size=0;

  gnomeprint_verifier_nouvelle_page ();

  if ( etat_courant -> afficher_opes )
    {
      /* on affiche ce qui est demandé pour les opés */


      /* si les titres ne sont pas affichés et qu'il faut le faire, c'est ici */

      if ( !titres_affiches
	   &&
	   etat_courant -> afficher_titre_colonnes
	   &&
	   etat_courant -> type_affichage_titres )
	{
	  ligne = gnomeprint_affichage . affiche_titres_colonnes ( ligne );
	  num_ope = 0;
	}

      colonne = 1;

      /* C'est laid, mais je n'ai pas le temps d'implémenter le
	 mécanisme propre auquel je pense : j'ai promis une
	 impression fonctionnelle pour demain */
      gnomeprint_set_color ( 1, 1, 1);
      gnomeprint_affiche_texte (itoa ( operation -> no_operation ), text_font);
      size = tmp_y - point_y;
      origine_y = point_y;      
      if (num_ope % 2)
	{
	  gnomeprint_rectangle ( HMARGIN, 
				 origine_y,
				 gnome_paper_pswidth(gnome_print_master_get_paper(gpm))-HMARGIN, 
				 origine_y + size);
	}
      gnomeprint_set_color ( 0, 0, 0);


      if ( etat_courant -> afficher_no_ope )
	{
	  gnomeprint_affiche_texte (itoa ( operation -> no_operation ), text_font);

	  gnomeprint_update_point ();
	  gnomeprint_move_point ( 8, 0 );
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();
	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_date_ope )
	{
	  gnomeprint_verifier_nouvelle_ligne ( size, 20 );
	  gnomeprint_affiche_texte ( g_strdup_printf  ( "%.2d/%.2d/%d",
							operation -> jour,
							operation -> mois,
							operation -> annee ),
				     text_font);
	  size = tmp_y - point_y;
	  gnomeprint_update_point ();
	  gnomeprint_move_point ( 20, 0 );
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();
	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_exo_ope )
	{
	  gnomeprint_verifier_nouvelle_ligne ( size, 30 );
	  if ( operation -> no_exercice )
	    {
	      gnomeprint_affiche_texte( ((struct struct_exercice *)(g_slist_find_custom ( liste_struct_exercices,
											  GINT_TO_POINTER ( operation -> no_exercice ),
											  (GCompareFunc) recherche_exercice_par_no )->data)) -> nom_exercice, text_font );
	      size = tmp_y - point_y;
	    }

	  gnomeprint_update_point ();
	  gnomeprint_move_point ( 30, 0 );
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();

	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_tiers_ope )
	{
	  gnomeprint_verifier_nouvelle_ligne ( size, 80 );
	  if ( operation -> tiers )
	    {
	      gnomeprint_affiche_texte( ((struct struct_tiers *)
					 (g_slist_find_custom ( liste_struct_tiers,
								GINT_TO_POINTER ( operation -> tiers ),
								(GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers , 
					text_font);
	      size = tmp_y - point_y;
	    }
	  gnomeprint_update_point();
	  gnomeprint_move_point ( 80, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();
	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_categ_ope )
	{
	  gchar *pointeur;

	  pointeur = NULL;

	  gnomeprint_verifier_nouvelle_ligne ( size, 70 );

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
		    pointeur = g_strconcat ( _("Virement vers "),
					     NOM_DU_COMPTE,
					     NULL );
		  else
		    pointeur = g_strconcat ( _("Virement de "),
					     NOM_DU_COMPTE,
					     NULL );
		}
	    }

	  if ( pointeur )
	    {
	      gnomeprint_affiche_texte ( pointeur, text_font );
	      size = tmp_y - point_y;
	    }
	  gnomeprint_update_point();
	  gnomeprint_move_point ( 70, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();
	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_ib_ope )
	{
	  gnomeprint_verifier_nouvelle_ligne ( size, 50 );

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

	      gnomeprint_affiche_texte ( pointeur, text_font );
	      size = tmp_y - point_y;
	    }
	  gnomeprint_update_point();
	  gnomeprint_move_point ( 50, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();
	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_notes_ope )
	{
	  gnomeprint_verifier_nouvelle_ligne ( size, 70 );

	  if ( operation -> notes )
	    {
	      gnomeprint_affiche_texte ( operation -> notes, text_font );
	      size = tmp_y - point_y;
	    }
	  gnomeprint_update_point();
	  gnomeprint_move_point ( 70, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();

	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_type_ope )
	{
	  GSList *pointeur;

	  gnomeprint_verifier_nouvelle_ligne ( size, 30 );

	  p_tab_nom_de_compte_variable = p_tab_nom_de_compte + operation -> no_compte;

	  pointeur = g_slist_find_custom ( TYPES_OPES,
					   GINT_TO_POINTER ( operation -> type_ope ),
					   (GCompareFunc) recherche_type_ope_par_no );

	  if ( pointeur )
	    {
	      struct struct_type_ope *type;
	      type = pointeur -> data;

	      gnomeprint_affiche_texte ( type -> nom_type, text_font );
	      size = tmp_y - point_y;
	    }

	  gnomeprint_update_point();
	  gnomeprint_move_point ( 30, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();

	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_cheque_ope )
	{
	  gnomeprint_verifier_nouvelle_ligne ( size, 20 );

	  if ( operation -> contenu_type )
	    {
	      gnomeprint_affiche_texte ( operation -> contenu_type, text_font );
	      size = tmp_y - point_y;
	    }

	  gnomeprint_update_point();
	  gnomeprint_move_point ( 20, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();

	  colonne = colonne + 2;
	}


      if ( etat_courant -> afficher_pc_ope )
	{
	  gnomeprint_verifier_nouvelle_ligne ( size, 30 );

	  if ( operation -> no_piece_comptable )
	    {
	      gnomeprint_affiche_texte ( operation -> no_piece_comptable, text_font );
	      size = tmp_y - point_y;
	    }

	  gnomeprint_update_point();
	  gnomeprint_move_point ( 30, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();
	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_infobd_ope )
	{
	  if ( operation -> info_banque_guichet )
	    {
	      gnomeprint_affiche_texte ( operation -> info_banque_guichet, text_font );
	      size = tmp_y - point_y;
	    }

	  gnomeprint_update_point();
	  gnomeprint_move_point ( 30, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();

	  colonne = colonne + 2;
	}

      if ( etat_courant -> afficher_rappr_ope )
	{
	  GSList *pointeur;

	  gnomeprint_verifier_nouvelle_ligne ( size, 30 );

	  pointeur = g_slist_find_custom ( liste_no_rapprochements,
					   GINT_TO_POINTER ( operation -> no_rapprochement ),
					   (GCompareFunc) recherche_no_rapprochement_par_no );

	  if ( pointeur )
	    {
	      struct struct_no_rapprochement *rapprochement;
	      rapprochement = pointeur -> data;
	      gnomeprint_affiche_texte ( rapprochement -> nom_rapprochement, text_font );
	      size = tmp_y - point_y;
	    }

	  gnomeprint_update_point();
	  gnomeprint_move_point ( 30, 0);
	  gnomeprint_update_point ( );
	  gnomeprint_barre_verticale ( size );
	  gnomeprint_move_point ( 2, 0 );
	  gnomeprint_update_point ();

	  colonne = colonne + 2;
	}


      /* on affiche le montant au bout de la ligne */

      gnomeprint_verifier_nouvelle_ligne ( size, 20 );

      if ( devise_compte_en_cours_etat
	   &&
	   operation -> devise == devise_compte_en_cours_etat -> no_devise )
	gnomeprint_affiche_texte ( g_strdup_printf  ("%4.2f %s",
						     operation -> montant,
						     devise_compte_en_cours_etat -> code_devise ), text_font);
      else
	{
	  struct struct_devise *devise_operation;

	  devise_operation = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( operation -> devise ),
						   ( GCompareFunc ) recherche_devise_par_no ) -> data;
	  gnomeprint_affiche_texte ( g_strdup_printf  ("%4.2f %s",
						       operation -> montant,
						       devise_operation -> code_devise ), text_font);
	}

      if ( ligne_debut_partie == -1 )
	ligne_debut_partie = ligne;

      ligne++;
    }

  tmp_x = HMARGIN;
  gnomeprint_commit_point ();

  num_ope ++;

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gnomeprint_affiche_total_partiel ( gdouble total_partie,
			     gint ligne,
			     gint type )
{

  if ( etat_courant -> afficher_opes
       &&
       ligne_debut_partie != -1 )
    {
      gnomeprint_affiche_separateur ( ligne );
      ligne_debut_partie = -1;
    }
  
  gnomeprint_move_point ( 0, -5 );
  if ( type )
    gnomeprint_affiche_texte ( _("Total dépenses : "), header_font );
  else
    gnomeprint_affiche_texte ( _("Total revenus : "), header_font );

  gnomeprint_update_point ( );
  gnomeprint_move_point ( 110, 0 );

  gnomeprint_affiche_texte ( g_strdup_printf ( "%4.2f %s",
					       total_partie,
					       devise_generale_etat -> code_devise ),
			     text_font );
  tmp_x = HMARGIN;
  gnomeprint_commit_x ( );
  gnomeprint_affiche_separateur ( ligne );

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
gint gnomeprint_affiche_total_general ( gdouble total_general,
			     gint ligne )
{
  if ( etat_courant -> afficher_opes
       &&
       ligne_debut_partie != -1 )
    {
      gnomeprint_affiche_separateur ( ligne );
      ligne_debut_partie = -1;
    }      

  gnomeprint_affiche_texte ( _("Total général : "), header_font );
  gnomeprint_update_point ( );
  gnomeprint_move_point ( 110, 0 );
  
  gnomeprint_affiche_texte ( g_strdup_printf ( "%4.2f %s",
					       total_general,
					       devise_generale_etat -> code_devise ),
			     header_font );

  tmp_x = HMARGIN;
  gnomeprint_commit_x ( );
  gnomeprint_affiche_separateur ( ligne );

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gnomeprint_affiche_categ_etat ( struct structure_operation *operation,
			  			  gchar *decalage_categ,
				     gint ligne )
{
  gchar *pointeur_char;

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
	  /* on ajoute les totaux de tout ce qu'il y a derrière la catégorie */

	  ligne = gnomeprint_affichage . affiche_totaux_sous_jaccent ( 1,
								       ligne );

	  /* on ajoute le total de la categ */

	  ligne = gnomeprint_affichage . affiche_total_categories ( ligne );
	}

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
					    _("Virements"),
					    NULL );
	      ancienne_categ_speciale_etat = 1;
	    }
	  else
	    {
	      if ( operation -> operation_ventilee )
		{
		  pointeur_char = g_strconcat ( decalage_categ,
						_("Opération ventilée"),
						NULL );
		  ancienne_categ_speciale_etat = 2;
		}
	      else
		{
		  pointeur_char = g_strconcat ( decalage_categ,
						_("Pas de catégorie"),
						NULL );
		  ancienne_categ_speciale_etat = 3;
		}
	    }
	}

      gnomeprint_move_point ( 0, -2 );
      gnomeprint_affiche_texte ( pointeur_char, header_font );
      gnomeprint_commit_point ( );
      gnomeprint_move_point ( 0, -2 );

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
  gchar *pointeur_char;

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
	  /* on ajoute les totaux de tout ce qu'il y a derrière l'ib */

	  ligne = gnomeprint_affichage . affiche_totaux_sous_jaccent ( 3,
	ligne );

	  /* on ajoute le total de l'ib */

	  ligne = gnomeprint_affichage . affiche_total_ib ( ligne );
	}
 
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
				      _("Pas d'imputation budgétaire"),
				      NULL );

      gnomeprint_affiche_texte ( pointeur_char, text_font );
      gnomeprint_commit_y ( );
      ligne++;

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
  gnomeprint_affiche_texte ( _("Revenus"), subtitle_font );
  gnomeprint_commit_y ( );
  gnomeprint_move_point ( 0, -5 );

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gnomeprint_affiche_titre_depenses_etat ( gint ligne )
{
  gnomeprint_affiche_texte ( _("Dépenses"), subtitle_font );
  gnomeprint_commit_y ( );
  gnomeprint_move_point ( 0, -5 );

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
  GList *pointeur_glist;

  /* on doit partir du bout de la liste pour revenir vers la structure demandée */
  pointeur_glist = g_list_last ( etat_courant -> type_classement );


  while ( GPOINTER_TO_INT ( pointeur_glist -> data ) != origine )
    {
      switch ( GPOINTER_TO_INT ( pointeur_glist -> data ))
	{
	case 1:
	  ligne = gnomeprint_affichage . affiche_total_categories ( ligne );
	  break;

	case 2:
	  ligne = gnomeprint_affichage . affiche_total_sous_categ ( ligne );
	  break;

	case 3:
	  ligne = gnomeprint_affichage . affiche_total_ib ( ligne );
	  break;

	case 4:
	  ligne = gnomeprint_affichage . affiche_total_sous_ib ( ligne );
	  break;

	case 5:
	  ligne = gnomeprint_affichage . affiche_total_compte ( ligne );
	  break;

	case 6:
	  ligne = gnomeprint_affichage . affiche_total_tiers ( ligne );
	  break;
	}
      pointeur_glist = pointeur_glist -> prev;
    }

  return ( ligne );
}
/*****************************************************************************************************/



/*****************************************************************************************************/
gint gnomeprint_affiche_titres_colonnes ( gint ligne )
{
  gint colonne = 1, size = 0;

  if ( etat_courant -> afficher_no_ope )
    {
      gnomeprint_affiche_texte ( _("N°"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 8, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_date_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 20 );
      gnomeprint_affiche_texte ( _("Date"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 20, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_exo_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 30 );
      gnomeprint_affiche_texte ( _("Exercice"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 30, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_tiers_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 80 );
      gnomeprint_affiche_texte ( _("Tiers"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 80, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_categ_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 70 );
      gnomeprint_affiche_texte ( _("Catégorie"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 70, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_ib_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 50 );
      gnomeprint_affiche_texte ( _("Imputation budgétaire"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 50, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_notes_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 70 );
      gnomeprint_affiche_texte ( _("Notes"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 70, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_type_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 30 );
      gnomeprint_affiche_texte ( _("Type"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 30, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_cheque_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 20 );
      gnomeprint_affiche_texte ( _("Chèque"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 20, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_pc_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 30 );
      gnomeprint_affiche_texte ( _("Pièce comptable"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 30, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_infobd_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 30 );
      gnomeprint_affiche_texte ( _("Info banque/guichet"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 30, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();

      colonne = colonne + 2;
    }

  if ( etat_courant -> afficher_rappr_ope )
    {
      gnomeprint_verifier_nouvelle_ligne ( size, 30 );
      gnomeprint_affiche_texte ( _("Relevé"), header_font );
      size = tmp_y - point_y;
      gnomeprint_update_point();
      gnomeprint_move_point ( 30, 0);
      gnomeprint_update_point ( );
      gnomeprint_barre_verticale ( size );
      gnomeprint_move_point ( 2, 0 );
      gnomeprint_update_point ();
      colonne = colonne + 2;
    }

  gnomeprint_verifier_nouvelle_ligne ( size, 20 );
  gnomeprint_affiche_texte ( _("Montant"), header_font);
  gnomeprint_update_point ( );

  ligne++;

  tmp_x = HMARGIN;
  tmp_y += size;
  gnomeprint_commit_point ( );

  ligne++;

/*   gnome_print_gsave (pc); */
/*   gnome_print_setlinewidth (pc, 1); */
/*   gnome_print_moveto (pc, point_x, point_y); */
/*   gnome_print_lineto (pc,  */
/* 		      gnome_paper_pswidth(gnome_print_master_get_paper(gpm))-10,  */
/* 		      point_y);  */
/*   gnome_print_stroke (pc); */
/*   gnome_print_grestore (pc); */
/*   gnomeprint_move_point ( 0, -1 ); */

  titres_affiches = 1;

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
					   _("Prévisualisation de l'impression de Grisbi"));
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
