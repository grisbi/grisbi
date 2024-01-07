/* ***************************************************************************/
/*                                                                            */
/*     Copyright (C)    2000-2008 Cédric Auger (cedric@grisbi.org)            */
/*            2003-2008 Benjamin Drieu (bdrieu@april.org)                     */
/*            2008-2020 Pierre Biava (grisbi@pierre.biava.name)               */
/*             https://www.grisbi.org/                                        */
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
/* ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "include.h"
#include <glib/gi18n.h>

/*START_INCLUDE*/
#include "accueil.h"
#include "classement_echeances.h"
#include "dialog.h"
#include "grisbi_app.h"
#include "gsb_automem.h"
#include "gsb_data_account.h"
#include "gsb_data_currency.h"
#include "gsb_data_partial_balance.h"
#include "gsb_data_payee.h"
#include "gsb_data_scheduled.h"
#include "gsb_data_transaction.h"
#include "gsb_form.h"
#include "gsb_form_scheduler.h"
#include "gsb_real.h"
#include "gsb_scheduler.h"
#include "gsb_scheduler_list.h"
#include "gsb_select_icon.h"
#include "gsb_transactions_list.h"
#include "navigation.h"
#include "structures.h"
#include "utils.h"
#include "utils_dates.h"
#include "utils_real.h"
#include "utils_str.h"
#include "erreur.h"
/*END_INCLUDE*/

/*START_STATIC*/
static void update_liste_echeances_manuelles_accueil (gboolean force);

static GtkWidget *frame_etat_comptes_accueil = NULL;
static GtkWidget *frame_etat_echeances_auto_accueil = NULL;
static GtkWidget *frame_etat_echeances_manuelles_accueil = NULL;
static GtkWidget *frame_etat_fin_compte_passif = NULL;
static GtkWidget *frame_etat_soldes_minimaux_autorises = NULL;
static GtkWidget *frame_etat_soldes_minimaux_voulus = NULL;
static GtkWidget *hbox_title = NULL;
static GtkWidget *label_accounting_entity = NULL;
static GtkWidget *logo_accueil = NULL;
static GtkWidget *main_page_finished_scheduled_transactions_part = NULL;

static GtkSizeGroup *size_group_accueil;

static const gchar *chaine_espace = "                         ";

static gint LIGNE_SOMME_SIZE = 100;
/*END_STATIC*/

/*START_EXTERN*/
extern GSList *scheduled_transactions_taken;
extern GSList *scheduled_transactions_to_take;
/*END_EXTERN*/

#define show_paddingbox(child) gtk_widget_show_all (gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(child)))))
#define hide_paddingbox(child) gtk_widget_hide (gtk_widget_get_parent(gtk_widget_get_parent(gtk_widget_get_parent(GTK_WIDGET(child)))))

/* structure définissant une association entre un compte et un solde partiel */
typedef struct _StructAccountPartial StructAccountPartial;

struct _StructAccountPartial
{
    gint		account_number;
    gint		partial_number;
    gboolean	displayed;
};

/* structure buffer qui conserve un pointer sur le dernier compte possédant un solde partiel */
static StructAccountPartial *partial_buffer = NULL;

/******************************************************************************/
/* Private functions                                                            */
/******************************************************************************/
/**
 * affiche une ligne de solde partiel
 *
 * \param table
 * \param i						row
 * \param partial_number
 * \param currency_number
 *
 * \return
 **/
static void gsb_main_page_affiche_ligne_solde_partiel (GtkWidget *table,
													   gint i,
													   gint partial_number,
													   gint currency_number)
{
	GtkWidget *label;
	gchar *tmp_str;
	gchar *tmp_str2;
	KindAccount kind;

	/* Première colonne : elle contient le nom du solde partiel avec ou sans devise*/
	kind = gsb_data_partial_balance_get_kind (partial_number);
	if (kind == - 1)
	{
		currency_number = gsb_data_partial_balance_get_currency (partial_number);
		tmp_str2 = g_strdup_printf (_(" in %s"), gsb_data_currency_get_name (currency_number));
		tmp_str = g_strconcat (gsb_data_partial_balance_get_name (partial_number),
							   tmp_str2,
							   " : ",
							   NULL);
		g_free (tmp_str2);
	}
	else
	{
		tmp_str = g_strconcat (gsb_data_partial_balance_get_name (partial_number), " : ", NULL);
	}
	label = gtk_label_new (tmp_str);
	gtk_widget_set_name (label, "label_gsetting_option");
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
	gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group_accueil), label);
	gtk_grid_attach (GTK_GRID (table), label, 0, i, 1, 1);
	gtk_widget_show (label);

	/* Deuxième colonne : elle contient le solde pointé du solde partiel */
	tmp_str = gsb_data_partial_balance_get_marked_balance (partial_number);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), tmp_str);
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_grid_attach (GTK_GRID (table), label, 1, i, 1, 1);
	gtk_widget_show (label);

	/* Troisième colonne : elle contient le solde courant du solde partiel */
	tmp_str = gsb_data_partial_balance_get_current_balance (partial_number);
	label = gtk_label_new (NULL);
	gtk_label_set_markup (GTK_LABEL (label), tmp_str);
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_grid_attach (GTK_GRID (table), label, 2, i, 1, 1);
	gtk_widget_show (label);
}

/**
 * display an empty row
 *
 * \param table			grid
 * \param i				row number
 *
 * \return
 **/
static void gsb_main_page_affiche_ligne_vide (GtkWidget *table,
											  gint i)
{
	GtkWidget *label;

	label = gtk_label_new (chaine_espace);
	gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group_accueil), label);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_grid_attach (GTK_GRID (table), label, 0, i, 1, 1);
	gtk_widget_show (label);
}

/**
 * fonction renvoie TRUE si le compte a déjà été affiché
 *
 * \param account_number    compte à tester
 * \param list_partial      liste des structures concernées
 *
 * \return                  partial->displayed
 **/
static gint gsb_main_page_account_get_account_displayed (gint account_number,
														 GSList *list_partial)
{
	GSList *list_tmp;

	/* on retourne TRUE si le compte est clos */
	if (gsb_data_account_get_closed_account (account_number))
		return TRUE;

	list_tmp = list_partial;
	while (list_tmp)
	{
		StructAccountPartial *partial;

		partial = (list_tmp -> data);
		if (partial->account_number == account_number)
		{
			partial_buffer = partial;
			return partial_buffer->displayed;
		}

		list_tmp = list_tmp -> next;
	}

	return FALSE;
}

/**
 * retourne une ligne pour séparer les comptes de leur somme partielle
 *
 * \param table     table ou la libne va s'insérer
 * \param i         indice de placement dans la table
 *
 * \return
 **/
static void gsb_main_page_account_get_ligne_somme (GtkWidget *table,
												   gint i)
{
	GtkWidget *separator;

	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_halign (separator, GTK_ALIGN_END);
	gtk_widget_set_size_request (separator, LIGNE_SOMME_SIZE, -1);
	gtk_widget_show (separator);
	gtk_grid_attach (GTK_GRID (table), separator, 1, i, 1, 1);

	separator = gtk_separator_new (GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_halign (separator, GTK_ALIGN_END);
	gtk_widget_set_size_request (separator, LIGNE_SOMME_SIZE, -1);
	gtk_widget_show (separator);
	gtk_grid_attach (GTK_GRID (table), separator, 2, i, 1, 1);
}

/**
 * affiche une ligne de solde partiel regroupée sous ses comptes
 *
 * \param table
 * \param i						row
 * \param partial_number
 * \param currency_number
 *
 * \return
 **/
static void gsb_main_page_affiche_solde_partiel_groupe_account (GtkWidget *table,
																gint i,
																gint partial_number,
																gint currency_number)
{
	/* on commence par une ligne vide pour la somme */
	gsb_main_page_account_get_ligne_somme (table, i);
	i ++;

	/* on contnue avec une ligne de solde partiel */
	gsb_main_page_affiche_ligne_solde_partiel (table, i, partial_number, currency_number);
	i++;

	/* on finit par une ligne vide */
	gsb_main_page_affiche_ligne_vide (table, i);
}

/**
 * fonction si le compte passé en paramètre appartient à un solde partiel
 *
 * \param account_number    compte à tester
 * \param list_partial      liste des structures concernées
 *
 * \return TRUE si compte appartient à un solde partiel FALSE autrement
 **/
static gint gsb_main_page_account_have_partial_balance (gint account_number,
														GSList *list_partial)
{
	GSList *list_tmp;

	if (list_partial)
	{
		list_tmp = list_partial;
		while (list_tmp)
		{
			StructAccountPartial *partial;

			partial = (list_tmp -> data);
			if (partial->account_number == account_number)
			{
				partial_buffer = partial;

				return TRUE;
			}

			list_tmp = list_tmp -> next;
		}
	}

	/* le compte n'appartient pas à solde partiel */
	if (partial_buffer)
	{
		partial_buffer->partial_number = 0;
		partial_buffer->displayed = FALSE;
	}

	return FALSE;
}

/**
 * called by a click on an account name or balance of accounts on the main page
 * it's just a mediator between the signal and gsb_navigation_show_account
 *
 * \param account_number a pointer which is the number of account we want to switch
 *
 * \return FALSE
 **/
static gboolean gsb_main_page_click_on_account (gint *account_number)
{
	devel_debug_int (GPOINTER_TO_INT (account_number));
	gsb_gui_navigation_set_selection (GSB_ACCOUNT_PAGE, GPOINTER_TO_INT (account_number), 0);

	return FALSE;
}

/**
 * Affiche une ligne pour le compte donné en paramètre
 *
 * \param table
 * \param account number
 * \param ligne dans la table
 *
 * \return FALSE
 **/
static void gsb_main_page_affiche_ligne_du_compte (GtkWidget *pTable,
												   gint account_number,
												   gint i)
{
	GtkWidget *pEventBox;
	GtkWidget *pLabel;
	GtkStyleContext* context;
	gchar *tmp_str;
	GsbReal current_balance;

	/* Première colonne : elle contient le nom du compte */
	tmp_str = g_strconcat (gsb_data_account_get_name (account_number), " : ", NULL);
	pLabel = gtk_label_new (tmp_str);
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (pLabel), MISC_LEFT, MISC_VERT_CENTER);
	gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group_accueil), pLabel);

	/* Création d'une boite à évènement qui sera rattachée au nom du compte */
	pEventBox = gtk_event_box_new ();
	gtk_widget_set_name (pEventBox, "accueil_nom_compte");
	context = gtk_widget_get_style_context  (pEventBox);
	gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);

	g_signal_connect (G_OBJECT (pEventBox),
					  "enter-notify-event",
					  G_CALLBACK (utils_event_box_change_state),
					  context);
	g_signal_connect (G_OBJECT (pEventBox),
					  "leave-notify-event",
					  G_CALLBACK (utils_event_box_change_state),
					  context);
	g_signal_connect_swapped (G_OBJECT (pEventBox),
							  "button-press-event",
							  G_CALLBACK (gsb_main_page_click_on_account),
							  GINT_TO_POINTER (account_number));
	gtk_grid_attach (GTK_GRID (pTable), pEventBox, 0, i, 1, 1);
	gtk_widget_show (pEventBox);
	gtk_container_add (GTK_CONTAINER (pEventBox), pLabel);
	gtk_widget_show (pLabel);

	/* Deuxième colonne : elle contient le solde pointé du compte */
	tmp_str = utils_real_get_string_with_currency (gsb_data_account_get_marked_balance (account_number),
												   gsb_data_account_get_currency (account_number),
												   TRUE);
	pLabel = gtk_label_new (tmp_str);
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (pLabel), MISC_RIGHT, MISC_VERT_CENTER);

	/* Création d'une boite à évènement qui sera rattachée au solde pointé du compte */
	pEventBox = gtk_event_box_new ();

	/* Mise en place du style du label en fonction du solde courant */
	if (gsb_real_cmp (gsb_data_account_get_marked_balance (account_number),
					  gsb_data_account_get_mini_balance_wanted (account_number)) != -1)
	{
		gtk_widget_set_name (pEventBox, "accueil_solde_normal");
	}
	else
	{
		if (gsb_real_cmp (gsb_data_account_get_marked_balance (account_number),
						  gsb_data_account_get_mini_balance_authorized (account_number)) != -1)
		{
			gtk_widget_set_name (pEventBox, "accueil_solde_alarme_low");
		}
		else
		{
			gtk_widget_set_name (pEventBox, "accueil_solde_alarme_high");
		}
	}

	context = gtk_widget_get_style_context  (pEventBox);
	gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);

	g_signal_connect (G_OBJECT (pEventBox),
					  "enter-notify-event",
					  G_CALLBACK (utils_event_box_change_state),
					  context);
	g_signal_connect (G_OBJECT (pEventBox),
					   "leave-notify-event",
					  G_CALLBACK (utils_event_box_change_state),
					  context);
	g_signal_connect_swapped (G_OBJECT (pEventBox),
							  "button-press-event",
							  G_CALLBACK (gsb_main_page_click_on_account),
							  GINT_TO_POINTER (account_number));
	gtk_grid_attach (GTK_GRID (pTable), pEventBox, 1, i, 1, 1);
	gtk_widget_show (pEventBox);
	gtk_container_add (GTK_CONTAINER (pEventBox), pLabel);
	gtk_widget_show (pLabel);

	/* Troisième colonne : elle contient le solde courant du compte */
	current_balance = gsb_data_account_get_current_balance (account_number);
	if (current_balance.mantissa == G_MININT64)
		tmp_str =  g_strdup (ERROR_REAL_STRING);
	else
		tmp_str = utils_real_get_string_with_currency (current_balance,
													   gsb_data_account_get_currency (account_number),
													   TRUE);
	pLabel = gtk_label_new (tmp_str);
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (pLabel), MISC_RIGHT, MISC_VERT_CENTER);

	/* Création d'une boite à évènement qui sera rattachée au solde courant du compte */
	pEventBox = gtk_event_box_new ();

	/* Mise en place du style du label en fonction du solde courant */
	if (gsb_real_cmp (gsb_data_account_get_current_balance (account_number),
					  gsb_data_account_get_mini_balance_wanted (account_number)) != -1)
	{
		gtk_widget_set_name (pEventBox, "accueil_solde_normal");
	}
	else
	{
		if (gsb_real_cmp (gsb_data_account_get_current_balance (account_number),
						  gsb_data_account_get_mini_balance_authorized (account_number)) != -1)
		{
			gtk_widget_set_name (pEventBox, "accueil_solde_alarme_low");
		}
		else
		{
			gtk_widget_set_name (pEventBox, "accueil_solde_alarme_high");
		}
	}

	context = gtk_widget_get_style_context  (pEventBox);
	gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);

	g_signal_connect (G_OBJECT (pEventBox),
					  "enter-notify-event",
					  G_CALLBACK (utils_event_box_change_state),
					  context);
	g_signal_connect (G_OBJECT (pEventBox),
					  "leave-notify-event",
					  G_CALLBACK (utils_event_box_change_state),
					  context);
	g_signal_connect_swapped (G_OBJECT (pEventBox),
							  "button-press-event",
							  G_CALLBACK (gsb_main_page_click_on_account),
							  GINT_TO_POINTER (account_number));
	gtk_grid_attach (GTK_GRID (pTable), pEventBox, 2, i, 1, 1);
	gtk_widget_show (pEventBox);
	gtk_container_add (GTK_CONTAINER (pEventBox), pLabel);
	gtk_widget_show (pLabel);
}

/**
 * Création d'une ou des lignes de solde partiels
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gint affiche_soldes_partiels (GtkWidget *table,
									 gint i,
									 gint nb_comptes,
									 GSList *liste,
									 gint currency_number,
									 gint type_compte)
{
	GtkWidget *label;
	gchar *tmp_str;
	gint nbre_lignes = 0;
	gboolean concerne = FALSE;

	while (liste)
	{
		gint partial_number;
		KindAccount kind;

		partial_number = gsb_data_partial_balance_get_number (liste -> data);
		kind = gsb_data_partial_balance_get_kind (partial_number);

		if (kind == -1)
		{
			liste = liste -> next;
			continue;
		}
		else if ((kind == type_compte || (kind < GSB_TYPE_LIABILITIES && type_compte < GSB_TYPE_LIABILITIES))
				 && gsb_data_partial_balance_get_currency (partial_number) == currency_number)
		{
			if (concerne == FALSE)
			{
				/* on commence par une ligne vide */
				gsb_main_page_affiche_ligne_vide (table, i);
				i ++;
				nbre_lignes ++;

				/* On met les titres du sous ensemble solde(s) partiel(s) */
				label = gtk_label_new (NULL);
				if (nb_comptes == 1)
					tmp_str = g_strconcat ("<span weight=\"bold\">", _("Partial balance: "), "</span>", NULL);
				else
					tmp_str = g_strconcat ("<span weight=\"bold\">", _("Partial balances: "), "</span>", NULL);
				gtk_label_set_markup (GTK_LABEL (label), tmp_str);
				g_free (tmp_str);
				utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
				gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group_accueil), label);
				gtk_grid_attach (GTK_GRID (table), label, 0, i, 1, 1);
				gtk_widget_show (label);
				label = gtk_label_new (_("Reconciled balance"));
				utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
				gtk_grid_attach (GTK_GRID (table), label, 1, i, 1, 1);
				gtk_widget_show (label);
				label = gtk_label_new (_("Current balance"));
				utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
				gtk_grid_attach (GTK_GRID (table), label, 2, i, 1, 1);
				gtk_widget_show (label);
				i ++;
				nbre_lignes ++;
				concerne = TRUE;
			}

			gsb_main_page_affiche_ligne_solde_partiel (table, i, partial_number, currency_number);
			i++;
			nbre_lignes ++;
		}
		liste = liste -> next;
	}

	return nbre_lignes;
}

/**
 * Création de la ligne de solde des comptes
 *
 * \param
 * \param
 * \param
 * \param
 * \param
 * \param
 *
 * \return
 **/
static void affiche_solde_des_comptes (GtkWidget *table,
									   gint i,
									   gint nb_comptes,
									   gint currency_number,
									   GsbReal solde_global_courant,
									   GsbReal solde_global_pointe,
									   GrisbiAppConf *a_conf)
{
	GtkWidget *label;
	gchar *tmp_str;

	/* on commence par une ligne vide */
	gsb_main_page_affiche_ligne_vide (table, i);
	i ++;

	/* Première colonne */
	label = gtk_label_new (NULL);
	if (nb_comptes == 1)
		tmp_str = g_strconcat ("<span weight=\"bold\">", _("Global balance: "), "</span>", NULL);
	else if (a_conf->pluriel_final)
		tmp_str = g_strconcat ("<span weight=\"bold\">", "Soldes finaux: ", "</span>", NULL);
	else
		tmp_str = g_strconcat ("<span weight=\"bold\">", _("Global balances: "), "</span>", NULL);
	gtk_label_set_markup (GTK_LABEL (label), tmp_str);
	g_free (tmp_str);

	utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
	gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group_accueil), label);
	gtk_grid_attach (GTK_GRID (table), label, 0, i, 1, 1);
	gtk_widget_show (label);

	/* Deuxième colonne : elle contient le solde total pointé des comptes */
	tmp_str = utils_real_get_string_with_currency (solde_global_pointe, currency_number, TRUE);
	label = gtk_label_new (tmp_str);
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_grid_attach (GTK_GRID (table), label, 1, i, 1, 1);
	gtk_widget_show (label);

	/* Troisième colonne : elle contient le solde total courant des comptes */
	tmp_str = utils_real_get_string_with_currency (solde_global_courant, currency_number, TRUE);
	label = gtk_label_new (tmp_str);
	g_free (tmp_str);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_grid_attach (GTK_GRID (table), label, 2, i, 1, 1);
	gtk_widget_show (label);
}

/**
 * routine qui affiche les comptes dans la bonne frame
 *
 * \param  pTable           table qui contiendra les lignes
 * \param currency_number   devise concernée
 * \param nb_comptes        nombre de comptes à afficher
 * \param new_comptes       nombre de soldes partiels
 * \param type_compte       type de compte à afficher
 *
 * \return
 **/
static void gsb_main_page_diplays_accounts (GtkWidget *pTable,
											gint currency_number,
											gint nb_comptes,
											gint new_comptes,
											gint type_compte,
											GrisbiAppConf *a_conf)
{
	GSList *list_tmp;
	GsbReal solde_global_courant;
	GsbReal solde_global_pointe;
	gint i = 0;
	gint j = 0;

	/* Affichage des comptes et de leur solde */
	i = 1;
	solde_global_courant = null_real;
	solde_global_pointe = null_real;

	/* on traite les numéros des comptes composant le solde partiel si nécessaire */
	if (new_comptes > 0 && a_conf->group_partial_balance_under_accounts)
	{
		GSList *list_partial = NULL;
		gboolean compte_simple = FALSE;

		list_tmp = gsb_data_partial_balance_get_list ();
		while (list_tmp)
		{
			gint tmp_number;
			KindAccount kind;

			tmp_number = gsb_data_partial_balance_get_number (list_tmp -> data);

			kind = gsb_data_partial_balance_get_kind (tmp_number);
			if (kind == type_compte
				|| (kind >= 0 && kind < GSB_TYPE_LIABILITIES && type_compte < GSB_TYPE_LIABILITIES))
			{
				gchar **tab;
				const gchar *liste_cptes;

				liste_cptes = gsb_data_partial_balance_get_liste_cptes (tmp_number);

				tab = g_strsplit (liste_cptes, ";", 0);
				for (j = 0; j < (gint) g_strv_length (tab); j++)
				{
					StructAccountPartial *partial;

					partial = g_malloc0 (sizeof (StructAccountPartial));
					partial->account_number = utils_str_atoi (tab[j]);
					partial->partial_number = tmp_number;
					partial->displayed = FALSE;
					list_partial = g_slist_append (list_partial, partial);
				}

				g_strfreev (tab);
			}
			list_tmp = list_tmp -> next;
		}

		list_tmp = gsb_data_account_get_list_accounts ();
		while (list_tmp)
		{
			gint account_number;
			KindAccount kind;

			account_number = gsb_data_account_get_no_account (list_tmp -> data);

			if (gsb_data_account_get_closed_account (account_number)
				|| gsb_data_account_get_currency (account_number) != currency_number)
			{
				list_tmp = list_tmp -> next;
				continue;
			}

			kind = gsb_data_account_get_kind (account_number);
			if (kind == type_compte
			 || (kind < GSB_TYPE_LIABILITIES && type_compte < GSB_TYPE_LIABILITIES))
			{
				/* on regarde si ce compte appartient à un solde partiel */
				if (gsb_main_page_account_have_partial_balance (account_number, list_partial))
				{
					gchar **tab;
					const gchar *liste_cptes;
					gint partial_number;

					if (partial_buffer->displayed == TRUE)
					{
						list_tmp = list_tmp -> next;
						continue;
					}

					/* on traite le solde partiel si nécessaire */
					if (partial_buffer->partial_number == 0)
					{
						list_tmp = list_tmp -> next;
						continue;
					}
					else
						partial_number = partial_buffer->partial_number;

					/* on affiche tous les comptes du solde partiel */
					/* on affiche la ligne du compte avec les soldes pointé et courant */
					liste_cptes = gsb_data_partial_balance_get_liste_cptes (partial_number);
					tab = g_strsplit (liste_cptes, ";", 0);
					for (j = 0; j < (gint) g_strv_length (tab); j++)
					{
						gint tmp_number;

						tmp_number = utils_str_atoi (tab[j]);
						if (!gsb_main_page_account_get_account_displayed (tmp_number, list_partial))
						{
							if (compte_simple)
							{
								gsb_main_page_affiche_ligne_vide (pTable, i);
								i++;
								compte_simple = FALSE;
							}
							gsb_main_page_affiche_ligne_du_compte (pTable, tmp_number, i);
							solde_global_courant = gsb_real_add (solde_global_courant,
																 gsb_data_account_get_current_balance (tmp_number));
							solde_global_pointe = gsb_real_add (solde_global_pointe,
																gsb_data_account_get_marked_balance (tmp_number));

							partial_buffer->displayed = TRUE;
							i++;
						}
					}
					/* on affiche le solde partiel + une ligne vide */
					gsb_main_page_affiche_solde_partiel_groupe_account (pTable, i, partial_number, currency_number);
					i += 2;
					/* on ajoute une ligne vide pour séparer deux soldes partiels groupés */
					gsb_main_page_affiche_ligne_vide (pTable, i);
					i++;

					g_strfreev (tab);
				}
				else
				{
					/* on affiche la ligne du compte avec les soldes pointé et courant */
					gsb_main_page_affiche_ligne_du_compte (pTable, account_number, i);
					solde_global_courant = gsb_real_add (solde_global_courant,
														 gsb_data_account_get_current_balance (account_number));
					solde_global_pointe = gsb_real_add (solde_global_pointe,
														gsb_data_account_get_marked_balance (account_number));
					i++;
					compte_simple = TRUE;
				}
			}

			list_tmp = list_tmp -> next;
		}
		/* on supprime une ligne exédentaire si on termine par un solde partiel groupé */
		if (!compte_simple)
			i--;

		g_slist_free_full (list_partial, g_free);
		partial_buffer = NULL;
	}
	else
	{
		/* Pour chaque compte non cloturé (pour chaque ligne), */
		/* créer toutes les colonnes et les remplir            */
		list_tmp = gsb_data_account_get_list_accounts ();

		while (list_tmp)
		{
			gint account_number;
			KindAccount kind;

			account_number = gsb_data_account_get_no_account (list_tmp -> data);

			if (gsb_data_account_get_closed_account (account_number)
				|| gsb_data_account_get_currency (account_number) != currency_number)
			{
				list_tmp = list_tmp -> next;
				continue;
			}

			kind = gsb_data_account_get_kind (account_number);
			if (kind == type_compte
				|| (kind < GSB_TYPE_LIABILITIES && type_compte < GSB_TYPE_LIABILITIES))
			{
				/* on affiche la ligne du compte avec les soldes pointé et courant */
				gsb_main_page_affiche_ligne_du_compte (pTable, account_number, i);

				/* ATTENTION : les sommes effectuées ici présupposent que
				   TOUS les comptes sont dans la MÊME DEVISE !!!!!        */
				solde_global_courant = gsb_real_add (solde_global_courant,
													 gsb_data_account_get_current_balance (account_number));
				solde_global_pointe = gsb_real_add (solde_global_pointe,
													gsb_data_account_get_marked_balance (account_number));
				i++;
			}

			list_tmp = list_tmp -> next;
		}
		/* affichage des soldes partiels s'ils existent */
		if (new_comptes > 0 && a_conf->group_partial_balance_under_accounts == 0)
		{
			list_tmp = gsb_data_partial_balance_get_list ();
			if (list_tmp)
				i += affiche_soldes_partiels (pTable, i, new_comptes, list_tmp, currency_number, type_compte);
		}
	}

	/* Création et remplissage de la (nb_comptes + 3)ième ligne du tableau :
	   elle contient la somme des soldes de chaque compte */
	affiche_solde_des_comptes (pTable,
							   i,
							   nb_comptes,
							   currency_number,
							   solde_global_courant,
							   solde_global_pointe,
							   a_conf);
}

/**
 * teste si au moins 1 compte utilise la devise passée en paramètre.
 *
 * \param currency_number
 * \param
 *
 * \return TRUE si un compte utilise la devise FALSE sinon;
 **/
static gboolean gsb_main_page_get_devise_is_used (gint currency_number,
												  gint type_compte)
{
	GSList *list_tmp;

	list_tmp = gsb_data_account_get_list_accounts ();

	while (list_tmp)
	{
		gint i;

		i = gsb_data_account_get_no_account (list_tmp -> data);

		if (gsb_data_account_get_currency (i) == currency_number
			&& !gsb_data_account_get_closed_account (i)
			&& gsb_data_account_get_kind (i) == type_compte)

			return TRUE;

		list_tmp = list_tmp -> next;
	}

	return FALSE;
}

/**
 * Crée la table et sa première ligne
 *
 * \param
 *
 * \return table
 **/
static GtkWidget *gsb_main_page_get_table_for_accounts (void)
{
	GtkWidget *label;
	GtkWidget *table;

	table = gtk_grid_new ();
	gtk_widget_set_margin_start (table, MARGIN_START);
	gtk_widget_set_margin_end (table, MARGIN_END);

	/* Création et remplissage de la première ligne du tableau */
	label = gtk_label_new (chaine_espace);
	gtk_size_group_add_widget (GTK_SIZE_GROUP (size_group_accueil), label);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_grid_attach (GTK_GRID (table), label, 0, 0, 1, 1);

	gtk_widget_show (label);
	label = gtk_label_new (_("Reconciled balance"));
	gtk_widget_set_hexpand (label, TRUE);
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_grid_attach (GTK_GRID (table), label, 1, 0, 1, 1);
	gtk_widget_show (label);

	label = gtk_label_new (_("Current balance"));
	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_widget_set_hexpand (label, TRUE);
	gtk_grid_attach (GTK_GRID (table), label, 2, 0, 1, 1);
	gtk_widget_show (label);

	return table;
}

/**
 * Création de ou des lignes de solde additionnel
 *
 * \param
 * \param
 * \param
 *
 * \return
 **/
static gint affiche_soldes_additionnels (GtkWidget *table,
										 gint i,
										 GSList *liste)
{
	gint nbre_lignes = 0;
	gint currency_number;

	/* on commence par une ligne vide */
	gsb_main_page_affiche_ligne_vide (table, i);
	i ++;
	nbre_lignes ++;

	while (liste)
	{
		gint partial_number;
		KindAccount kind;

		partial_number = gsb_data_partial_balance_get_number (liste -> data);
		kind = gsb_data_partial_balance_get_kind (partial_number);

		if (kind == - 1)
		{
			currency_number = gsb_data_partial_balance_get_currency (partial_number);
			gsb_main_page_affiche_ligne_solde_partiel (table, i, partial_number, currency_number);
			i++;
			nbre_lignes ++;
		}
		liste = liste -> next;
	}

	return nbre_lignes;
}

/**
 * affiche la liste des comptes et leur solde courant dans la frame qui leur
 * est réservée dans l'accueil
 *
 * \param force     TRUE if we want to update all
 *
 * \return
 **/
static void update_liste_comptes_accueil (gboolean force,
										  GrisbiAppConf *a_conf)
{
	GtkWidget *paddingbox;
	GtkWidget *pTable;
	GtkWidget *vbox;
	GSList *devise;
	GSList *list_tmp;
	gchar* tmp_str;
	gint i = 0;
	gint nb_comptes_actif=0;
	gint nb_comptes_bancaires=0;
	gint nb_comptes_passif=0;
	gint new_comptes_actif=0;
	gint new_comptes_bancaires=0;
	gint new_comptes_passif=0;
	gint soldes_mixtes = 0;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (!force
		&& !(w_run->mise_a_jour_liste_comptes_accueil
			 && gsb_data_account_get_number_of_accounts ()))
		return;

	w_run->mise_a_jour_liste_comptes_accueil = FALSE;

	/* Remove previous child */
	utils_container_remove_children (frame_etat_comptes_accueil);

	/* Create the handle vbox  */
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
	gtk_container_add (GTK_CONTAINER (frame_etat_comptes_accueil), vbox);

	/* Préparation de la séparation de l'affichage des comptes en fonction de leur type */
	list_tmp = gsb_data_account_get_list_accounts ();

	while (list_tmp)
	{
		i = gsb_data_account_get_no_account (list_tmp -> data);

		if (!gsb_data_account_get_closed_account (i))
		{
			if (gsb_data_account_get_kind (i) == GSB_TYPE_ASSET)
			{
				nb_comptes_actif++;
		   }
			else if (gsb_data_account_get_kind (i) == GSB_TYPE_LIABILITIES)
			{
				nb_comptes_passif++;
			}
			else
			{
				nb_comptes_bancaires++;
			}
		}
		list_tmp = list_tmp -> next;
	}

	/* ajout des soldes partiels */
	list_tmp = gsb_data_partial_balance_get_list ();
	if (list_tmp)
	{
		while (list_tmp)
		{
			KindAccount kind;
			kind = gsb_data_partial_balance_get_number (list_tmp -> data);

			switch (gsb_data_partial_balance_get_kind (kind))
			{
				case GSB_TYPE_ASSET:
					new_comptes_actif++;
					break;

				case GSB_TYPE_LIABILITIES:
					new_comptes_passif++;
					break;

				case -1:
					soldes_mixtes++;
					break;

				case GSB_TYPE_BANK:
				case GSB_TYPE_CASH:
				default:
					new_comptes_bancaires++;
			}
			list_tmp = list_tmp -> next;
		}
   }

	/* Affichage des comptes bancaires et de caisse */
	if (nb_comptes_bancaires)
	{
		for (devise = gsb_data_currency_get_currency_list (); devise ; devise = devise->next)
		{
			gint currency_number;

			currency_number = gsb_data_currency_get_no_currency (devise -> data);

			if (!gsb_main_page_get_devise_is_used (currency_number, GSB_TYPE_BANK)
				&& !gsb_main_page_get_devise_is_used (currency_number, GSB_TYPE_CASH))
				continue;

			/* Creating the table which will store accounts with their balances. */
			if (a_conf->balances_with_scheduled == FALSE)
				tmp_str = g_strdup_printf (_("Accounts balance in %s at %s"),
								gsb_data_currency_get_name (currency_number),
								gsb_date_today ());
			else
				tmp_str = g_strdup_printf (_("Accounts balance in %s"),
								gsb_data_currency_get_name (currency_number));

			paddingbox = new_paddingbox_with_title (vbox, FALSE, tmp_str);
			g_free (tmp_str);

			pTable = gsb_main_page_get_table_for_accounts ();
			gtk_box_pack_start (GTK_BOX (paddingbox), pTable, FALSE, FALSE, 0);

			/* Affichage des comptes et de leur solde */
			gsb_main_page_diplays_accounts (pTable,
											currency_number,
											nb_comptes_bancaires,
											new_comptes_bancaires,
											GSB_TYPE_BANK | GSB_TYPE_CASH,
											a_conf);

			gtk_widget_show_all (paddingbox);
			gtk_widget_show_all (pTable);
		}
	}

	/* Affichage des comptes de passif */
	if (nb_comptes_passif)
	{
		for (devise = gsb_data_currency_get_currency_list (); devise ; devise = devise->next)
		{
			gint currency_number;

			currency_number = gsb_data_currency_get_no_currency (devise -> data);

			if (!gsb_main_page_get_devise_is_used (currency_number, GSB_TYPE_LIABILITIES))
				continue;

			/* Creating the table which will store accounts with their balances   */
			if (a_conf->balances_with_scheduled == FALSE)
				tmp_str = g_strdup_printf (_("Liabilities accounts balance in %s at %s"),
										   gsb_data_currency_get_name (currency_number),
										   gsb_date_today ());
			else
				tmp_str = g_strdup_printf (_("Liabilities accounts balance in %s"),
										   gsb_data_currency_get_name (currency_number));

			paddingbox = new_paddingbox_with_title (vbox, FALSE, tmp_str);
			g_free (tmp_str);

			pTable = gsb_main_page_get_table_for_accounts ();
			gtk_box_pack_start (GTK_BOX (paddingbox), pTable, FALSE, FALSE, 0);

			/* Affichage des comptes et de leur solde */
			gsb_main_page_diplays_accounts (pTable,
											currency_number,
											nb_comptes_passif,
											new_comptes_passif,
											GSB_TYPE_LIABILITIES,
											a_conf);

			gtk_widget_show_all (paddingbox);
			gtk_widget_show_all (pTable);
		}
	}

	/* Affichage des comptes d'actif */
	if (nb_comptes_actif)
	{
		for (devise = gsb_data_currency_get_currency_list (); devise ; devise = devise->next)
		{
			gint currency_number;

			currency_number = gsb_data_currency_get_no_currency (devise -> data);

		   if (!gsb_main_page_get_devise_is_used (currency_number, GSB_TYPE_ASSET))
				continue;

			/* Creating the table which will store accounts with their balances    */
			if (a_conf->balances_with_scheduled == FALSE)
				tmp_str = g_strdup_printf (_("Assets accounts balance in %s at %s"),
										   gsb_data_currency_get_name (currency_number),
										   gsb_date_today ());
			else
				tmp_str = g_strdup_printf (_("Assets accounts balance in %s"),
										   gsb_data_currency_get_name (currency_number));

			paddingbox = new_paddingbox_with_title (vbox, FALSE, tmp_str);
			g_free (tmp_str);

			pTable = gsb_main_page_get_table_for_accounts ();
			gtk_box_pack_start (GTK_BOX (paddingbox), pTable, FALSE, FALSE, 0);

			/* Affichage des comptes et de leur solde */
			gsb_main_page_diplays_accounts (pTable,
											currency_number,
											nb_comptes_actif,
											new_comptes_actif,
											GSB_TYPE_ASSET,
											a_conf);

			gtk_widget_show_all (paddingbox);
			gtk_widget_show_all (pTable);
		}
	}

	/* Affichage des soldes mixtes */
	if (soldes_mixtes > 0)
	{
		gchar *tmp_str_2;

		if (soldes_mixtes == 1)
			tmp_str_2 = g_strdup (_("Additional balance"));
		else
			tmp_str_2 = g_strdup (_("Additional balances"));
		if (a_conf->balances_with_scheduled == FALSE)
		{
			tmp_str = g_strconcat (tmp_str_2, _(" at "), gsb_date_today (), NULL);
			g_free (tmp_str_2);
		}
		else
			tmp_str = tmp_str_2;

		paddingbox = new_paddingbox_with_title (vbox, FALSE, tmp_str);
		g_free (tmp_str);

		pTable = gsb_main_page_get_table_for_accounts ();
		gtk_box_pack_start (GTK_BOX (paddingbox), pTable, FALSE, FALSE, 0);

		list_tmp = gsb_data_partial_balance_get_list ();

		if (list_tmp)
			affiche_soldes_additionnels (pTable, i, list_tmp);
	}

	gtk_widget_show_all (vbox);
}

/**
 * Fonction appelée lorsqu'on clicke sur une échéance à saisir
 *
 * \param event_box         object clicked
 * \param event
 * \param scheduled_number  scheduled transaction numnber
 *
 * \return
 **/
static gboolean saisie_echeance_accueil (GtkWidget *event_box,
										 GdkEventButton *event,
										 gint scheduled_number)
{
	GtkWidget *dialog;
	GtkWidget *button;
	GtkWidget *button_cancel;
	GtkWidget *button_OK;
	GtkWidget *form_transaction_part;
	GtkWidget *hbox;
	GtkWidget *parent_save;
	gint result;
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	form_transaction_part = gsb_form_get_form_transaction_part ();
	parent_save = gtk_widget_get_parent (form_transaction_part);

	/* Create the dialog */
	dialog = gtk_dialog_new_with_buttons (_("Enter a scheduled transaction"),
										  GTK_WINDOW (grisbi_app_get_active_window (NULL)),
										  GTK_DIALOG_MODAL,
										  NULL, NULL,
										  NULL);

	button_cancel = gtk_button_new_with_label (_("Cancel"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_cancel, GTK_RESPONSE_CANCEL);
	gtk_widget_set_can_default (button_cancel, TRUE);

	button_OK = gtk_button_new_with_label (_("Validate"));
	gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button_OK, GTK_RESPONSE_OK);
	gtk_widget_set_can_default (button_OK, TRUE);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
	gtk_widget_set_size_request (dialog, 700, -1);
	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	/* first we reparent the form in the dialog */
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_pack_start (GTK_BOX (dialog_get_content_area (dialog)), hbox, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER(hbox), BOX_BORDER_WIDTH);

	/* gtk_widget_reparent is broken according to upstream gtk+ devs, so use
	 * gtk_container_add/remove instead to prevent segfaults. */
	g_object_ref (form_transaction_part);
	gtk_container_remove (GTK_CONTAINER (parent_save), form_transaction_part);
	gtk_container_add (GTK_CONTAINER (hbox), form_transaction_part);
	gtk_widget_show_all (hbox);

	/* next we fill the form,
	 * don't use gsb_form_show because we are neither on transactions list, neither scheduled list */
	button = gsb_form_scheduler_get_element_widget(SCHEDULED_FORM_ACCOUNT);
	g_signal_handlers_block_by_func (G_OBJECT (button),
									 G_CALLBACK (gsb_form_scheduler_change_account),
									 NULL);
	gsb_form_clean (gsb_data_scheduled_get_account_number (scheduled_number));

	/* fill the form with the scheduled transaction */
	gsb_scheduler_list_execute_transaction(scheduled_number);

	g_signal_handlers_unblock_by_func (G_OBJECT (button),
									   G_CALLBACK (gsb_form_scheduler_change_account),
									   NULL);

	gtk_widget_show_all (dialog);

	result = gtk_dialog_run (GTK_DIALOG (dialog));

	if (result == GTK_RESPONSE_OK)
		gsb_form_finish_edition ();

	gtk_container_remove (GTK_CONTAINER (hbox), form_transaction_part);
	gtk_container_add (GTK_CONTAINER (parent_save), form_transaction_part);
	g_object_unref (form_transaction_part);
	gtk_widget_destroy (dialog);

	/* update the home page */
	update_liste_echeances_manuelles_accueil (TRUE);
	update_liste_comptes_accueil (TRUE, a_conf);
	return FALSE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void update_liste_echeances_manuelles_accueil (gboolean force)
{
	GrisbiWinRun *w_run;

	devel_debug_int (force);
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	/* need to set that in first because can change mise_a_jour_liste_echeances_manuelles_accueil */
	gsb_scheduler_check_scheduled_transactions_time_limit ();

	if (!force && !w_run->mise_a_jour_liste_echeances_manuelles_accueil)
		return;

	w_run->mise_a_jour_liste_echeances_manuelles_accueil = FALSE;

	if (scheduled_transactions_to_take)
	{
		GtkWidget *event_box;
		GtkWidget *hbox;
		GtkWidget *label;
		GtkWidget *vbox;
		GSList *pointeur_liste;
		gint manual = 1;

		/* s'il y avait déjà un fils dans la frame, le détruit */
		utils_container_remove_children (frame_etat_echeances_manuelles_accueil);

		/* on affiche la seconde frame dans laquelle on place les échéances à saisir */
		show_paddingbox (frame_etat_echeances_manuelles_accueil);

		/* on y place la liste des échéances */
		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
		gtk_container_add (GTK_CONTAINER(frame_etat_echeances_manuelles_accueil), vbox);
		gtk_widget_show (vbox);

		/* on met une ligne vide pour faire joli */
		label = gtk_label_new (NULL);

		/* création du style normal -> bleu */
		/* pointeur dessus -> jaune-rouge */

		gtk_widget_destroy (label);

		pointeur_liste = g_slist_sort_with_data (scheduled_transactions_to_take,
												 (GCompareDataFunc) classement_gslist_echeance_par_date,
												 GINT_TO_POINTER (manual));

		while (pointeur_liste)
		{
			gint scheduled_number;
			gint account_number;
			gint currency_number;
			gchar* tmp_str;
			gchar* tmp_str2;
			GtkStyleContext* context;
			gchar *date;

			scheduled_number = GPOINTER_TO_INT (pointeur_liste -> data);
			account_number = gsb_data_scheduled_get_account_number (scheduled_number);
			currency_number = gsb_data_scheduled_get_currency_number (scheduled_number);

			hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
			gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
			gtk_widget_show (hbox);

			/* bouton à gauche */
			event_box = gtk_event_box_new ();
			gtk_widget_set_name (event_box, "accueil_nom_compte");
			context = gtk_widget_get_style_context  (event_box);
			gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);

			g_signal_connect (G_OBJECT (event_box),
							  "enter-notify-event",
							  G_CALLBACK (utils_event_box_change_state),
							  context);
			g_signal_connect (G_OBJECT (event_box),
							  "leave-notify-event",
							  G_CALLBACK (utils_event_box_change_state),
							  context);
			g_signal_connect (G_OBJECT (event_box),
							  "button-press-event",
							  G_CALLBACK (saisie_echeance_accueil),
							  GINT_TO_POINTER (scheduled_number));
			gtk_box_pack_start (GTK_BOX (hbox), event_box, TRUE, TRUE, 5);
			gtk_widget_show (event_box);

			date = gsb_format_gdate (gsb_data_scheduled_get_date (scheduled_number));
			tmp_str = g_strconcat (date,
								   " : ",
								   gsb_data_payee_get_name (gsb_data_scheduled_get_party_number (scheduled_number),
															FALSE),
								  NULL);
			label = gtk_label_new (tmp_str);
			g_free (tmp_str);
			g_free(date);

			utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
			gtk_container_add (GTK_CONTAINER (event_box), label);
			gtk_widget_show (label);

			/* label à droite */
			if (gsb_data_scheduled_get_amount (scheduled_number).mantissa >= 0)
			{

				tmp_str2 = utils_real_get_string_with_currency (gsb_data_scheduled_get_amount (scheduled_number),
																currency_number,
																TRUE);
				tmp_str = g_strdup_printf (_("%s credited on %s"),
										   tmp_str2,
										   gsb_data_account_get_name (account_number));
				g_free (tmp_str2);
				label = gtk_label_new (tmp_str);
				g_free (tmp_str);
			}
			else
			{
				tmp_str2 = utils_real_get_string_with_currency (gsb_real_abs (gsb_data_scheduled_get_amount
																			  (scheduled_number)),
																currency_number,
																TRUE);
				tmp_str = g_strdup_printf (_("%s debited on %s"),
										   tmp_str2,
										   gsb_data_account_get_name (account_number));
				g_free (tmp_str2);
				label = gtk_label_new (tmp_str);
				g_free (tmp_str);
			}

			utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
			gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);
			gtk_widget_show (label);

			pointeur_liste = pointeur_liste -> next;
		}
	}
	else
	{
		hide_paddingbox (frame_etat_echeances_manuelles_accueil);
	}
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void update_liste_echeances_auto_accueil (gboolean force)
{
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (!force && !w_run->mise_a_jour_liste_echeances_auto_accueil)
		return;

	devel_debug_int (force);

	w_run->mise_a_jour_liste_echeances_auto_accueil = FALSE;

	if (scheduled_transactions_taken)
	{
		GtkWidget *event_box;
		GtkWidget *hbox;
		GtkWidget *label;
		GtkWidget *vbox;
		GSList *pointeur_liste;
		gint manual = 0;

		/* s'il y avait déjà un fils dans la frame, le détruit */
		utils_container_remove_children (frame_etat_echeances_auto_accueil);

		/* on affiche la seconde frame dans laquelle on place les échéances à saisir */
		show_paddingbox (frame_etat_echeances_auto_accueil);

		/* on y place la liste des échéances */
		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
		gtk_container_add (GTK_CONTAINER (frame_etat_echeances_auto_accueil), vbox);
		gtk_widget_show (vbox);

		pointeur_liste = g_slist_sort_with_data (scheduled_transactions_taken,
												 (GCompareDataFunc) classement_gslist_echeance_par_date,
												 GINT_TO_POINTER (manual));

		while (pointeur_liste)
		{
			GtkStyleContext* context;
			gchar *date;
			gchar* tmp_str;
			gchar* tmp_str2;
			gint account_number;
			gint currency_number;
			gint transaction_number;

			transaction_number = GPOINTER_TO_INT (pointeur_liste -> data);
			account_number = gsb_data_transaction_get_account_number (transaction_number);
			currency_number = gsb_data_transaction_get_currency_number (transaction_number);

			hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, MARGIN_BOX);
			gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
			gtk_widget_show (hbox);

			event_box = gtk_event_box_new ();
			gtk_widget_set_name (event_box, "accueil_nom_compte");
			context = gtk_widget_get_style_context  (event_box);
			gtk_style_context_set_state (context, GTK_STATE_FLAG_ACTIVE);

			g_signal_connect (G_OBJECT (event_box),
							  "enter-notify-event",
							  G_CALLBACK (utils_event_box_change_state),
							  context);
			g_signal_connect (G_OBJECT (event_box),
							  "leave-notify-event",
							  G_CALLBACK (utils_event_box_change_state),
							  context);
			g_signal_connect_swapped (G_OBJECT (event_box),
									  "button-press-event",
									  G_CALLBACK (gsb_transactions_list_edit_transaction_by_pointer),
									  GINT_TO_POINTER (transaction_number));
			gtk_widget_show (event_box);

			/* label à gauche */
			date = gsb_format_gdate (gsb_data_transaction_get_date (transaction_number));
			tmp_str = g_strconcat (date,
								   " : ",
								   gsb_data_payee_get_name (gsb_data_transaction_get_party_number
															(transaction_number),
															FALSE),
								  NULL);
			label = gtk_label_new (tmp_str);
			g_free (tmp_str);
			g_free(date);

			utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
			gtk_box_pack_start (GTK_BOX (hbox), event_box, TRUE, TRUE, 5);
			gtk_container_add (GTK_CONTAINER (event_box), label);
			gtk_widget_show (label);

			/* label à droite */
			if (gsb_data_transaction_get_amount (transaction_number).mantissa >= 0)
			{
				tmp_str2 = utils_real_get_string_with_currency (gsb_data_transaction_get_amount
																(transaction_number),
																currency_number,
																TRUE);
				tmp_str = g_strdup_printf (_("%s credited on %s"),
										   tmp_str2,
										   gsb_data_account_get_name (account_number));
				g_free (tmp_str2);
				label = gtk_label_new (tmp_str);
				g_free (tmp_str);
			}
			else
			{
				tmp_str2 = utils_real_get_string_with_currency (gsb_real_abs
																(gsb_data_transaction_get_amount
																 (transaction_number)),
																currency_number,
																TRUE);
				tmp_str = g_strdup_printf (_("%s debited on %s"),
										   tmp_str2,
										   gsb_data_account_get_name (account_number));
				g_free (tmp_str2);
				label = gtk_label_new (tmp_str);
				g_free (tmp_str);
			}

			utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
			gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 5);
			gtk_widget_show (label);

			pointeur_liste = pointeur_liste -> next;
		}
	}
	else
	{
		hide_paddingbox (frame_etat_echeances_auto_accueil);
	}
}

/**
 * Fonction update_soldes_minimaux
 * vérifie les soldes de tous les comptes, affiche un message d'alerte si nécessaire
 *
 * \param
 *
 * \return
 **/
static void update_soldes_minimaux (gboolean force,
									GrisbiAppConf *a_conf)
{
	GtkWidget *label;
	GtkWidget *vbox_1;
	GtkWidget *vbox_2;
	GSList *list_tmp;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (!force && !w_run->mise_a_jour_soldes_minimaux)
		return;

	devel_debug (NULL);
	w_run->mise_a_jour_soldes_minimaux = FALSE;

	/* s'il y avait déjà un fils dans la frame, le détruit */
	utils_container_remove_children (frame_etat_soldes_minimaux_autorises);
	utils_container_remove_children (frame_etat_soldes_minimaux_voulus);

	hide_paddingbox (frame_etat_soldes_minimaux_autorises);
	hide_paddingbox (frame_etat_soldes_minimaux_voulus);

	vbox_1 = NULL;
	vbox_2 = NULL;

	list_tmp = gsb_data_account_get_list_accounts ();
	while (list_tmp)
	{
		gint i;

		i = gsb_data_account_get_no_account (list_tmp -> data);

		if (gsb_data_account_get_closed_account (i) && !a_conf->show_closed_accounts)
		{
			list_tmp = list_tmp -> next;
			continue;
		}

		if (gsb_real_cmp (gsb_data_account_get_current_balance (i),
						  gsb_data_account_get_mini_balance_authorized (i)) == -1
			&& gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES)
		{
			if (!vbox_1)
			{
				vbox_1 = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
				gtk_box_set_homogeneous (GTK_BOX (vbox_1), TRUE);
				gtk_container_add (GTK_CONTAINER (frame_etat_soldes_minimaux_autorises), vbox_1);
				gtk_widget_show (vbox_1);
				show_paddingbox (frame_etat_soldes_minimaux_autorises);
			}
			label = gtk_label_new (gsb_data_account_get_name (i));
			gtk_box_pack_start (GTK_BOX (vbox_1), label, FALSE, FALSE, 0);
			utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_TOP);
			gtk_widget_show (label);

			show_paddingbox (frame_etat_soldes_minimaux_autorises);
		}

		if (gsb_real_cmp (gsb_data_account_get_current_balance (i),
						  gsb_data_account_get_mini_balance_wanted (i)) == -1
			&& gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES
			&& gsb_real_cmp (gsb_data_account_get_current_balance (i),
							 gsb_data_account_get_mini_balance_authorized (i)) == -1
			&& gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES)
		{
			if (!vbox_2)
			{
				vbox_2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
				gtk_box_set_homogeneous (GTK_BOX (vbox_2), TRUE);
				gtk_container_add (GTK_CONTAINER (frame_etat_soldes_minimaux_voulus), vbox_2);
				gtk_widget_show (vbox_2);
				show_paddingbox (frame_etat_soldes_minimaux_voulus);
			}

			label = gtk_label_new (gsb_data_account_get_name (i));
			utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
			gtk_box_pack_start (GTK_BOX (vbox_2), label, FALSE, FALSE, 0);
			gtk_widget_show (label);

			show_paddingbox (frame_etat_soldes_minimaux_voulus);
		}

		list_tmp = list_tmp -> next;
	}

	/* on affiche une boite d'avertissement si nécessaire */
	affiche_dialogue_soldes_minimaux ();
	w_run->mise_a_jour_liste_comptes_accueil = TRUE;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
static void update_fin_comptes_passifs (gboolean force,
										GrisbiAppConf *a_conf)
{
	GtkWidget *label;
	GtkWidget *vbox;
	GSList *liabilities_account;
	GSList *pointeur;
	GSList *list_tmp;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (!force && !w_run->mise_a_jour_fin_comptes_passifs)
		return;

	devel_debug (NULL);

	w_run->mise_a_jour_fin_comptes_passifs = FALSE;

	utils_container_remove_children (frame_etat_fin_compte_passif);
	hide_paddingbox (frame_etat_fin_compte_passif);

	if (!a_conf->show_closed_accounts)
		return;

	list_tmp = gsb_data_account_get_list_accounts ();
	liabilities_account = NULL;

	while (list_tmp)
	{
		gint i;

		i = gsb_data_account_get_no_account (list_tmp -> data);

		if (gsb_data_account_get_kind (i) == GSB_TYPE_LIABILITIES
			 && gsb_data_account_get_current_balance (i).mantissa >= 0)
			liabilities_account = g_slist_append (liabilities_account, gsb_data_account_get_name (i));

		list_tmp = list_tmp -> next;
	}

	if (g_slist_length (liabilities_account))
	{
		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add (GTK_CONTAINER (frame_etat_fin_compte_passif), vbox);
		gtk_widget_show (vbox);

		pointeur = liabilities_account;
		while (pointeur)
		{
			label = gtk_label_new (pointeur -> data);
			gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
			utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
			gtk_widget_show (label);

			pointeur = pointeur -> next;
		}

		show_paddingbox (frame_etat_fin_compte_passif);
	}
}

/******************************************************************************/
/* Public functions                                                             */
/******************************************************************************/
/**
 * Create the home page of Grisbi
 *
 * \param   none
 *
 * \return
 **/
GtkWidget *creation_onglet_accueil (void)
{
	GtkWidget *base;
	GtkWidget *base_scroll;
	GtkWidget *eb;
	GtkWidget *paddingbox;
	GtkWidget *vbox;
	GrisbiAppConf *a_conf;
	GrisbiWinEtat *w_etat;
	GrisbiWinRun *w_run;

	devel_debug (NULL);
	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();
	w_etat = grisbi_win_get_w_etat ();
	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();

	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
	gtk_widget_show (vbox);

	/* on met le titre du fichier */
	hbox_title = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_widget_set_size_request (hbox_title, -1, LOGO_HEIGHT);

	eb = gtk_event_box_new ();
	gtk_widget_set_name (hbox_title, "grey_box");

	label_accounting_entity = gtk_label_new (NULL);
	g_object_add_weak_pointer (G_OBJECT (label_accounting_entity),
								(gpointer*)&label_accounting_entity);

	if (w_etat->utilise_logo)
	{
		logo_accueil =  gtk_image_new_from_pixbuf (gsb_select_icon_get_logo_pixbuf ());
		gtk_box_pack_start (GTK_BOX (hbox_title), logo_accueil, FALSE, FALSE, 20);
		gtk_widget_set_size_request (hbox_title, -1, LOGO_HEIGHT + 20);
	}

	gtk_box_pack_end (GTK_BOX (hbox_title), label_accounting_entity, TRUE, TRUE, 20);
	gtk_container_set_border_width (GTK_CONTAINER (hbox_title), 6);
	gtk_container_add (GTK_CONTAINER (eb), hbox_title);
	gtk_box_pack_start (GTK_BOX (vbox), eb, FALSE, FALSE, 0);
	gtk_widget_show_all (eb);

	/* on crée à ce niveau base_scroll qui est aussi une vbox mais qui peut
	   scroller verticalement */
	base_scroll = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (base_scroll),
									GTK_POLICY_NEVER,
									GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (base_scroll),
										 GTK_SHADOW_IN);
	gtk_widget_set_vexpand (base_scroll, TRUE);

	base = gtk_box_new (GTK_ORIENTATION_VERTICAL, MARGIN_BOX);
	gtk_widget_set_margin_start (base, MARGIN_START);
	gtk_widget_set_margin_end (base, MARGIN_END);

	gtk_container_set_border_width (GTK_CONTAINER (base), BOX_BORDER_WIDTH);
	gtk_container_add (GTK_CONTAINER (base_scroll), base);
	gtk_widget_show (base_scroll);
	gtk_widget_show (base);

	/* on crée le size_group pour l'alignement des tableaux */
	size_group_accueil = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	/* on crée la première frame dans laquelle on met les états des comptes */
	frame_etat_comptes_accueil = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (base), frame_etat_comptes_accueil, FALSE, FALSE, 0);

	/* on met la liste des comptes et leur état dans la frame */
	w_run->mise_a_jour_liste_comptes_accueil = TRUE;
	gtk_widget_show_all (frame_etat_comptes_accueil);


	/* mise en place de la partie fin des comptes passif */
	paddingbox = new_paddingbox_with_title (base, FALSE, _("Closed liabilities accounts"));
	frame_etat_fin_compte_passif = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_pack_start (GTK_BOX (paddingbox), frame_etat_fin_compte_passif, FALSE, FALSE, 0);
	w_run->mise_a_jour_fin_comptes_passifs = TRUE;


	/* mise en place de la partie des échéances manuelles (non affiché) */
	/* sera mis à jour automatiquement si nécessaire */
	paddingbox = new_paddingbox_with_title (base, FALSE, _("Run out manual scheduled transactions"));
	frame_etat_echeances_manuelles_accueil = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_spacing (GTK_BOX (paddingbox), 6);
	gtk_box_pack_start (GTK_BOX (paddingbox), frame_etat_echeances_manuelles_accueil, FALSE, FALSE, 6);


	/* mise en place de la partie des échéances auto  (non affiché)*/
	/* sera mis à jour automatiquement si nécessaire */
	paddingbox = new_paddingbox_with_title (base, FALSE, _("Automatic scheduled transactions entered"));
	frame_etat_echeances_auto_accueil = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_spacing (GTK_BOX (paddingbox), 6);
	gtk_box_pack_start (GTK_BOX (paddingbox), frame_etat_echeances_auto_accueil, FALSE, FALSE, 6);


	/* partie des fin d'échéances */
	paddingbox = new_paddingbox_with_title (base, FALSE, _("Closed scheduled transactions"));
	main_page_finished_scheduled_transactions_part = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_spacing (GTK_BOX (paddingbox), 6);
	gtk_box_pack_start (GTK_BOX (paddingbox), main_page_finished_scheduled_transactions_part, FALSE, FALSE, 6);


	/* partie des soldes minimaux autorisés */
	paddingbox = new_paddingbox_with_title (base, FALSE, _("Accounts under authorized balance"));
	frame_etat_soldes_minimaux_autorises = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_spacing (GTK_BOX (paddingbox), 6);
	gtk_box_pack_start (GTK_BOX (paddingbox), frame_etat_soldes_minimaux_autorises, FALSE, FALSE, 6);


	/* partie des soldes minimaux voulus */
	paddingbox = new_paddingbox_with_title (base, FALSE, _("Accounts under desired balance"));
	frame_etat_soldes_minimaux_voulus = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	gtk_box_set_spacing (GTK_BOX (paddingbox), 6);
	gtk_box_pack_start (GTK_BOX (paddingbox), frame_etat_soldes_minimaux_voulus, FALSE, FALSE, 6);

	w_run->mise_a_jour_soldes_minimaux = TRUE;

	gtk_box_pack_start (GTK_BOX (vbox), base_scroll, TRUE, TRUE, 0);

	update_liste_comptes_accueil (TRUE, a_conf);

	return (vbox);
}

/**
 * update the first page, force the updating if asked,
 * else, each function will decide if it need to be
 * updated or not
 *
 * \param force TRUE if we want to update all
 *
 * \return
 **/
void mise_a_jour_accueil (gboolean force)
{
	GrisbiAppConf *a_conf;

	a_conf = (GrisbiAppConf *) grisbi_app_get_a_conf ();

	update_liste_comptes_accueil (force, a_conf);
	update_liste_echeances_manuelles_accueil (force);
	update_liste_echeances_auto_accueil (force);
	update_soldes_minimaux (force, a_conf);
	update_fin_comptes_passifs (force, a_conf);
}

/**
 * cette fonction vérifie les soldes minimaux et affiche une boite de dialogue
 * avec les comptes en dessous des seuils si non désactivé
 *
 * \param   none
 *
 * \return
 **/
void affiche_dialogue_soldes_minimaux (void)
{
	GSList *liste_autorise;
	GSList *liste_autorise_et_voulu;
	GSList *liste_voulu;
	GSList *list_tmp;
	gchar *texte_affiche;
	GrisbiWinRun *w_run;

	w_run = (GrisbiWinRun *) grisbi_win_get_w_run ();
	if (!w_run->mise_a_jour_soldes_minimaux)
		return;

	liste_autorise = NULL;
	liste_voulu = NULL;
	liste_autorise_et_voulu = NULL;

	list_tmp = gsb_data_account_get_list_accounts ();

	while (list_tmp)
	{
		gint i;

		i = gsb_data_account_get_no_account (list_tmp -> data);

		if (gsb_real_cmp (gsb_data_account_get_current_balance (i),
						  gsb_data_account_get_mini_balance_authorized (i)) == -1
			&& gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES
			&& !gsb_data_account_get_mini_balance_authorized_message (i))
		{
			if (gsb_real_cmp (gsb_data_account_get_current_balance (i),
							  gsb_data_account_get_mini_balance_wanted (i)) == -1)
			{
				liste_autorise_et_voulu = g_slist_append (liste_autorise_et_voulu,
														  gsb_data_account_get_name (i));
				gsb_data_account_set_mini_balance_wanted_message (i, 1);
			}
			else
			{
				liste_autorise = g_slist_append (liste_autorise, gsb_data_account_get_name (i));
			}
			gsb_data_account_set_mini_balance_authorized_message (i, 1);
		}

		if (gsb_real_cmp (gsb_data_account_get_current_balance (i),
						  gsb_data_account_get_mini_balance_wanted (i)) == -1
			&& gsb_real_cmp (gsb_data_account_get_current_balance (i),
							 gsb_data_account_get_mini_balance_authorized (i)) == 1
			&& gsb_data_account_get_kind (i) != GSB_TYPE_LIABILITIES
			&& !gsb_data_account_get_mini_balance_wanted_message (i))
		{
			liste_voulu = g_slist_append (liste_voulu, gsb_data_account_get_name (i));
			gsb_data_account_set_mini_balance_wanted_message (i, 1);
		}

		/* 	si on repasse au dessus des seuils, c'est comme si on n'avait rien affiché */
		if (gsb_real_cmp (gsb_data_account_get_current_balance (i),
						  gsb_data_account_get_mini_balance_authorized (i)) == 1)
			gsb_data_account_set_mini_balance_authorized_message (i, 0);
		if (gsb_real_cmp (gsb_data_account_get_current_balance (i),
						  gsb_data_account_get_mini_balance_wanted (i)) == 1)
			gsb_data_account_set_mini_balance_wanted_message (i, 0);

		list_tmp = list_tmp -> next;
	}

	/*     on crée le texte récapilutatif */
	texte_affiche = g_strdup("");

	if (liste_autorise_et_voulu)
	{
		if (g_slist_length (liste_autorise_et_voulu) == 1)
			texte_affiche = g_strdup_printf (_("balance of account %s is under desired "
											   "and authorised minima!"),
											 (gchar *) liste_autorise_et_voulu -> data);
		else
		{
			texte_affiche = g_strdup(_("accounts with the balance under desired and "
									   "authorised minimal:\n\n"));
			list_tmp = liste_autorise_et_voulu;
			while (list_tmp)
			{
				gchar* old_str;

				old_str = texte_affiche;
				texte_affiche = g_strconcat (old_str, list_tmp -> data, "\n", NULL);
				g_free (old_str);

				list_tmp = list_tmp -> next;
			}
		}
	}

	if (liste_autorise)
	{
		gchar* old_str;

		if (strlen (texte_affiche))
		{
			old_str = texte_affiche;
			texte_affiche = g_strconcat (old_str, "\n\n", NULL);
			g_free (old_str);
		}

		if (g_slist_length (liste_autorise) == 1)
		{
			gchar* tmp_str;

			old_str = texte_affiche;
			tmp_str = g_strdup_printf (_("balance of account %s is under authorised minimum!"),
									   (gchar *) liste_autorise -> data);
			texte_affiche = g_strconcat (old_str, tmp_str , NULL);
			g_free (old_str);
			g_free (tmp_str);
		}
		else
		{
			old_str = texte_affiche;
			texte_affiche = g_strconcat (old_str,
										 _("accounts with the balance under authorised minimal:\n\n"),
										 NULL);
			g_free (old_str);
			list_tmp = liste_autorise;
			while (list_tmp)
			{
				old_str = texte_affiche;
				texte_affiche = g_strconcat (old_str, list_tmp -> data, "\n", NULL);
				g_free (old_str);

				list_tmp = list_tmp -> next;
			}
		}
	}

	if (liste_voulu)
	{
		gchar* old_str;

		if (strlen (texte_affiche))
		{
			old_str = texte_affiche;
			texte_affiche = g_strconcat (old_str, "\n\n", NULL);
			g_free (old_str);
		}

		if (g_slist_length (liste_voulu) == 1)
		{
			gchar* tmp_str;

			old_str = texte_affiche;
			tmp_str = g_strdup_printf (_("balance of account %s is under desired minimum!"),
									   (gchar *) liste_voulu -> data);
			texte_affiche = g_strconcat (texte_affiche, tmp_str , NULL);
			g_free (tmp_str);
			g_free (old_str);
		}
		else
		{
			old_str = texte_affiche;
			texte_affiche = g_strconcat (old_str,
										 _("accounts with the balance under desired minimal:\n\n"),
										 NULL);
			g_free (old_str);
			list_tmp = liste_voulu;
			while (list_tmp)
			{
				old_str = texte_affiche;
				texte_affiche = g_strconcat (old_str, list_tmp -> data, "\n", NULL);
				g_free (old_str);

				list_tmp = list_tmp -> next;
			}
		}
	}

	if (strlen (texte_affiche))
		dialogue_message ("minimum-balance-alert", texte_affiche);
	g_free (texte_affiche);
}

/**
 * update the finished scheduled transactions part in the main page
 * the scheduled transaction in param is finished
 *
 * \param scheduled_number
 *
 * \return FALSE
 **/

/* Disable: error: format not a string literal, argument types not checked [-Werror=format-nonliteral]
 2126 |     tmp_str = g_strdup_printf (way, tmp_str2, gsb_data_account_get_name (account_number));
	  |     ^~~~~~~
*/
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#else
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif

gboolean gsb_main_page_update_finished_scheduled_transactions (gint scheduled_number)
{
	GtkWidget *label;
	GtkWidget *hbox;
	gchar *date;
	gchar *tmp_str;
	gchar *tmp_str2;
	gchar *way;
	gint account_number;
	gint currency_number;
	GsbReal amount;

	account_number = gsb_data_scheduled_get_account_number (scheduled_number);
	currency_number = gsb_data_scheduled_get_currency_number (scheduled_number) ;

	/* append in the page the finished scheduled transaction */
	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_box_set_homogeneous (GTK_BOX (hbox), TRUE);
	gtk_widget_show (hbox);

	/* label à gauche */
	date = gsb_format_gdate (gsb_data_scheduled_get_date (scheduled_number));
	tmp_str = g_strconcat (date,
						   " : ",
						   gsb_data_payee_get_name (gsb_data_scheduled_get_party_number
													(scheduled_number),
													FALSE),
						   NULL);
	label = gtk_label_new (tmp_str);

	g_free (tmp_str);
	g_free(date);

	utils_labels_set_alignment (GTK_LABEL (label), MISC_LEFT, MISC_VERT_CENTER);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0);
	gtk_widget_show (label);

	/* label à droite */
	amount = gsb_data_scheduled_get_amount (scheduled_number);

	if (amount.mantissa >= 0)
	{
		amount = gsb_real_abs (amount);
		way = _("%s credited on %s");
	}
	else
	{
		way = _("%s debited on %s");
	}

	tmp_str2 = utils_real_get_string_with_currency (amount, currency_number, TRUE);
	tmp_str = g_strdup_printf (way, tmp_str2, gsb_data_account_get_name (account_number));
	g_free (tmp_str2);

	label = gtk_label_new (tmp_str);
	g_free (tmp_str);

	utils_labels_set_alignment (GTK_LABEL (label), MISC_RIGHT, MISC_VERT_CENTER);
	gtk_box_pack_end (GTK_BOX (hbox), label, FALSE, TRUE, 0);
	gtk_widget_show (label);

	gtk_box_pack_start (GTK_BOX (main_page_finished_scheduled_transactions_part), hbox, FALSE, TRUE, 0);
	gtk_widget_show (label);

	show_paddingbox (main_page_finished_scheduled_transactions_part);

	return FALSE;
}
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#else
#pragma clang diagnostic pop
#endif

/**
 * update the title of the main page
 *
 * \param title
 *
 * \return
 **/
void gsb_main_page_update_homepage_title (const gchar *title)
{
	gchar * tmp_str;

	/* at the first use of grisbi,label_accounting_entity doesn't still exist */
	if (!label_accounting_entity)
		return;

	tmp_str = g_markup_printf_escaped ("<span size=\"x-large\">%s</span>", title);
	gtk_label_set_markup (GTK_LABEL (label_accounting_entity), tmp_str);

	g_free (tmp_str);
}

/**
 *
 *
 * \param
 *
 * \return
 **/
GtkWidget *gsb_main_page_get_logo_accueil (void)
{
	return logo_accueil;
}

/**
 *
 *
 * \param
 *
 * \return
 **/
gboolean gsb_main_page_set_logo_accueil (GtkWidget *logo)
{
	if (logo)
	{
		logo_accueil = logo;
		gtk_box_pack_start (GTK_BOX (hbox_title), logo_accueil, FALSE, FALSE, 0);
		gtk_widget_set_size_request (hbox_title, -1, -1);
		gtk_widget_show (logo_accueil);
	}
	else
	{
		GrisbiWinEtat *w_etat;

		w_etat = grisbi_win_get_w_etat ();
		gtk_widget_destroy (logo_accueil);
		logo_accueil = NULL;
		gtk_widget_set_size_request (hbox_title, -1, -1);
		if (w_etat->name_logo && strlen (w_etat->name_logo))
			g_free (w_etat->name_logo);
		w_etat->name_logo = NULL;
	}

	return (FALSE);
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
