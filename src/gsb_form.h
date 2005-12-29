#ifndef _GSB_FORM_H
#define _GSB_FORM_H (1)

/* START_INCLUDE_H */
#include "gsb_form.h"
/* END_INCLUDE_H */

/* the  the size of the scheduled part of the form */
#define SCHEDULED_WIDTH 3
#define SCHEDULED_HEIGHT 2

enum scheduled_form_widget {
    SCHEDULED_FORM_ACCOUNT = 1,
    SCHEDULED_FORM_AUTO,
    SCHEDULED_FORM_FREQUENCY_BUTTON,
    SCHEDULED_FORM_LIMIT_DATE,
    SCHEDULED_FORM_FREQUENCY_USER_ENTRY,
    SCHEDULED_FORM_FREQUENCY_USER_BUTTON,
    SCHEDULED_FORM_MAX_WIDGETS,
};

/* START_DECLARATION */
gboolean clique_champ_formulaire ( GtkWidget *entree,
				   GdkEventButton *ev,
				   gint *ptr_origin );
gboolean entree_perd_focus ( GtkWidget *entree,
			     GdkEventFocus *ev,
			     gint *ptr_origin );
gboolean entree_prend_focus ( GtkWidget *entree,
			     GdkEventFocus *ev,
			     gint *ptr_origin );
gchar *gsb_form_get_element_name ( gint element_number );
GtkWidget *gsb_form_get_element_widget ( gint element_number );
gboolean gsb_form_hide ( void );
GtkWidget *gsb_form_new ( void );
gboolean gsb_form_set_expander_visible ( gboolean visible,
					 gboolean transactions_list );
gboolean gsb_form_show ( void );
/* END_DECLARATION */
#endif
