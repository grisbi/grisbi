/* *******************************************************************************/
/*                                 GRISBI                                        */
/*              Programme de gestion financière personnelle                      */
/*                              license : GPLv2                                  */
/*                                                                               */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)               */
/*                      2003-2008 Benjamin Drieu (bdrieu@april.org)              */
/*          2008-2022 Pierre Biava (grisbi@pierre.biava.name)                    */
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
#include "widget_account_property.h"
#include "categories_onglet.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_account.h"
#include "gsb_autofunc.h"
#include "gsb_bank.h"
#include "gsb_category.h"
#include "gsb_currency.h"
#include "gsb_data_account.h"
#include "gsb_data_bank.h"
#include "gsb_data_currency.h"
#include "gsb_data_transaction.h"
#include "gsb_file.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_scheduler_list.h"
#include "imputation_budgetaire.h"
#include "navigation.h"
#include "structures.h"
#include "tiers_onglet.h"
#include "transaction_list.h"
#include "utils_buttons.h"
#include "utils_editables.h"
#include "utils_prefs.h"
#include "utils_str.h"
#include "erreur.h"

/*END_INCLUDE*/

/*START_EXTERN*/
/*END_EXTERN*/

typedef struct	_WidgetAccountPropertyPrivate		WidgetAccountPropertyPrivate;
typedef enum	_PropertyOrigin						PropertyOrigin;

struct _WidgetAccountPropertyPrivate
{
	GtkWidget *			vbox_account_property;

	GtkWidget *			bouton_icon;

	GtkWidget *			title_detail_compte;
	GtkWidget *			hbox_account_name;
	GtkWidget *			entry_account_name;
	GtkWidget *			hbox_account_detail;
	GtkWidget *			checkbutton_closed_account;
	GtkWidget *			combo_account_type;
	GtkWidget *			combo_account_currency;

	GtkWidget *			entry_holder_name;
	GtkWidget *			checkbutton_holder_address;
	GtkWidget *			textview_holder_address;

	GtkWidget *			combo_bank_list;
	GtkWidget *			button_edit_bank;
	GtkWidget *			entry_bic_code;
	GtkWidget *			hbox_avertissement_iban;
	GtkWidget *			entry_account_iban;
	GtkWidget *			hbox_bank_codes;
	GtkWidget *			entry_sort_code;
	GtkWidget *			entry_bank_branch_code;
	GtkWidget *			hbox_account_number_key;
	GtkWidget *			entry_bank_account_number;
	GtkWidget *			entry_bank_account_key;

	GtkWidget *			entry_initial_balance;
	GtkWidget *			entry_minimal_authorized;
	GtkWidget *			entry_minimal_desired;

	GtkWidget *			textview_comment_text;
};

G_DEFINE_TYPE_WITH_PRIVATE (WidgetAccountProperty, widget_account_property, GTK_TYPE_BOX)

enum PropertyOrigin
{

	PROPERTY_NAME = 0,
	PROPERTY_KIND,
	PROPERTY_CURRENCY,
	PROPERTY_CLOSED_ACCOUNT,
	PROPERTY_INIT_BALANCE,
	PROPERTY_WANTED_BALANCE,
	PROPERTY_HOLDER_NAME
};

struct Iso13616Iban
{
	const gchar *	locale;
	const gchar *	iban;
	gint			nbre_char;
	gint			nbre_space;
};

/*START_STATIC*/
static void widget_account_property_iban_set_iban (const gchar *iban,
												   WidgetAccountPropertyPrivate *priv);
static struct Iso13616Iban iso_13616_ibans [] = {
	{ "XX", "XXkk XXXX XXXX XXXX XXXX XXXX XXXX XXXX XX", 34, 8},
	{ "AD", "ADkk BBBB SSSS CCCC CCCC CCCC", 24, 5},
	{ "AT", "ATkk BBBB BCCC CCCC CCCC", 20, 4},
	{ "BA", "BAkk BBBS SSCC CCCC CCKK", 20, 4},
	{ "BE", "BEkk BBBC CCCC CCKK", 16, 3},
	{ "BG", "BGkk BBBB SSSS DDCC CCCC CC", 22, 5},
	{ "CH", "CHkk BBBB BCCC CCCC CCCC C", 21, 5},
	{ "CY", "CYkk BBBS SSSS CCCC CCCC CCCC CCCC", 28, 6},
	{ "CZ", "CZkk BBBB CCCC CCCC CCCC CCCC", 24, 5},
	{ "DE", "DEkk BBBB BBBB CCCC CCCC CC", 22, 5},
	{ "DK", "DKkk BBBB CCCC CCCC CC", 18, 4},
	{ "EE", "EEkk BBBB CCCC CCCC CCCK", 20, 4},
	{ "ES", "ESkk BBBB GGGG KKCC CCCC CCCC", 24, 5},
	{ "FI", "FIkk BBBB BBCC CCCC CK", 18, 4},
	{ "FO", "FOkk CCCC CCCC CCCC CC", 18, 4},
	{ "FR", "FRkk BBBB BGGG GGCC CCCC CCCC CKK", 27, 6},
	{ "GB", "GBkk BBBB SSSS SSCC CCCC CC", 22, 5},
	{ "GI", "GIkk BBBB CCCC CCCC CCCC CCC", 23, 5},
	{ "GR", "GRkk BBB BBBB CCCC CCCC CCCC CCCC", 27, 6},
	{ "HR", "HRkk BBBB BBBC CCCC CCCC C", 21, 5},
	{ "IE", "IEkk AAAA BBBB BBCC CCCC CC", 22, 5},
	{ "LU", "LUkk BBBC CCCC CCCC CCCC", 20, 4},
	{ "NL", "NLkk BBBB CCCC CCCC CC", 18, 4},
	{ "PT", "PTkk BBBB BBBB CCCC CCCC CCCK K", 25, 5},
	{ "SE", "SEkk BBBB CCCC CCCC CCCC CCCC", 24, 5},
	{ NULL, NULL, 0, 0},
};

/*END_STATIC*/
/******************************************************************************/
/* Private functions                                                          */
/******************************************************************************/
/**
 * Affiche ou cache les données bancaires en fonction de la présence
 * du N°IBAN
 *
 * \param show Si TRUE affiche les données classiques si FALSE les cache
 * \param
 *
 * \return
 **/
static void widget_account_property_bank_data_show (gboolean show,
														 WidgetAccountPropertyPrivate *priv)
{
	gtk_widget_set_sensitive (GTK_WIDGET (priv->combo_bank_list), show);
	gtk_widget_set_sensitive (GTK_WIDGET (priv->button_edit_bank), show);

	if (show)
	{
		gtk_widget_show (GTK_WIDGET (priv->hbox_bank_codes));
		gtk_widget_show (GTK_WIDGET (priv->hbox_account_number_key));
	}
	else
	{
		gtk_widget_hide (GTK_WIDGET (priv->hbox_bank_codes));
		gtk_widget_hide (GTK_WIDGET (priv->hbox_account_number_key));
	}
}

/**
 * efface les data agence numero de compte et clef qui ne sont plus
 * pertinentes si changement de banque
 *
 * \param
 * \param
 *
 * \return
 **/
static void widget_account_property_clear_account_bank_data (gint account_number,
															 WidgetAccountPropertyPrivate *priv)
{
	gsb_autofunc_entry_set_value (priv->entry_bank_branch_code, "", account_number);
	gsb_autofunc_entry_set_value (priv->entry_bank_account_number, "", account_number);
	gsb_autofunc_entry_set_value (priv->entry_bank_account_key, "", account_number);
}

/**
 * positionne le code BIC
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_account_property_set_bic_code (gint account_number,
												  gint bank_number,
												  WidgetAccountPropertyPrivate *priv)
{
	gsb_autofunc_entry_set_value (priv->entry_bic_code,
								  gsb_data_bank_get_bic (bank_number),
								  account_number);
}

/**
 * retourne la structure du compte IBAN pour le pays concerné
 *
 * \param le code du pays concerné
 *
 * \return une structure modèle (XX si pays non défini)
 **/
static struct Iso13616Iban *widget_account_property_iban_get_struc (gchar *pays)
{
	struct Iso13616Iban *s_iban = iso_13616_ibans;
	gchar *pays_2lettres;

	pays_2lettres = g_strndup (pays, 2);
	while (s_iban->iban)
	{
		if (g_strstr_len (s_iban->locale, 2, pays_2lettres))
			break;
		s_iban ++;
	}
	if (s_iban->iban == NULL)
		s_iban = iso_13616_ibans;

	g_free(pays_2lettres);

	return s_iban;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static gboolean widget_account_property_iban_control_bank (const gchar *iban,
														   gint account_number)
{
	const gchar *bank_code;
	gchar *model;
	gchar *tmp_str;
	gchar *ptr_1;
	gchar *ptr_2;
	gchar *code;
	glong pos_char_1;
	glong pos_char_2;
	gunichar c;
	gint bank_number;
	gboolean is_code = FALSE;
	struct Iso13616Iban *s_iban;

	/* si entry vide retour sans rien faire */
	if (strlen (iban) == 0)
		return TRUE;

	bank_number = gsb_data_account_get_bank (account_number);
	bank_code = gsb_data_bank_get_code (bank_number);

	if (!bank_code || strlen (bank_code) == 0)
		return FALSE;

	/* get bank code from iban */
	tmp_str = g_strndup (iban, 2);
	s_iban = widget_account_property_iban_get_struc (tmp_str);
	g_free (tmp_str);

	model = my_strdelimit (s_iban -> iban, " -", "");
	tmp_str = my_strdelimit (iban, " -", "");
	code = g_malloc0 (36 * sizeof (gunichar));

	/* get code */
	c = 'A';
	ptr_1 = g_utf8_strchr (model, -1, c);
	if (ptr_1)
	{
		pos_char_1 = g_utf8_pointer_to_offset (model, ptr_1);
		ptr_2 = g_utf8_strrchr (model, -1, c);
		pos_char_2 = g_utf8_pointer_to_offset (model, ptr_2);
		if ((pos_char_2 - pos_char_1) > 0)
		{
			code = g_utf8_strncpy (code, ptr_1, (pos_char_2 - pos_char_1) + 1);
			is_code = TRUE;
		}
	}
	c = 'B';
	ptr_1 = g_utf8_strchr (model, -1, c);
	if (is_code == FALSE && ptr_1)
	{
		pos_char_1 = g_utf8_pointer_to_offset (model, ptr_1);
		ptr_1 = g_utf8_offset_to_pointer (tmp_str, pos_char_1);

		ptr_2 = g_utf8_strrchr (model, -1, c);
		pos_char_2 = g_utf8_pointer_to_offset (model, ptr_2);

		code = g_utf8_strncpy (code, ptr_1, (pos_char_2 - pos_char_1) + 1);
	}
	if (code && strlen (code))
	{
		if (g_strcmp0 (code, bank_code) == 0)
		{
			g_free (code);

			return TRUE;
		}
	}

	g_free (code);

	return FALSE;
}

/**
 * positionne le code bank
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_account_property_set_sort_code (gint account_number,
												   gint bank_number,
												   WidgetAccountPropertyPrivate *priv)
{
	gsb_autofunc_entry_set_value (priv->entry_sort_code,
								  gsb_data_bank_get_code (bank_number),
								  account_number);
}

/**
 * callback called when change the bank for the account
 * change the bank code label under the combo
 *
 * \param combo the combobox containing the banks
 * \param
 *
 * \return FALSE
 **/
static void widget_account_property_bank_list_changed (GtkWidget *combo,
													   WidgetAccountPropertyPrivate *priv)
{
	gint account_number;
	gint bank_number;

	if (!combo)
		return;

	account_number = gsb_gui_navigation_get_current_account ();
	bank_number = gsb_bank_combo_list_get_bank_number (combo);
	if (bank_number == 0)
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_edit_bank), FALSE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (priv->button_edit_bank), TRUE);

	if (gtk_widget_get_visible (priv->hbox_avertissement_iban))
	{
		const gchar *iban;

		iban = gtk_entry_get_text (GTK_ENTRY (priv->entry_account_iban));
		if (widget_account_property_iban_control_bank (iban, account_number))
		{
			gtk_widget_set_sensitive (GTK_WIDGET (priv->button_edit_bank), FALSE);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->combo_bank_list), FALSE);
			gtk_widget_hide (priv->hbox_avertissement_iban);
		}
	}
	else
	{
		widget_account_property_set_bic_code (account_number, bank_number, priv);
	}
	widget_account_property_set_sort_code (account_number, bank_number, priv);
	widget_account_property_clear_account_bank_data (account_number, priv);
}

/**
 * called when change the currency of the account,
 * propose to change the currency of all the transactions, or let them
 *
 * \param combobox the combobox of currencies
 * \param null a pointer not used
 *
 * \return
 **/
static void widget_account_property_currency_changed (GtkWidget *combo,
													  gint account_number)
{
	gint new_currency_number;
	gint account_currency_number;
	gint result;
	GSList *list_tmp;
	gchar* tmp_str;

	account_currency_number = gsb_data_account_get_currency (account_number);

	new_currency_number = gsb_currency_get_currency_from_combobox (combo);

	if (account_currency_number == new_currency_number)
		return;

	/* set the new currency, must set here and no in the autofunc directly  */
	gsb_data_account_set_currency (account_number, new_currency_number);

	/* ask for the currency of the transactions */
	tmp_str = g_strdup_printf (_("You are changing the currency of the account, do you want to change "
								 "the currency of the transactions too?\n"
								 "(yes will change all the transactions currency from %s to %s, "
								 "all the transactions with another currency will stay the same).\n\n"
								 "Archived and reconcilied transactions will be left unmodified."),
							   gsb_data_currency_get_name (account_currency_number),
							   gsb_data_currency_get_name (new_currency_number));
	result = dialogue_yes_no (tmp_str, _("Change the transactions currency"), GTK_RESPONSE_NO);
	g_free (tmp_str);

	if (result)
	{
		/* we have to change the currency of the transactions,
		 * we never want to change the archives, neither the marked R transactions
		 * so change only the non archived and non marked R transactions */
		list_tmp = gsb_data_transaction_get_transactions_list ();

		while (list_tmp)
		{
			gint transaction_number;

			transaction_number = gsb_data_transaction_get_transaction_number (list_tmp->data);
			if (gsb_data_transaction_get_account_number (transaction_number) == account_number
				&& gsb_data_transaction_get_marked_transaction (transaction_number) != OPERATION_RAPPROCHEE)
				gsb_data_transaction_set_currency_number (transaction_number, new_currency_number);

			list_tmp = list_tmp->next;
		}
	}

	transaction_list_update_element (ELEMENT_CREDIT);
	transaction_list_update_element (ELEMENT_DEBIT);
	transaction_list_update_element (ELEMENT_AMOUNT);

	/* in each cases, we had to update the account balance */
	transaction_list_set_balances ();

	/* update the headings balance */
	gsb_data_account_colorize_current_balance (account_number);
}

/**
 * Affiche les données classiques du compte, données que l'on peut modifier
 * en ligne.
 *
 * \param account_number
 * \param bank_number
 *
 * \return
 **/
static void widget_account_property_bank_data_display (gint account_number,
													   gint bank_number,
													   WidgetAccountPropertyPrivate *priv)
{
	gsb_autofunc_entry_set_value (priv->entry_sort_code,
								  gsb_data_bank_get_code (bank_number),
								  account_number);
	gsb_autofunc_entry_set_value (priv->entry_bank_branch_code,
								  gsb_data_account_get_bank_branch_code (account_number),
								  account_number);
	gsb_autofunc_entry_set_value (priv->entry_bank_account_number,
								  gsb_data_account_get_bank_account_number (account_number),
								  account_number);
	gsb_autofunc_entry_set_value (priv->entry_bank_account_key,
								  gsb_data_account_get_bank_account_key (account_number),
								  account_number);
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
static gboolean widget_account_property_focus_out (GtkWidget *widget,
												   GdkEventFocus *event,
												   gint *p_property_origin)
{
	gint account_number;
	gint property_origin;
	GrisbiWinRun *w_run;

	property_origin = GPOINTER_TO_INT (p_property_origin);
	account_number = gsb_gui_navigation_get_current_account ();
	if (account_number == -1)
		return FALSE;

	switch (property_origin)
	{
		case PROPERTY_NAME:
			/* update the scheduler list */
			gsb_scheduler_list_fill_list ();

			/*update the view menu */
			grisbi_win_menu_move_to_acc_delete ();
			grisbi_win_menu_move_to_acc_new ();

			/* update the name of accounts in form */
			gsb_account_update_combo_list (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT), FALSE);

			/* update categories combofix */
			gsb_category_update_combofix (FALSE);

			/* Replace trees contents. */
			categories_fill_list ();
			budgetary_lines_fill_list ();
			payees_fill_list ();
			break;

		case PROPERTY_HOLDER_NAME:
			grisbi_win_set_window_title (account_number);
			break;
	}

	/* update main page */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->mise_a_jour_liste_comptes_accueil = TRUE;
	w_run->mise_a_jour_liste_echeances_manuelles_accueil = TRUE;
	w_run->mise_a_jour_soldes_minimaux = TRUE;
	w_run->mise_a_jour_fin_comptes_passifs = TRUE;

	return FALSE;
}

/**
 * called when something changed and need to modify other things
 *
 * \param widget various widget according of the origin
 * \param p_property_origin the origin as a pointer
 *
 * \return;
 **/
static void widget_account_property_changed (GtkWidget *widget,
											 gint *p_property_origin)
{
	GtkWidget *bouton_icon;
	GtkWidget *image;
	gint property_origin;
	gint account_number;
	gboolean state;
	GrisbiAppConf *a_conf;
	GrisbiWinRun *w_run;

	account_number = gsb_gui_navigation_get_current_account ();
	if (account_number == -1)
		return;

	property_origin = GPOINTER_TO_INT (p_property_origin);
	switch (property_origin)
	{
		case PROPERTY_NAME:
			gsb_gui_navigation_update_account_label (account_number);
			gsb_gui_navigation_update_account (account_number);
			break;

		case PROPERTY_CLOSED_ACCOUNT:
			a_conf = grisbi_app_get_a_conf ();
			state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));
			if (state)
			{
				if (a_conf->show_closed_accounts)
					gsb_gui_navigation_update_account (account_number);
				else
					gsb_gui_navigation_remove_account (account_number);
				grisbi_win_menu_move_to_acc_update  (FALSE);
			}
			else
			{
				if (a_conf->show_closed_accounts)
					gsb_gui_navigation_update_account (account_number);
				else
					gsb_gui_navigation_add_account (account_number, TRUE);
				grisbi_win_menu_move_to_acc_delete ();
				grisbi_win_menu_move_to_acc_new ();
			}

			/* update the name of accounts in form */
			gsb_account_update_combo_list (gsb_form_scheduler_get_element_widget (SCHEDULED_FORM_ACCOUNT), FALSE);

			/* Replace trees contents. */
			categories_fill_list ();
			budgetary_lines_fill_list ();
			payees_fill_list ();
			break;

		case PROPERTY_KIND:
			gsb_gui_navigation_update_account (account_number);
			image = gsb_data_account_get_account_icon_image (account_number);
			bouton_icon = g_object_get_data (G_OBJECT (widget),"bouton_icon");
			gtk_button_set_image (GTK_BUTTON (bouton_icon), image);
			gsb_form_clean (gsb_form_get_account_number ());
			break;

		case PROPERTY_CURRENCY:
			widget_account_property_currency_changed (widget, account_number);
			break;

		case PROPERTY_INIT_BALANCE:
			/* as we changed the initial balance, we need to recalculate the amount
			 * of each line in the list */
			transaction_list_set_balances ();
			break;

		case PROPERTY_WANTED_BALANCE:
			break;
	}

	/* update main page */
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	w_run->mise_a_jour_liste_comptes_accueil = TRUE;
	w_run->mise_a_jour_liste_echeances_manuelles_accueil = TRUE;
	w_run->mise_a_jour_soldes_minimaux = TRUE;
	w_run->mise_a_jour_fin_comptes_passifs = TRUE;
}

/* IBAN FUNCTIONS */
/**
 * Contrôle la validité du numéro IBAN (non opérationnel pour la partie IBAN)
 *
 * \param le numéro IBAN
 *
 * \return 1 si OK 0 si NON OK -1 si longueur IBAN = 0
 **/
static gint widget_account_property_iban_control_iban (const gchar *iban)
{
	gchar *model;
	gchar *tmp_str = NULL;
	gchar *substr;
	gchar *ptr = NULL;
	gchar *buffer = NULL;
	GString *gstring;
	gint i = 0;
	gulong lnum;
	gint reste = 0;
	gint code_controle;
	gint result = 0;
	struct Iso13616Iban *s_iban;

	if (iban == NULL)
		return 0;
	else if (strlen (iban) == 0)
		return -1;

	tmp_str = g_strndup (iban, 2);
	s_iban = widget_account_property_iban_get_struc (tmp_str);
	g_free(tmp_str);

	model = my_strdelimit (s_iban->iban, " -", "");
	tmp_str = my_strdelimit (iban, " -", "");

	/* on ne contrôle pas la longueur d'un IBAN sans modèle */
	if (g_strcmp0 (s_iban->locale, "XX") != 0 && g_utf8_strlen (model, -1) != g_utf8_strlen (tmp_str, -1))
	{
		g_free (model);
		g_free (tmp_str);

		return 0;
	}

	/* mise en forme de l'IBAN avant contrôle
	 * on place les 4 premiers caractères en fin de chaine et on remplace
	 * les 2 chiffres du code de contrôle par des 0.
	 * Ensuite on remplace les lettres par des chiffres selon un
	 * codage particulier A = 10 B = 11 etc...
	 */

	if (strlen (tmp_str) > 4)
	{
		gchar *tmp_str2;
		gchar *tmp_str3;

		tmp_str2 = g_strndup (tmp_str, 2);
		tmp_str3 = g_strconcat (tmp_str + 4, tmp_str2, "00", NULL);
		g_free(tmp_str2);
		g_free(tmp_str);
		tmp_str = tmp_str3;
	}
	else
	{
		g_free(tmp_str);

		return 0;
	}

	ptr = tmp_str;
	while (ptr[i])
	{
		gchar *tmp_str2;
		gchar *tmp_buffer;
		if (g_ascii_isdigit (ptr[i]))
		{
			tmp_str2 = g_strdup_printf ("%c", ptr[i]);
			if (buffer == NULL)
				buffer = tmp_str2;
			else
			{
				tmp_buffer = g_strconcat (buffer, tmp_str2, NULL);
				g_free(tmp_str2);
				g_free(buffer);
				buffer = tmp_buffer;
			}
		}
		else
		{
			tmp_str2 = g_strdup_printf ("%d", ptr[i] - 55);
			if (buffer == NULL)
				buffer = tmp_str2;
			else
			{
				tmp_buffer = g_strconcat (buffer, tmp_str2, NULL);
				g_free(tmp_str2);
				g_free(buffer);
				buffer = tmp_buffer;
			}
		}
		i++;
	}

	g_free (tmp_str);

	/* vérification du calcul de l'IBAN
	 *
	 * methode : on procède par étapes
	 * on prend les 9 premiers chiffres et on les divise par 97
	 * on ajoute au reste les 7 ou 8 chiffres suivants que l'on divise par 97
	 * on procède ainsi jusqu'a la fin de la chaine. Ensuite on soustrait le
	 * dernier reste à 98 ce qui donne le code de contrôle
	 */
	gstring = g_string_new (buffer);
	g_free(buffer);

	while (gstring->len > 0)
	{
		substr = g_strndup (gstring->str, 9);
		lnum = utils_str_safe_strtod (substr, NULL);
		reste = lnum % 97;

		g_free (substr);

		if (gstring->len >= 9)
		{
			gstring = g_string_erase (gstring, 0, 9);
			gstring = g_string_prepend (gstring, g_strdup_printf ("%d", reste));
		}
		else
			break;
	}

	g_string_free (gstring, TRUE);

	reste = 98 - reste;

	tmp_str = g_strndup (iban + 2, 2);
	code_controle =  utils_str_atoi (tmp_str);

	if (code_controle - reste == 0)
		result = 1;
	else
		result = 0;

	g_free (tmp_str);

	return result;
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_account_property_iban_delete_text (GtkEditable *entry,
													  gint start_pos,
													  gint end_pos,
													  WidgetAccountPropertyPrivate *priv)
{
	/* on autorise ou pas la saisie des données banquaires */
	if (start_pos == 0)
		widget_account_property_bank_data_show (TRUE, priv);
	else if (gtk_widget_is_sensitive (priv->combo_bank_list))
		widget_account_property_bank_data_show (FALSE, priv);
}

/**
 *
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void widget_account_property_iban_insert_text (GtkEditable *entry,
													  gchar *text,
													  gint length,
													  gint *position,
													  WidgetAccountPropertyPrivate *priv)
{
	static struct Iso13616Iban *s_iban = iso_13616_ibans;
	gchar *iban;
	gint nbre_char;

	iban = g_utf8_strup (text, length);

	/* on bloque l'appel de la fonction */
	g_signal_handlers_block_by_func (G_OBJECT (entry),
									 G_CALLBACK (widget_account_property_iban_insert_text),
									 priv);

	/* on met en majuscule l'entrée */
	gtk_editable_insert_text (entry, iban, length, position);
	g_free (iban);

	/* on fait les traitements complémentaires */
	iban = g_utf8_strup (gtk_editable_get_chars (entry, 0, -1), -1);

	/* on autorise ou pas la saisie des données banquaires */
	if (g_utf8_strlen (iban, -1) == 0)
		widget_account_property_bank_data_show (TRUE, priv);
	else if (gtk_widget_is_sensitive (priv->combo_bank_list))
		widget_account_property_bank_data_show (FALSE, priv);

	/* on contrôle l'existence d'un modèle pour le numéro IBAN */
	if (g_utf8_strlen (iban, -1) >= 2)
	{
		gchar *free_str;

		free_str = g_strndup (iban, 2);
		if (g_utf8_collate (s_iban->locale, free_str) != 0)
		{
			s_iban = widget_account_property_iban_get_struc (iban);

			if (s_iban->nbre_char % 4 == 0)
				nbre_char = s_iban->nbre_char + (s_iban->nbre_char / 4) - 1;
			else
				nbre_char = s_iban->nbre_char + (s_iban->nbre_char / 4);
			gtk_entry_set_max_length (GTK_ENTRY (entry), nbre_char);
		}
		g_free (free_str);
	}

	g_free (iban);
	g_signal_handlers_unblock_by_func (G_OBJECT (entry),
									   G_CALLBACK (widget_account_property_iban_insert_text),
									   priv);
	g_signal_stop_emission_by_name (entry, "insert_text");
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
static gboolean widget_account_property_iban_focus_in_event (GtkWidget *entry,
															 GdkEventFocus *ev,
															 gpointer data)
{
	gint current_account;

	/* on sauvegarde le numéro de compte initiateur pour éviter de conserver
	 * les données de l'ancien compte lorsque l'on change de compte */
	current_account = gsb_gui_navigation_get_current_account ();
	g_object_set_data (G_OBJECT (entry), "old_account", GINT_TO_POINTER (current_account));

	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void widget_account_property_iban_set_iban (const gchar *iban,
												   WidgetAccountPropertyPrivate *priv)
{
	gint position = 0;

	g_signal_handlers_block_by_func (G_OBJECT (priv->entry_account_iban),
									 G_CALLBACK (widget_account_property_iban_insert_text),
									 priv->combo_bank_list);

	gtk_editable_delete_text (GTK_EDITABLE (priv->entry_account_iban), 0, -1);
	if (iban && strlen (iban) > 0)
		gtk_editable_insert_text (GTK_EDITABLE (priv->entry_account_iban), iban, -1, &position);

	g_signal_handlers_unblock_by_func (G_OBJECT (priv->entry_account_iban),
									   G_CALLBACK (widget_account_property_iban_insert_text),
									   priv->combo_bank_list);
}

/**
 * If the IBAN is correct it is saved, otherwise we return the
 * previous version.
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gboolean widget_account_property_iban_focus_out_event (GtkWidget *entry,
															  GdkEventFocus *ev,
															  WidgetAccountPropertyPrivate *priv)
{
	gchar *iban;
	gint account_number;
	gint old_account;

	old_account = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (entry), "old_account"));
	account_number = gsb_gui_navigation_get_current_account ();
	if (old_account != account_number)
		return FALSE;

	iban = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);

	if (widget_account_property_iban_control_iban (iban) == 0)
	{
		widget_account_property_iban_set_iban (gsb_data_account_get_bank_account_iban (account_number),
											   priv);
	}
	else
	{
		if (!widget_account_property_iban_control_bank (iban, account_number))
		{
			gtk_widget_set_sensitive (GTK_WIDGET (priv->button_edit_bank), TRUE);
			gtk_widget_set_sensitive (GTK_WIDGET (priv->combo_bank_list), TRUE);
			gtk_widget_show (priv->hbox_avertissement_iban);
		}
		gsb_data_account_set_bank_account_iban (account_number, iban);
		gsb_file_set_modified (TRUE);
	}

	if (iban && strlen (iban) > 0)
		g_free (iban);

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
static gboolean widget_account_property_iban_key_press_event (GtkWidget *entry,
															  GdkEventKey *ev,
															  WidgetAccountPropertyPrivate *priv)
{
	gchar *iban;
	gint account_number;

	switch (ev->keyval)
	{
		case GDK_KEY_Escape :
			return TRUE;
			break;

		case GDK_KEY_KP_Enter :
		case GDK_KEY_Return :
			iban = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
			if (widget_account_property_iban_control_iban (iban) == 0)
			{
				gchar *tmp_str;
				gint position = 0;

				tmp_str = g_strdup_printf (_("Your IBAN number is not correct. Please check your entry."));
				dialogue_warning (tmp_str);
				g_free (tmp_str);
				gtk_editable_delete_text (GTK_EDITABLE (priv->entry_account_iban), 0, -1);
				gtk_editable_insert_text (GTK_EDITABLE (priv->entry_account_iban), iban, -1, &position);
			}
			else
			{
				account_number = gsb_gui_navigation_get_current_account ();
				if (!widget_account_property_iban_control_bank (iban, account_number))
				{
					gtk_widget_set_sensitive (GTK_WIDGET (priv->button_edit_bank), TRUE);
					gtk_widget_set_sensitive (GTK_WIDGET (priv->combo_bank_list), TRUE);
					gtk_widget_show (priv->hbox_avertissement_iban);
				}
				gsb_data_account_set_bank_account_iban (account_number, iban);
				gsb_file_set_modified (TRUE);
			}

			if (iban && strlen (iban) > 0)
				g_free (iban);

			return TRUE;
			break;
	}
	return FALSE;
}

/**
 * Création du widget account_property
 *
 * \param
 * \return
 **/
static void widget_account_property_setup (WidgetAccountProperty *widget)
{
	GSList *list;
	WidgetAccountPropertyPrivate *priv;

	devel_debug (NULL);
	priv = widget_account_property_get_instance_private (widget);

	/* set account name */
	gsb_autofunc_entry_new_from_ui (priv->entry_account_name,
									G_CALLBACK (widget_account_property_changed),
									GINT_TO_POINTER (PROPERTY_NAME),
									G_CALLBACK (gsb_data_account_set_name),
									0);
	g_signal_connect (G_OBJECT (priv->entry_account_name),
					  "focus-out-event",
					  G_CALLBACK (widget_account_property_focus_out),
					  GINT_TO_POINTER (PROPERTY_NAME));

	/* create the box of kind of account */
	list = gsb_data_bank_get_bank_type_list ();
	gsb_autofunc_combobox_new_from_ui (priv->combo_account_type,
									   list,
									   0,
									   G_CALLBACK (widget_account_property_changed),
									   GINT_TO_POINTER (PROPERTY_KIND),
									   G_CALLBACK (gsb_data_account_set_kind),
									   0);
	g_object_set_data (G_OBJECT (priv->combo_account_type),"bouton_icon", priv->bouton_icon);
	g_slist_free (list);

	/* create the currency widget */
	gsb_autofunc_currency_new_from_ui (priv->combo_account_currency,
									   TRUE,
									   0,
									   G_CALLBACK (widget_account_property_changed),
									   GINT_TO_POINTER (PROPERTY_CURRENCY),
									   NULL,
									   0);

	/* create checkbutton account closed */
	gsb_autofunc_checkbutton_new_from_ui (priv->checkbutton_closed_account,
										  FALSE,
										  G_CALLBACK (widget_account_property_changed),
										  GINT_TO_POINTER (PROPERTY_CLOSED_ACCOUNT),
										  G_CALLBACK (gsb_data_account_set_closed_account),
										  0);

	/* create account holder */
	gsb_autofunc_entry_new_from_ui (priv->entry_holder_name,
									NULL,
									NULL,
									G_CALLBACK (gsb_data_account_set_holder_name),
									0);

	g_signal_connect (G_OBJECT (priv->entry_holder_name),
					  "focus-out-event",
					  G_CALLBACK (widget_account_property_focus_out),
					  GINT_TO_POINTER (PROPERTY_HOLDER_NAME));

	/* create checkbutton for address of the holder */
	gsb_autofunc_checkbutton_new_from_ui (priv->checkbutton_holder_address,
										  FALSE,
										  G_CALLBACK (gsb_editable_erase_text_view),
										  priv->textview_holder_address,
										  NULL,
										  0);

	/* create textview for address of the holder */
	gsb_autofunc_textview_new_from_ui (priv->textview_holder_address,
									   NULL,
									   NULL,
									   G_CALLBACK (gsb_data_account_set_holder_address),
									   0);
	g_signal_connect (G_OBJECT (priv->checkbutton_holder_address),
					  "toggled",
					  G_CALLBACK (utils_buttons_sensitive_by_checkbutton),
					  priv->textview_holder_address);

	/* create bank list */
	gsb_bank_new_combobox_from_ui (priv->combo_bank_list, 0);
	g_signal_connect (G_OBJECT (priv->combo_bank_list),
					  "changed",
					  G_CALLBACK (widget_account_property_bank_list_changed),
					  priv);

	/* create bic code */
	gsb_autofunc_entry_new_from_ui (priv->entry_bic_code, NULL, NULL, NULL, 0);

	/* signals entry_account_iban */
	g_signal_connect (G_OBJECT (priv->entry_account_iban),
					  "insert-text",
					  G_CALLBACK (widget_account_property_iban_insert_text),
					  priv);
	g_signal_connect (G_OBJECT (priv->entry_account_iban),
					  "delete-text",
					  G_CALLBACK (widget_account_property_iban_delete_text),
					  priv);
	g_signal_connect (G_OBJECT (priv->entry_account_iban),
					  "key-press-event",
					  G_CALLBACK (widget_account_property_iban_key_press_event),
					  priv);
	g_signal_connect (G_OBJECT (priv->entry_account_iban),
					  "focus-in-event",
					  G_CALLBACK (widget_account_property_iban_focus_in_event),
					  NULL);
	g_signal_connect (G_OBJECT (priv->entry_account_iban),
					  "focus-out-event",
					  G_CALLBACK (widget_account_property_iban_focus_out_event),
					  priv);

	/* create sort code */
	gsb_autofunc_entry_new_from_ui (priv->entry_sort_code, NULL, NULL, NULL, 0);

	/* create bank branch code */
	gsb_autofunc_entry_new_from_ui (priv->entry_bank_branch_code,
									NULL,
									NULL,
									G_CALLBACK (gsb_data_account_set_bank_branch_code),
									0);

	/* create bank account number */
	gsb_autofunc_entry_new_from_ui (priv->entry_bank_account_number,
											   NULL,
											   NULL,
											   G_CALLBACK (gsb_data_account_set_bank_account_number),
											   0);

	/* create bank account key entry */
	gsb_autofunc_entry_new_from_ui (priv->entry_bank_account_key,
												NULL,
												NULL,
												G_CALLBACK (gsb_data_account_set_bank_account_key),
												0);

	/* create the entry_initial_balance */
	gsb_autofunc_real_new_from_ui (priv->entry_initial_balance,
								   G_CALLBACK (widget_account_property_changed),
								   GINT_TO_POINTER (PROPERTY_INIT_BALANCE),
								   G_CALLBACK (gsb_data_account_set_init_balance),
								   0);

	/* create the entry_minimal_authorized */
	gsb_autofunc_real_new_from_ui (priv->entry_minimal_authorized,
								   G_CALLBACK (widget_account_property_changed),
								   GINT_TO_POINTER (PROPERTY_WANTED_BALANCE),
								   G_CALLBACK (gsb_data_account_set_mini_balance_authorized),
								  0);

	/* create the entry_minimal_desired */
	gsb_autofunc_real_new_from_ui (priv->entry_minimal_desired,
								   G_CALLBACK (widget_account_property_changed),
								   GINT_TO_POINTER (PROPERTY_WANTED_BALANCE),
								   G_CALLBACK (gsb_data_account_set_mini_balance_wanted),
								   0);

	/* create the comment textview */
	gsb_autofunc_textview_new_from_ui (priv->textview_comment_text,
									   NULL,
									   NULL,
									   G_CALLBACK (gsb_data_account_set_comment),
									   0);

	/* set signal bouton_icon*/
	g_signal_connect (G_OBJECT(priv->bouton_icon),
					  "clicked",
					  G_CALLBACK(gsb_data_account_change_account_icon),
					  NULL);

	g_signal_connect (G_OBJECT (priv->button_edit_bank),
					  "clicked",
					  G_CALLBACK (gsb_bank_edit_from_button),
					  priv->combo_bank_list);
}

/******************************************************************************/
/* Fonctions propres à l'initialisation des fenêtres                          */
/******************************************************************************/
static void widget_account_property_init (WidgetAccountProperty *widget)
{
	gtk_widget_init_template (GTK_WIDGET (widget));
}

static void widget_account_property_dispose (GObject *object)
{
	G_OBJECT_CLASS (widget_account_property_parent_class)->dispose (object);
}

static void widget_account_property_class_init (WidgetAccountPropertyClass *klass)
{
	G_OBJECT_CLASS (klass)->dispose = widget_account_property_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (klass),
												 "/org/gtk/grisbi/widgets/widget_account_property.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, vbox_account_property);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, bouton_icon);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, title_detail_compte);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, hbox_account_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_account_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, hbox_account_detail);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, checkbutton_closed_account);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, combo_account_type);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, combo_account_currency);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_holder_name);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, checkbutton_holder_address);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, textview_holder_address);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, combo_bank_list);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, button_edit_bank);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_bic_code);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_account_iban);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, hbox_avertissement_iban);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, hbox_bank_codes);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_sort_code);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_bank_branch_code);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, hbox_account_number_key);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_bank_account_number);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_bank_account_key);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_minimal_desired);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_minimal_authorized);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, entry_initial_balance);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (klass), WidgetAccountProperty, textview_comment_text);
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
WidgetAccountProperty *widget_account_property_new (void)
{
	WidgetAccountProperty *widget;

	widget = g_object_new (WIDGET_ACCOUNT_PROPERTY_TYPE, NULL);
	widget_account_property_setup (widget);

	return widget;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void widget_account_property_fill (void)
{
	GtkWidget *account_property_page;
	GtkWidget *image;
	gchar *str_to_free;
	gchar *tmp_str1;
	gchar *tmp_str2;
	const gchar *tmp_str;
	gint account_number;
	gint bank_number = 0;
	WidgetAccountPropertyPrivate *priv;

	devel_debug (NULL);
	account_property_page = grisbi_win_get_account_property_page ();
	priv = widget_account_property_get_instance_private (WIDGET_ACCOUNT_PROPERTY (account_property_page));
	account_number = gsb_gui_navigation_get_current_account ();

	/* set account_icon */
	image = gsb_data_account_get_account_icon_image (account_number);
	gtk_button_set_always_show_image (GTK_BUTTON (priv->bouton_icon), TRUE);
	gtk_button_set_image (GTK_BUTTON (priv->bouton_icon), image);
	gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (image, GTK_ALIGN_CENTER);
	gtk_widget_show (image);

	/* set account number */
	str_to_free = g_strdup_printf (" N° %d", account_number);
	tmp_str1 = g_strconcat (_("Account details"), str_to_free, NULL);
	tmp_str2 = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>", tmp_str1);
	gtk_label_set_markup (GTK_LABEL (priv->title_detail_compte), tmp_str2);
	g_free (str_to_free);
	g_free(tmp_str1);
	g_free (tmp_str2);
	str_to_free = NULL;
	tmp_str1 = NULL;
	tmp_str2 = NULL;

	/* set account name */
	gsb_autofunc_entry_set_value (priv->entry_account_name,
								  gsb_data_account_get_name (account_number), account_number);

	/* set account type */
	gsb_autofunc_combobox_set_index (priv->combo_account_type,
									 gsb_data_account_get_kind (account_number), account_number);

	/* set account currency */
	gsb_autofunc_currency_set_currency_number (priv->combo_account_currency,
											   gsb_data_account_get_currency (account_number), account_number);

	/* set account closed */
	gsb_autofunc_checkbutton_set_value (priv->checkbutton_closed_account,
										gsb_data_account_get_closed_account (account_number), account_number);

	/* set account holder */
	gsb_autofunc_entry_set_value (priv->entry_holder_name,
								  gsb_data_account_get_holder_name (account_number), account_number);

	/* set holder address */
	tmp_str = gsb_data_account_get_holder_address (account_number);
	if (tmp_str && strlen (tmp_str))
	{
		gsb_autofunc_checkbutton_set_value (priv->checkbutton_holder_address, TRUE, 0);
		gsb_autofunc_textview_set_value (priv->textview_holder_address, tmp_str, account_number);
	}
	else
	{
		gsb_autofunc_checkbutton_set_value (priv->checkbutton_holder_address, FALSE, 0);
		gsb_autofunc_textview_set_value (priv->textview_holder_address, "", account_number);
	}

	/* set bank in list */
	bank_number = gsb_data_account_get_bank (account_number);
	gsb_bank_combo_list_set_bank (priv->combo_bank_list, bank_number);

	/* set bank bic */
	tmp_str = gsb_data_bank_get_bic (bank_number);
	if (tmp_str && strlen (tmp_str))
		gtk_entry_set_text (GTK_ENTRY (priv->entry_bic_code), tmp_str);
	else
		gtk_entry_set_text (GTK_ENTRY (priv->entry_bic_code), "");

	/* if iban data hide bank details */
	tmp_str = gsb_data_account_get_bank_account_iban (account_number);
	if (tmp_str && strlen (tmp_str))
	{
		widget_account_property_iban_set_iban (tmp_str, priv);
		widget_account_property_bank_data_show (FALSE, priv);
	}
	else
	{
		/* delete iban_code */
		widget_account_property_iban_set_iban ("", priv);

		/* init and show bank data */
		widget_account_property_bank_data_display (account_number, bank_number, priv);
		widget_account_property_bank_data_show (TRUE, priv);
	}

	/* show balances */
	gsb_autofunc_real_set (priv->entry_initial_balance,
						   gsb_data_account_get_init_balance (account_number,
															  gsb_data_currency_get_floating_point
															  (gsb_data_account_get_currency (account_number))),
						   account_number);
	gsb_autofunc_real_set (priv->entry_minimal_authorized,
						   gsb_data_account_get_mini_balance_authorized (account_number),
						   account_number);
	gsb_autofunc_real_set (priv->entry_minimal_desired,
						   gsb_data_account_get_mini_balance_wanted (account_number),
						   account_number);

	/* set comments */
	gsb_autofunc_textview_set_value (priv->textview_comment_text,
									 gsb_data_account_get_comment (account_number), account_number);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
void widget_account_property_update_bank_data (gint bank_number)
{
	GtkWidget *widget;
	gint account_number;
	gint tmp_bank_number;
	WidgetAccountPropertyPrivate *priv;

	devel_debug (NULL);
	widget = grisbi_win_get_account_property_page ();
	priv = widget_account_property_get_instance_private (WIDGET_ACCOUNT_PROPERTY (widget));

	/* on test si on modifie la banque du compte affiché */
	account_number = gsb_gui_navigation_get_current_account ();
	tmp_bank_number = gsb_data_account_get_bank (account_number);

	if (tmp_bank_number == bank_number)
	{
		/* set bic code */
		gsb_autofunc_entry_set_value (priv->entry_bic_code, gsb_data_bank_get_bic (bank_number), account_number);

		/* set sort code */
		gsb_autofunc_entry_set_value (priv->entry_sort_code, gsb_data_bank_get_code (bank_number), account_number);
	}
}

/**
 * return the bank_combob used in prefs_page_bank
 *
 * \param
 *
 * \return
 **/
GtkWidget *widget_account_property_get_combo_account_currency (void)
{
	GtkWidget *widget;
	WidgetAccountPropertyPrivate *priv;

	devel_debug (NULL);
	widget = grisbi_win_get_account_property_page ();
	if (GTK_IS_WIDGET (widget))
	{
		priv = widget_account_property_get_instance_private (WIDGET_ACCOUNT_PROPERTY (widget));

		return priv->combo_account_currency;
	}
	else
		return NULL;
}

/**
 * return the bank_combob used in prefs_page_bank
 *
 * \param
 *
 * \return
 **/
GtkWidget *widget_account_property_get_combo_bank_list (void)
{
	GtkWidget *widget;
	WidgetAccountPropertyPrivate *priv;

	devel_debug (NULL);
	widget = grisbi_win_get_account_property_page ();
	if (GTK_IS_WIDGET (widget))
	{
		priv = widget_account_property_get_instance_private (WIDGET_ACCOUNT_PROPERTY (widget));

		return priv->combo_bank_list;
	}
	else
		return NULL;
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

