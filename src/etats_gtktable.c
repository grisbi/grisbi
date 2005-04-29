/*  Fichier qui s'occupe d'afficher les états via une gtktable */
/*      etats_gtktable.c */

/*     Copyright (C)	2000-2003 Cédric Auger (cedric@grisbi.org)	      */
/*			2004 Benjamin Drieu (bdrieu@april.org)		      */
/* 			http://www.grisbi.org				      */

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


/*START_INCLUDE*/
#include "etats_gtktable.h"
#include "barre_outils.h"
#include "gsb_account.h"
#include "operations_comptes.h"
#include "gsb_transaction_data.h"
#include "operations_liste.h"
#include "utils.h"
#include "utils_operations.h"
#include "structures.h"
#include "etats_config.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gtktable_attach_hsep ( int x, int x2, int y, int y2);;
static void gtktable_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			     enum alignement align, struct structure_operation * ope );;
static void gtktable_attach_vsep ( int x, int x2, int y, int y2);;
static void gtktable_click_sur_ope_etat ( struct structure_operation *operation );
static gint gtktable_finish ();;
static gint gtktable_initialise (GSList * opes_selectionnees);;
/*END_STATIC*/




gint gtktable_initialise (GSList * opes_selectionnees);
gint gtktable_finish ();
void gtktable_attach_hsep ( int x, int x2, int y, int y2);
void gtktable_attach_vsep ( int x, int x2, int y, int y2);
void gtktable_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			     enum alignement align, struct structure_operation * ope );
GtkWidget *table_etat;
struct struct_etat_affichage gtktable_affichage;

struct struct_etat_affichage gtktable_affichage = {
    gtktable_initialise,
    gtktable_finish,
    gtktable_attach_hsep,
    gtktable_attach_vsep,
    gtktable_attach_label,
};




/*START_EXTERN*/
extern gint nb_colonnes;
extern GtkWidget *scrolled_window_etat;
/*END_EXTERN*/



/**
 * Draw a label 
 *
 */
void gtktable_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			     enum alignement align, struct structure_operation * ope )
{
    GtkWidget * label;
    GtkStyle * style;

    if (!text)
    {
	text = "";
    }

    label = gtk_label_new ( text );
    gtk_label_set_line_wrap ( GTK_LABEL(label), TRUE );

    switch (align) 
    {
	case LEFT:
	    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
	    break;
	case CENTER:
	    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.5, 0.0 );
	    break;
	case RIGHT:
	    gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.0 );
	    break;
    }

    style = gtk_style_copy ( gtk_widget_get_style (label));

    if ( ope )
    {
	GtkWidget *event_box;
	GdkColor color;

	/* Put prelight */
	color.red =   1.00 * 65535 ;
	color.green = 0.00 * 65535 ;
	color.blue =  0.00 * 65535 ;
	style->fg[GTK_STATE_PRELIGHT] = color;

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
				    (GtkObject *) ope );
	gtk_table_attach ( GTK_TABLE ( table_etat ), event_box,
			   x, x2, y, y2,
			   GTK_SHRINK | GTK_FILL,
			   GTK_SHRINK | GTK_FILL,
			   0, 0 );
	gtk_widget_show ( event_box );
	gtk_container_add ( GTK_CONTAINER ( event_box ), label );
    }
    else
    {
	gtk_table_attach ( GTK_TABLE ( table_etat ), label,
			   x, x2, y, y2,
			   GTK_SHRINK | GTK_FILL,
			   GTK_SHRINK | GTK_FILL,
			   0, 0 );
    }

    if ( ((gint) properties) & TEXT_ITALIC)
	pango_font_description_set_style ( style -> font_desc, 
					   PANGO_STYLE_ITALIC );
    if ( ((gint) properties) & TEXT_BOLD)
	pango_font_description_set_weight ( style -> font_desc, 
					    PANGO_WEIGHT_BOLD );
    if ( ((gint) properties) & TEXT_HUGE )
	pango_font_description_set_size ( style -> font_desc, 
					  pango_font_description_get_size(style->font_desc) + 100 );
    if ( ((gint) properties) & TEXT_LARGE )
	pango_font_description_set_size ( style -> font_desc, 
					  pango_font_description_get_size(style->font_desc) + 2 );
    if ( ((gint) properties) & TEXT_SMALL )
	pango_font_description_set_size ( style -> font_desc, 
					  pango_font_description_get_size(style->font_desc) - 2 );

    gtk_widget_set_style ( label, style );
    gtk_widget_show ( label );
}



/**
 * FIXME: TODO
 *
 */
void gtktable_attach_vsep ( int x, int x2, int y, int y2)
{
    GtkWidget * separateur;

    separateur = gtk_vseparator_new ();
    gtk_table_attach ( GTK_TABLE ( table_etat ),
		       separateur,
		       x, x2, y, y2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( separateur );
}


/**
 * FIXME: TODO
 *
 */
void gtktable_attach_hsep ( int x, int x2, int y, int y2)
{
    GtkWidget * separateur;

    separateur = gtk_hseparator_new ();
    gtk_table_attach ( GTK_TABLE ( table_etat ),
		       separateur,
		       x, x2, y, y2,
		       GTK_SHRINK | GTK_FILL,
		       GTK_SHRINK | GTK_FILL,
		       0, 0 );
    gtk_widget_show ( separateur );
}



/*****************************************************************************************************/
gint gtktable_initialise (GSList * opes_selectionnees)
{
    /* on peut maintenant créer la table */
    /* pas besoin d'indiquer la hauteur, elle grandit automatiquement */

    if ( GTK_IS_WIDGET(table_etat) )
	gtk_widget_destroy (table_etat);

    if ( GTK_BIN ( scrolled_window_etat ) -> child )
	gtk_widget_hide ( GTK_BIN ( scrolled_window_etat ) -> child );

    table_etat = gtk_table_new ( 0, nb_colonnes, FALSE );
    gtk_table_set_col_spacings ( GTK_TABLE ( table_etat ), 5 );

    return 1;
}
/*****************************************************************************************************/


/*****************************************************************************************************/
gint gtktable_finish ()
{

    gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( scrolled_window_etat ), table_etat );
    gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( scrolled_window_etat ), GTK_SHADOW_NONE );

    gtk_container_set_border_width ( GTK_CONTAINER(table_etat), 6 );
    gtk_widget_show_all ( table_etat );

    if ( GTK_BIN ( scrolled_window_etat ) -> child )
	gtk_widget_show ( GTK_BIN ( scrolled_window_etat ) -> child );

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

    operation = operation_par_no ( operation -> no_operation_ventilee_associee,
				   gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (operation)));

    if ( operation )
    {
	/* passage sur le compte concerné */

	gsb_account_list_gui_change_current_account ( GINT_TO_POINTER ( gsb_transaction_data_get_account_number (gsb_transaction_data_get_transaction_number (operation))));

	/* récupération de la ligne de l'opé dans la liste ; affichage de toutes les opé si nécessaire */

	if ( gsb_transaction_data_get_marked_transaction ( gsb_transaction_data_get_transaction_number (operation )) == 3
	     &&
	     !gsb_account_get_r (gsb_account_get_current_account ()) )
	    change_aspect_liste ( 5 );

	gsb_transactions_list_set_current_transaction ( gsb_account_get_current_transaction (gsb_account_get_current_account ()),
							gsb_account_get_current_account () );
    }
}
/*****************************************************************************************************/

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
