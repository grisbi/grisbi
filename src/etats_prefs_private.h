#ifndef _ETATS_PREFS_PRIVATE_H
#define _ETATS_PREFS_PRIVATE_H

#include <gtk/gtk.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

/*START_GLOBAL_VARIABLES*/
/* liste des plages de date possibles */
static gchar *etats_config_liste_plages_dates[] =
{
    N_("All"),
    N_("Custom"),
    N_("Total to now"),
    N_("Current month"),
    N_("Current year"),
    N_("Current month to now"),
    N_("Current year to now"),
    N_("Previous month"),
    N_("Previous year"),
    N_("Last 30 days"),
    N_("Last 3 months"),
    N_("Last 6 months"),
    N_("Last 12 months"),
    NULL,
};

static gchar *jours_semaine[] =
{
    N_("Monday"),
    N_("Tuesday"),
    N_("Wednesday"),
    N_("Thursday"),
    N_("Friday"),
    N_("Saturday"),
    N_("Sunday"),
    NULL,
};

static gchar *data_separation_periodes[] =
{
    N_("Day"),
    N_("Week"),
    N_("Month"),
    N_("Year"),
    NULL,
};

/* données de classement des opérations */
static gchar *etats_prefs_classement_operations[] =
{
    N_("date"),
    N_("value date"),
    N_("transaction number"),
    N_("payee"),
    N_("category"),
    N_("budgetary line"),
    N_("note"),
    N_("method of payment"),
    N_("cheque/transfer number"),
    N_("voucher"),
    N_("bank reference"),
    N_("reconciliation reference"),
    NULL,
};

/* mémorisation du togglebutton utilisé servira à remplacer son label */
GtkToggleButton *prev_togglebutton = NULL;

/* variables utilisées pour la gestion des tiers*/
GtkTreePath *tiers_selected = NULL;

/* gint last_page */
static gint last_page;

/* builder */
static GtkBuilder *etats_prefs_builder = NULL;
/*END_GLOBAL_VARIABLES*/


struct _EtatsPrefsPrivate
{
    GtkWidget           *hpaned;
};


/*START_PRIVATE_FUNCTIONS*/
static GtkWidget *	etats_prefs_onglet_budgets_create_page 					(gint page);
static GtkWidget *	etats_prefs_onglet_categories_create_page 				(gint page);
static GtkWidget *	etats_prefs_onglet_comptes_create_page 					(gint page);
static GtkWidget *	etats_prefs_onglet_divers_create_page 					(gint page);
static GtkWidget *	etats_prefs_onglet_mode_paiement_create_page 			(gint page);
static GtkWidget *	etats_prefs_onglet_montants_create_page 				(gint page);
static GtkWidget *	etats_prefs_onglet_periode_create_page 					(gint page);
static GtkWidget *	etats_prefs_onglet_textes_create_page 					(gint page);
static GtkWidget *	etats_prefs_onglet_tiers_create_page 					(gint page);
static GtkWidget *	etats_prefs_onglet_virements_create_page 				(gint page);
static GtkWidget *	etats_prefs_onglet_data_grouping_create_page 			(gint page);
static GtkWidget *	etats_prefs_onglet_data_separation_create_page 			(gint page);
static GtkWidget *	etats_prefs_onglet_affichage_generalites_create_page 	(gint page);
static GtkWidget *	etats_prefs_onglet_affichage_titles_create_page 		(gint page);
static GtkWidget *	etats_prefs_onglet_affichage_operations_create_page 	(gint page);
static GtkWidget *	etats_prefs_onglet_affichage_devises_create_page 		(gint page);
/*END_PRIVATE_FUNCTIONS*/


G_END_DECLS

#endif  /* _ETATS_PREFS_H_ */
