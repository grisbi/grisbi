#ifndef _GSB_RECONCILE_H
#define _GSB_RECONCILE_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
GtkWidget *gsb_reconcile_create_box ( void );
gboolean gsb_reconcile_run_reconciliation ( GtkWidget *button,
                        gpointer null );
gboolean gsb_reconcile_set_last_scheduled_transaction ( gint scheduled_transaction );
gboolean gsb_reconcile_update_amounts ( GtkWidget *entry,
					    gpointer null );
/* END_DECLARATION */
#endif
