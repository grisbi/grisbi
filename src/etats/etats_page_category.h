#ifndef __ETATS_PAGE_CATEGORY_H__
#define __ETATS_PAGE_CATEGORY_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATS_PAGE_CATEGORY_TYPE    	(etats_page_category_get_type ())
#define ETATS_PAGE_CATEGORY(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATS_PAGE_CATEGORY_TYPE, EtatsPageCategory))
#define WIDGET_IS_ETAT_CATEGORY(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PAGE_CATEGORY_TYPE))

typedef struct _EtatsPageCategory			EtatsPageCategory;
typedef struct _EtatsPageCategoryClass		EtatsPageCategoryClass;


struct _EtatsPageCategory
{
    GtkBox parent;
};

struct _EtatsPageCategoryClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					etats_page_category_get_type				(void) G_GNUC_CONST;

EtatsPageCategory *		etats_page_category_new						(GtkWidget *etats_prefs);
void 					etats_page_category_initialise_onglet		(GtkWidget *etats_prefs,
																	 gint report_number);
void					etats_page_category_get_info				(GtkWidget *etats_prefs,
																	 gint report_number);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATS_PAGE_CATEGORY_H__ */
