#ifndef METATREE_H
#define METATREE_H

/** Interface between various tree data and metatree */
typedef struct metatree_interface {
    gint depth;
    struct struct_devise * (* tree_currency) ();
    /* Read operations */
    gchar * no_div_label;
    gchar * no_sub_div_label;
    gpointer (* get_without_div_pointer) ();
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
    GSList * (* div_list) ();
    GSList * (* div_sub_div_list) (gpointer);
    gint (* div_type) (gpointer);

    /* Transaction operations */
    gint (* transaction_div_id) (struct structure_operation *);
    gint (* transaction_sub_div_id) (struct structure_operation *);
    void (* transaction_set_div_id) (struct structure_operation *, int);
    void (* transaction_set_sub_div_id) (struct structure_operation *, int);
    gint (* scheduled_div_id) (struct operation_echeance *);
    gint (* scheduled_sub_div_id) (struct operation_echeance *);
    void (* scheduled_set_div_id) (struct operation_echeance *, int);
    void (* scheduled_set_sub_div_id) (struct operation_echeance *, int);

    /* Write operations on div */
    gint (* add_div) ();
    gint (* add_sub_div) (int);
    gboolean (* remove_div) (int);
    gboolean (* remove_sub_div) (int,int);
    gboolean (* add_transaction_to_div) (struct structure_operation *, int);
    gboolean (* add_transaction_to_sub_div) (struct structure_operation *, int, int);
    gboolean (* remove_transaction_from_div) (struct structure_operation *, int);
    gboolean (* remove_transaction_from_sub_div) (struct structure_operation *, int, int);

} MetatreeInterface;

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



/*START_DECLARATION*/
gboolean metatree_get_row_properties ( GtkTreeModel * tree_model, GtkTreePath * path, 
				       gchar ** text, gint * lvl1, gint * lvl2, 
				       gpointer * data );
enum meta_tree_row_type metatree_get_row_type ( GtkTreeModel * tree_model, 
						GtkTreePath * path );
gboolean metatree_get ( GtkTreeModel * model, GtkTreePath * path,
			gint column, gpointer * data );
gboolean supprimer_division ( GtkWidget * button, GtkTreeView * tree_view );
void supprimer_sub_division ( GtkTreeView * tree_view, GtkTreeModel * model,
			      MetatreeInterface * iface, 
			      gpointer sub_division, gint no_division );
gboolean division_column_expanded  (GtkTreeView * treeview, GtkTreeIter * iter, 
				 GtkTreePath * tree_path, gpointer user_data );
gboolean division_activated ( GtkTreeView * treeview, GtkTreePath * path,
			   GtkTreeViewColumn * col, gpointer userdata );
void fill_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
		      GtkTreeIter * iter, gpointer division );
void fill_sub_division_row ( GtkTreeModel * model, MetatreeInterface * iface, 
			  GtkTreeIter * iter, gpointer division,
			  gpointer sub_division );
gboolean division_drag_data_get ( GtkTreeDragSource * drag_source, GtkTreePath * path,
			       GtkSelectionData * selection_data );
gboolean division_row_drop_possible (GtkTreeDragDest * drag_dest, 
				  GtkTreePath * dest_path,
				  GtkSelectionData * selection_data);
gboolean division_drag_data_received ( GtkTreeDragDest   *drag_dest,
				    GtkTreePath       *dest_path,
				    GtkSelectionData  *selection_data );
void move_transaction_to_sub_division ( struct structure_operation * transaction,
					GtkTreeModel * model,
					GtkTreePath * orig_path, GtkTreePath * dest_path,
					gint no_division, gint no_sub_division );
gboolean find_destination_blob ( MetatreeInterface * iface, GtkTreeModel * model,
				 gpointer division, gpointer sub_division, 
				 gint * no_div, gint * no_sub_div );
gboolean find_associated_transactions ( MetatreeInterface * iface, 
					gint no_division, gint no_sub_division );
void expand_arbre_division ( GtkWidget *bouton, gint depth );
void appui_sur_ajout_division ( GtkWidget * button, GtkTreeModel * model );
void appui_sur_ajout_sub_division ( GtkWidget * button, GtkTreeModel * model );
GtkTreeIter * get_iter_from_div ( GtkTreeModel * model, int div, int sub_div );
/*END_DECLARATION*/

#endif
