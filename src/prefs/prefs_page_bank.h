#ifndef __PREFS_PAGE_BANK_H__
#define __PREFS_PAGE_BANK_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_BANK_TYPE    	(prefs_page_bank_get_type ())
#define PREFS_PAGE_BANK(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_BANK_TYPE, PrefsPageBank))
#define PREFS_IS_PAGE_BANK(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_BANK_TYPE))

typedef struct _PrefsPageBank			PrefsPageBank;
typedef struct _PrefsPageBankClass		PrefsPageBankClass;


struct _PrefsPageBank
{
    GtkBox parent;
};

struct _PrefsPageBankClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_bank_get_type				(void) G_GNUC_CONST;

PrefsPageBank *		prefs_page_bank_new						(GrisbiPrefs *prefs);
GtkWidget *			prefs_page_bank_get_button_remove		(PrefsPageBank *page);
GtkWidget *			prefs_page_bank_get_treeview			(PrefsPageBank *page);
void	 			prefs_page_bank_update_selected_line	(GtkEntry *entry,
											   				 PrefsPageBank *page);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_BANK_H__ */
