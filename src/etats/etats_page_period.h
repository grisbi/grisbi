#ifndef __ETATSPAGE_PERIOD_H__
#define __ETATSPAGE_PERIOD_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATSPAGE_PERIOD_TYPE    	(etats_page_period_get_type ())
#define ETATSPAGE_PERIOD(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATSPAGE_PERIOD_TYPE, EtatsPagePeriod))
#define WIDGET_IS_ETAT_PERIOD(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATSPAGE_PERIOD_TYPE))

typedef struct _EtatsPagePeriod			EtatsPagePeriod;
typedef struct _EtatsPagePeriodClass		EtatsPagePeriodClass;


struct _EtatsPagePeriod
{
    GtkBox parent;
};

struct _EtatsPagePeriodClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				etats_page_period_get_type					(void) G_GNUC_CONST;

EtatsPagePeriod * 	etats_page_period_new						(GtkWidget *etats_prefs);
void				etats_page_period_get_info					(GtkWidget *etats_prefs,
																 gint report_number);

void 				etats_page_period_initialise_onglet			(GtkWidget *etats_prefs,
																 gint report_number);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATSPAGE_PERIOD_H__ */
