/*     Copyright (C) 2007 Dominique Parisot
 * 			zionly@free.org
 * 			http://www.grisbi.org
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of 
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 *     GNU General Public License for more details. 
 *
 *     You should have received a copy of the GNU General Public License 
 *     along with this program; if not, write to the Free Software 
 *     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef _BALANCE_ESTIMATE_TAB_H
#define _BALANCE_ESTIMATE_TAB_H

#ifdef ENABLE_BALANCE_ESTIMATE 
#include <gtk/gtk.h>

/* START_INCLUDE_H */
#include "./gsb_real.h"
/* END_INCLUDE_H */

typedef struct _bet_range           SBR;
typedef struct _historical          SH;

/* START_DECLARATION */
GtkWidget *bet_create_balance_estimate_tab(void);
void bet_update_balance_estimate_tab(void);
/* END_DECLARATION */

#endif /* ENABLE_BALANCE_ESTIMATE */

#endif /*_BALANCE_ESTIMATE_TAB_H*/

