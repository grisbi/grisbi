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
  gnomeprint_affiche_total_periode,
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
GnomeFont *title_font, *subtitle_font, *header_font, *total_font, *text_font;
float point_x, point_y, tmp_x, tmp_y;
gint red=0, green=0, blue=0;
gint color;


ArtPoint point;
ArtPoint tmp_point;

#define HMARGIN 15
#define VMARGIN 15
#define PAPER_WIDTH gnome_paper_pswidth(gnome_print_master_get_paper(gpm))
#define PAPER_HEIGHT gnome_paper_psheight(gnome_print_master_get_paper(gpm))


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

  gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new(_("Grisbi Print"), 
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
  text_font = gnome_font_new_closest ("Times", GNOME_FONT_BOOK, 0, 12);
  header_font = gnome_font_new_closest ("Times", GNOME_FONT_BOOK, 0, 14);
  total_font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 0, 14);

  tmp_x = point_x = HMARGIN;
  tmp_y = point_y = PAPER_HEIGHT - VMARGIN;

  gnomeprint_balancer_colonnes(pc, header_font, opes_selectionnees);
  point.x = point_x;
  point.y = point_y;

  color = 0x000000ff;

  return 1;
}
/*****************************************************************************************************/


gdouble columns_max[32];
gdouble columns_total[32];
gdouble columns_pos[32];
gdouble columns_min[32];
gdouble columns_size[32];

char *columns[256][256];

gint g_unichar_to_utf8 (guint32, char *);

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
gnomeprint_show_text (GnomePrintContext *pc, GnomeFont *font, 
		      gchar * text, gdouble mwidth)
{
  GSList * words;

  words = text_to_words ( text );
  gnomeprint_show_words ( pc, font, words, mwidth );
  g_slist_free (words);	  
}


void
gnomeprint_show_words(GnomePrintContext *pc, GnomeFont *font, GSList *words,
		      gdouble mwidth)
{
  gnomeprint_verifier_nouvelle_page ();
  show_words ( pc, font, words, point.x, point.y, mwidth );
}


void
show_words(GnomePrintContext *pc, GnomeFont *font, GSList *words, 
	   gdouble x, gdouble y, gdouble mwidth)
{
  ArtPoint spadv;
  gdouble accwidth = 0.0, max_accwidth = 0.0, fontheight;
  gint lines = 1;

  fontheight = gnome_font_get_ascender (font) + gnome_font_get_descender (font);

  gnome_print_gsave (pc);
  gnome_print_setfont (pc, font);
  gnome_print_setrgbcolor (pc, red, green, blue); 

  while (words != NULL)
    {
      gdouble width;
      GnomeGlyphList * gl;

      gl = gnome_glyphlist_from_text_dumb (font, color, 0.0, 0.0, "");
      gnome_font_get_glyph_stdadvance (font, 
				       gnome_font_face_lookup_default (gnome_font_get_face (font), ' '), 
				       &spadv);
      /* FIXME: gnome_font_get_width_string semble être confus à cause
	 des caractères accentués unicode. */
      width = gnome_font_get_width_string (font, (gchar *) words->data);

      if (accwidth > max_accwidth)
	max_accwidth = accwidth;

      if ( accwidth && ((accwidth+width) > mwidth))
	{
	  y-=(1.0*fontheight);
	  accwidth=0;
	  lines ++;
	}

      gnome_glyphlist_moveto (gl, x+accwidth, y-fontheight);

      if (words->next) accwidth += spadv.x; 
      accwidth += width;

      gnome_glyphlist_advance (gl, TRUE);
      gnome_glyphlist_text_dumb (gl, words->data);
      gnome_print_moveto (pc, 0.0, 0.0);
      gnome_print_glyphlist (pc, gl);
      gnome_glyphlist_unref (gl);
      words = words->next;
    }
  gnome_print_grestore (pc);

  tmp_point.x = point.x + max_accwidth;
  tmp_point.y = point.y - (lines * fontheight);

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
      gnome_glyphlist_moveto (gl, HMARGIN+accwidth, 100.0);

      width = gnome_font_get_width_string (font, (gchar *) words->data);
      if (words->next) accwidth += (spadv.x/2);
      accwidth += width;

      if (width > maxword)
	{
	  maxword = width;
	}

      printf ("%s, %f\n", (char *) words->data, width);

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


gint
update_columns (GnomePrintContext *pc, GnomeFont *font, gint column, gchar * text)
{
  GSList * words;
  gdouble min, max;

  words = text_to_words(text);
  text_get_min_max(pc, font, words, &min, &max);
  if (min > columns_min[column])
    {
      columns_min[column] = min;
    }
  columns_total[column] += max;
  g_slist_free (words);	  
  return max;
}


void
gnomeprint_balancer_colonnes (GnomePrintContext *pc, GnomeFont *font, 
			      GSList * list)
{
  int i, lines=0, column=0;
  gdouble total_text=0;
  gdouble total_width = PAPER_WIDTH-20;
  struct structure_operation *operation;
  GSList * list_pointeur;

  for (i=0; i<32; i++)
    columns_total[i] = 0;

#ifdef DEBUG
  gnome_print_beginpage (pc, "Glyph test page");
  gnome_print_setrgbcolor (pc, 0.0, 0.0, 0.0);
  gnome_print_setlinewidth (pc, 1.0);
#endif

  list_pointeur = list;
  while (list_pointeur)
    {
      column=0; /* Gruik! */
      operation = list_pointeur->data;
      if (etat_courant -> afficher_no_ope)
	{
	  total_text += update_columns (pc, font, column, g_strdup_printf ( "%d", operation -> no_operation ) );
	  column++;
	}
      if (etat_courant -> afficher_date_ope)
	{
	  total_text += update_columns (pc, font, column,
					g_strdup_printf  ( "%.2d/%.2d/%d",
							   operation -> jour,
							   operation -> mois,
							   operation -> annee ) ) ;
	  column++;
	}
      if (etat_courant -> afficher_exo_ope &&
	  operation -> no_exercice)
	{
	  total_text += update_columns (pc, font, column,
					((struct struct_exercice *)
					 (g_slist_find_custom ( liste_struct_exercices,
								GINT_TO_POINTER ( operation -> no_exercice ),
								(GCompareFunc) recherche_exercice_par_no )->data)) -> nom_exercice);
	    column++;
	}
      if (etat_courant -> afficher_tiers_ope &&
	  operation -> tiers)
	{
	  total_text += update_columns (pc, font, column,
					((struct struct_tiers *)
					 (g_slist_find_custom ( liste_struct_tiers,
								GINT_TO_POINTER ( operation -> tiers ),
								(GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers);
	  column++;
	}
      if (etat_courant -> afficher_categ_ope)
	{
	  gchar * pointeur=NULL;
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
		pointeur = 
		  g_strconcat ( pointeur,
				" : ",
				((struct struct_sous_categ *)
				 (g_slist_find_custom ( categ -> liste_sous_categ,
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

	  if (pointeur)
	    total_text += update_columns (pc, font, column,
					  pointeur);
	  column++;
	}
      if (etat_courant -> afficher_ib_ope)
	{
	  gchar *pointeur = NULL;
	  if ( operation -> imputation )
	    {
	      struct struct_imputation *ib;

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

	    }
	  if (pointeur)
	    total_text += update_columns (pc, font, column, pointeur);
	  column++;
	}
      if (etat_courant -> afficher_notes_ope)
	{
	  if ( operation -> notes )
	    total_text += update_columns (pc, font, column, operation -> notes);
	  column++;
	}
      if (etat_courant -> afficher_type_ope)
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
	      total_text += update_columns (pc, font, column, type -> nom_type);
	    }

	  column++;
	}
      if (etat_courant -> afficher_cheque_ope)
	{
	  
	  if ( operation -> contenu_type )
	    {
	      total_text += update_columns (pc, font, column, operation -> contenu_type);
	    }

	  column++;
	}
      if (etat_courant -> afficher_pc_ope)
	{
	  if ( operation -> no_piece_comptable )
	    {
	      total_text += update_columns (pc, font, column, operation -> no_piece_comptable);
	    }
	  column++;
	}
      if (etat_courant -> afficher_infobd_ope)
	{
	  if ( operation -> info_banque_guichet )
	    {
	      total_text += update_columns (pc, font, column, operation -> info_banque_guichet);
	    }
	  column++;
	}
      if (etat_courant -> afficher_rappr_ope)
	{
	  GSList *pointeur;

	  pointeur = g_slist_find_custom ( liste_no_rapprochements,
					   GINT_TO_POINTER ( operation -> no_rapprochement ),
					   (GCompareFunc) recherche_no_rapprochement_par_no );

	  if ( pointeur )
	    {
	      struct struct_no_rapprochement *rapprochement;
	      rapprochement = pointeur -> data;
	      total_text += update_columns (pc, font, column, rapprochement -> nom_rapprochement);
	    }
	  column++;
	}

      if ( devise_compte_en_cours_etat
	   &&
	   operation -> devise == devise_compte_en_cours_etat -> no_devise )
	total_text += update_columns (pc, font, column, g_strdup_printf  ("%4.2f %s",
									  operation -> montant,
									  devise_compte_en_cours_etat -> code_devise ));
      else
	{
	  struct struct_devise *devise_operation;
	  
	  devise_operation = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( operation -> devise ),
						   ( GCompareFunc ) recherche_devise_par_no ) -> data;
	  total_text += update_columns (pc, font, column, 
					g_strdup_printf  ("%4.2f %s",
							  operation -> montant,
							  devise_operation -> code_devise ));
	}
      column++;
     
      lines++;
      list_pointeur = list_pointeur->next;
    }

#ifdef DEBUG
  gnome_print_setlinewidth (pc, 1.0);
  gnome_print_moveto (pc, 10, 800);
  gnome_print_lineto (pc, 10, 200);
  gnome_print_lineto (pc, total_width+10, 200);
  gnome_print_lineto (pc, total_width+10, 800);
  gnome_print_closepath (pc);
  gnome_print_stroke (pc);
#endif

  for (i=0; i<column; i++)
    {
#ifdef DEBUG
      printf ("?? %d, %f, %f\n", i, total_width, total_text);
#endif
      if (((columns_total[i]/total_text) * total_width) <
	  columns_min[i])
	{
#ifdef DEBUG
	  printf (">> %d, %f, %f\n", i, columns_min[i], ((columns_total[i]/total_text) * total_width));
#endif
	  columns_max[i] = columns_min[i];
	  total_width -= columns_min[i];
	  total_text -= columns_total[i];
	}
      else
	{
#ifdef DEBUG
	  printf ("<< %d, %f, %f\n", i, columns_min[i], ((columns_total[i]/total_text) * total_width));
#endif
	  columns_max[i]=0;
	}
    }

  for (i=0; i<column; i++)
    {
      if (! columns_max[i])
	{
	  columns_max[i] = ((columns_total[i]/total_text) * total_width);
#ifdef DEBUG
	  printf ("++ %d, %f, %f, %f\n", i, columns_max[i], columns_total[i], total_text);
#endif
	}
      else
	{
#ifdef DEBUG
	  printf ("-- %d, %f, %f, %f\n", i, columns_max[i], columns_total[i], total_text);
#endif
	}
      if (i > 0)
	{
	  columns_pos[i] = columns_pos[i-1] + columns_max[i-1];
#ifdef DEBUG
	  printf ("** %d, %f, %f\n", i, columns_pos[i-1], columns_pos[i]);
#endif
	}
#ifdef DEBUG
      gnome_print_gsave (pc);
      gnome_print_setlinewidth (pc, 1);
      gnome_print_moveto (pc, columns_pos[i]+10, 800);
      gnome_print_lineto (pc, columns_pos[i]+10, 200);
      gnome_print_stroke (pc);
      gnome_print_grestore (pc);
#endif
    }

#ifdef DEBUG
  gnome_print_showpage (pc);
#endif
}

/*****************************************************************************************************/
/* passe à la page suivante si nécessaire */
/*****************************************************************************************************/
void gnomeprint_verifier_nouvelle_page ( void )
{
  if ( point.y < 40 )
    {
      gnome_print_showpage ( pc );
      gnome_print_beginpage ( pc, g_strdup_printf ("%d", ++page) );
/*       point.x = point_x = HMARGIN; */
      point.y = point.y = PAPER_HEIGHT - VMARGIN;
    }
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* passe à la ligne suivante si nécessaire */
/*****************************************************************************************************/
void gnomeprint_verifier_nouvelle_ligne ( int height, int margin )
{
  if ( tmp_x > (PAPER_WIDTH - margin - HMARGIN))
    {
      tmp_y += height;
      tmp_x = HMARGIN;
      gnomeprint_commit_point ( );
      if (num_ope % 2)
	{
	  gnomeprint_rectangle ( HMARGIN, 
				 tmp_y,
				 PAPER_WIDTH-HMARGIN, 
				 tmp_y + height);
	}
    }
}
/*****************************************************************************************************/

/*****************************************************************************************************/
/* modifie la couleur utilisée pour l'affichage */
/*****************************************************************************************************/
void gnomeprint_set_color ( gint tred, gint tgreen, gint tblue )
{
/*   red = tred/256; */
/*   green = tgreen/256; */
/*   blue = tblue/256; */

  color = 0x000000ff |
    tred << 24 |
    tgreen << 16 |
    tblue << 8;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* déplace le point de référence d'un certain nombre de millimètres */
/*****************************************************************************************************/
void gnomeprint_move_point ( gfloat x, gfloat y )
{
  point_x = tmp_x + x;
  point_y = tmp_y + y;
/*   point_x = tmp_x + gnome_paper_convert_to_points(x, gnome_unit_with_name ("Millimeter")); */
/*   point_y = tmp_y + gnome_paper_convert_to_points(y, gnome_unit_with_name ("Millimeter")); */
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


void gnomeprint_set_pointer ( gint x, gint y )
{
  point.x = x;
  point.y = y;
}


void gnomeprint_update_pointer_x ( d )
{
  point.x = tmp_point.x;
}


void gnomeprint_update_pointer_y ( )
{
  point.y = tmp_point.y;
}


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

  gnome_print_gsave (pc);
  gnome_print_setlinewidth (pc, 2);
  point.x = HMARGIN;
  point.y -= 5;
  tmp_point.y = point.y;
  gnome_print_moveto (pc, point.x, point.y);
  gnome_print_lineto (pc, 
		      PAPER_WIDTH - HMARGIN, 
		      point.y); 
  gnome_print_stroke (pc);
  gnome_print_grestore (pc);

/*   point.y -= 5;  */

  return ligne + 1;
}






/*****************************************************************************************************/
/* affiche le titre sur la première page du rapport */
/*****************************************************************************************************/
gint gnomeprint_affiche_titre ( gint ligne )
{
  gchar *titre;

  gnome_print_beginpage ( pc, g_strdup_printf ( "%d", ++page ) );
  gnomeprint_set_color ( 255, 0, 0 );

  titre = etats_titre () ;
  
  gnomeprint_show_text ( pc, title_font, titre, PAPER_WIDTH );

  gnomeprint_update_pointer_y ( );
  gnomeprint_set_color ( 0, 0, 0 );

  return 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
/* affiche le total à l'endroit donné de la table */
/* si les catégories sont affichées */
/* retourne le ligne suivante de la table */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_categories ( gint ligne )
{
  gchar * pointeur_char;

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
/* 	      gint i; */
/* 	      gint colonne; */

/* 	      colonne = 2; */

/* 	      for ( i=0 ; i<((nb_colonnes-2)/2) ; i++ ) */
/* 		{ */
		  gnomeprint_affiche_separateur ( ligne );
/* 		  colonne = colonne + 2; */
/* 		} */
	      ligne_debut_partie = -1;
	    }

	  /* Ligne suivante ? */
	  ligne++;

	  if ( nom_categ_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		pointeur_char = g_strdup_printf ( _("Total %s (%d transactions)"),
						  nom_categ_en_cours,
						  nb_ope_categ_etat);
	      else
		pointeur_char =  g_strconcat ( _("Total "),
					       nom_categ_en_cours,
					       NULL );
	    }
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		pointeur_char = g_strconcat ( _("Category total"),
					      "( ",
					      itoa ( nb_ope_categ_etat ),
					      _("operations ): "),
					      NULL );
	      else
		pointeur_char = _("Category total: ");
	    }

	  point.x = 400;	/* FIXME: trouver le bon alignement */
	  gnomeprint_show_text (pc, text_font, pointeur_char, PAPER_WIDTH-point.x);
	  gnomeprint_update_pointer_x ();

	  pointeur_char = g_strdup_printf ( "%4.2f %s",
					    montant_categ_etat,
					    devise_categ_etat -> code_devise );
	  gnomeprint_show_text (pc, text_font, pointeur_char, PAPER_WIDTH-point.x);
	  gnomeprint_update_pointer_y ();
	  point.x = HMARGIN;

	  ligne++;

	}
      else
	{
	  ligne--;

	  if ( etat_courant -> afficher_nb_opes )
	    pointeur_char = g_strdup_printf ( "%4.2f %s ( %d opérations )",
					      montant_categ_etat,
					      devise_categ_etat -> code_devise,
					      nb_ope_categ_etat );
	  else
	    pointeur_char = g_strdup_printf ( "%4.2f %s",
					      montant_categ_etat,
					      devise_categ_etat -> code_devise );

	  point.x = 400;	/* FIXME: trouver le bon alignement */
	  gnomeprint_show_text (pc, text_font, pointeur_char, PAPER_WIDTH-point.x);
	  gnomeprint_update_pointer_y ();
	  point.x = HMARGIN;

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
gint gnomeprint_affiche_total_sous_categ ( gint ligne )
{
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
	      
	  ligne++;
	  
	  gnomeprint_move_point ( HMARGIN, 0 );

	  ligne++;

	  if ( nom_ib_en_cours )
	    gnomeprint_affiche_texte ( g_strdup_printf ( _("Total %s"),
							 nom_ib_en_cours ),
				       text_font );
	  else
	    gnomeprint_affiche_texte ( COLON(_("Budgetary lines total")),
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
  nb_ope_ib_etat = 0;

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
  nb_ope_sous_ib_etat = 0;

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
  nb_ope_compte_etat = 0;

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
  gchar * pointeur_char;

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
	      gnomeprint_affiche_separateur ( ligne );
	      gnomeprint_update_pointer_y ( );
	      ligne++;
	    }

	  if ( nom_tiers_en_cours )
	    {
	      if ( etat_courant -> afficher_nb_opes )
		pointeur_char = g_strdup_printf ( _("Total %s (%d transactions)"),
						  nom_tiers_en_cours,
						  nb_ope_tiers_etat );
	      else
		pointeur_char = g_strconcat ( _("Total "),
					      nom_tiers_en_cours,
					      NULL );
	    }
	  else
	    {
	      if ( etat_courant -> afficher_nb_opes )
		pointeur_char = g_strconcat ( _("Third party total: "),
					      "( ",
					      itoa ( nb_ope_tiers_etat ),
					      _("operations ): "),
					      NULL );
	      else
		pointeur_char = _("Third party total: ") ;
	    }

	  gnomeprint_show_text ( pc, total_font, pointeur_char, 400 );

	  point.x = 400;
	  gnomeprint_show_text ( pc, header_font, 
				 g_strdup_printf ( "%4.2f %s",
						   montant_tiers_etat,
						   devise_tiers_etat -> code_devise ),
				 PAPER_WIDTH - point.x );
	  ligne++;
	  gnomeprint_update_pointer_y ( );
	  point.y -= 10;
	  point.x = HMARGIN;
	}
      else
	{
	  ligne--;

	  point.x = 400;		/*  FIXME: trouver le bon alignement */

	  if ( etat_courant -> afficher_nb_opes )
	    pointeur_char = g_strdup_printf ( "%4.2f %s ( %d opérations )",
					      montant_tiers_etat,
					      devise_tiers_etat -> code_devise,
					      nb_ope_tiers_etat );
	  else
	    pointeur_char = g_strdup_printf ( "%4.2f %s",
					      montant_tiers_etat,
					      devise_tiers_etat -> code_devise );

	  gnomeprint_show_text ( pc, header_font, pointeur_char, PAPER_WIDTH - point.x );
	  gnomeprint_update_pointer_y ( );
	  point.x = HMARGIN;

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
gint gnomeprint_affichage_ligne_ope ( struct structure_operation *operation,
				      gint ligne )
{
  int column=0;
  GSList * words = NULL;

  if ( etat_courant -> afficher_opes )
    {
      gint max_y = point.y;

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

      /* On affiche l'opération elle-même */

      if (etat_courant -> afficher_no_ope)
	{
	  words = text_to_words( g_strdup_printf ( "%d", operation -> no_operation ) );
	  gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	  gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
          if (tmp_point.y < max_y) max_y = tmp_point.y;
	  column++;
	}
      if (etat_courant -> afficher_date_ope)
	{
	  words = text_to_words ( g_strdup_printf  ( "%.2d/%.2d/%d",
						     operation -> jour,
						     operation -> mois,
						     operation -> annee ) ) ;
	  gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	  gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
          if (tmp_point.y < max_y) max_y = tmp_point.y;
	  column++;
	}
      if (etat_courant -> afficher_exo_ope)
	{
	  words = text_to_words(((struct struct_exercice *)
				 (g_slist_find_custom ( liste_struct_exercices,
							GINT_TO_POINTER ( operation -> no_exercice ),
							(GCompareFunc) recherche_exercice_par_no )->data)) -> nom_exercice);
	  gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	  gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
          if (tmp_point.y < max_y) max_y = tmp_point.y;
	  column++;
	}

      if (etat_courant -> afficher_tiers_ope)
	{
	  if ( operation -> tiers )
	    {
	      words = text_to_words( ((struct struct_tiers *)
				      (g_slist_find_custom ( liste_struct_tiers,
							     GINT_TO_POINTER ( operation -> tiers ),
							     (GCompareFunc) recherche_tiers_par_no )->data)) -> nom_tiers);
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;
	    }
	  column++;
	}

      if (etat_courant -> afficher_categ_ope)
	{
	  gchar * pointeur=NULL;

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

	  if (pointeur)
	    {
	      words = text_to_words( pointeur );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;

	    }
	  
	  column++;
	}

      if (etat_courant -> afficher_ib_ope)
	{
	  gchar *pointeur = NULL;
	  if ( operation -> imputation )
	    {
	      struct struct_imputation *ib;

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

	    }
	  if (pointeur)
	    {
	      words = text_to_words( pointeur );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;

	    }
	  column++;
	}
      if (etat_courant -> afficher_notes_ope)
	{
	  if ( operation -> notes )
	    {
	      words = text_to_words( operation -> notes );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;

	    }
	  column++;
	}
      if (etat_courant -> afficher_type_ope)
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
	      words = text_to_words( type -> nom_type );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;

	    }

	  column++;
	}
      if (etat_courant -> afficher_cheque_ope)
	{
	  
	  if ( operation -> contenu_type )
	    {
	      words = text_to_words ( operation -> contenu_type );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;


	    }

	  column++;
	}
      if (etat_courant -> afficher_pc_ope)
	{
	  if ( operation -> no_piece_comptable )
	    {
	      words = text_to_words ( operation -> no_piece_comptable );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;

	    }
	  column++;
	}
      if (etat_courant -> afficher_infobd_ope)
	{
	  if ( operation -> info_banque_guichet )
	    {
	      words = text_to_words ( operation -> info_banque_guichet );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;

	    }
	  column++;
	}
      if (etat_courant -> afficher_rappr_ope)
	{
	  GSList *pointeur;

	  pointeur = g_slist_find_custom ( liste_no_rapprochements,
					   GINT_TO_POINTER ( operation -> no_rapprochement ),
					   (GCompareFunc) recherche_no_rapprochement_par_no );

	  if ( pointeur )
	    {
	      struct struct_no_rapprochement *rapprochement;
	      rapprochement = pointeur -> data;
	      words = text_to_words ( rapprochement -> nom_rapprochement );
	      gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	      gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	      if (tmp_point.y < max_y) max_y = tmp_point.y;

	    }
	  column++;
	}

      if ( devise_compte_en_cours_etat
	   &&
	   operation -> devise == devise_compte_en_cours_etat -> no_devise )
	{
	  words = text_to_words ( g_strdup_printf  ("%4.2f %s",
						    operation -> montant,
						    devise_compte_en_cours_etat -> code_devise ));
	  gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	  gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
          if (tmp_point.y < max_y) max_y = tmp_point.y;
	}
      else
	{
	  struct struct_devise *devise_operation;
	  
	  devise_operation = g_slist_find_custom ( liste_struct_devises,
						   GINT_TO_POINTER ( operation -> devise ),
						   ( GCompareFunc ) recherche_devise_par_no ) -> data;
	  {
	    words = text_to_words ( g_strdup_printf  ("%4.2f %s",
						      operation -> montant,
						      devise_operation -> code_devise ));
	    gnomeprint_set_pointer ( HMARGIN+columns_pos[column], point.y );
	    gnomeprint_show_words ( pc, text_font, words, columns_max[column] );
	    if (tmp_point.y < max_y) max_y = tmp_point.y;

	  }
	}
           
      point.y = max_y;
    }

  tmp_x = HMARGIN;
  tmp_y = point.y;
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

  gnomeprint_affiche_separateur ( ligne );
  
  if ( type )
    {
      if ( etat_courant -> afficher_nb_opes )
	gnomeprint_show_text ( pc, total_font, 
			       g_strdup_printf ( _("Outgoings total (%d transactions): "),
						 nb_ope_partie_etat ), PAPER_WIDTH );
      else
	gnomeprint_show_text ( pc, total_font, _("Outgoings total: "), PAPER_WIDTH );
    }
  else
    {
      if ( etat_courant -> afficher_nb_opes )
	gnomeprint_show_text ( pc, total_font, 
			       g_strdup_printf ( _("Incomes total (%d transactions):"),
						 nb_ope_partie_etat ),
			       PAPER_WIDTH );
      else
	gnomeprint_show_text ( pc, total_font, _("Incomes total: "), PAPER_WIDTH );
    }

/*   gnomeprint_update_pointer_x ( ); */
  point.x = 400;		/*  FIXME: trouver le bon alignement */

  gnomeprint_show_text ( pc, total_font, 
			 g_strdup_printf ( "%4.2f %s",
					   total_partie,
					   devise_generale_etat -> code_devise ),
			 PAPER_WIDTH);
  gnomeprint_update_pointer_y ( );

  gnomeprint_affiche_separateur ( ligne );
  point.y -= 20;

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
/* affiche le total de la période à l'endroit donné de la table */
/* retourne le ligne suivante de la table */
/* si force = 0, vérifie les dates et affiche si nécessaire */
/*   si force = 1, affiche le total (chgt de categ, ib ...) */
/*****************************************************************************************************/
gint gnomeprint_affiche_total_periode ( struct structure_operation *operation,
					gint ligne,
					gint force )
{

}



/*****************************************************************************************************/
gint gnomeprint_affiche_total_general ( gdouble total_general,
			     gint ligne )
{
  gnomeprint_affiche_separateur ( ligne );

  if ( etat_courant -> afficher_nb_opes )
    gnomeprint_show_text ( pc, subtitle_font, 
			   g_strdup_printf ( _("General total (%d transactions): "),
					     nb_ope_general_etat ), 400 );
  else
    gnomeprint_show_text ( pc, subtitle_font, _("General total: "), 400 );

  point.x = 400;	/* FIXME: trouver le bon alignement */  
  gnomeprint_show_text ( pc, subtitle_font, 
			 g_strdup_printf ( "%4.2f %s",
					   total_general,
					   devise_generale_etat -> code_devise ), 
			 PAPER_WIDTH );
  gnomeprint_update_pointer_y ( );

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

	  gnomeprint_move_point ( 0, -2 );
	  gnomeprint_show_text ( pc, header_font, pointeur_char, 400 );

/* FIXME: be sure this is the right thing(tm) */
	  if (etat_courant -> afficher_opes)
	    gnomeprint_update_pointer_y ( );
/*       gnomeprint_move_point ( 0, -2 ); */

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
				      _("No budgetary line"),
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
  gchar *pointeur_char;

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

	  ligne = gnomeprint_affichage . affiche_total_periode ( operation,
								 ligne,
								 1 );

	  /* on ajoute les totaux de tout ce qu'il y a derrière le tiers */

	  ligne = gnomeprint_affichage . affiche_totaux_sous_jaccent ( 6,
	ligne );

	  /* on ajoute le total du tiers */

	  ligne = gnomeprint_affichage . affiche_total_tiers ( ligne );
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

	  gnomeprint_show_text ( pc, total_font, pointeur_char, 400);
	  gnomeprint_update_pointer_y ( );

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
gint gnomeprint_affiche_titre_revenus_etat ( gint ligne )
{
  gnomeprint_show_text ( pc, subtitle_font, _("Incomes"), PAPER_WIDTH );
  gnomeprint_update_pointer_y ( );
/*   gnomeprint_commit_y ( ); */
/*   gnomeprint_move_point ( 0, -5 ); */

  return ( ligne );
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gnomeprint_affiche_titre_depenses_etat ( gint ligne )
{
  gnomeprint_show_text ( pc, subtitle_font, _("Outgoings"), PAPER_WIDTH );
  gnomeprint_update_pointer_y ( );
/*   gnomeprint_move_point ( 0, -5 ); */

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
      gnomeprint_affiche_texte ( _("Financial year"), header_font );
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
      gnomeprint_affiche_texte ( _("Third party"), header_font );
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
      gnomeprint_affiche_texte ( _("Category"), header_font );
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
      gnomeprint_affiche_texte ( _("Budgetary line"), header_font );
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
      gnomeprint_affiche_texte ( _("Method of payment"), header_font );
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
      gnomeprint_affiche_texte ( _("Cheque"), header_font );
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
      gnomeprint_affiche_texte ( _("Voucher"), header_font );
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
      gnomeprint_affiche_texte ( _("Bank references"), header_font );
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
      gnomeprint_affiche_texte ( _("Statement"), header_font );
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
  gnomeprint_affiche_texte ( _("Amount"), header_font);
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
/* 		      PAPER_WIDTH-10,  */
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
					   _("Print Preview"));
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
