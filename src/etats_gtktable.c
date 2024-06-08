/* ************************************************************************** */
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*          2004-2008 Benjamin Drieu (bdrieu@april.org)                       */
/*         2008-2021 Pierre Biava (grisbi@pierre.biava.name)                  */
/*          https://www.grisbi.org/                                           */
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


#include "config.h"

#include "include.h"


/*START_INCLUDE*/
#include "etats_gtktable.h"
#include "gsb_data_account.h"
#include "gsb_data_report.h"
#include "gsb_data_transaction.h"
#include "navigation.h"
#include "menu.h"
#include "gsb_transactions_list.h"
#include "utils.h"
#include "transaction_list.h"
#include "transaction_list_select.h"
#include "structures.h"
#include "etats_config.h"
#include "etats_affiche.h"
#include "utils_widgets.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static GtkWidget *table_etat = NULL;
/*END_STATIC*/

/*START_EXTERN*/
extern GtkWidget *scrolled_window_etat;
/*END_EXTERN*/

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * cette fonction est appelée si on click sur une opé dans un état
 * elle affiche la liste des opés sur cette opé
 *
 * \param
 *
 * \return
 **/
static void gtktable_click_sur_ope_etat (gint transaction_number)
{
	if (transaction_number)
	{
		gint account_number;
		gint archive_number;
		gint mother_transaction;
		gint report_number;

		account_number = gsb_data_transaction_get_account_number (transaction_number);

		/* go on the good account */
		gsb_gui_navigation_set_selection (GSB_ACCOUNT_PAGE, account_number, -1);

		/* if it's an archived transaction, open the archive */
		archive_number = gsb_data_transaction_get_archive_number (transaction_number);
		if (archive_number)
		{
			/* re-filter the tree view because if we go directly into the report
			 * and the model was never filtered, we have a nice crash */
			transaction_list_filter (account_number);
			gsb_transactions_list_restore_archive (archive_number, FALSE);
		}

		/* récupération de la ligne de l'opé dans la liste ; affichage de toutes les opé si nécessaire */
		if (gsb_data_transaction_get_marked_transaction (transaction_number) == OPERATION_RAPPROCHEE
			 && !gsb_data_account_get_r (account_number))
		{
			gsb_data_account_set_r (account_number, TRUE);
			gsb_menu_update_view_menu (account_number);
			gsb_transactions_list_mise_a_jour_affichage_r (TRUE);
		}

		/* if it's a child, open the mother */
		report_number = gsb_gui_navigation_get_current_report ();
		if (!gsb_data_report_get_not_detail_split (report_number))
		{
			mother_transaction = gsb_data_transaction_get_mother_transaction_number (transaction_number);
			if (mother_transaction)
				gsb_transactions_list_switch_expander (mother_transaction);
		}
		transaction_list_select (transaction_number);
	}
}

/**
 * Attach a label at given positions.
 *
 * \param text					Text to display in label
 * \param properties			Bit mask of text properties
 * \param x						Left horizontal position
 * \param x2					Right horizontal position
 * \param y						Top vertical position
 * \param y2					Bottom vertical position
 * \param align					Horizonal align of the label
 * \param transaction_number	Number of the related transaction if label is part of a transaction.
 *								Make an hyperlink if applicable
 *
 * \return
 **/
static void gtktable_attach_label (gchar *text,
								   gdouble properties,
								   int x, int x2, int y, int y2,
								   GtkJustification align,
								   gint transaction_number)
{
	GtkWidget *label;
	PangoContext *p_context;
	PangoFontDescription *font_desc;
	gint x_dim;
	gint y_dim;

	if (!text)
	{
		text = (gchar*)"";
	}

	label = gtk_label_new (text);
	gtk_label_set_line_wrap (GTK_LABEL(label), TRUE);

	switch (align)
	{
		case GTK_JUSTIFY_LEFT:
			utils_labels_set_alignment (GTK_LABEL (label), 0.0, 0.0);
			break;
		case GTK_JUSTIFY_CENTER:
			utils_labels_set_alignment (GTK_LABEL (label), 0.5, 0.0);
			break;
		case GTK_JUSTIFY_RIGHT:
			utils_labels_set_alignment (GTK_LABEL (label), 1.0, 0.0);
			break;
		case GTK_JUSTIFY_FILL:
			break;
	}

	if (transaction_number)
	{
		GtkWidget *event_box;
		GtkStyleContext *context;

		event_box = gtk_event_box_new ();
		gtk_widget_set_name (event_box, "etat_event_box");
		context = gtk_widget_get_style_context  (event_box);
		gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);
		g_signal_connect (G_OBJECT (event_box),
						  "enter_notify_event",
						  G_CALLBACK (utils_event_box_change_state),
						  context);
		g_signal_connect (G_OBJECT (event_box),
						  "leave_notify_event",
						  G_CALLBACK (utils_event_box_change_state),
						  context);
		g_signal_connect_swapped (G_OBJECT (event_box),
								  "button_press_event",
								  G_CALLBACK (gtktable_click_sur_ope_etat),
								  GINT_TO_POINTER (transaction_number));
		x_dim = x2 - x;
		y_dim = y2 - y;
		gtk_grid_attach (GTK_GRID (table_etat), event_box, x, y, x_dim, y_dim);
		g_object_set_data (G_OBJECT (event_box), "x_dim", GINT_TO_POINTER (x_dim));

		gtk_widget_show (event_box);
		gtk_container_add (GTK_CONTAINER (event_box), label);
	}
	else
	{
		x_dim = x2 - x;
		y_dim = y2 - y;
		gtk_grid_attach (GTK_GRID (table_etat), label, x, y, x_dim, y_dim);
		g_object_set_data (G_OBJECT (label), "x_dim", GINT_TO_POINTER (x_dim));
	}
	p_context = gtk_widget_get_pango_context (label);
	font_desc = pango_context_get_font_description (p_context);

	if (((gint) properties) & TEXT_ITALIC)
		pango_font_description_set_style (font_desc, PANGO_STYLE_ITALIC);
	if (((gint) properties) & TEXT_BOLD)
		pango_font_description_set_weight (font_desc, PANGO_WEIGHT_BOLD);
	if (((gint) properties) & TEXT_HUGE)
		pango_font_description_set_size (font_desc,
										 pango_font_description_get_size (font_desc) + 100);
	if (((gint) properties) & TEXT_LARGE)
		pango_font_description_set_size (font_desc,
										 pango_font_description_get_size (font_desc) + 2);
	if (((gint) properties) & TEXT_SMALL)
		pango_font_description_set_size (font_desc,
										 pango_font_description_get_size (font_desc) - 2);
	gtk_widget_show (label);
}

/**
 * Attach a vertical separator at given positions.
 *
 * \param x			Left horizontal position
 * \param x2		Right horizontal position
 * \param y			Top vertical position
 * \param y2		Bottom vertical position
 *
 * Martin Stromberger, 28.08.2008:
 * changed xpadding from 0 to 2 for a better printlayout.
 * xpadding=0->no space between separator and text in printlayout.
 *
 * \return
 **/
static void gtktable_attach_vsep (int x, int x2, int y, int y2)
{
	GtkWidget *separateur;
	gint x_dim;
	gint y_dim;

	separateur = gtk_separator_new (GTK_ORIENTATION_VERTICAL);
	x_dim = x2 - x;
	y_dim = y2 - y;
	gtk_grid_attach (GTK_GRID (table_etat), separateur, x, y, x_dim, y_dim);
	utils_widget_set_padding (separateur, 2,0);
	g_object_set_data (G_OBJECT (separateur), "type_separator", GINT_TO_POINTER (GTK_ORIENTATION_VERTICAL));

	gtk_widget_show (separateur);
}

/**
 * Attach an horizontal separator at given positions.
 *
 * \param x			Left horizontal position
 * \param x2		Right horizontal position
 * \param y			Top vertical position
 * \param y2		Bottom vertical position
 *
 * \return
 **/
static void gtktable_attach_hsep (int x, int x2, int y, int y2)
{
	GtkWidget *separateur;
	gint x_dim;
	gint y_dim;

	separateur = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	x_dim = x2 - x;
	y_dim = y2 - y;
	gtk_grid_attach (GTK_GRID (table_etat), separateur, x, y, x_dim, y_dim);
	g_object_set_data (G_OBJECT (separateur), "type_separator", GINT_TO_POINTER (GTK_ORIENTATION_HORIZONTAL));

	gtk_widget_show (separateur);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 *
 */
static gint gtktable_initialise (GSList *opes_selectionnees,
								 gchar *filename)
{
	/* on peut maintenant créer la table */
	/* pas besoin d'indiquer la hauteur, elle grandit automatiquement */

	if (table_etat && GTK_IS_GRID (table_etat))
		gtk_widget_destroy (table_etat);

	/* regarder la liberation de mémoire */
	if (scrolled_window_etat && gtk_bin_get_child (GTK_BIN (scrolled_window_etat)))
		gtk_widget_destroy (gtk_bin_get_child (GTK_BIN (scrolled_window_etat)));

	/* just update screen so that the user does not see the previous report anymore
	 * while we are processing the new report */
	update_gui ();

	table_etat = gtk_grid_new ();
	gtk_grid_set_column_spacing (GTK_GRID (table_etat), 5);

	return 1;
}

/**
 *
 *
 * \param
 *
 * \return
 *
 */
static gint gtktable_finish (void)
{
	gtk_container_add (GTK_CONTAINER (scrolled_window_etat), table_etat);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window_etat), GTK_SHADOW_NONE);

	gtk_container_set_border_width (GTK_CONTAINER (table_etat), 6);
	gtk_widget_show_all (table_etat);

	if (gtk_bin_get_child (GTK_BIN (scrolled_window_etat)))
		gtk_widget_show (gtk_bin_get_child (GTK_BIN (scrolled_window_etat)));

	return 1;
}

/*START_GLOBAL*/
struct EtatAffichage gtktable_affichage = {
	gtktable_initialise,
	gtktable_finish,
	gtktable_attach_hsep,
	gtktable_attach_vsep,
	gtktable_attach_label,
};
/* END_GLOBAL */

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 * Set table_etat = NULL
 *
 * \param
 *
 * \return
 **/
void etats_gtktable_free_table_etat (void)
{
	if (table_etat)
	{
		table_etat = NULL;
	}
}

/**
 *
 *
 * \param
 *
 * \return
 *
 */
GtkWidget *etats_gtktable_get_table_etat (void)
{
	return table_etat;
}

/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */
