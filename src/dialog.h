#ifndef _DIALOG_H
#define _DIALOG_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */

typedef struct _ConditionalMsg			ConditionalMsg;

struct _ConditionalMsg
{
    const gchar * name;
    const gchar * hint;
    const gchar * message;
    gboolean hidden;
    gboolean default_answer;	/** Useful for yes/no questions. */
};

/* START_DECLARATION */
void 		dialogue 									(const gchar *texte_dialogue);
void 		dialogue_conditional_hint 					(const gchar *text,
														 const gchar *hint,
														 const gchar *var);
gboolean 	dialogue_conditional_yes_no 				(const gchar *var);
gint 		dialogue_conditional_yes_no_get_no_struct 	(ConditionalMsg *msg,
														 const gchar *name);
gboolean 	dialogue_conditional_yes_no_with_items 		(const gchar *tab_name,
														 const gchar *struct_name,
														 const gchar *tmp_msg);
gboolean 	dialogue_conditional_yes_no_with_struct 	(ConditionalMsg *msg);
void 		dialogue_error 								(const gchar *text);
void 		dialogue_error_brain_damage 				(void);
void 		dialogue_error_hint 						(const gchar *text,
														 const gchar *hint);
void 		dialogue_error_memory 						(void);
GtkWidget *	dialog_get_content_area 					(GtkWidget *dialog);
gpointer	dialogue_get_tab_delete_msg					(void);
gpointer 	dialogue_get_tab_warning_msg				(void);
void 		dialogue_hint 								(const gchar *text,
														 const gchar *hint);
gchar *		dialogue_hint_with_entry 					(const gchar *text,
														 const gchar *hint,
														 const gchar *entry_description);
void 		dialogue_message 							(const gchar *label, ...);
gchar *		dialogue_make_blue 							(const gchar *text);
gchar *		dialogue_make_pango_attribut 				(const gchar *attribut,
														 const gchar *text);
gchar *		dialogue_make_red 							(const gchar *text);
GtkWidget *	dialogue_special_no_run 					(GtkMessageType param,
														 GtkButtonsType buttons,
														 const gchar *text,
														 const gchar* hint);
gboolean 	dialogue_update_struct_message 				(GtkWidget *checkbox,
														 ConditionalMsg *msg);
void 		dialogue_warning 							(const gchar *text);
void 		dialogue_warning_hint 						(const gchar *text,
														 const gchar *hint);
gboolean 	dialogue_yes_no 							(const gchar *text,
														 const gchar *hint,
														 gint default_answer);
/* END_DECLARATION */
#endif
