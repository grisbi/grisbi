/* ComboFix Widget
 * Copyright (C) 2001 Cédric Auger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef __GTK_COMBOFIX__H__
#define __GTK_COMBOFIX__H__


#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
  
  
#define GTK_COMBOFIX(obj) GTK_CHECK_CAST(obj, gtk_combofix_get_type(), GtkComboFix )
#define GTK_COMBOFIX_CLASS(klass) GTK_CHECK_CLASS_CAST( klass, gtk_combofix_get_type(), GtkComboFixClass )
#define GTK_IS_COMBOFIX(obj) GTK_CHECK_TYPE ( obj, gtk_combofix_get_type() )
  
  
  typedef struct _GtkComboFix GtkComboFix;
  typedef struct _GtkComboFixClass GtkComboFixClass;


  /* stucture de base de la ComboFix */

  struct _GtkComboFix
  {
    GtkVBox vbox;

    /* adr de l'entrée */

    GtkWidget *entry;

    /*  adr de la liste contenant les labels */

    GSList *liste;

    /* à 1 si on veut que la liste s'affiche en tapant, à 0 sinon */

    gint affiche_liste;

    /* à 1 si le texte doit obligatoirement correspondre à un mot dans la liste, à 0 sinon  */

    gint force_text;


    /* variables privées */

    /* liste complexe */
    gint complex;
    /* adresse de la flèche  */
    GtkWidget *arrow; 
    /*   adresse de la fenetre de la liste de la combofix */
    GtkWidget *popup;
    /* adresse de la boite ( scrolledWindow ) contenant la liste */
    GtkWidget *box;
    /* liste des event_box */
    GSList *event_box;
    /* label sélectionné */
    gint label_selectionne;
    /* liste des complétions */
    GSList *liste_completion;
    /* automatiquement classé */
    gint auto_sort;

  };

  struct _GtkComboFixClass
  {
    GtkVBoxClass parent_class;
  };


  guint gtk_combofix_get_type ( void );


  /* **************************************************************************************************** */
  /* gtk_combofix_new : */
  /* arguments :             */
  /* liste :                      liste de noms ( \t en début de chaine pour une sous catégorie ) */
  /* force_text :            TRUE ( le texte doit correspondre à la liste ) / FALSE */
  /* affiche_liste :         TRUE ( la liste sera affichée en tapant le mot ) / FALSE */
  /* classement_auto : TRUE ( la liste est toujours classée par ordre alphabétique ) / FALSE  */
  /*                                                                                                                                      */
  /* retour : le widget gtk_combofix ( une hbox contenant l'entrée et d'une arrow ) */
  /* **************************************************************************************************** */

  GtkWidget *gtk_combofix_new ( GSList *liste,
				gint force_text,
				gint affiche_liste,
				gint classement_auto );

  /* **************************************************************************************************** */
  /* gtk_combofix_new_complex : */
  /* arguments :                             */
  /* liste :                      liste de liste de noms ( \t en début de chaine pour une sous catégorie ) */
  /*                               chaque liste sera séparée d'une ligne */
  /* force_text :            TRUE ( le texte doit correspondre à la liste ) / FALSE */
  /* affiche_liste :         TRUE ( la liste sera affichée en tapant le mot ) / FALSE */
  /* classement_auto : TRUE ( la liste est toujours classée par ordre alphabétique ) / FALSE  */
  /*                                                                                                                                      */
  /* retour : le widget gtk_combofix ( une hbox contenant l'entrée et d'une arrow ) */
  /* **************************************************************************************************** */
  
  GtkWidget *gtk_combofix_new_complex ( GSList *liste,
					gint force_text,
					gint affiche_liste,
					gint classement_auto );


  /* **************************************************************************************************** */
  /* fonction gtk_combofix_set_text */
  /* arguments : le combofix et le texte à mettre dedans */
  /* affiche le texte correspondant dans le combofix */
  /* **************************************************************************************************** */

  void gtk_combofix_set_text ( GtkComboFix *combofix,
			       gchar *text );


  /* **************************************************************************************************** */
  /* fonction gtk_combofix_get_text */
  /* arguments : le combofix  */
  /* retourne un pointeur vers le texte  dans le combofix */
  /* **************************************************************************************************** */

  gchar *gtk_combofix_get_text ( GtkComboFix *combofix );


  /* **************************************************************************************************** */
  /* Fonction gtk_combofix_force_text : */
  /* lorsqu'il est activé, l'utilisateur est obligé de taper un texte présent dans la liste */
  /* argument : la combofix */
  /*                     booleen TRUE ( force ) / FALSE */
  /* **************************************************************************************************** */

  void gtk_combofix_force_text ( GtkComboFix *combofix,
				 gint valeur );


  /* **************************************************************************************************** */
  /* Fonction gtk_combofix_view_list : */
  /* lorsqu'il est activé, une liste déroulante contenant les complétions possibles */
  /* s'affiche avec la frappe de l'utilisateur */
  /* argument :  la combofix */
  /*                      booleen TRUE ( met la liste ) / FALSE */
  /* **************************************************************************************************** */

  void gtk_combofix_view_list ( GtkComboFix *combofix,
				gint valeur );


  /* **************************************************************************************************** */
  /* Fonction gtk_combofix_show_arrow : */
  /* affiche ou non la flèche permettant d'afficher la liste */
  /* argument : la combofix */
  /*                     booleen TRUE ( affiche la flèche ) / FALSE */
  /* **************************************************************************************************** */

  void gtk_combofix_show_arrow ( GtkComboFix *combofix,
				 gint valeur );


  /* **************************************************************************************************** */
  /* Fonction gtk_combofix_sort : */
  /* classe la liste en respectant les différentes listes si c'est un complex, et en respectant les catég / sous-categ  */
  /* argument : la combofix */
  /* **************************************************************************************************** */

  void gtk_combofix_sort ( GtkComboFix *combofix );


  /* **************************************************************************************************** */
  /* Fonction gtk_combofix_set_list : */
  /* efface l'ancienne liste si elle existe et met la nouvelle  */
  /* arguments :                           */
  /*       combofix : la combofix          */
  /*       liste : une gslist              */
  /*       complex : TRUE : complex / FALSE */
  /*       classement_auto : TRUE / FALSE   */
  /* **************************************************************************************************** */

  void gtk_combofix_set_list ( GtkComboFix *combofix,
			       GSList *liste,
			       gint complex,
			       gint classement_auto );





#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* __GTK_COMBOFIX_H__ */
