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


#ifndef _TREELOAD_INTERNAL_H
#define _TREELOAD_INTERNAL_H 1


#include "EAVL.h"
#include "naming_internal.h"


typedef void (*FOREIGN(_, load_setchild_t))(
		void**			nodep,
		unsigned int		parentindex,
		unsigned int		childindex,
		EAVL_dir_t		dir
		);
typedef void (*FOREIGN(_, load_setbal_t))(
		void**			nodep,
		unsigned int		nodeindex,
		EAVL_dir_t		bal
		);
typedef int (*FOREIGN(_, load_cbFixup_t))(
		EAVL_node_t*		node,
		EAVL_node_t*		childL,
		EAVL_node_t*		childR,
		void*			cbdata
		);
typedef int (*FOREIGN(_, load_fixup_t))(
		void**			nodep,
		unsigned int		nodeindex,
		FOREIGN(_, load_cbFixup_t)	fixup,
		void*			cbdata
		);

typedef struct {
		FOREIGN(_, load_setchild_t)	setchild;
		FOREIGN(_, load_setbal_t)	setbal;
		FOREIGN(_, load_fixup_t)	fixup;
		} FOREIGN(_, load_cbset_t);


int FOREIGN(_, load)(
		unsigned int*		rootindex,
		unsigned int		count,
		void**			nodep,
		FOREIGN(_, load_cbset_t)*	cbset,
		FOREIGN(_, load_cbFixup_t)	fixup,
		void*			cbdata
		);


#endif	/* _TREELOAD_INTERNAL_H */
