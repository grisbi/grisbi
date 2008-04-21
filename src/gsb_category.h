#ifndef __GSB_CATEGORY_H
#define __GSB_CATEGORY_H (1)


/* START_INCLUDE_H */
/* END_INCLUDE_H */


#define CATEGORY_ASSISTANT_CHOICE_NONE 0
#define CATEGORY_ASSISTANT_CHOICE_GENERAL 1
#define CATEGORY_ASSISTANT_CHOICE_ASSOCIATION 2
#define CATEGORY_ASSISTANT_CHOICE_BUSINESS 3

/* START_DECLARATION */
gboolean gsb_category_assistant_create_categories ( GtkWidget *assistant );
GtkWidget *gsb_category_assistant_create_choice_page ( GtkWidget *assistant );
gboolean gsb_category_choose_default_category ( void );
gboolean gsb_category_update_combofix ( void );
/* END_DECLARATION */
#endif
