#ifndef METATREE_H
#define METATREE_H

/*START_DECLARATION*/
gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gpointer * data );
enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
						GtkTreePath * path );
gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
			gint column, gpointer * data );
/*END_DECLARATION*/


enum meta_tree_columns {
    META_TREE_TEXT_COLUMN,
    META_TREE_ACCOUNT_COLUMN,
    META_TREE_BALANCE_COLUMN,
    META_TREE_POINTER_COLUMN,
    META_TREE_NO_DIV_COLUMN,
    META_TREE_NO_SUB_DIV_COLUMN,
    META_TREE_FONT_COLUMN,
    META_TREE_XALIGN_COLUMN,
    META_TREE_NUM_COLUMNS,
}; 

enum meta_tree_row_type {
    META_TREE_DIV,
    META_TREE_SUB_DIV,
    META_TREE_TRANSACTION,
    META_TREE_INVALID,
};


/** Interface between various tree data and metatree */
typedef struct metatree_interface {
  gchar * no_div_label;
  gchar * no_sub_div_label;
  gpointer (* get_div_pointer) (int);
  gpointer (* get_sub_div_pointer) (int,int);
  gpointer (* get_div_pointer_from_name) (gchar *,gboolean);
  gpointer (* get_sub_div_pointer_from_name) (int,gchar *,gboolean);
  gint (* div_nb_transactions) (gpointer);
  gint (* sub_div_nb_transactions) (gpointer,gpointer);
  gchar * (* div_name) (gpointer);
  gchar * (* sub_div_name) (gpointer);
  gdouble (* div_balance) (gpointer);
  gdouble (* sub_div_balance) (gpointer,gpointer);
  gint (* div_id) (gpointer);
  gint (* sub_div_id) (gpointer);
  gint (* transaction_div_id) (struct structure_operation *);
  gint (* transaction_sub_div_id) (struct structure_operation *);
  void (* transaction_set_div_id) (struct structure_operation *, int);
  void (* transaction_set_sub_div_id) (struct structure_operation *, int);
  gboolean (* remove_div) (int);
  gboolean (* remove_sub_div) (int, int);
  gboolean (* add_transaction_to_div) (struct structure_operation *, int);
  gboolean (* add_transaction_to_sub_div) (struct structure_operation *, int, int);
  gboolean (* remove_transaction_from_div) (struct structure_operation *, int);
  gboolean (* remove_transaction_from_sub_div) (struct structure_operation *, int, int);
} MetatreeInterface;

#endif
