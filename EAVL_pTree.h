/*\
***  EAVL_Tree: Embedded AVL Tree
\*/

/*\
*#*  Copyright (c) 2012, 2017, Raymond S Brand
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


#ifndef _EAVL_PTREE_H
#define _EAVL_PTREE_H 1


#include "EAVL.h"


typedef struct EAVLp_tree	EAVLp_tree_t;
typedef struct EAVLp_context	EAVLp_context_t;
typedef EAVL_pnode_t		EAVLp_node_t;
typedef struct EAVLp_cbset	EAVLp_cbset_t;

typedef EAVL_dir_t (*EAVLp_cbCompare_t)(
		void*		ref_value,
		EAVLp_node_t*	ref_node,
		EAVLp_node_t*	node,
		void*		cbdata
		);

typedef int (*EAVLp_cbFixup_t)(
		EAVLp_node_t*	node,
		EAVLp_node_t*	childL,
		EAVLp_node_t*	childR,
		void*		cbdata
		);

typedef int (*EAVLp_cbVerify_t)(
		EAVLp_node_t*	node,
		EAVLp_node_t*	childL,
		EAVLp_node_t*	childR,
		void*		cbdata
		);

typedef int (*EAVLp_cbRelease_t)(
		EAVLp_node_t*	node,
		void*		cbdata
		);


struct EAVLp_tree
	{
	EAVLp_node_t*		root;
	EAVLp_cbset_t*		cbset;
	EAVL_tree_common_t	common;
	};

struct EAVLp_context
	{
	EAVLp_tree_t*		tree;
	EAVLp_node_t*		recent;
	EAVL_context_common_t	common;
	};

struct EAVLp_cbset
	{
	EAVLp_cbCompare_t	compare;
	EAVLp_cbFixup_t		fixup;
	EAVLp_cbVerify_t	verify;
	};


extern unsigned int	EAVLp_Checks_Available;
extern unsigned int	EAVLp_Checks_Enabled;


int EAVLp_Tree_Init(
		EAVLp_tree_t*		tree,
		EAVLp_tree_t*		existing,
		EAVLp_cbset_t*		cbset
		);

int EAVLp_Load(
		EAVLp_context_t*	context,
		unsigned int		node_count,
		EAVLp_node_t*		nodes[]
		);

int EAVLp_Clear(
		EAVLp_context_t*	context,
		EAVLp_cbRelease_t	noderelease
		);

int EAVLp_Release(
		EAVLp_tree_t*		tree
		);

int EAVLp_Context_Init(
		EAVLp_context_t*	context,
		void*			cbdata
		);

int EAVLp_Context_Associate(
		EAVLp_context_t*	context,
		EAVLp_tree_t*		tree
		);

int EAVLp_Context_Disassociate(
		EAVLp_context_t*	context
		);

int EAVLp_Insert(
		EAVLp_context_t*	context,
		EAVLp_node_t*		node,
		EAVLp_node_t**		resultp
		);

int EAVLp_Remove(
		EAVLp_context_t*	context,
		EAVLp_node_t**		nodep
		);

int EAVLp_Find(
		EAVLp_context_t*	context,
		EAVL_rel_t		rel,
		EAVLp_cbCompare_t	compare,
		void*			ref_value,
		EAVLp_node_t*		ref_node,
		EAVLp_node_t**		resultp
		);

int EAVLp_First(
		EAVLp_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLp_node_t**		resultp
		);
#define EAVLp_Last(C, D, O, R)						\
	EAVLp_First((C), EAVL_DIR_OTHER((D)), EAVL_ORDER_INVERSE((O)), (R))

int EAVLp_Next(
		EAVLp_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order,
		EAVLp_node_t**		resultp
		);
#define EAVLp_Prev(C, D, O, R)						\
	EAVLp_Next((C), EAVL_DIR_OTHER((D)), EAVL_ORDER_INVERSE((O)), (R))

int EAVLp_Fixup(
		EAVLp_context_t*	context
		);


#define EAVLp_GET_CHILD(NODE, DIR)					\
	(EAVLp_node_t*)EAVL_GET_CHILD(&(NODE)->EAVLnode, (DIR))
#define EAVLp_GET_BAL(NODE)		EAVL_GET_BAL(&(NODE)->EAVLnode)

#define EAVLp_GET_PARENT(NODE)		(EAVLp_node_t *)((NODE)->parent)

#define EAVLp_CONTEXT_TREE(CONTEXT)	(EAVLp_tree_t*)((CONTEXT)->tree)
#define EAVLp_TREE_ROOT(TREE)		(EAVLp_node_t*)((TREE)->root)


#endif	/* _EAVL_PTREE_H */
