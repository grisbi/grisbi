#ifndef GSB_FILE_DEBUG_H
#define GSB_FILE_DEBUG_H (1)

/* START_INCLUDE_H */
/* END_INCLUDE_H */


/** Unit test based upon the Grisbi file  */
struct gsb_debug_test {
    gchar * name;		/** Name of the test.  */
    gchar * description;	/** Short description.  */
    gchar * instructions;	/** Instructions to do if test matches. */
    gchar * (* test) ();	/** Pointer to function doing the job.  */
    gboolean (* fix) ();	/** Pointer to function fixing the problem. */
};


/* START_DECLARATION */
gboolean gsb_debug ( void );
/* END_DECLARATION */

#endif
