/*\
***  EAVL_Tree: Embedded AVL Tree
\*/

/*\
*#*  Copyright (c) 2018, Raymond S Brand
*#*  All rights reserved.
*#*
*#*  Redistribution and use in source and binary forms, with or without
*#*  modification, are permitted provided that the following conditions
*#*  are met:
*#*
*#*   * Redistributions of source code must retain the above copyright
*#*     notice, this list of conditions and the following disclaimer.
*#*
*#*   * Redistributions in binary form must reproduce the above copyright
*#*     notice, this list of conditions and the following disclaimer in
*#*     the documentation and/or other materials provided with the
*#*     distribution.
*#*
*#*   * Redistributions in source or binary form must carry prominent
*#*     notices of any modifications.
*#*
*#*   * Neither the name of the Raymond S Brand nor the names of its
*#*     contributors may be used to endorse or promote products derived
*#*     from this software without specific prior written permission.
*#*
*#*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*#*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*#*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*#*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*#*  COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*#*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*#*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*#*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*#*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*#*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*#*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*#*  POSSIBILITY OF SUCH DAMAGE.
\*/


#ifndef _EAVL_STREE_H
#define _EAVL_STREE_H 1


#include "EAVL.h"


typedef struct EAVLs_tree	EAVLs_tree_t;
typedef struct EAVLs_context	EAVLs_context_t;
typedef struct
	{
	EAVL_node_t		EAVLnode;
	}			EAVLs_node_t;
typedef struct EAVLs_cbset	EAVLs_cbset_t;
typedef EAVLs_node_t*		EAVLs_pathelement_t;

typedef EAVL_dir_t (*EAVLs_cbCompare_t)(
		void*			ref_value,
		EAVLs_node_t*		ref_node,
		EAVLs_node_t*		node,
		void*			cbdata
		);

typedef int (*EAVLs_cbFixup_t)(
		EAVLs_node_t*		node,
		EAVLs_node_t*		childL,
		EAVLs_node_t*		childR,
		void*			cbdata
		);

typedef int (*EAVLs_cbVerify_t)(
		EAVLs_node_t*		node,
		EAVLs_node_t*		childL,
		EAVLs_node_t*		childR,
		void*			cbdata
		);

typedef int (*EAVLs_cbRelease_t)(
		EAVLs_node_t*		node,
		void*			cbdata
		);

typedef EAVLs_pathelement_t* (*EAVLs_cbPathe_t)(
		unsigned int		index,
		unsigned int		param,
		void*			cbdata
		);


struct EAVLs_tree
	{
	EAVLs_node_t*		root;
	EAVLs_cbset_t*		cbset;
	EAVL_tree_common_t	common;
	};

struct EAVLs_context
	{
	EAVLs_tree_t*		tree;
	EAVLs_node_t*		recent;
	EAVL_context_common_t	common;
	unsigned int		pathlen;
	EAVLs_cbPathe_t		cbpathe;
	};

struct EAVLs_cbset
	{
	EAVLs_cbCompare_t	compare;
	EAVLs_cbFixup_t		fixup;
	EAVLs_cbVerify_t	verify;
	};


extern unsigned int	EAVLs_Checks_Available;
extern unsigned int	EAVLs_Checks_Enabled;


int EAVLs_Tree_Init(
		EAVLs_tree_t*		tree,
		EAVLs_tree_t*		existing,
		EAVLs_cbset_t*		cbset
		);

int EAVLs_Load(
		EAVLs_context_t*	context,
		unsigned int		node_count,
		EAVLs_node_t*		nodes[]
		);

int EAVLs_Clear(
		EAVLs_context_t*	context,
		EAVLs_cbRelease_t	noderelease
		);

int EAVLs_Release(
		EAVLs_tree_t*		tree
		);

int EAVLs_Context_Init(
		EAVLs_context_t*	context,
		EAVLs_cbPathe_t		cbpathe,
		void*			cbdata
		);

int EAVLs_Context_Associate(
		EAVLs_context_t*	context,
		EAVLs_tree_t*		tree
		);

int EAVLs_Context_Disassociate(
		EAVLs_context_t*	context
		);

int EAVLs_Insert(
		EAVLs_context_t*	context,
		EAVLs_node_t*		node,
		EAVLs_node_t**		resultp
		);

int EAVLs_Remove(
		EAVLs_context_t*	context,
		EAVLs_node_t**		nodep
		);

int EAVLs_Find(
		EAVLs_context_t*	context,
		EAVL_rel_t		rel,
		EAVLs_cbCompare_t	compare,
		void*			ref_value,
		EAVLs_node_t*		ref_node,
		EAVLs_node_t**		resultp
		);

int EAVLs_First(
		EAVLs_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLs_node_t**		resultp
		);
#define EAVLs_Last(C, D, O, R)						\
	EAVLs_First((C), EAVL_DIR_OTHER((D)), EAVL_ORDER_INVERSE((O)), (R))

int EAVLs_Next(
		EAVLs_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLs_node_t**		resultp
		);
#define EAVLs_Prev(C, D, O, R)						\
	EAVLs_Next((C), EAVL_DIR_OTHER((D)), EAVL_ORDER_INVERSE((O)), (R))

int EAVLs_Fixup(
		EAVLs_context_t*	context
		);


#define EAVLs_GET_CHILD(NODE, DIR)					\
	(EAVLs_node_t*)EAVL_GET_CHILD(&(NODE)->EAVLnode, (DIR))
#define EAVLs_GET_BAL(NODE)		EAVL_GET_BAL(&(NODE)->EAVLnode)

#define EAVLs_CONTEXT_TREE(CONTEXT)	(EAVLs_tree_t*)((CONTEXT)->tree)
#define EAVLs_TREE_ROOT(TREE)		(EAVLs_node_t*)((TREE)->root)


#endif	/* _EAVL_STREE_H */
