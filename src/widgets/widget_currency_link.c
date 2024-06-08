/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          http://www.grisbi.org                                                */
/*                                                                               */
/*     This program is free software; you can redistribute it and/or modify      */
/*     it under the terms of the GNU General Public License as published by      */
/*     the Free Software Foundation; either version 2 of the License, or         */
/*     (at your option) any later version.                                       */
/*                                                                               */
/*     This program is distributed in the hope that it will be useful,           */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/*     GNU General Public License for more details.                              */
/*                                                                               */
/*     You should have received a copy of the GNU General Public License         */
/*     along with this program; if not, write to the Free Software               */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_currency_link.h"
#include "gsb_currency.h"
#include "gsb_data_currency_link.h"
#include "gsb_file.h"
#include "prefs_page_currency_link.h"
#include "structures.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetCurrencyLinkPrivate   WidgetCurrencyLinkPrivate;

struct _WidgetCurrencyLinkPrivate
{
	GtkWidget *			vbox_currency_link;

	GtkWidget *			box_currency_link;			/* sensibilise le widget si un lien existe */
	GtkWidget *			checkbutton_link_fixed;
	GtkWidget *			combobox_link_devise_1;
    GtkWidget *         combobox_link_devise_2;
    GtkWidget *			entry_link_exchange;
	GtkWidget *			label_link_warning;

};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetCurrencyLink, widget_currency_link, GTK_TYPE_BOX)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des currency_link
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_currency_link_setup_page (WidgetCurrencyLink *page,
											 PrefsPageCurrencyLink *page_link)
{
	WidgetCurrencyLinkPrivate *priv;

	devel_debug (NULL);
	priv = widget_currency_link_get_instance_private (page);
	gsb_currency_make_combobox_from_ui (priv->combobox_link_devise_1, TRUE);
	gsb_currency_make_combobox_from_ui (priv->combobox_link_devise_2, TRUE);

    g_signal_connect (G_OBJECT (priv->combobox_link_devise_1),
					  "changed",
					  G_CALLBACK (prefs_page_currency_link_widget_link_changed),
					  GTK_WIDGET (page_link));
	g_signal_connect (G_OBJECT (priv->combobox_link_devise_2),
					  "changed",
					  G_CALLBACK (prefs_page_currency_link_widget_link_changed),
					  GTK_WIDGET (page_link));

	g_signal_connect (G_OBJECT (priv->entry_link_exchange),
					  "changed",
					  G_CALLBACK (prefs_page_currency_link_widget_link_changed),
					  GTK_WIDGET (page_link));

	/* set checkbutton signal */
	g_signal_connect (G_OBJECT (priv->checkbutton_link_fixed),
					  "toggled",
					  G_CALLBACK (prefs_page_currency_link_checkbutton_fixed_changed),
					  GTK_WIDGET (page_link));

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_currency_link_init (WidgetCurrencyLink *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void widget_currency_link_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_currency_link_parent_class)->dispose (object);
}

static void widget_currency_link_class_init (WidgetCurrencyLinkClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_currency_link_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_currency_link.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyLink, vbox_currency_link);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyLink, box_currency_link);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyLink, checkbutton_link_fixed);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyLink, entry_link_exchange);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyLink, combobox_link_devise_1);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyLink, combobox_link_devise_2);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyLink, label_link_warning);
}

/******************************************************************************/
/* Public functions                                                           */
/******************************************************************************/
/**
 *
 *
 * \param
 *
 * \return
 **/
WidgetCurrencyLink *widget_currency_link_new (GtkWidget *page_link)
{
	WidgetCurrencyLink *page;

	page = g_object_new (WIDGET_CURRENCY_LINK_TYPE, NULL);
	widget_currency_link_setup_page (page, PREFS_PAGE_CURRENCY_LINK (page_link));

	return page;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *widget_currency_link_get_widget (GtkWidget *w_currency_link,
											const gchar *widget_name)
{
	WidgetCurrencyLinkPrivate *priv;

	priv = widget_currency_link_get_instance_private (WIDGET_CURRENCY_LINK (w_currency_link));

	if (g_strcmp0 (widget_name, "combo_1") == 0)
			return priv->combobox_link_devise_1;
	else if (g_strcmp0 (widget_name, "combo_2") == 0)
		return priv->combobox_link_devise_2;
	else if (g_strcmp0 (widget_name, "entry_exchange") == 0)
		return priv->entry_link_exchange;
	else if (g_strcmp0 (widget_name, "checkbutton_fixed") == 0)
		return priv->checkbutton_link_fixed;
	else if (g_strcmp0 (widget_name, "label_warning") == 0)
		return priv->label_link_warning;
	else
		return NULL;
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void widget_currency_link_set_sensitive (GtkWidget *w_currency_link,
										 gboolean sensitive)
{
	WidgetCurrencyLinkPrivate *priv;

	priv = widget_currency_link_get_instance_private (WIDGET_CURRENCY_LINK (w_currency_link));

	gtk_widget_set_sensitive (priv->box_currency_link, sensitive);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void widget_currency_details_update_link (gint link_number,
										  GtkWidget *w_currency_link)
{
	gchar *tmp_str;
	WidgetCurrencyLinkPrivate *priv;

	priv = widget_currency_link_get_instance_private (WIDGET_CURRENCY_LINK (w_currency_link));

	gsb_currency_set_combobox_history (priv->combobox_link_devise_1,
									   gsb_data_currency_link_get_first_currency (link_number));
	gsb_currency_set_combobox_history (priv->combobox_link_devise_2,
									   gsb_data_currency_link_get_second_currency (link_number));

	tmp_str = utils_real_get_string (gsb_data_currency_link_get_change_rate (link_number));
    gtk_entry_set_text (GTK_ENTRY (priv->entry_link_exchange), tmp_str);
    g_free (tmp_str);

	if (link_number)
	{
		gtk_widget_set_sensitive (priv->box_currency_link, TRUE);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_link_fixed),
									  gsb_data_currency_link_get_fixed_link (link_number));

		/* set or hide the warning label */
		if (gsb_data_currency_link_get_invalid_link (link_number))
		{
			gchar *markup;

			markup = g_strdup (gsb_data_currency_link_get_invalid_message (link_number));
			gtk_label_set_markup (GTK_LABEL (priv->label_link_warning), markup);
			g_free (markup);
			gtk_widget_show (priv->label_link_warning);
		}
		else
			gtk_widget_hide (priv->label_link_warning);
	}
	else
		gtk_widget_set_sensitive (priv->box_currency_link, FALSE);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
/* Local Variables: */
/* c-basic-offset: 4 */
/* End: */

