#ifndef METATREE_H
#define METATREE_H

/*START_DECLARATION*/
gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gpointer * data );
enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
					       GtkTreePath * path );
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

#endif
