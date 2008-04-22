#ifndef __GSB_CATEGORY_H
#define __GSB_CATEGORY_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */


/* START_DECLARATION */
gboolean gsb_category_assistant_create_categories ( GtkWidget *assistant );
GtkWidget *gsb_category_assistant_create_choice_page ( GtkWidget *assistant );
gboolean gsb_category_choose_default_category ( void );
gboolean gsb_category_update_combofix ( void );
/* END_DECLARATION */
#endif
