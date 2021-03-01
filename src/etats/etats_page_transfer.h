#ifndef __ETATS_PAGE_TRANSFER_H__
#define __ETATS_PAGE_TRANSFER_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define ETATS_PAGE_TRANSFER_TYPE    	(etats_page_transfer_get_type ())
#define ETATS_PAGE_TRANSFER(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), ETATS_PAGE_TRANSFER_TYPE, EtatsPageTransfer))
#define WIDGET_IS_ETAT_TRANSFER(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), ETATS_PAGE_TRANSFER_TYPE))

typedef struct _EtatsPageTransfer			EtatsPageTransfer;
typedef struct _EtatsPageTransferClass		EtatsPageTransferClass;


struct _EtatsPageTransfer
{
    GtkBox parent;
};

struct _EtatsPageTransferClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType					etats_page_transfer_get_type				(void) G_GNUC_CONST;

EtatsPageTransfer *	etats_page_transfer_new 					(GtkWidget *etats_prefs);
void					etats_page_transfer_initialise_onglet		(GtkWidget *etats_prefs,
																	 gint report_number);
void					etats_page_transfer_get_info				(GtkWidget *etats_prefs,
																	 gint report_number);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __ETATS_PAGE_TRANSFER_H__ */
