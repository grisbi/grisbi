/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2025 Pierre Biava (grisbi@pierre.biava.name)                    */
/*          https://www.grisbi.org                                               */
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <glib/gstdio.h>
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "widget_search_tiers_categ_ib.h"
#include "dialog.h"
#include "etats_onglet.h"
#include "grisbi_app.h"
#include "gsb_data_category.h"
#include "gsb_data_currency.h"
#include "gsb_data_payee.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "navigation.h"
#include "structures.h"
#include "utils_prefs.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/

 /*START_EXTERN*/
/*END_EXTERN*/

typedef struct _WidgetSearchTiersCategIbPrivate   WidgetSearchTiersCategIbPrivate;

struct _WidgetSearchTiersCategIbPrivate
{
	GtkWidget *			vbox_search_tiers_categ_ib;

	GtkWidget *			box_action_area;
	GtkWidget *			box_search_amount;
	GtkWidget *			button_cancel;
	GtkWidget *			button_search;
	GtkWidget *			checkbutton_delta_amount;
	GtkWidget *			checkbutton_ignore_case;
	GtkWidget *			checkbutton_search_in_archive;
	GtkWidget *			checkbutton_search_save_report;
	GtkWidget *			entry_search_str;
	GtkWidget *			label_devise;
	GtkWidget *			radiobutton_search_amount;
	GtkWidget *			radiobutton_search_ope;
	GtkWidget *			spinbutton_delta_amount;

	gboolean			delta_amount;			/* VRAI : on ajoute une variation en € du montant */
	gboolean			ignore_case;
	gboolean			ignore_sign;
	gboolean			search_in_archive;		/* VRAI : on recherche aussi les données dans les archives */
	gboolean			search_save_report;		/* VRAI sauvegarde la recherche comme etat */

	gint				search_type;			/* 1 = text, 2 = amount */
	gint				page_num;				/* GSB_PAYEES_PAGE, GSB_CATEGORIES_PAGE, GSB_BUDGETARY_LINES_PAGE */

	gboolean			file_is_modified;
	glong				file_modified;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetSearchTiersCategIb, widget_search_tiers_categ_ib, GTK_TYPE_WINDOW)
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_checkbutton_delta_amount_toggled (GtkToggleButton *togglebutton,
																		   WidgetSearchTiersCategIbPrivate *priv)
{
	priv->delta_amount = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (priv->spinbutton_delta_amount, priv->delta_amount);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_checkbutton_ignore_case_sign_toggled (GtkToggleButton *togglebutton,
																			   WidgetSearchTiersCategIbPrivate *priv)
{
	devel_debug (NULL);
	if (priv->search_type == 2)
		priv->ignore_sign = gtk_toggle_button_get_active (togglebutton);
	else
		priv->ignore_case = gtk_toggle_button_get_active (togglebutton);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_checkbutton_search_in_archive_toggled (GtkToggleButton *togglebutton,
																				WidgetSearchTiersCategIbPrivate *priv)
{
	priv->search_in_archive = gtk_toggle_button_get_active (togglebutton);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_checkbutton_search_save_report_toggled (GtkToggleButton *togglebutton,
																				 WidgetSearchTiersCategIbPrivate *priv)
{
	priv->search_save_report = gtk_toggle_button_get_active (togglebutton);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_radiobutton_toggled (GtkWidget *radiobutton,
															  WidgetSearchTiersCategIbPrivate *priv)
{
	gint button_number;

	button_number = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (radiobutton), "pointer"));

	/* blocage du signal de callback du bouton priv->checkbutton_ignore_case */
	g_signal_handlers_block_by_func (G_OBJECT (priv->checkbutton_ignore_case),
									 G_CALLBACK (widget_search_tiers_categ_ib_checkbutton_ignore_case_sign_toggled),
									 priv);
	switch (button_number)
	{
		case 1:
			gtk_button_set_label (GTK_BUTTON (priv->checkbutton_ignore_case), _("Ignore case"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), priv->ignore_case);
			gtk_widget_set_sensitive (priv->box_search_amount, FALSE);
			priv->search_type = 1;
			break;
		case 2:
			gtk_button_set_label (GTK_BUTTON (priv->checkbutton_ignore_case), _("Ignore sign"));
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), priv->ignore_sign);
			gtk_widget_set_sensitive (priv->box_search_amount, TRUE);
			priv->search_type = 2;
			break;
	}

	/* deblocage du signal de callback du bouton priv->checkbutton_ignore_case */
	g_signal_handlers_unblock_by_func (G_OBJECT (priv->checkbutton_ignore_case),
									   G_CALLBACK (widget_search_tiers_categ_ib_checkbutton_ignore_case_sign_toggled),
									   priv);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_warning_msg (GtkWindow *parent,
													  const gchar *text,
													  const gchar *hint)
{
    GtkWidget *dialog;
	gchar *tmp_markup_str;

    tmp_markup_str = g_markup_printf_escaped ("<span size=\"larger\" weight=\"bold\">%s</span>\n\n", hint);

    dialog = gtk_message_dialog_new (GTK_WINDOW (grisbi_app_get_active_window (NULL)),
									 GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_WARNING,
									 GTK_BUTTONS_CLOSE,
									 NULL);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (dialog), tmp_markup_str);
	gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog), "%s", text);

    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	gtk_window_set_transient_for  (GTK_WINDOW (dialog), GTK_WINDOW (parent));
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_button_cancel_clicked (GtkButton *button,
																WidgetSearchTiersCategIb *dialog)
{
	GrisbiWinRun *w_run;

	w_run = grisbi_win_get_w_run ();
	w_run->no_show_prefs = FALSE;

	gtk_widget_destroy (GTK_WIDGET (dialog));
}

/**
 *
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_button_search_clicked (GtkButton *button,
																WidgetSearchTiersCategIb *dialog)
{
	const gchar *text = NULL;
	SearchDataReport *search_data_report;
	WidgetSearchTiersCategIbPrivate *priv;

	devel_debug (NULL);
	priv = widget_search_tiers_categ_ib_get_instance_private (dialog);

	/* regarde si on a un text avant toute chose */
	text = gtk_entry_get_text (GTK_ENTRY (priv->entry_search_str));
	if (priv->search_type == 1)
	{
		if (text && strlen (text) == 0)
		{
			gchar *tmp_str1;
			gchar *tmp_str2;

			tmp_str1 = g_strdup (_("You must enter a search text"));
			tmp_str2 = g_strdup (_("WARNING"));

			widget_search_tiers_categ_ib_warning_msg (GTK_WINDOW (dialog), tmp_str1, tmp_str2);
			g_free (tmp_str1);
			g_free (tmp_str2);

			return;
		}
	}
	else if (priv->search_type == 2)
	{
		GsbReal amount;

		amount = utils_real_get_from_string (text);
		if (amount.mantissa == error_real.mantissa)
		{
			gchar * string_amount = utils_real_get_string (amount);
			gboolean test = strlen (string_amount) != strlen (text); /* test impose par gsb_real_raw_get_from_string () */
			g_free(string_amount);
			if (test)
			{
				gchar *tmp_str1;
				gchar *tmp_str2;

				tmp_str1 = g_strdup (_("You must enter a valid amount"));
				tmp_str2 = g_strdup (_("WARNING"));

				widget_search_tiers_categ_ib_warning_msg (GTK_WINDOW (dialog), tmp_str1, tmp_str2);

				g_free (tmp_str1);
				g_free (tmp_str2);

				return;
			}
		}
	}

	/* init data for search report */
	search_data_report = g_new0 (SearchDataReport, 1);
	search_data_report->spin_value = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (priv->spinbutton_delta_amount));
	search_data_report->search_type = priv->search_type;
	search_data_report->page_num = priv->page_num;
	search_data_report->ignore_case = priv->ignore_case;
	search_data_report->ignore_sign = priv->ignore_sign;
	search_data_report->search_in_archive = priv->search_in_archive;
	search_data_report->search_delta_amount = priv->delta_amount;
	search_data_report->search_save_report = priv->search_save_report;

	/* add a new search report for tiers, categ ib from Ctrl F */
	etats_onglet_create_search_tiers_categ_ib_report (GTK_WINDOW (dialog) , text, search_data_report);
	g_free(search_data_report);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_entry_changed  (GtkWidget *entry,
														 WidgetSearchTiersCategIbPrivate *priv)
{
	guint length;

	length = gtk_entry_get_text_length (GTK_ENTRY (entry));
	if (length)
		gtk_widget_set_sensitive (priv->button_search, TRUE);
	else
		gtk_widget_set_sensitive (priv->button_search, FALSE);
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_search_tiers_categ_ib_entry_key_press_event  (GtkWidget *entry,
																	 GdkEventKey *ev,
																	 WidgetSearchTiersCategIb *dialog)
{
	WidgetSearchTiersCategIbPrivate *priv;

	priv = widget_search_tiers_categ_ib_get_instance_private (dialog);
	switch (ev->keyval)
    {
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			if (gtk_entry_get_text_length (GTK_ENTRY (entry)))
			{
				widget_search_tiers_categ_ib_button_search_clicked (NULL, dialog);
			}
			else
			{
				gchar *tmp_str1;
				gchar *tmp_str2;

				gtk_entry_set_text (GTK_ENTRY (entry), "");
				gtk_editable_set_position (GTK_EDITABLE (entry), 0);
				gtk_widget_grab_focus (entry);

				tmp_str1 = g_strdup (_("You must enter a search text"));
				tmp_str2 = g_strdup (_("WARNING"));

				dialogue_warning_hint (tmp_str1, tmp_str2);

				g_free (tmp_str1);
				g_free (tmp_str2);

				return TRUE;
			}
			break;

		default:
			if (gtk_entry_get_text_length (GTK_ENTRY (entry)))
				gtk_widget_set_sensitive (priv->button_search, TRUE);
			else
				gtk_widget_set_sensitive (priv->button_search, FALSE);
	}
	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_search_tiers_categ_ib_entry_lose_focus (GtkWidget *entry,
															   GdkEvent  *event,
															   WidgetSearchTiersCategIbPrivate *priv)
{
	if (gtk_entry_buffer_get_length (gtk_entry_get_buffer (GTK_ENTRY (entry))))
	{
		gtk_widget_set_sensitive (priv->button_search, TRUE);
	}
	else
	{
		gtk_widget_set_sensitive (priv->button_search, FALSE);
	}

	return FALSE;
}

/**
 *
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_search_tiers_categ_ib_spinbutton_delta_amount_key_press_event (GtkWidget *entry,
																					  GdkEventKey *ev,
																					  WidgetSearchTiersCategIb *dialog)
{
	switch (ev->keyval)
    {
		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			widget_search_tiers_categ_ib_button_search_clicked (NULL, dialog);
	}
	return FALSE;
}

/**
 * Création du widget search_tiers_categ_ib
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_search_tiers_categ_ib_setup_widget (WidgetSearchTiersCategIb *dialog,
													   gint page_num)
{
	gchar *search_title;
	const gchar *currency_code;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;
	WidgetSearchTiersCategIbPrivate *priv;

	devel_debug_int (page_num);
	priv = widget_search_tiers_categ_ib_get_instance_private (dialog);
	w_etat = grisbi_win_get_w_etat ();
	w_run = grisbi_win_get_w_run ();

	priv->page_num = page_num;
	priv->search_type = 1;

	/* set button box */
	gtk_button_box_set_layout (GTK_BUTTON_BOX (priv->box_action_area), GTK_BUTTONBOX_END);
	gtk_box_pack_end (GTK_BOX (priv->vbox_search_tiers_categ_ib), priv->box_action_area, FALSE, TRUE, MARGIN_BOX);

	gtk_box_pack_end (GTK_BOX (priv->box_action_area), priv->button_cancel, FALSE, TRUE, MARGIN_BOX);
	gtk_box_pack_end (GTK_BOX (priv->box_action_area), priv->button_search, FALSE, TRUE, MARGIN_BOX);

	/* set notebook page from page */
	switch (page_num)
	{
		case GSB_CATEGORIES_PAGE:
			search_title = g_strdup (_("Search in categories"));

			/* set currency */
			currency_code = gsb_data_currency_get_nickname_or_code_iso (w_etat->no_devise_totaux_categ);
			gtk_label_set_text (GTK_LABEL (priv->label_devise), currency_code);

			/* set ignore_case a TRUE pour categ et IB */
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), TRUE);
			priv->ignore_case = TRUE;
			break;

		case GSB_BUDGETARY_LINES_PAGE:
			search_title = g_strdup (_("Search in budgetary lines"));

			/* set currency */
			currency_code = gsb_data_currency_get_nickname_or_code_iso (w_etat->no_devise_totaux_ib);
			gtk_label_set_text (GTK_LABEL (priv->label_devise), currency_code);

			/* set ignore_case a TRUE pour categ et IB */
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), TRUE);
			priv->ignore_case = TRUE;
			break;

		case GSB_PAYEES_PAGE:
		default :
			search_title = g_strdup (_("Search in payees"));

			/* set currency */
			currency_code = gsb_data_currency_get_nickname_or_code_iso (w_etat->no_devise_totaux_tiers);
			gtk_label_set_text (GTK_LABEL (priv->label_devise), currency_code);

			/* init ignore_case comme dans les preferences du formulaire pour les tiers seulement*/
			if (w_etat->combofix_case_sensitive)
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), FALSE);
				priv->ignore_case = FALSE;
			}
			else
			{
				gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_ignore_case), TRUE);
				priv->ignore_case = TRUE;
			}
			break;
	}

	/*set title */
	gtk_window_set_title (GTK_WINDOW (dialog), search_title);
	g_free (search_title);

	/* set number for radiobutton */
    g_object_set_data (G_OBJECT (priv->radiobutton_search_ope), "pointer", GINT_TO_POINTER (1));
    g_object_set_data (G_OBJECT (priv->radiobutton_search_amount), "pointer", GINT_TO_POINTER (2));

	/* set search_in_archive */
	if (w_etat->metatree_add_archive_in_totals)
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_search_in_archive), TRUE);
		priv->search_in_archive = TRUE;
	}
	else
	{
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (priv->checkbutton_search_in_archive), FALSE);
		priv->search_in_archive = FALSE;
	}

	/* set bouton search */
	gtk_widget_set_sensitive (priv->button_search, FALSE);

	/* set no_show_prefs à TRUE => pas de preferences */
	w_run->no_show_prefs = TRUE;

	/* memorise le statut de modification */
	if (w_run->file_modification)
		priv->file_is_modified = TRUE;
	else
		priv->file_modified = FALSE;

	/* set signals */
	g_signal_connect (G_OBJECT (priv->button_cancel),
					  "clicked",
					  G_CALLBACK (widget_search_tiers_categ_ib_button_cancel_clicked),
					  dialog);

	g_signal_connect (G_OBJECT (priv->button_search),
					  "clicked",
					  G_CALLBACK (widget_search_tiers_categ_ib_button_search_clicked),
					  dialog);

	g_signal_connect (G_OBJECT (priv->checkbutton_ignore_case),
					  "toggled",
					  G_CALLBACK (widget_search_tiers_categ_ib_checkbutton_ignore_case_sign_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->checkbutton_search_in_archive),
					  "toggled",
					  G_CALLBACK (widget_search_tiers_categ_ib_checkbutton_search_in_archive_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->checkbutton_search_save_report),
					  "toggled",
					  G_CALLBACK (widget_search_tiers_categ_ib_checkbutton_search_save_report_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->checkbutton_delta_amount),
					  "toggled",
					  G_CALLBACK (widget_search_tiers_categ_ib_checkbutton_delta_amount_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->entry_search_str),
					  "search-changed",
					  G_CALLBACK (widget_search_tiers_categ_ib_entry_changed),
					  priv);

	g_signal_connect (G_OBJECT (priv->entry_search_str),
					  "focus-out-event",
					  G_CALLBACK (widget_search_tiers_categ_ib_entry_lose_focus),
					  priv);

	g_signal_connect (G_OBJECT (priv->entry_search_str),
					  "key-press-event",
					  G_CALLBACK (widget_search_tiers_categ_ib_entry_key_press_event),
					  dialog);

	g_signal_connect (G_OBJECT (priv->radiobutton_search_amount),
					  "toggled",
					  G_CALLBACK (widget_search_tiers_categ_ib_radiobutton_toggled),
					  priv);
	g_signal_connect (G_OBJECT (priv->radiobutton_search_ope),
					  "toggled",
					  G_CALLBACK (widget_search_tiers_categ_ib_radiobutton_toggled),
					  priv);

	g_signal_connect (G_OBJECT (priv->spinbutton_delta_amount),
					  "key-press-event",
					  G_CALLBACK (widget_search_tiers_categ_ib_spinbutton_delta_amount_key_press_event),
					  dialog);

	g_signal_connect (G_OBJECT (dialog),
					  "destroy",
					  G_CALLBACK (widget_search_tiers_categ_ib_button_cancel_clicked),
					  dialog);

}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_search_tiers_categ_ib_init (WidgetSearchTiersCategIb *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));
}

static void widget_search_tiers_categ_ib_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_search_tiers_categ_ib_parent_class)->dispose (object);
}

static void widget_search_tiers_categ_ib_class_init (WidgetSearchTiersCategIbClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_search_tiers_categ_ib_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_search_tiers_categ_ib.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, vbox_search_tiers_categ_ib);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, box_action_area);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, box_search_amount);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, button_cancel);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, button_search);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, checkbutton_delta_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, checkbutton_ignore_case);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, checkbutton_search_in_archive);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, checkbutton_search_save_report);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, entry_search_str);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, label_devise);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, radiobutton_search_amount);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, radiobutton_search_ope);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, spinbutton_delta_amount);

	//~ gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetSearchTiersCategIb, );
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
WidgetSearchTiersCategIb *widget_search_tiers_categ_ib_new (GtkWidget *win,
															gint page)
{
	gint height;
	gint width;
	gint root_x;
	gint root_y;
	GrisbiAppConf *a_conf;
	WidgetSearchTiersCategIb *dialog;

	dialog = g_object_new (WIDGET_SEARCH_TIERS_CATEG_IB_TYPE, NULL);

	/* set position of dialog */
	a_conf = grisbi_app_get_a_conf ();
    if (!a_conf->low_definition_screen)
	{
		gtk_window_set_gravity (GTK_WINDOW (dialog), GDK_GRAVITY_SOUTH_WEST);
		gtk_window_get_size (GTK_WINDOW (win), &width, &height);
		gtk_window_get_position (GTK_WINDOW (win),&root_x, &root_y);
		gtk_window_move (GTK_WINDOW (dialog), root_x - 50, height/3);
	}

	widget_search_tiers_categ_ib_setup_widget (dialog, page);
	gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (win));

	return dialog;
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

