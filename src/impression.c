/* Fichier impression.c */
/* Contient toutes les procédures relatives à l'impression */

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

#include <gnome.h>
#include <libxml/encoding.h>
#include <sys/stat.h>

#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-print-pixbuf.h>
#include <libgnomeprint/gnome-font.h>
#include <libgnomeprint/gnome-printer-dialog.h>
#include <libgnomeprint/gnome-print-master.h>
#include <libgnomeprint/gnome-print-master-preview.h>
#include <libgnomeprint/gnome-print-dialog.h>

/* Prototypes */
gboolean imprimer_pc ( GnomePrintContext *pc, struct struct_etat *etat );
void ecrire_texte ( GnomePrintContext * pc, char * texte );

/* Globales ... à refaire je pense */
/* GnomeFont *title_font, *text_font; */
/* int point_x=10, point_y=600; */


int
myisolat1ToUTF8(unsigned char* out, int *outlen,
              const unsigned char* in, int *inlen) {
    unsigned char* outstart = out;
    const unsigned char* base = in;
    unsigned char* outend = out + *outlen;
    const unsigned char* inend;
    const unsigned char* instop;
    xmlChar c = *in;

    inend = in + (*inlen);
    instop = inend;
    
    while (in < inend && out < outend - 1) {
    	if (c >= 0x80) {
	    *out++= ((c >>  6) & 0x1F) | 0xC0;
            *out++= (c & 0x3F) | 0x80;
	    ++in;
	    c = *in;
	}
	if (instop - in > outend - out) instop = in + (outend - out); 
	while (c < 0x80 && in < instop) {
	    *out++ =  c;
	    ++in;
	    c = *in;
	}
    }	
    if (in < inend && out < outend && c < 0x80) {
        *out++ =  c;
	++in;
    }
    *outlen = out - outstart;
    *inlen = in - base;
    return(0);
}


char * latin2utf8 (char * inchar)
{
  char buffer[1024];
  int outlen, inlen, res;

  inlen = strlen(inchar);
  outlen = 1024;

  res = myisolat1ToUTF8(buffer, &outlen, inchar, &inlen);
  buffer[outlen] = 0;

  return (g_strdup ( buffer ));
}


gboolean imprimer_pc ( GnomePrintContext * pc, struct struct_etat *etat )
{
/*   int i; */

/*   gnome_print_beginpage (pc, "Impression grisbi"); */

/*   title_font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 1, 36); */
/*   text_font = gnome_font_new_closest ("Times", GNOME_FONT_BOLD, 1, 14); */

/*   p_tab_nom_de_compte_variable = p_tab_nom_de_compte; */

/*   for ( i=0 ; i<nb_comptes ; i++ ) */
/*     { */
/*       /\* on commence par vérifier que le compte fait partie de l'état *\/ */

/*       if ( !etat -> utilise_detail_comptes */
/* 	   || */
/* 	   g_slist_index ( etat -> no_comptes, */
/* 			   GINT_TO_POINTER ( i )) != -1 ) */
/* 	{ */
/* 	  /\* 	  le compte est bon, passe à la suite de la sélection *\/ */

/* 	  /\* on va faire le tour de toutes les opés du compte *\/ */

/* 	  GSList *pointeur_tmp; */

/* 	  pointeur_tmp = LISTE_OPERATIONS; */

/* 	  printf ("On imprime %s\n", NOM_DU_COMPTE); */
/* 	  ecrire_texte(pc, NOM_DU_COMPTE); */
/* 	} */
/*       p_tab_nom_de_compte_variable++; */
/*     } */

/*   gnome_print_showpage (pc); */

/*   /\*   gtk_object_unref (GTK_OBJECT (title_font)); *\/ */
/*   /\*   gtk_object_unref (GTK_OBJECT (text_font)); *\/ */
/*   /\*   gnome_print_context_close (pc); *\/ */

/*   return 1; */
}


void ecrire_texte(GnomePrintContext * pc, char * texte)
{
/*   gnome_print_gsave (pc); */
/*   gnome_print_setfont (pc, title_font); */
/*   gnome_print_setrgbcolor (pc, 1, 0, 0); */
/*   gnome_print_moveto (pc, point_x, point_y); */
/*   gnome_print_show (pc, latin2utf8(NOM_DU_COMPTE)); */
/*   gnome_print_grestore (pc); */
/*   point_x+=0; */
/*   point_y+=30; */
}

/*
gboolean impression_etat ( struct struct_etat *etat )
{
  GnomePrintContext *pc = NULL;
  GnomePrintMaster *gpm = NULL;
  
  GnomePrintDialog *gpd;
  static int copies=1, collate;
  int do_preview=0;
  

  if ( !etat )
    {
      if ( etat_courant )
	etat = etat_courant;
      else
	return 0;
    }

  gpd = GNOME_PRINT_DIALOG (gnome_print_dialog_new("Impression de Grisbi", GNOME_PRINT_DIALOG_COPIES));
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


  imprimer_pc(pc, etat);

  gnome_print_master_close(gpm);
  if (do_preview)
  {
      GnomePrintMasterPreview * pmp;
      pmp = gnome_print_master_preview_new(gpm, "Prévisualisation de l'impression de Grisbi");
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
*/
