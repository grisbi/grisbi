/* ComboFix Widget
 * Copyright (C) 2001-2006 Cédric Auger
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#ifndef __GTK_COMBOFIX__H__
#define __GTK_COMBOFIX__H__


#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtkoldeditable.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#define GTK_COMBOFIX(obj) GTK_CHECK_CAST(obj, gtk_combofix_get_type(), GtkComboFix )
#define GTK_COMBOFIX_CLASS(klass) GTK_CHECK_CLASS_CAST( klass, gtk_combofix_get_type(), GtkComboFixClass )
#define GTK_IS_COMBOFIX(obj) GTK_CHECK_TYPE ( obj, gtk_combofix_get_type() )


typedef struct _GtkComboFix GtkComboFix;
typedef struct _GtkComboFixClass GtkComboFixClass;


/* structure of the ComboFix */
struct _GtkComboFix
{
    GtkVBox vbox;

    /* entry of the combofix */
    GtkWidget *entry;

    /* TRUE if the entry content must belong to the list  */
    gboolean force;

    /* 0 to show all the items */
    gint max_items;

    /* TRUE for case sensitive (in that case, the first entry give the case) */
    gboolean case_sensitive;

    /* TRUE keep the completion, FALSE take the selection */
    gboolean enter_function;

    /* TRUE mix the different list, FALSE separate them */
    gboolean mixed_sort;

    /* *** private entries *** */

    /* the tree_store is 3 columns :
     * COMBOFIX_COL_VISIBLE_STRING (a string) : what we see in the combofix
     * COMBOFIX_COL_REAL_STRING (a string) : what we set in the entry when selecting something
     * COMBOFIX_COL_VISIBLE (a boolean) : if that line has to be showed
     * COMBOFIX_COL_LIST_NUMBER (a int) : the number of the list for a complex combofix (0 else), -1 for separator */
    GtkTreeStore *store;
    GtkTreeModel *model_filter;
    GtkTreeModel *model_sort;
    GtkWidget *tree_view;

    gint visible_items;
    GtkWidget *popup;
    /* complex combofix */
    gboolean complex;
    /* automatic sorting */
    gint auto_sort;
};

struct _GtkComboFixClass
{
    GtkVBoxClass parent_class;
};


guint gtk_combofix_get_type ( void );


/**
 * create a normal combofix, ie just 1 list
 * by default, force is not set, auto_sort is TRUE, no max items
 * and case unsensitive
 *
 * \param list a g_slist of name (\t at the begining makes it as a child)
 * \param force TRUE and the text must be in the list
 * \param sort TRUE and the list will be sorted automatickly
 * \param min_length the minimum of characters to show the popup
 * 
 * \return the new widget
 * */
GtkWidget *gtk_combofix_new ( GSList *list );

/**
 * create a complex combofix, ie several list set one after the others
 * by default, force is not set, auto_sort is TRUE, no max items
 * and case unsensitive
 *
 * \param list a g_slist of name (\t at the begining makes it as a child)
 * \param force TRUE and the text must be in the list
 * \param sort TRUE and the list will be sorted automatickly
 * \param min_length the minimum of characters to show the popup
 * 
 * \return the new widget
 * */
GtkWidget *gtk_combofix_new_complex ( GSList *list );


/**
 * set the text in the combofix without showing the popup or
 * doing any check
 *
 * \param combofix
 * \param text
 *
 * \return
 * */
void gtk_combofix_set_text ( GtkComboFix *combofix,
			     const gchar *text );


/**
 * get the text in the combofix
 *
 * \param combofix
 *
 * \return a const gchar
 * */
const gchar *gtk_combofix_get_text ( GtkComboFix *combofix );


/**
 * set the flag to force/unforce the text in the entry
 * if force is set, the value in the entry must belong to the list
 *
 * \param combofix
 * \param value
 *
 * \return
 * */
void gtk_combofix_set_force_text ( GtkComboFix *combofix,
				   gboolean value );

/**
 * set the maximum items viewable in the popup,
 * if there is more items corresponding to the entry than that number,
 * the popup is not showed
 *
 * \param combofix
 * \param max_items
 *
 * \return
 * */
void gtk_combofix_set_max_items ( GtkComboFix *combofix,
				  gint max_items );

/**
 * set if the list has to be automatickly sorted or not
 *
 * \param combofix
 * \param auto_sort TRUE for automatic sort
 *
 * \return
 * */
void gtk_combofix_set_sort ( GtkComboFix *combofix,
			     gboolean auto_sort );

/**
 * set if the completion is case sensitive or not
 *
 * \param combofix
 * \param case_sensitive TRUE or FALSE
 *
 * \return
 * */
void gtk_combofix_set_case_sensitive ( GtkComboFix *combofix,
				       gboolean case_sensitive );

/**
 * set the function of the enter key
 * either take the current selection and set it in the entry (FALSE)
 * either keep the current completion and close the popup (TRUE)
 *
 * \param combofix
 * \param enter_function TRUE or FALSE
 *
 * \return
 * */
void gtk_combofix_set_enter_function ( GtkComboFix *combofix,
				       gboolean  enter_function );

/**
 * set for the complex combofix if the different list have to
 * be mixed or separate
 *
 * \param combofix
 * \param mixed_sort TRUE or FALSE
 *
 * \return
 * */
void gtk_combofix_set_mixed_sort ( GtkComboFix *combofix,
				   gboolean mixed_sort );

/**
 * show or hide the popup
 *
 * \param combofix
 * \param show TRUE to show the popup
 *
 * \return
 * */
void gtk_combofix_view_list ( GtkComboFix *combofix,
			      gboolean show );


/**
 * change the list of an existing combofix
 *
 * \param combofix
 * \param list the new list
 *
 * \return TRUE if ok, FALSE if problem
 * */
gboolean gtk_combofix_set_list ( GtkComboFix *combofix,
				 GSList *list );


#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* __GTK_COMBOFIX_H__ */
