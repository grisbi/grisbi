/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2020 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                                */
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
/*     along with this program; if not, see <https://www.gnu.org/licenses/>.     */
/*                                                                               */
/* *******************************************************************************/

#include "config.h"

#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_currency_details.h"
#include "gsb_autofunc.h"
#include "gsb_data_currency.h"
#include "gsb_file.h"
#include "prefs_page_currency.h"
#include "structures.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetCurrencyDetailsPrivate   WidgetCurrencyDetailsPrivate;

struct _WidgetCurrencyDetailsPrivate
{
	GtkWidget *			vbox_currency_details;

	GtkWidget *			entry_currency_floating_point;
	GtkWidget *			entry_currency_iso_code;
	GtkWidget *			entry_currency_name;
	GtkWidget *			entry_currency_nickname;
	GtkWidget *			grid_currency_details;

};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetCurrencyDetails, widget_currency_details, GTK_TYPE_BOX)

/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Création de la page de gestion des currency_details
 *
 * \param
 *
 * \return
 **/
static void widget_currency_details_setup_page (WidgetCurrencyDetails *page,
												PrefsPageCurrency *page_currency,
												gboolean no_callback)
{
    GtkSizeGroup *size_group;
	WidgetCurrencyDetailsPrivate *priv;

	devel_debug (NULL);
	priv = widget_currency_details_get_instance_private (page);
    size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

    /* Create entries */
	if (page_currency == NULL || no_callback)
	{
        priv->entry_currency_name = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_currency_nickname = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_currency_iso_code = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
        priv->entry_currency_floating_point = gsb_autofunc_entry_new (NULL, NULL, NULL, NULL, 0);
	}
    else
	{
		priv->entry_currency_name = gsb_autofunc_entry_new (NULL,
										G_CALLBACK (prefs_page_currency_entry_changed),
										page_currency,
										G_CALLBACK (gsb_data_currency_set_name),
										0);
		priv->entry_currency_nickname = gsb_autofunc_entry_new (NULL,
										G_CALLBACK (prefs_page_currency_entry_changed),
										page_currency,
										G_CALLBACK (gsb_data_currency_set_nickname),
										0);
		priv->entry_currency_iso_code = gsb_autofunc_entry_new (NULL,
										G_CALLBACK (prefs_page_currency_entry_changed),
										page_currency,
										G_CALLBACK (gsb_data_currency_set_code_iso4217),
										0);
		priv->entry_currency_floating_point = gsb_autofunc_int_new (0,
									  G_CALLBACK (prefs_page_currency_entry_changed),
									  page_currency,
									  G_CALLBACK (gsb_data_currency_set_floating_point),
									  0);
	}

	/* attach widgets */
	gtk_size_group_add_widget (size_group, priv->entry_currency_name);
    gtk_grid_attach (GTK_GRID (priv->grid_currency_details), priv->entry_currency_name, 2, 1, 1, 1);
	gtk_widget_set_hexpand (priv->entry_currency_name, TRUE);
	gtk_widget_show (priv->entry_currency_name);

    gtk_size_group_add_widget (size_group, priv->entry_currency_nickname);
    gtk_grid_attach (GTK_GRID (priv->grid_currency_details), priv->entry_currency_nickname, 2, 2, 1, 1);
	gtk_widget_show (priv->entry_currency_nickname);

	gtk_size_group_add_widget (size_group, priv->entry_currency_iso_code);
    gtk_grid_attach (GTK_GRID (priv->grid_currency_details), priv->entry_currency_iso_code, 2, 3, 1, 1);
	gtk_widget_show (priv->entry_currency_iso_code);

	gtk_size_group_add_widget (size_group, priv->entry_currency_floating_point);
    gtk_grid_attach (GTK_GRID (priv->grid_currency_details), priv->entry_currency_floating_point, 2, 4, 1, 1);
	gtk_widget_show (priv->entry_currency_floating_point);

    /* for now we want nothing in the entry of floating point */
    gsb_autofunc_int_erase_entry (priv->entry_currency_floating_point);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_currency_details_init (WidgetCurrencyDetails *page)
{
	gtk_widget_init_template (GTK_WIDGET (page));
}

static void widget_currency_details_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_currency_details_parent_class)->dispose (object);
}

static void widget_currency_details_class_init (WidgetCurrencyDetailsClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_currency_details_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_currency_details.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyDetails, vbox_currency_details);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetCurrencyDetails, grid_currency_details);
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
WidgetCurrencyDetails *widget_currency_details_new (GtkWidget *page_currency,
													gboolean no_callback)
{
	WidgetCurrencyDetails *w_currency_details;

	w_currency_details = g_object_new (WIDGET_CURRENCY_DETAILS_TYPE, NULL);
	widget_currency_details_setup_page (w_currency_details, PREFS_PAGE_CURRENCY (page_currency), no_callback);

  return w_currency_details;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *widget_currency_details_get_entry (GtkWidget *w_currency_details,
											  const gchar *entry_name)
{
	WidgetCurrencyDetailsPrivate *priv;

	priv = widget_currency_details_get_instance_private (WIDGET_CURRENCY_DETAILS (w_currency_details));

	if (g_strcmp0 (entry_name, "entry_name") == 0)
			return priv->entry_currency_name;
	else if (g_strcmp0 (entry_name, "entry_iso_code") == 0)
		return priv->entry_currency_iso_code;
	else if (g_strcmp0 (entry_name, "entry_nickname") == 0)
		return priv->entry_currency_nickname;
	else if (g_strcmp0 (entry_name, "entry_floating_point") == 0)
		return priv->entry_currency_floating_point;
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
void widget_currency_details_set_entry_editable (GtkWidget *w_currency_details,
												 gboolean editable)
{
	WidgetCurrencyDetailsPrivate *priv;

	priv = widget_currency_details_get_instance_private (WIDGET_CURRENCY_DETAILS (w_currency_details));

		gtk_editable_set_editable (GTK_EDITABLE (priv->entry_currency_name), editable);
		gtk_editable_set_editable (GTK_EDITABLE (priv->entry_currency_iso_code), editable);
		gtk_editable_set_editable (GTK_EDITABLE (priv->entry_currency_nickname), editable);
		gtk_editable_set_editable (GTK_EDITABLE (priv->entry_currency_floating_point), editable);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
void widget_currency_details_update_currency (gint currency_number,
											  GtkWidget *w_currency_details)
{
	WidgetCurrencyDetailsPrivate *priv;

	priv = widget_currency_details_get_instance_private (WIDGET_CURRENCY_DETAILS (w_currency_details));

	if (currency_number)
	{
		gsb_autofunc_entry_set_value (priv->entry_currency_name,
									  gsb_data_currency_get_name (currency_number),
									  currency_number);
		gsb_autofunc_entry_set_value (priv->entry_currency_iso_code,
									  gsb_data_currency_get_code_iso4217 (currency_number),
									  currency_number);
		gsb_autofunc_entry_set_value (priv->entry_currency_nickname,
									  gsb_data_currency_get_nickname (currency_number),
									  currency_number);
		gsb_autofunc_int_set_value (priv->entry_currency_floating_point,
									gsb_data_currency_get_floating_point (currency_number),
									currency_number);
	}
	else
	{
		gsb_autofunc_int_erase_entry (priv->entry_currency_name);
		gsb_autofunc_int_erase_entry (priv->entry_currency_iso_code);
		gsb_autofunc_int_erase_entry (priv->entry_currency_nickname);
		gsb_autofunc_int_erase_entry (priv->entry_currency_floating_point);
	}
}

/**
 * met à jour les détails d'une devise issue du tableau des devises
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
void widget_currency_details_update_from_iso_4217 (GtkWidget *w_currency_details,
												   const gchar *name,
												   const gchar *iso_code,
												   const gchar *nickname,
												   gint floating_point)
{
	gchar *tmp_str;
	WidgetCurrencyDetailsPrivate *priv;

	priv = widget_currency_details_get_instance_private (WIDGET_CURRENCY_DETAILS (w_currency_details));

    gtk_entry_set_text (GTK_ENTRY (priv->entry_currency_name), name);
    gtk_entry_set_text (GTK_ENTRY (priv->entry_currency_iso_code), iso_code);
    gtk_entry_set_text (GTK_ENTRY (priv->entry_currency_nickname), nickname);
    tmp_str = utils_str_itoa (floating_point);
    gtk_entry_set_text (GTK_ENTRY (priv->entry_currency_floating_point), tmp_str);
    g_free (tmp_str);


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

