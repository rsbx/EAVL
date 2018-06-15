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


#ifndef _EAVL_CTREE_H
#define _EAVL_CTREE_H 1


#include "EAVL.h"


typedef struct EAVLc_tree	EAVLc_tree_t;
typedef struct EAVLc_context	EAVLc_context_t;
typedef struct
	{
	EAVL_node_t		EAVLnode;
	uintptr_t		references;
	}			EAVLc_node_t;
typedef struct EAVLc_cbset	EAVLc_cbset_t;
typedef EAVLc_node_t*		EAVLc_pathelement_t;

typedef EAVL_dir_t (*EAVLc_cbCompare_t)(
		void*			ref_value,
		EAVLc_node_t*		ref_node,
		EAVLc_node_t*		node,
		void*			cbdata
		);

typedef EAVLc_node_t* (*EAVLc_cbDup_t)(
		EAVLc_node_t*		node,
		void*			cbdata
		);

typedef int (*EAVLc_cbFixup_t)(
		EAVLc_node_t*		node,
		EAVLc_node_t*		childL,
		EAVLc_node_t*		childR,
		void*			cbdata
		);

typedef int (*EAVLc_cbVerify_t)(
		EAVLc_node_t*		node,
		EAVLc_node_t*		childL,
		EAVLc_node_t*		childR,
		void*			cbdata
		);

typedef int (*EAVLc_cbRelease_t)(
		EAVLc_node_t*		node,
		void*			cbdata
		);

typedef EAVLc_pathelement_t* (*EAVLc_cbPathe_t)(
		unsigned int		index,
		unsigned int		param,
		void*			cbdata
		);


struct EAVLc_tree
	{
	EAVLc_node_t*		root;
	EAVLc_cbset_t*		cbset;
	EAVL_tree_common_t	common;
	};

struct EAVLc_context
	{
	EAVLc_tree_t*		tree;
	EAVLc_node_t*		recent;
	EAVL_context_common_t	common;
	unsigned int		pathlen;
	EAVLc_cbPathe_t		cbpathe;
	};

struct EAVLc_cbset
	{
	EAVLc_cbCompare_t	compare;
	EAVLc_cbDup_t		dup;
	EAVLc_cbFixup_t		fixup;
	EAVLc_cbVerify_t	verify;
	};


extern unsigned int	EAVLc_Checks_Available;
extern unsigned int	EAVLc_Checks_Enabled;


int EAVLc_Tree_Init(
		EAVLc_tree_t*		tree,
		EAVLc_tree_t*		existing,
		EAVLc_cbset_t*		cbset
		);

int EAVLc_Load(
		EAVLc_context_t*	context,
		unsigned int		node_count,
		EAVLc_node_t*		nodes[]
		);

int EAVLc_Split(
		EAVLc_context_t*	context,
		EAVLc_node_t**		nodep
		);

int EAVLc_Clear(
		EAVLc_context_t*	context,
		EAVLc_cbRelease_t	noderelease
		);

int EAVLc_Release(
		EAVLc_tree_t*		tree
		);

int EAVLc_Context_Init(
		EAVLc_context_t*	context,
		EAVLc_cbPathe_t		cbpathe,
		void*			cbdata
		);

int EAVLc_Context_Associate(
		EAVLc_context_t*	context,
		EAVLc_tree_t*		tree
		);

int EAVLc_Context_Disassociate(
		EAVLc_context_t*	context
		);

int EAVLc_Insert(
		EAVLc_context_t*	context,
		EAVLc_node_t*		node,
		EAVLc_node_t**		resultp
		);

int EAVLc_Remove(
		EAVLc_context_t*	context,
		EAVLc_node_t**		nodep
		);

int EAVLc_Find(
		EAVLc_context_t*	context,
		EAVL_rel_t		rel,
		EAVLc_cbCompare_t	compare,
		void*			ref_value,
		EAVLc_node_t*		ref_node,
		EAVLc_node_t**		resultp
		);

int EAVLc_First(
		EAVLc_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLc_node_t**		resultp
		);
#define EAVLc_Last(C, D, O, R)						\
	EAVLc_First((C), EAVL_DIR_OTHER((D)), EAVL_ORDER_INVERSE((O)), (R))

int EAVLc_Next(
		EAVLc_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLc_node_t**		resultp
		);
#define EAVLc_Prev(C, D, O, R)						\
	EAVLc_Next((C), EAVL_DIR_OTHER((D)), EAVL_ORDER_INVERSE((O)), (R))

int EAVLc_Fixup(
		EAVLc_context_t*	context
		);


#define EAVLc_GET_CHILD(NODE, DIR)					\
	(EAVLc_node_t*)EAVL_GET_CHILD(&(NODE)->EAVLnode, (DIR))
#define EAVLc_GET_BAL(NODE)		EAVL_GET_BAL(&(NODE)->EAVLnode)

#define EAVLc_GET_REFS(NODE)		((NODE)->references+1)

#define EAVLc_CONTEXT_TREE(CONTEXT)	(EAVLc_tree_t*)((CONTEXT)->tree)
#define EAVLc_TREE_ROOT(TREE)		(EAVLc_node_t*)((TREE)->root)


#endif	/* _EAVL_CTREE_H */
