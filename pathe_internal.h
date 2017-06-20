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


#ifndef _PATHE_INTERNAL_H
#define _PATHE_INTERNAL_H 1


#include "callback_internal.h"
#include "naming_internal.h"


#define PATHE_GET_SAFE(INDEX, CB, CBDATA, VAR)				\
	do								\
		{							\
		PUBLIC(pathelement_t)*		TPE;			\
									\
		CB_PATHE_ADDR((INDEX), (CB), (CBDATA), EAVL_CALLBACK, TPE);	\
		(VAR) = *TPE;						\
		} while (0)


#define PATHE_SET_SAFE(INDEX, CB, CBDATA, VAL)				\
	do								\
		{							\
		PUBLIC(pathelement_t)*		TPE;			\
									\
		CB_PATHE_ADDR((INDEX), (CB), (CBDATA), EAVL_CALLBACK, TPE);    \
		*TPE = (VAL);						\
		} while (0)


#define PATHE_GET_DANGER(INDEX, CB, CBDATA, VAR)			\
	do								\
		{							\
		PUBLIC(pathelement_t)*		TPE;			\
									\
		CB_PATHE_ADDR((INDEX), (CB), (CBDATA), EAVL_ERROR_CALLBACK, TPE);	\
		(VAR) = *TPE;						\
		} while (0)


#define PATHE_SET_DANGER(INDEX, CB, CBDATA, VAL)			\
	do								\
		{							\
		PUBLIC(pathelement_t)*		TPE;			\
									\
		CB_PATHE_ADDR((INDEX), (CB), (CBDATA), EAVL_ERROR_CALLBACK, TPE);	\
		*TPE = (VAL);						\
		} while (0)


#define PATHE_GET_SELECT(INDEX, CB, CBDATA, VAR, SAFE)			\
	do								\
		{							\
		if ((SAFE))						\
			{						\
			PATHE_GET_SAFE((INDEX), (CB), (CBDATA), (VAR));	\
			}						\
		else							\
			{						\
			PATHE_GET_DANGER((INDEX), (CB), (CBDATA), (VAR));	\
			}						\
		} while (0)


#define PATHE_SET_SELECT(INDEX, CB, CBDATA, VAL, SAFE)			\
	do								\
		{							\
		if ((SAFE))						\
			{						\
			PATHE_SET_SAFE((INDEX), (CB), (CBDATA), (VAL));	\
			}						\
		else							\
			{						\
			PATHE_SET_DANGER((INDEX), (CB), (CBDATA), (VAL));	\
			}						\
		} while (0)


#define PATHE_SHIFT(INDEX, PARAM, CB, CBDATA)				\
	CB_PATHE_NULL((INDEX), (PARAM), (CB), (CBDATA))


#define PATHE_TRUNCATE(SIZE, CB, CBDATA)				\
	CB_PATHE_NULL(-1u, (SIZE), (CB), (CBDATA))


#endif	/* _PATHE_INTERNAL_H */
