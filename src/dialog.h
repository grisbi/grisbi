#ifndef _DIALOG_H
#define _DIALOG_H (1)

#include <gtk/gtk.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


struct ConditionalMessage
{
    gchar * name;
    gchar * hint;
    gchar * message;
    gboolean hidden;
    gboolean default_answer;	/** Useful for yes/no questions. */
};


/* START_DECLARATION */
void 		dialog_message 								(const gchar *label, ...);
void 		dialogue 									(gchar *texte_dialogue);
void 		dialogue_conditional_hint 					(gchar *hint,
														 gchar *text,
														 gchar *var);
void 		dialogue_error 								(gchar *text);
void 		dialogue_error_brain_damage 				(void);
void 		dialogue_error_hint 						(const gchar *text,
														 gchar *hint);
void 		dialogue_error_memory 						(void);
GtkWidget *	dialog_get_content_area 					(GtkWidget *dialog);
void 		dialogue_hint 								(gchar *text,
														 gchar *hint);
gchar *		dialogue_hint_with_entry 					(gchar *text,
														 gchar *hint,
														 gchar *entry_description);
GtkWidget *	dialogue_special_no_run 					(GtkMessageType param,
														 GtkButtonsType buttons,
														 const gchar *text,
														 const gchar* hint);
gboolean 	dialogue_update_struct_message 				(GtkWidget *checkbox,
														 struct ConditionalMessage *message);
void 		dialogue_warning (gchar *text);
void 		dialogue_warning_hint 						(gchar *text,
														 gchar *hint);
gchar *		make_blue 									(const gchar *text);
gchar *		make_pango_attribut 						(gchar *attribut,
														 const gchar *text);
gchar *		make_red 									(const gchar *text);
gboolean 	question_conditional_yes_no 				(gchar *var);
gint 		question_conditional_yes_no_get_no_struct 	(struct ConditionalMessage *msg,
														 gchar *name);
gboolean 	question_conditional_yes_no_with_struct 	(struct ConditionalMessage *message);
gboolean 	question_yes_no 							(const gchar *text,
														 const gchar *hint,
														 gint default_answer);
/* END_DECLARATION */
#endif
