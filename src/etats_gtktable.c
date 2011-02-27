/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*          http://www.grisbi.org                                             */
/*                                                                            */
/*  This program is free software; you can redistribute it and/or modify      */
/*  it under the terms of the GNU General Public License as published by      */
/*  the Free Software Foundation; either version 2 of the License, or         */
/*  (at your option) any later version.                                       */
/*                                                                            */
/*  This program is distributed in the hope that it will be useful,           */
/*  but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*  GNU General Public License for more details.                              */
/*                                                                            */
/*  You should have received a copy of the GNU General Public License         */
/*  along with this program; if not, write to the Free Software               */
/*  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                            */
/* ************************************************************************** */

/**
 * \file etats_gtktable.c
 * show the report in gtktable
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "include.h"


/*START_INCLUDE*/
#include "etats_gtktable.h"
#include "gsb_data_account.h"
#include "gsb_data_transaction.h"
#include "navigation.h"
#include "menu.h"
#include "gsb_transactions_list.h"
#include "utils.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "structures.h"
#include "fenetre_principale.h"
#include "etats_config.h"
#include "etats_affiche.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void gtktable_attach_hsep ( int x, int x2, int y, int y2);
static void gtktable_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			     enum alignement align, gint transaction_number );
static void gtktable_attach_vsep ( int x, int x2, int y, int y2);
static void gtktable_click_sur_ope_etat ( gint transaction_number );
static gint gtktable_finish ();
static gint gtktable_initialise ( GSList * opes_selectionnees, gchar * filename );
/*END_STATIC*/

GtkWidget *table_etat = NULL;
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
 * Attach a label at given positions.
 *
 * \param text			Text to display in label
 * \param properties		Bit mask of text properties
 * \param x			Left horizontal position
 * \param x2			Right horizontal position
 * \param y			Top vertical position
 * \param y2			Bottom vertical position
 * \param align			Horizonal align of the label
 * \param transaction_number	Number of the related transaction if
 *				label is part of a transaction.  Make
 *				an hyperlink if applicable
 */
void gtktable_attach_label ( gchar * text, gdouble properties, int x, int x2, int y, int y2, 
			     enum alignement align, gint transaction_number )
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
	case ALIGN_LEFT:
	    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.0, 0.0 );
	    break;
	case ALIGN_CENTER:
	    gtk_misc_set_alignment ( GTK_MISC ( label ), 0.5, 0.0 );
	    break;
	case ALIGN_RIGHT:
	    gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.0 );
	    break;
    }

    style = gtk_style_copy ( gtk_widget_get_style (label));

    if (transaction_number)
    {
	GtkWidget *event_box;
	GdkColor color;

	/* Put prelight */
	color.red =   1.00 * 65535 ;
	color.green = 0.00 * 65535 ;
	color.blue =  0.00 * 65535 ;
	color.pixel = 0;
	style->fg[GTK_STATE_PRELIGHT] = color;

	event_box = gtk_event_box_new ();
	g_signal_connect ( G_OBJECT ( event_box ),
			     "enter_notify_event",
			     G_CALLBACK ( met_en_prelight ),
			     NULL );
	g_signal_connect ( G_OBJECT ( event_box ),
			     "leave_notify_event",
			     G_CALLBACK ( met_en_normal ),
			     NULL );
	g_signal_connect_swapped ( G_OBJECT ( event_box ),
				    "button_press_event",
				    G_CALLBACK ( gtktable_click_sur_ope_etat ),
				    (GtkObject *) GINT_TO_POINTER (transaction_number) );
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
 * Attach a vertical separator at given positions.
 *
 * \param x		Left horizontal position
 * \param x2		Right horizontal position
 * \param y		Top vertical position
 * \param y2		Bottom vertical position
 *
 * Martin Stromberger, 28.08.2008:
 * changed xpadding from 0 to 2 for a better printlayout.
 * xpadding=0 -> no space between separator and text in printlayout.
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
		       2, 0 );
    gtk_widget_show ( separateur );
}


/**
 * Attach an horizontal separator at given positions.
 *
 * \param x		Left horizontal position
 * \param x2		Right horizontal position
 * \param y		Top vertical position
 * \param y2		Bottom vertical position
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
gint gtktable_initialise ( GSList * opes_selectionnees, gchar * filename )
{
    /* on peut maintenant créer la table */
    /* pas besoin d'indiquer la hauteur, elle grandit automatiquement */

    if ( table_etat && GTK_IS_TABLE(table_etat) )
	gtk_widget_destroy (table_etat);

    if ( scrolled_window_etat && GTK_BIN ( scrolled_window_etat ) -> child )
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

void gtktable_click_sur_ope_etat ( gint transaction_number )
{
    gint archive_number;
    gint account_number;

    account_number = gsb_data_transaction_get_account_number (transaction_number);

    /* if it's an archived transaction, open the archive */
    archive_number = gsb_data_transaction_get_archive_number (transaction_number);
    if (archive_number)
    {
	/* re-filter the tree view because if we go directly into the report
	 * and the model was never filtered, we have a nice crash */
	transaction_list_filter (account_number);
	gsb_transactions_list_restore_archive (archive_number, FALSE);
    }

    if ( transaction_number )
    {
	gint mother_transaction;

	/* go on the good account */
	gsb_gui_navigation_set_selection ( GSB_ACCOUNT_PAGE, 
					   account_number,
					   GINT_TO_POINTER (-1));

	/* récupération de la ligne de l'opé dans la liste ; affichage de toutes les opé si nécessaire */
	if ( gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE 
	     &&
	     !gsb_data_account_get_r ( account_number ) )
    {
        gsb_data_account_set_r ( account_number, TRUE );
        gsb_menu_update_view_menu ( account_number );
        mise_a_jour_affichage_r ( TRUE );
    }
	
	/* if it's a child, open the mother */
	mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);
	if (mother_transaction)
	    gsb_transactions_list_switch_expander (mother_transaction);

	transaction_list_select ( transaction_number );
    }
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
