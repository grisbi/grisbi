#ifndef _GSB_ASSISTANT_RECONCILE_CONFIG_H
#define _GSB_ASSISTANT_RECONCILE_CONFIG_H (1)


/* START_INCLUDE_H */
#include "gsb_assistant_reconcile_config.h"
/* END_INCLUDE_H */

enum reconcile_assistant_page
{
    RECONCILE_ASSISTANT_INTRO= 0,
    RECONCILE_ASSISTANT_MENU,
    RECONCILE_ASSISTANT_NEW_RECONCILE,
    RECONCILE_ASSISTANT_AUTOMATICALY_ASSOCIATE,
    RECONCILE_ASSISTANT_MANUALLY_ASSOCIATE,
    RECONCILE_ASSISTANT_SUCCESS
};


/* START_DECLARATION */
GtkWidget *gsb_assistant_reconcile_config_page_automaticaly_associate ( GtkWidget *assistant );
GtkWidget *gsb_assistant_reconcile_config_page_manually_associate ( GtkWidget *assistant );
GtkWidget *gsb_assistant_reconcile_config_page_menu ( GtkWidget *assistant );
GtkWidget *gsb_assistant_reconcile_config_page_new_reconcile ( void );
GtkWidget *gsb_assistant_reconcile_config_page_success ( void );
gboolean gsb_assistant_reconcile_config_update_auto_asso ( GtkWidget *assistant );
gboolean gsb_assistant_reconcile_config_update_manu_asso ( GtkWidget *assistant );
/* END_DECLARATION */
#endif
