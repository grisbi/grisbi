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
#include <gtk/gtkeditable.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GTK_TYPE_COMBOFIX             ( gtk_combofix_get_type ( ) )
#define GTK_COMBOFIX(obj) GTK_CHECK_CAST(obj, gtk_combofix_get_type(), GtkComboFix )
#define GTK_COMBOFIX_CLASS(klass) GTK_CHECK_CLASS_CAST( klass, gtk_combofix_get_type(), GtkComboFixClass )
#define GTK_IS_COMBOFIX(obj) GTK_CHECK_TYPE ( obj, gtk_combofix_get_type() )


typedef struct _GtkComboFix GtkComboFix;
typedef struct _GtkComboFixClass GtkComboFixClass;
typedef struct _GtkComboFixPrivate GtkComboFixPrivate;


/* structure of the ComboFix */
struct _GtkComboFix
{
    GtkVBox vbox;

    /* entry of the combofix */
    GtkWidget *entry;
    /* *** private entries *** */
<<<<<<< HEAD

    /* the tree_store is 5 columns :
     * COMBOFIX_COL_VISIBLE_STRING (a string) : what we see in the combofix
     * COMBOFIX_COL_REAL_STRING (a string) : what we set in the entry when selecting something
     * COMBOFIX_COL_VISIBLE (a boolean) : if that line has to be showed
     * COMBOFIX_COL_LIST_NUMBER (a int) : the number of the list 1, 2 ou 3 (CREDIT DEBIT SPECIAL
     * COMBOFIX_COL_SEPARATOR TRUE if this is a separator */
    GtkTreeStore *store;
    GtkTreeModel *model_filter;
    GtkTreeModel *model_sort;
    GtkWidget *tree_view;

    gint visible_items;
    GtkWidget *popup;
    /* automatic sorting */
    gint auto_sort;
=======
    GtkComboFixPrivate *GSEAL (priv);
>>>>>>> Change gtk_combofix and minor changes
};

struct _GtkComboFixClass
{
    GtkVBoxClass parent_class;
};


/* construction */
guint gtk_combofix_get_type ( void );
GtkWidget *gtk_combofix_new ( GSList *list );

/* text */
const gchar *gtk_combofix_get_text ( GtkComboFix *combofix );
void gtk_combofix_set_text ( GtkComboFix *combofix, const gchar *text );
void gtk_combofix_set_force_text ( GtkComboFix *combofix, gboolean value );
void gtk_combofix_set_case_sensitive ( GtkComboFix *combofix, gboolean case_sensitive );

/* popup */
gboolean gtk_combofix_show_popup ( GtkComboFix *combofix );
gboolean gtk_combofix_hide_popup ( GtkComboFix *combofix );

/* list of items */
gboolean gtk_combofix_set_list ( GtkComboFix *combofix, GSList *list );
void gtk_combofix_set_max_items ( GtkComboFix *combofix, gint max_items );
void gtk_combofix_set_mixed_sort ( GtkComboFix *combofix, gboolean mixed_sort );
void gtk_combofix_set_sort ( GtkComboFix *combofix, gboolean auto_sort );

/* set callback */
void gtk_combofix_set_selection_callback ( GtkComboFix *combofix,
						GCallback func,
					    gpointer data );

#ifdef __cplusplus
}
#endif				/* __cplusplus */

#endif				/* __GTK_COMBOFIX_H__ */
