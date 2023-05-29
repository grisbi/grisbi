#ifndef __ETATS_PAGE_PAYEE_H__
#define __ETATS_PAGE_PAYEE_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATS_PAGE_PAYEE_TYPE    	(etats_page_payee_get_type ())
#define ETATS_PAGE_PAYEE(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATS_PAGE_PAYEE_TYPE, EtatsPagePayee))
#define WIDGET_IS_ETAT_PAYEE(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PAGE_PAYEE_TYPE))

typedef struct _EtatsPagePayee			EtatsPagePayee;
typedef struct _EtatsPagePayeeClass		EtatsPagePayeeClass;


struct _EtatsPagePayee
{
    GtkBox parent;
};

struct _EtatsPagePayeeClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				etats_page_payee_get_type					(void) G_GNUC_CONST;

EtatsPagePayee *	etats_page_payee_new						(GtkWidget *etats_prefs);

void				etats_page_payee_get_info					(GtkWidget *etats_prefs,
															 gint report_number);
void				etats_page_payee_initialise_onglet			(GtkWidget *etats_prefs,
																 gint report_number);
gboolean			etats_page_payee_show_first_row_selected	(GtkWidget *etats_prefs);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATS_PAGE_PAYEE_H__ */
