#ifndef __ETATS_PAGE_ACCOUNTS_H__
#define __ETATS_PAGE_ACCOUNTS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATS_PAGE_ACCOUNTS_TYPE    	(etats_page_accounts_get_type ())
#define ETATS_PAGE_ACCOUNTS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATS_PAGE_ACCOUNTS_TYPE, EtatsPageAccounts))
#define WIDGET_IS_ETAT_ACCOUNTS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PAGE_ACCOUNTS_TYPE))

typedef struct _EtatsPageAccounts			EtatsPageAccounts;
typedef struct _EtatsPageAccountsClass		EtatsPageAccountsClass;


struct _EtatsPageAccounts
{
    GtkBox parent;
};

struct _EtatsPageAccountsClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					etats_page_accounts_get_type			(void) G_GNUC_CONST;

EtatsPageAccounts *		etats_page_accounts_new					(GtkWidget *etats_prefs);

void					etats_page_accounts_get_info			(GtkWidget *etats_prefs,
																 gint report_number);

void 					etats_page_accounts_initialise_onglet	(GtkWidget *etats_prefs,
																 gint report_number);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATS_PAGE_ACCOUNTS_H__ */
