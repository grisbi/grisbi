#ifndef __ETATS_PAGE_BUDGET_H__
#define __ETATS_PAGE_BUDGET_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATS_PAGE_BUDGET_TYPE    	(etats_page_budget_get_type ())
#define ETATS_PAGE_BUDGET(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATS_PAGE_BUDGET_TYPE, EtatsPageBudget))
#define WIDGET_IS_ETAT_BUDGET(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PAGE_BUDGET_TYPE))

typedef struct _EtatsPageBudget				EtatsPageBudget;
typedef struct _EtatsPageBudgetClass		EtatsPageBudgetClass;


struct _EtatsPageBudget
{
    GtkBox parent;
};

struct _EtatsPageBudgetClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					etats_page_budget_get_type				(void) G_GNUC_CONST;

EtatsPageBudget *		etats_page_budget_new					(GtkWidget *etats_prefs);
void 					etats_page_budget_initialise_onglet		(GtkWidget *etats_prefs,
																	 gint report_number);
void					etats_page_budget_get_info				(GtkWidget *etats_prefs,
																	 gint report_number);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATS_PAGE_BUDGET_H__ */
