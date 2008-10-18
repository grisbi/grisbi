#ifndef _CUSTOM_LIST_H
#define _CUSTOM_LIST_H (1)

#define CUSTOM_TYPE_LIST            (custom_list_get_type ())
#define CUSTOM_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), CUSTOM_TYPE_LIST, CustomList))
#define CUSTOM_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  CUSTOM_TYPE_LIST, CustomListClass))
#define CUSTOM_IS_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CUSTOM_TYPE_LIST))
#define CUSTOM_IS_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  CUSTOM_TYPE_LIST))
#define CUSTOM_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  CUSTOM_TYPE_LIST, CustomListClass))

/* The data columns that we export via the tree model interface */
enum
{
    /* for the 6 first col, this can be changed by user,
     * so juste name col_x, and comment the by default */
    CUSTOM_MODEL_COL_0 = 0,	/* by default, check */
    CUSTOM_MODEL_COL_1,		/* by default, date */
    CUSTOM_MODEL_COL_2,		/* by default, payee */
    CUSTOM_MODEL_COL_3,		/* by default, P/R */
    CUSTOM_MODEL_COL_4,		/* by default, debit */
    CUSTOM_MODEL_COL_5,		/* by default, credit */
    CUSTOM_MODEL_COL_6,		/* by default, balance */

    CUSTOM_MODEL_BACKGROUND,			/*< color of the background (a GdkColor) */
    CUSTOM_MODEL_SAVE_BACKGROUND, 		/*< when selection, save of the normal color of background (a GdkColor) */
    CUSTOM_MODEL_AMOUNT_COLOR,			/*< color of the amount (a string like "red" or NULL)*/
    CUSTOM_MODEL_TEXT_COLOR,			/*< color of the text */
    CUSTOM_MODEL_TRANSACTION_ADDRESS,		/* pointer to the transaction structure */
    CUSTOM_MODEL_WHAT_IS_LINE,			/*< on what the address point to ? IS_TRANSACTION, IS_ARCHIVE (see below) */
    CUSTOM_MODEL_FONT, 				/*< PangoFontDescription if used */
    CUSTOM_MODEL_TRANSACTION_LINE, 		/*< the line in the transaction (0, 1, 2 or 3) */
    CUSTOM_MODEL_VISIBLE, 			/*< whether that transaction is visible or not */
    CUSTOM_MODEL_CHECKBOX_VISIBLE,   		/*< whether the checkbox is visible or not */
    CUSTOM_MODEL_CHECKBOX_VISIBLE_RECONCILE,   	/*< whether the checkbox is visible or not during reconciliation */
    CUSTOM_MODEL_CHECKBOX_ACTIVE,   		/*< whether the checkbox is active or not */

    CUSTOM_MODEL_N_COLUMNS,
};

/* number of visible columns */
#define CUSTOM_MODEL_VISIBLE_COLUMNS	7

/* definition of the number of max rows for a line, for now limit to 4 */
#define TRANSACTION_LIST_ROWS_NB 4

/* possible values to CUSTOM_MODEL_WHAT_IS_LINE */
#define IS_TRANSACTION 0
#define IS_ARCHIVE 1


typedef struct _CustomRecord     CustomRecord;
typedef struct _CustomList       CustomList;
typedef struct _CustomListClass  CustomListClass;

/* number of visibles columns */
#define CUSTOM_MODEL_N_VISIBLES_COLUMN 7

/* CustomRecord: this structure represents a row */
struct _CustomRecord
{
    /* first the 7 visibles columns */
    gchar *visible_col[7];

    GdkColor *row_bg;			/* bg color */
    GdkColor *row_bg_save;		/* save bg */
    gchar *amount_color;		/* amout color */
    GdkColor *text_color;		/* color of the text (to highlight the splitted transactions not finished */

    gpointer transaction_pointer;	/* transaction struct address */
    gint what_is_line;			/* IS_TRANSACTION /IS_ARCHIVE */
    gchar *font;			/* font */
    gint line_in_transaction;		/* line in transaction (0,1, 2 or 3) */
    gboolean line_visible;		/* is line visible (TRUE/FALSE, this value shouldn't be changed by gsb_list_model_set */
    gboolean checkbox_visible;		/* checkbox visible */
    gboolean checkbox_visible_reconcile;	/* checkbox visible during reconcile */
    gboolean checkbox_active;		/* checkbox active */

    /* admin stuff used by the custom list model */

    /* this array contains the adress of the records of the transaction
     * in the order of the lines
     * so it contains itself, and the 3 (for now) others
     * this is used to increase the speed of filling the list at the opening */
    CustomRecord *transaction_records[TRANSACTION_LIST_ROWS_NB];

    /* pos in the array of the mother,
     * so pos within the 'rows' array of the model for mother,
     * or pos within the 'children_rows' array of the mother for a child */
    /* position in the list, and in the filtered list
     * for a child, pos = filtered_pos */
    guint pos;
    guint filtered_pos;		/* -1 if the record is not visible */

    /* number of children, 0 when no child,
     * increase for each child added */
    guint number_of_children;

    /* array of pointers to the structure of the chidren */
    CustomRecord	**children_rows;

    /* a transaction is 4 lines, if there are some children, the 4 lines
     * have children_rows and number_of_children filled, but has_expander
     * will be TRUE or FALSE to set what row will have the expender in the
     * tree view, depending of the visibles number of row in the account.
     * this is changed when the list is filtered
     * only 1 row of the 4 can be a TREE, mother_row is changed according
     * the last visible row */
    gboolean has_expander;

    /* pointer to the mother if anum_rows child, NULL for a mother */
    CustomRecord	*mother_row;
};



/**
 * this structure contains the model implementation
 * */
struct _CustomList
{
    GObject	parent;      		/* this MUST be the first member */

    /* we keep here ONLY the mothers row,
     * the children and number of children are saved directly into their mother
     * this increase a lot the speed of the list */
    guint		num_rows;    		/* number of mother rows that we have   */
    CustomRecord	**rows;        		/* a dynamically allocated array of pointers to
						 *   the CustomRecord structure for each mother row    */

    /* this is the same as before but we keep here only
     * the visible transactions and the are sorted
     * visibles_rows has the same size in memory as rows, so the end cannot be reached
     * num_visibles_rows is the number of visible rows */
    guint		num_visibles_rows;
    CustomRecord	**visibles_rows;

    /* to avoid to tell all the time the account to the model functions */
    gint		nb_rows_by_transaction;	/* contains 1, 2, 3 or 4 */

    /* the first row of the current selection */
    CustomRecord	*selected_row;

    /* These two fields are not absolutely necessary, but they    */
    /*   speed things up a bit in our get_value implementation    */
    gint		n_columns;
    GType		column_types[CUSTOM_MODEL_N_COLUMNS];

    /* sort variables */
    gint            	sort_col;
    GtkSortType     	sort_order;		/* GTK_SORT_ASCENDING / GTK_SORT_DESCENDING */
    gboolean		user_sort_reconcile;	/* TRUE when the sorting function is the user defined for reconciliation */

    gint		stamp;			/* Random integer to check whether an iter belongs to our model */
};



/* CustomListClass: more boilerplate GObject stuff */
struct _CustomListClass
{
  GObjectClass parent_class;
};


/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
extern GType custom_list_get_type (void);
CustomList *custom_list_new (void);
void custom_list_set_value (GtkTreeModel *tree_model,
			    GtkTreeIter  *iter,
			    gint          column,
			    GValue       *value);
/* END_DECLARATION */
#endif
