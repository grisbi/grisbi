#ifndef __PREFS_PAGE_BET_GENERAL_H__
#define __PREFS_PAGE_BET_GENERAL_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "grisbi_prefs.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define PREFS_PAGE_BET_GENERAL_TYPE    	(prefs_page_bet_general_get_type ())
#define PREFS_PAGE_BET_GENERAL(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), PREFS_PAGE_BET_GENERAL_TYPE, PrefsPageBetGeneral))
#define PREFS_IS_PAGE_BET_GENERAL(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), PREFS_PAGE_BET_GENERAL_TYPE))

typedef struct _PrefsPageBetGeneral			PrefsPageBetGeneral;
typedef struct _PrefsPageBetGeneralClass		PrefsPageBetGeneralClass;


struct _PrefsPageBetGeneral
{
    GtkBox parent;
};

struct _PrefsPageBetGeneralClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType				prefs_page_bet_general_get_type				(void) G_GNUC_CONST;

PrefsPageBetGeneral * 	prefs_page_bet_general_new					(GrisbiPrefs *prefs);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __PREFS_PAGE_BET_GENERAL_H__ */
