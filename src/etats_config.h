#ifndef _ETATS_CONFIG_H
#define _ETATS_CONFIG_H (1)

#define TEXT_NORMAL	(0<<0)
#define TEXT_BOLD	(1<<0)
#define TEXT_ITALIC	(1<<1)
#define TEXT_HUGE	(1<<2)
#define TEXT_LARGE	(1<<3)
#define TEXT_SMALL	(1<<4)

enum report_tree_columns {
    REPORT_TREE_TEXT_COLUMN,
    REPORT_TREE_PAGE_COLUMN,
    REPORT_TREE_BOLD_COLUMN,
    REPORT_TREE_ITALIC_COLUMN,
    REPORT_TREE_NUM_COLUMNS,
};

/* START_INCLUDE_H */
/* END_INCLUDE_H */

/* START_DECLARATION */
void personnalisation_etat (void);
/* END_DECLARATION */
#endif
