#ifndef GSB_FILE_DEBUG_H
#define GSB_FILE_DEBUG_H (1)

#include <glib.h>

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/** Unit test based upon the Grisbi file  */
struct GsbDebugTest
{
    const gchar * 	name;				/** Name of the test.  */
    const gchar * 	description;		/** Short description.  */
    const gchar * 	instructions;		/** Instructions to do if test matches. */
    gchar * 		(* test) (void);	/** Pointer to function doing the job.  */
    gboolean		(* fix) (void);		/** Pointer to function fixing the problem. */
};

/* START_DECLARATION */
gboolean	gsb_debug	(void);
/* END_DECLARATION */

#endif
