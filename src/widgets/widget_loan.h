#ifndef __WIDGET_LOAN_H__
#define __WIDGET_LOAN_H__

#include <gtk/gtk.h>
#include <glib.h>

/*START_INCLUDE*/
#include "bet_data_finance.h"
/*END_INCLUDE*/

G_BEGIN_DECLS

#define WIDGET_LOAN_TYPE    	(widget_loan_get_type ())
#define WIDGET_LOAN(obj)    	(G_TYPE_CHECK_INSTANCE_CAST ((obj), WIDGET_LOAN_TYPE, WidgetLoan))
#define WIDGET_IS_LOAN(obj) 	(G_TYPE_CHECK_INSTANCE_TYPE((obj), WIDGET_LOAN_TYPE))

typedef struct _WidgetLoan          WidgetLoan;
typedef struct _WidgetLoanClass     WidgetLoanClass;


struct _WidgetLoan
{
    GtkBox parent;
};

struct _WidgetLoanClass
{
    GtkBoxClass parent_class;
};

/* START_DECLARATION */
GType               widget_loan_get_type			(void) G_GNUC_CONST;

void 				widget_loan_initialise_widget	(WidgetLoan *w_loan,
															 LoanStruct *s_loan);
WidgetLoan * 		widget_loan_new					(LoanStruct *s_loan);

/* END_DECLARATION */

G_END_DECLS

#endif  /* __WIDGET_LOAN_H__ */
