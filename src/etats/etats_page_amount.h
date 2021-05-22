#ifndef __ETATS_PAGE_AMOUNT_H__
#define __ETATS_PAGE_AMOUNT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATS_PAGE_AMOUNT_TYPE		(etats_page_amount_get_type ())
#define ETATS_PAGE_AMOUNT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATS_PAGE_AMOUNT_TYPE, EtatsPageAmount))
#define WIDGET_IS_ETAT_AMOUNT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PAGE_AMOUNT_TYPE))

typedef struct _EtatsPageAmount			EtatsPageAmount;
typedef struct _EtatsPageAmountClass		EtatsPageAmountClass;


struct _EtatsPageAmount
{
    GtkBox parent;
};

struct _EtatsPageAmountClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				etats_page_amount_get_type				(void) G_GNUC_CONST;

EtatsPageAmount *	etats_page_amount_new					(GtkWidget *etats_prefs);
void				etats_page_amount_get_data				(GtkWidget *etats_prefs,
															 gint report_number);
void				etats_page_amount_init_data				(GtkWidget *etats_prefs,
															 gint report_number);
gboolean			etats_page_amount_line_add				(gint amount_comparison_number,
															 GtkWidget *page);
gboolean			etats_page_amount_line_remove			(gint amount_comparison_number,
															 GtkWidget *page);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATS_PAGE_AMOUNT_H__ */
