/* xclnm_node.h
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

#ifndef _XAS_NODE_H
#define _XAS_NODE_H

#include <stdio.h>

typedef struct node_struct {

	struct node_struct* prev;
	struct node_struct* next;
	int ntyp;

	union {

		struct {
			int flags;
			struct node_struct* rettype;
			int sym;
			struct node_struct* args;
		} _n_info_func;

		struct {
			struct node_struct* argtype;
			int sym;
		} _n_info_arg;

		struct {
			int datatype;
			int addrspace;
			int ptrc;
			int vecn;
			int arrn;
		} _n_info_type;

	} _n_info;

} node_t;

#define NTYP_EMPTY		0x0
#define NTYP_FUNC			0x3
#define NTYP_ARG			0x4
#define NTYP_TYPE			0x5

#define F_FUNC_DEC	0x1
#define F_FUNC_DEF	0x2

#define n_info	_n_info
#define n_func _n_info._n_info_func
#define n_arg _n_info._n_info_arg
#define n_type _n_info._n_info_type

node_t* node_alloc();
void node_free(node_t* nptr);
node_t* node_get_head(node_t* nptr);
node_t* node_get_tail(node_t* nptr);
int node_count(node_t* nptr);
node_t* node_insert(node_t* nptr0, node_t* nptr);
node_t* node_insert_head(node_t* nptr0, node_t* nptr);
node_t* node_insert_tail(node_t* nptr0, node_t* nptr);
node_t* node_remove(node_t* nptr);
node_t* node_extract(node_t* nptr0, node_t* nptr1);
node_t* node_delete(node_t* nptr);
void node_destroy(node_t* nptr);
node_t* node_create();

node_t* node_create_func_dec( node_t* rettype, int sym, node_t* args );
node_t* node_create_func_def( node_t* rettype, int sym, node_t* args );
node_t* node_create_arg( node_t* type, int sym );
node_t* node_create_type(
	int arrn, int vecn,int datatype,int addrspace, int ptrc);

node_t* node_set_addrspace( node_t* type, int addrspace );
node_t* node_set_ptrc( node_t* type, int ptrc );
node_t* node_set_vecn( node_t* type, int vecn );
node_t* node_set_arrn( node_t* type, int arrn );

void node_fprintf(FILE* fp, node_t* nptr, int level );

#endif

