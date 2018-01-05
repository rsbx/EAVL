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


#ifndef _CALLBACK_INTERNAL_H
#define _CALLBACK_INTERNAL_H 1

#include "naming_internal.h"


/*
** cbCompare:
*/
#if CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK


#define CB_COMPARE(REF_VAL, REF_NODE, NODE, CB, CBDATA, VAR)		\
	do								\
		{							\
		(VAR) = (*(CB))((REF_VAL), (REF_NODE), (NODE), (CBDATA));	\
		if ((PUBLIC(Checks_Enabled) & EAVL_CHECK_CALLBACK)	\
				&& ((VAR) > EAVL_CMP_SAME)		\
				)					\
			{						\
			return EAVL_ERROR_CALLBACK;			\
			}						\
		} while (0)


#else


#define CB_COMPARE(REF_VAL, REF_NODE, NODE, CB, CBDATA, VAR)		\
	do								\
		{							\
		(VAR) = (*(CB))((REF_VAL), (REF_NODE), (NODE), (CBDATA));	\
		} while (0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK */


/*
** cbDup:
*/
#if CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK


#define CB_DUP(NODE, CB, CBDATA, VAR)					\
	do								\
		{							\
		(VAR) = (*(CB))((NODE), (CBDATA));			\
		if ((PUBLIC(Checks_Enabled) & EAVL_CHECK_CALLBACK)	\
				&& !(VAR)				\
				)					\
			{						\
			return EAVL_CALLBACK;				\
			}						\
		} while (0)


#else


#define CB_DUP(NODE, CB, CBDATA, VAR)					\
	do								\
		{							\
		(VAR) = (*(CB))((NODE), (CBDATA));			\
		} while (0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK */


/*
** cbFixup:
*/
#if CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK


#define CB_FIXUP(NODE, CHILDL, CHILDR, CB, CBDATA)			\
	do								\
		{							\
		if ((CB))						\
			{						\
			int		CB_fixup;			\
									\
			CB_fixup = (*(CB))(				\
					(NODE),				\
					(CHILDL),			\
					(CHILDR),			\
					(CBDATA)			\
					);				\
			switch (CB_fixup)				\
				{					\
				case EAVL_CB_OK:			\
					break;				\
									\
				case EAVL_CB_FINISHED:			\
					(CB) = NULL;			\
					break;				\
									\
				default:				\
					if (PUBLIC(Checks_Enabled)	\
							& EAVL_CHECK_CALLBACK	\
							)		\
						{			\
						return EAVL_ERROR_CALLBACK;	\
						}			\
				}					\
			}						\
		} while (0)


#else


#define CB_FIXUP(NODE, CHILDL, CHILDR, CB, CBDATA)			\
	do								\
		{							\
		if ((CB))						\
			{						\
			int		CB_fixup;			\
									\
			CB_fixup = (*(CB))(				\
					(NODE),				\
					(CHILDL),			\
					(CHILDR),			\
					(CBDATA)			\
					);				\
			if (CB_fixup == EAVL_CB_FINISHED)		\
				{					\
				(CB) = NULL;				\
				}					\
			}						\
		} while (0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK */


/*
** cbPathe:
*/
#if CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK


#define CB_PATHE_ADDR(INDEX, CB, CBDATA, ERROR, VAR)			\
	do								\
		{							\
		if (!((VAR) = (*(CB))((INDEX), 0, (CBDATA)))		\
				&& (PUBLIC(Checks_Enabled)		\
					& EAVL_CHECK_CALLBACK		\
					)				\
				)					\
			{						\
			return (ERROR);					\
			}						\
		} while (0)


#define CB_PATHE_NULL(INDEX, PARAM, CB, CBDATA)				\
	do								\
		{							\
		if ((*(CB))((INDEX), (PARAM), (CBDATA))			\
				&& (PUBLIC(Checks_Enabled)		\
					& EAVL_CHECK_CALLBACK		\
					)				\
				)					\
			{						\
			return EAVL_ERROR_CALLBACK;			\
			}						\
		} while (0)


#else


#define CB_PATHE_ADDR(INDEX, CB, CBDATA, ERROR, VAR)			\
	do								\
		{							\
		(VAR) = (*(CB))((INDEX), 0, (CBDATA));			\
		} while (0)


#define CB_PATHE_NULL(INDEX, PARAM, CB, CBDATA)				\
	do								\
		{							\
		(*(CB))((INDEX), (PARAM), (CBDATA));			\
		} while (0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK */


/*
** cbRelease:
*/
#if CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK


#define CB_RELEASE(NODE, CB, CBDATA)					\
	do								\
		{							\
		if ((CB))						\
			{						\
			int		CB_release;			\
									\
			CB_release = (*(CB))((NODE), (CBDATA));		\
			switch (CB_release)				\
				{					\
				case EAVL_CB_OK:			\
					break;				\
									\
				case EAVL_CB_FINISHED:			\
					(CB) = NULL;			\
					break;				\
									\
				default:				\
					if (PUBLIC(Checks_Enabled)	\
							& EAVL_CHECK_CALLBACK	\
							)		\
						{			\
						return EAVL_ERROR_CALLBACK;	\
						}			\
				}					\
			}						\
		} while (0)


#else


#define CB_RELEASE(NODE, CB, CBDATA)					\
	do								\
		{							\
		if ((CB))						\
			{						\
			int		CB_release;			\
									\
			CB_release = (*(CB))((NODE), (CBDATA));		\
			if (CB_release == EAVL_CB_FINISHED)		\
				{					\
				(CB) = NULL;				\
				}					\
			}						\
		} while (0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK */


/*
** cbTraverse:
*/
#if CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK


#define CB_TRAVERSE(NODE, COVER, SAFE, TTCB, DATA, LIMITER, RES)	\
	do								\
		{							\
		if ((TTCB))						\
			{						\
			int		TTres;				\
									\
			TTres = (*(TTCB))((NODE), (COVER), (SAFE), (DATA));	\
			switch (TTres)					\
				{					\
				case EAVL_CB_OK:			\
					break;				\
									\
				case EAVL_CB_FINISHED:			\
					(TTCB) = NULL;			\
					break;				\
									\
				case EAVL_CB_LIMIT:			\
					(LIMITER) = 1;			\
					break;				\
									\
				case EAVL_CB_CALLBACK:			\
					return EAVL_CALLBACK;		\
									\
				default:				\
					if (PUBLIC(Checks_Enabled)	\
							& EAVL_CHECK_CALLBACK	\
							)		\
						{			\
						return EAVL_ERROR_CALLBACK;	\
						}			\
				}					\
			}						\
		} while (0)


#else


#define CB_TRAVERSE(NODE, COVER, SAFE, TTCB, DATA, LIMITER, RES)	\
	do								\
		{							\
		if ((TTCB))						\
			{						\
			int		TTres;				\
									\
			TTres = (*(TTCB))((NODE), (COVER), (SAFE), (DATA));	\
			switch (TTres)					\
				{					\
				case EAVL_CB_OK:			\
					break;				\
									\
				case EAVL_CB_FINISHED:			\
					(TTCB) = NULL;			\
					break;				\
									\
				case EAVL_CB_LIMIT:			\
					(LIMITER) = 1;			\
					break;				\
									\
				case EAVL_CB_CALLBACK:			\
					return EAVL_CALLBACK;		\
				}					\
			}						\
		} while (0)


#endif	/* CHECKS_AVAILABLE & EAVL_CHECK_CALLBACK */


/*
** cbVerify:
*/
#define CB_VERIFY(NODE, CHILDL, CHILDR, CB, CBDATA)			\
	do								\
		{							\
		if ((CB))						\
			{						\
			int		CB_verify;			\
									\
			CB_verify = (*(CB))(				\
					(NODE),				\
					(CHILDL),			\
					(CHILDR),			\
					(CBDATA)			\
					);				\
			switch (CB_verify)				\
				{					\
				case EAVL_CB_OK:			\
					break;				\
									\
				case EAVL_CB_FINISHED:			\
					(CB) = NULL;			\
					break;				\
									\
				default:				\
					return EAVL_ERROR_CALLBACK;	\
				}					\
			}						\
		} while (0)


#endif	/* _CALLBACK_INTERNAL_H */
