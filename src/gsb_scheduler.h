#ifndef _GSB_SCHEDULER_H
#define _GSB_SCHEDULER_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void gsb_scheduler_check_scheduled_transactions_time_limit ( void );
gboolean gsb_scheduler_execute_children_of_scheduled_transaction ( gint scheduled_number,
								   gint transaction_number );
GDate *gsb_scheduler_get_next_date ( gint scheduled_number,
				     const GDate *date );
gboolean gsb_scheduler_increase_scheduled ( gint scheduled_number );
/* END_DECLARATION */
#endif
