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


#ifndef _PTREE_INTERNAL_H
#define _PTREE_INTERNAL_H 1


#include "EAVL_pTree.h"

#include "naming_internal.h"


int FOREIGN(p_, find)(
		EAVLp_node_t*		curr,
		EAVL_rel_t		rel,
		EAVLp_cbCompare_t*	compare,
		void*			cbdata,
		void*			ref_value,
		EAVLp_node_t*		ref_node,
		EAVLp_node_t**		resultp
		);

int FOREIGN(p_, insert)(
		EAVLp_node_t**		rootp,
		EAVLp_node_t*		new_node,
		EAVLp_cbCompare_t*	compare,
		EAVLp_cbFuxup_t*	fixup,
		void*			cbdata,
		EAVLp_node_t**		resultp
		);

int FOREIGN(p_, remove)(
		EAVLp_node_t**		rootp,
		EAVLp_node_t*		del_node,
		EAVLp_cbFuxup_t*	fixup,
		void*			cbdata,
		EAVLp_node_t**		nodep
		);


#endif	/* _PTREE_INTERNAL_H */
