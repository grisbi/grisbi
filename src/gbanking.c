/* ce fichier de la gestion du format gbanking */

/*     Copyright (C) 2000-2003  xxx */
/* 			xxx@grisbi.org */
/* 			http://www.grisbi.org */

/*     This program is free software; you can redistribute it and/or modify */
/*     it under the terms of the GNU General Public License as published by */
/*     the Free Software Foundation; either version 2 of the License, or */
/*     (at your option) any later version. */

/*     This program is distributed in the hope that it will be useful, */
/*     but WITHOUT ANY WARRANTY; without even the implied warranty of */
/*     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/*     GNU General Public License for more details. */

/*     You should have received a copy of the GNU General Public License */
/*     along with this program; if not, write to the Free Software */
/*     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef HAVE_G2BANKING

#include "include.h"

#include <g2banking/gbanking.h>
#include <aqbanking/imexporter.h>
#include <gwenhywfar/debug.h>

/*START_INCLUDE*/
#include "gbanking.h"
#include "import.h"
#include "dialog.h"
#include "include.h"
#include "structures.h"
/*END_INCLUDE*/

/*START_STATIC*/
/*END_STATIC*/


/*START_EXTERN*/
extern AB_BANKING *gbanking;
extern GSList *liste_comptes_importes;
/*END_EXTERN*/


/* *******************************************************************************/
int GrisbiBanking_ImportContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *ctx){
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GSList *liste_comptes_importes_gbanking;
  int errors;

  errors=0;
  liste_comptes_importes_gbanking=NULL;
  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(ai) {
    struct struct_compte_importation *compte_nouveau;
    const char *s;
    AB_TRANSACTION *t;

    fprintf(stderr, "Importing account.\n");
    compte_nouveau=calloc(1, sizeof(struct struct_compte_importation));
    s=AB_ImExporterAccountInfo_GetAccountNumber(ai);
    if (s)
      compte_nouveau->id_compte=g_strdup(s);
    s=AB_ImExporterAccountInfo_GetAccountName(ai);
    if (s)
      compte_nouveau->nom_de_compte=g_strdup(s);
    compte_nouveau->origine=GBANKING_IMPORT;

    t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
    while(t) {
      struct struct_ope_importation *ope_import;
      const GWEN_TIME *ti;
      const AB_VALUE *va;
      const GWEN_STRINGLIST *sl;

      ope_import=calloc(1, sizeof(struct struct_ope_importation));

      /* date */
      ope_import->date_de_valeur=NULL;
      ti=AB_Transaction_GetValutaDate(t);
      if (ti) {
	int year, month, day;

	if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
	  GDate *date;

	  date=g_date_new_dmy(day, month+1, year);
	  ope_import->date_de_valeur=date;
	}
      }

      ope_import->date=NULL;
      ti=AB_Transaction_GetDate(t);
      if (ti) {
	int year, month, day;

	if (!GWEN_Time_GetBrokenDownDate(ti, &day, &month, &year)) {
	  GDate *date;

	  date=g_date_new_dmy(day, month+1, year);
	  ope_import->date=date;
	}
      }

      va=AB_Transaction_GetValue(t);
      if (va) {
	ope_import->montant=AB_Value_GetValue(va);
      }

      sl=AB_Transaction_GetRemoteName(t);
      if (sl) {
	GWEN_BUFFER *nbuf;
	GWEN_STRINGLISTENTRY *se;

	nbuf=GWEN_Buffer_new(0, 256, 0, 1);
	se=GWEN_StringList_FirstEntry(sl);
	while(se) {
	  const char *s;

	  s=GWEN_StringListEntry_Data(se);
	  assert(s);
	  if (GWEN_Buffer_GetUsedBytes(nbuf))
	    GWEN_Buffer_AppendByte(nbuf, ' ');
          GWEN_Buffer_AppendString(nbuf, s);
	  se=GWEN_StringListEntry_Next(se);
	} /* while */
        if (GWEN_Buffer_GetUsedBytes(nbuf))
	  ope_import->tiers=g_strdup(GWEN_Buffer_GetStart(nbuf));
        GWEN_Buffer_free(nbuf);
      } /* if remote name */

      sl=AB_Transaction_GetPurpose(t);
      if (sl) {
	GWEN_BUFFER *nbuf;
	GWEN_STRINGLISTENTRY *se;

	nbuf=GWEN_Buffer_new(0, 256, 0, 1);
	se=GWEN_StringList_FirstEntry(sl);
	while(se) {
	  const char *s;

	  s=GWEN_StringListEntry_Data(se);
	  assert(s);
	  if (GWEN_Buffer_GetUsedBytes(nbuf))
	    GWEN_Buffer_AppendByte(nbuf, ' ');
          GWEN_Buffer_AppendString(nbuf, s);
	  se=GWEN_StringListEntry_Next(se);
	} /* while */
        if (GWEN_Buffer_GetUsedBytes(nbuf))
	  ope_import->notes=g_strdup(GWEN_Buffer_GetStart(nbuf));
	GWEN_Buffer_free(nbuf);
      } /* if purpose */

      /* append transaction */
      DBG_NOTICE(0, "Adding transaction");
      compte_nouveau->operations_importees=
	g_slist_append(compte_nouveau->operations_importees,
		       ope_import);

      t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
    } /* while */

    /* append account */
    liste_comptes_importes_gbanking=
      g_slist_append(liste_comptes_importes_gbanking,
		     compte_nouveau);
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  } /* while */

  if (errors)
    errors=!question_yes_no_hint(_("Warning" ),
				 _("An error or warning has occured. "
				   "Do you still want to import the data ?"));
  if (!errors){
    GSList *liste_tmp;

    liste_tmp = liste_comptes_importes_gbanking;

    /* ajoute le ou les compte aux autres comptes importés */

    while (liste_tmp){
      liste_comptes_importes=g_slist_append(liste_comptes_importes,
					    liste_tmp->data);
      liste_tmp=liste_tmp->next;
    }
    return affichage_recapitulatif_importation();
  }

  return 0;
}



/* *******************************************************************************/



#endif /* if gbanking is available */



