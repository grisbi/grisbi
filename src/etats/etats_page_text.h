#ifndef __ETATS_PAGE_TEXT_H__
#define __ETATS_PAGE_TEXT_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATS_PAGE_TEXT_TYPE		(etats_page_text_get_type ())
#define ETATS_PAGE_TEXT(obj)		(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATS_PAGE_TEXT_TYPE, EtatsPageText))
#define ETATS_IS_ETAT_TEXT(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PAGE_TEXT_TYPE))

typedef struct _EtatsPageText			EtatsPageText;
typedef struct _EtatsPageTextClass		EtatsPageTextClass;


struct _EtatsPageText
{
	GtkBox parent;
};

struct _EtatsPageTextClass
{
	GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				etats_page_text_get_type		(void) G_GNUC_CONST;

EtatsPageText *		etats_page_text_new					(GtkWidget *etats_prefs);
void				etats_page_text_get_info			(GtkWidget *etats_prefs,
														 gint report_number);
void				etats_page_text_initialise_onglet	(GtkWidget *etats_prefs,
														 gint report_number);
gboolean			etats_page_text_line_add			(gint text_comparison_number,
														 GtkWidget *page);
gboolean			etats_page_text_line_remove			(gint text_comparison_number,
														 GtkWidget *page);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATS_PAGE_TEXT_H__ */
