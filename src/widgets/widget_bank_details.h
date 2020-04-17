#ifndef __WIDGET_BANK_DETAILS_H__
#define __WIDGET_BANK_DETAILS_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "./prefs/prefs_page_bank.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_BANK_DETAILS_TYPE    	(widget_bank_details_get_type ())
#define WIDGET_BANK_DETAILS(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_BANK_DETAILS_TYPE, WidgetBankDetails))
#define WIDGET_IS_BANK_DETAILS(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_BANK_DETAILS_TYPE))

typedef struct _WidgetBankDetails          WidgetBankDetails;
typedef struct _WidgetBankDetailsClass     WidgetBankDetailsClass;


struct _WidgetBankDetails
{
    GtkBox parent;
};

struct _WidgetBankDetailsClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               	widget_bank_details_get_type			(void) G_GNUC_CONST;

WidgetBankDetails *		widget_bank_details_new					(PrefsPageBank *page_bank,
																 GtkWidget *combobox);
void 					widget_bank_details_update_bank_data 	(gint bank_number,
										   					 	 GtkWidget *w_bank_details);
void					widget_bank_details_update_form			(gint bank_number,
																 GtkWidget *w_bank_details);
void					widget_bank_details_select_name_entry	(gint bank_number,
																 GtkWidget *w_bank_details);
/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_BANK_DETAILS_H__ */
