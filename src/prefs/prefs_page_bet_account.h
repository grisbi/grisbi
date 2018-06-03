#ifndef __PREFS_PAGE_BET_ACCOUNT_H__
#define __PREFS_PAGE_BET_ACCOUNT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_BET_ACCOUNT_TYPE    	(prefs_page_bet_account_get_type ())
#define PREFS_PAGE_BET_ACCOUNT(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_BET_ACCOUNT_TYPE, PrefsPageBetAccount))
#define PREFS_IS_PAGE_BET_ACCOUNT(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_BET_ACCOUNT_TYPE))

typedef struct _PrefsPageBetAccount          PrefsPageBetAccount;
typedef struct _PrefsPageBetAccountClass     PrefsPageBetAccountClass;


struct _PrefsPageBetAccount
{
    GtkBox parent;
};

struct _PrefsPageBetAccountClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	prefs_page_bet_account_get_type			(void) G_GNUC_CONST;

PrefsPageBetAccount * 	prefs_page_bet_account_new				(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_BET_ACCOUNT_H__ */
