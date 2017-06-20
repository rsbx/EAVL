/*\
***  EAVL_Tree: Embedded AVL Tree
\*/

/*\
*#*  Copyright (c) 2017, Raymond S Brand
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


#ifndef _CTREE_TRAVERSE_H
#define _CTREE_TRAVERSE_H 1


#include "EAVL_cTree.h"

#include "naming_internal.h"


#define	ORDER_MASK_PRE		(1u<<EAVL_ORDER_PRE)
#define ORDER_MASK_IN		(1u<<EAVL_ORDER_IN)
#define ORDER_MASK_POST		(1u<<EAVL_ORDER_POST)


typedef unsigned int			order_mask_t;

typedef int (EAVLc_cbTraverse_t)(
		EAVLc_node_t*		node,
		order_mask_t		cover,
		unsigned int		safe,
		void*			cbdata
		);


int PRIVATE(traverse)(
		EAVLc_node_t*		node,
		EAVL_dir_t		dir,
		order_mask_t		interests,
		unsigned int		safe,
		EAVLc_cbTraverse_t*	callback,
		void*			cbtdata,
		EAVLc_cbPathe_t*	cbpathe,
		void*			cbdata
		);


#endif	/* _CTREE_TRAVERSE_H */
