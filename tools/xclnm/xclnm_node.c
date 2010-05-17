/* xclnm_node.c
 *
 * Copyright (c) 2008-2010 Brown Deer Technology, LLC.  All Rights Reserved.
 *
 * This software was developed by Brown Deer Technology, LLC.
 * For more information contact info@browndeertechnology.com
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 (GPLv3)
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* DAR */


/*
 * Defines the nodes used to for a tree representation of the assembly
 * code.  The style/design follows that used in PCC (Portable C Compiler),
 * but is much simpler - there is much less to represent in assembly.
 */

/* DAR */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "xclnm_node.h"
#include "xclnm_gram.h"

extern char* symbuf;

node_t* 
node_alloc()
{ return((node_t*)malloc(sizeof(node_t))); }


void
node_free(node_t* nptr)
{ if (nptr) free(nptr); }


node_t* 
node_get_head(node_t* nptr)
{
	if (!nptr) return(0);
	node_t* tmp = nptr;
	while (tmp->prev) tmp = tmp->prev;
	return(tmp);
}


node_t* 
node_get_tail(node_t* nptr)
{
	if (!nptr) return(0);
	node_t* tmp = nptr;
	while (tmp->next) tmp = tmp->next;
	return(tmp);
}


int
node_count(node_t* nptr)
{
	if (!nptr) return(0);
	int count = 1;
	node_t* tmp = nptr;
	while (tmp->next) {tmp = tmp->next; ++count; }
	return(count);
}


node_t*
node_insert(node_t* nptr0, node_t* nptr)
{
	if (!nptr0 || !nptr) return(0);

	if (nptr0->next) nptr0->next->prev = nptr;
	nptr->next = nptr0->next;
	nptr0->next = nptr;
	nptr->prev = nptr0;

	return(nptr);
}


node_t*
node_insert_head(node_t* nptr0, node_t* nptr)
{
	if (!nptr0 || !nptr) return(0);

	node_t* tmp = nptr0;
	while (tmp->prev) tmp = tmp->prev;

	nptr->next = tmp;
	tmp->prev = nptr;
	nptr->prev = 0;

	return(nptr);
}


node_t*
node_insert_tail(node_t* nptr0, node_t* nptr)
{
	if (!nptr0 || !nptr) return(0);

	node_t* tmp = nptr0;
	while (tmp->next) tmp = tmp->next;

	nptr->next = 0;
	tmp->next = nptr;
	nptr->prev = tmp;

	return(nptr0);
}


node_t*
node_remove(node_t* nptr)
{
   if (!nptr) return(0);
	
	node_t* prev = nptr->prev;
	node_t* next = nptr->next;

	if (prev) prev->next = next;
	if (next) next->prev = prev;

	nptr->prev = 0;
	nptr->next = 0;

	return(next);
}


node_t*
node_extract(node_t* nptr0, node_t* nptr1)
{
   if (!nptr0 || ! nptr1) return(0);
	
	node_t* prev = nptr0->prev;
	node_t* next = nptr1->next;

	if (prev) prev->next = next;
	if (next) next->prev = prev;

	nptr0->prev = 0;
	nptr1->next = 0;

	return(nptr0);
}

node_t*
node_delete(node_t* nptr)
{
	node_t* next = node_remove(nptr);
	node_destroy(nptr);
	return(next);
}


void
node_destroy(node_t* nptr)
{
	if (!nptr) return;

	node_t* tmp;

#if(0)
	switch(nptr->ntyp) {
		case NTYP_EMPTY:
		case NTYP_FUNC_DEC:
		case NTYP_FUNC_DEC:
			while (tmp) tmp = node_delete(tmp);
			break;
		case NTYP_ARG:
			/* for each instr, call destroy */
			while (tmp) tmp = node_delete(tmp);
			break;
			break;
		case NTYP_BLOCK:
			/* for each instr, call destroy */
			tmp = nptr->n_block.ins;
			while (tmp) tmp = node_delete(tmp);
			break;
		default:
			/* XXX there could be some nasty error message here */
			break;
	}
#endif

	node_free(nptr);
}


node_t*
node_create()
{
	node_t* tmp = node_alloc();
	tmp->prev = tmp->next = 0;
	tmp->ntyp = NTYP_EMPTY;
	return(tmp);
}


node_t*
node_create_func_dec( node_t* rettype, int sym, node_t* args )
{
	node_t* tmp = node_create();
	tmp->ntyp = NTYP_FUNC;
	tmp->n_func.flags = F_FUNC_DEC;
	tmp->n_func.rettype = rettype;
	tmp->n_func.sym = sym;
	tmp->n_func.args = args;
	return(tmp);
}


node_t*
node_create_func_def( node_t* rettype, int sym, node_t* args )
{
	node_t* tmp = node_create();
	tmp->ntyp = NTYP_FUNC;
	tmp->n_func.flags = F_FUNC_DEF;
	tmp->n_func.rettype = rettype;
	tmp->n_func.sym = sym;
	tmp->n_func.args = args;
	return(tmp);
}


node_t*
node_create_arg( node_t* type, int sym)
{
	node_t* tmp = node_create();
	tmp->ntyp = NTYP_ARG;
	tmp->n_arg.argtype = type;
	tmp->n_arg.sym = sym;
	return(tmp);
}


node_t*
node_create_type( int arrn, int vecn, int datatype, int addrspace, int ptrc)
{
	node_t* tmp = node_create();
	tmp->ntyp = NTYP_TYPE;
	tmp->n_type.datatype = datatype;
	tmp->n_type.addrspace = addrspace;
	tmp->n_type.ptrc = ptrc;
	tmp->n_type.vecn = vecn;
	tmp->n_type.arrn = arrn;
	return(tmp);
}


node_t*
node_set_addrspace( node_t* type, int addrspace )
{
	type->n_type.addrspace = addrspace;
	return(type);
}


node_t*
node_set_ptrc( node_t* type, int ptrc )
{
	type->n_type.ptrc = ptrc;
	return(type);
}


node_t*
node_set_vecn( node_t* type, int vecn )
{
	type->n_type.vecn = vecn;
	return(type);
}


node_t*
node_set_arrn( node_t* type, int arrn )
{
	type->n_type.arrn = arrn;
	return(type);
}


void
node_fprintf(FILE* fp, node_t* nptr, int level)
{
	int i;
	node_t* tmp = nptr;

	while (tmp) {

		switch(tmp->ntyp) {

			case NTYP_EMPTY:
				for(i=0;i<level*2;i++) fprintf(fp," ");
				fprintf(fp,"[%p] EMPTY\n",tmp);
				break;

			case NTYP_FUNC:
				for(i=0;i<level*2;i++) fprintf(fp," ");
				fprintf(fp,"[%p] FUNC %d",tmp,tmp->n_func.rettype);
//				fprintf(fp," %d",tmp->n_func.sym);
				fprintf(fp," %s",symbuf+tmp->n_func.sym);
				fprintf(fp,"\n");
				node_fprintf(fp,tmp->n_func.rettype,level+1);
				node_fprintf(fp,tmp->n_func.args,level+1);
				break;

			case NTYP_ARG:
				for(i=0;i<level*2;i++) fprintf(fp," ");
				fprintf(fp,"[%p] ARG",tmp);
				fprintf(fp," %d",tmp->n_arg.sym);
				node_fprintf(fp,tmp->n_arg.argtype,level+1);
				break;

			case NTYP_TYPE:
				for(i=0;i<level*2;i++) fprintf(fp," ");
				fprintf(fp,"[%p] TYPE %d",tmp,tmp->n_type.datatype);
				if (tmp->n_type.vecn > 1) fprintf(fp," <%dx>",tmp->n_type.vecn);
				if (tmp->n_type.arrn > 1) fprintf(fp," [%dx]",tmp->n_type.arrn);
				if (tmp->n_type.ptrc) fprintf(fp," (%d)",tmp->n_type.addrspace);
				for(i=0;i<tmp->n_type.ptrc;i++) fprintf(fp,"*");
				fprintf(fp,"\n");
				break;

			default:
				for(i=0;i<level*2;i++) fprintf(fp," ");
				fprintf(fp,"[%p] ERROR: UNDEFINED NODE TYPE\n",tmp);
				break;

		}

		tmp = tmp->next;
	}

}
