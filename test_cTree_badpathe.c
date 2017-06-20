/*
**
*/


#define _XOPEN_SOURCE 1000


#include "EAVL_cTree.h"
#include "container_of.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifndef __GNUC__
#define INFO(STR, TYPE) \
	printf("%-40s%6ld\n", (STR), sizeof(TYPE))
#else
#define INFO(STR, TYPE) \
	printf("%-40s%6ld / %6ld\n", (STR), sizeof(TYPE), __alignof__(TYPE))
#endif	/* __GNUC__ */

#ifndef UNUSED
#define UNUSED(var)	var = var
#endif	/* UNUSED */

#ifndef MAX
#define MAX(A, B)	(((A) < (B)) ? (B) : (A))
#endif



#define NODES		65537



char buffer[1024];
EAVLc_node_t		*nodep[NODES];
struct node
	{
	unsigned int	val;
	unsigned int	height;
	unsigned int	weight;
	unsigned int	count;
	unsigned int	sum;
	EAVLc_node_t	node;
	};
struct node			nodes[NODES];


unsigned char	member[NODES];
unsigned int	opcount;
unsigned int	checksize;


#define PATHE_STORE_INITIAL	(4)

typedef struct
	{
	EAVLc_pathelement_t*	elements;
	unsigned int		count;
	} pathestore_t;

typedef struct
	{
	pathestore_t*		pathestore;
	int			bad_pathe;
	} cbData_t;


#define RIGHT			EAVL_DIR_RIGHT
#define LEFT			EAVL_DIR_LEFT

#define SAME			"     "
#define DIFF			"|    "
//char bend[] = "`, ";
//char bend[] = "\\/ ";
char bend[] = "++ ";


void EAVLc_Node_print(EAVLc_node_t *eavl_node);
void EAVLc_Tree_print(char *prefix, char *end, char *prefix2, int dir, EAVLc_node_t *root);
EAVL_dir_t Node_CMP(void *ref_value, EAVLc_node_t *ref_node, EAVLc_node_t *node, void *data);
int Node_verify(
		EAVLc_node_t* eavl_node,
		EAVLc_node_t* childL, EAVLc_node_t* childR,
		void* data
		);
int Node_fixup(EAVLc_node_t* eavl_node, EAVLc_node_t* childL, EAVLc_node_t* childR, void* data);
void Init_nodes(struct node fnodes[], EAVLc_node_t *fnodep[], unsigned int count);
void check_reset(unsigned int count, unsigned int state);
int check_tree(EAVLc_context_t *context);
void check_op(
		EAVLc_context_t *context,
		unsigned int index,
		int (*op)(EAVLc_context_t *context, unsigned int k),
		int expect,
		char *operation,
		unsigned int state
		);
void init_tree_context(EAVLc_tree_t *tree, EAVLc_context_t *context);
void build_tree(EAVLc_tree_t *tree, EAVLc_context_t *context, unsigned int count);
int insert(EAVLc_context_t *context, unsigned int k);
int Eremove(EAVLc_context_t *context, unsigned int k);
int Nfixup(EAVLc_context_t *context, unsigned int k);
int traverse(EAVLc_context_t* context, EAVL_dir_t dir, EAVL_order_t order);


static pathestore_t* create_pathestore(void)
	{
	pathestore_t*		store;

	if (!(store = (pathestore_t*)malloc(sizeof(pathestore_t))))
		{
		return NULL;
		}

	store->elements = NULL;
	store->count = 0;

	return store;
	}


static EAVLc_pathelement_t* cb_pathe(
		unsigned int		index,
		unsigned int		param,
		void*			data
		)
	{
	pathestore_t*		pathestore;
	int			bad;

	if (!data || !(pathestore = ((cbData_t*)(data))->pathestore))
		{
		// WARNING: This indicates corruption of some sort.
printf("!!\n");
		exit(1);
		return NULL;
		}

	bad = ((cbData_t*)(data))->bad_pathe;

//printf("Pathe:: %4u  %4u  (%u)\n", index, param, pathestore->count);
	if (index == -1u)	// set size
		{
		unsigned int		count = param;
		void*			temp = NULL;

//printf("Pathe-set_size:: %4d  %4u  (%u)\n", index, param, pathestore->count);fflush(NULL);
		if (pathestore->count < param)
			{
			printf("Pathe-set_size:: %4d  %4u  (%u)\n",
					index, param, pathestore->count);
			fflush(NULL);
			exit(1);
			}

		if (!count && pathestore->elements)
			{
			free((void*)pathestore->elements);
			temp = NULL;}
		if (count)
			{
			if (pathestore->elements)
				{
				temp = realloc(pathestore->elements, sizeof(EAVLc_pathelement_t)*count);
				}
			else
				{
				temp = malloc(sizeof(EAVLc_pathelement_t)*count);
				}
			}
		if (!temp)
			{
			count = 0;
			}

		pathestore->elements = (EAVLc_pathelement_t*)temp;
		pathestore->count = count;

		return NULL;
		}

	if (param != 0)	// shift down 1
		{
//printf("Pathe-shift::    %4u  %4u  (%u)\n", index, param, pathestore->count);fflush(NULL);

		if (index == 0 || pathestore->count <= param || param < index)
			{
			// WARNING: This indicates corruption of some sort.
printf("!!!\n");
			exit(1);
			return NULL;
			}

		memmove(
				&pathestore->elements[index-1],
				&pathestore->elements[index],
				sizeof(EAVLc_pathelement_t)*(param-index+1)
				);

		return NULL;
		}

	if (index >= pathestore->count)
		{
		unsigned int		count = index+1;
		void*			temp;

//printf("Pathe-expand::   %4u  %4u  (%u)\n", index, param, pathestore->count);fflush(NULL);

		temp = realloc(pathestore->elements, sizeof(EAVLc_pathelement_t)*count);

		if (!temp)
			{
			// WARNING: This MAY cause a SEGFAULT!
printf("!!!!\n");
			exit(1);
			return NULL;
			}

		pathestore->elements = (EAVLc_pathelement_t*)temp;
		pathestore->count = count;

		if (bad)
			{
			return NULL;
			}
		}

	return &pathestore->elements[index];
	}


static void destroy_pathestore(
		pathestore_t*		pathestore
		)
	{
	free(pathestore->elements);
	free(pathestore);
	}


static void print_pathestore(
		pathestore_t*		pathestore
		)
	{
	unsigned int		i;

	for (i=0; i<pathestore->count; i++)
		{
		printf("PatheStore::  %3u  %p\n", i, (void*)pathestore->elements[i]);
		}
	}


static cbData_t* create_cbData(
		int			bad_pathe
		)
	{
	cbData_t*		cbdata;

	if (!(cbdata = (cbData_t*)malloc(sizeof(cbData_t))))
		{
		return NULL;
		}

	if (!(cbdata->pathestore = create_pathestore()))
		{
		free(cbdata);
		return NULL;
		}

	cbdata->bad_pathe = !!bad_pathe;

	return cbdata;
	}


static void destroy_cbData(
		cbData_t*		cbdata
		)
	{
	destroy_pathestore(cbdata->pathestore);
	free(cbdata);
	}


void check_reset(unsigned int count, unsigned int state)
	{
	unsigned int	i;

	opcount = 0;
	checksize = count;
	for (i=0; i<count; i++)
		{
		member[i] = (unsigned char)state;
		}
	}


int check_tree(
		EAVLc_context_t*	context
		)
	{
	EAVLc_context_t		checkcontext;
	unsigned int		i;
	EAVLc_node_t		*dummy;
	int			error;
	int			result = 0;

	if ((error = EAVLc_Context_Init(&checkcontext, &cb_pathe, create_cbData(0))))
		{
		printf("ERROR: Context_Init: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}
	if ((error = EAVLc_Context_Associate(&checkcontext, context->tree)))
		{
		printf("ERROR: Context_Associate: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}

	for (i=0; i<checksize; i++)
		{
		do
			{
			error = EAVLc_Find(
					&checkcontext,
					EAVL_FIND_EQ,
					NULL,
					NULL,
					nodep[i],
					&dummy
					);
			} while (error == EAVL_CALLBACK);
//printf("\t%d\n", error);

		if (member[i] && error != EAVL_OK)
			{
//printf("\t-1-\n");
			result = 1;
			goto out;
			}
		else if (!member[i] && error != EAVL_NOTFOUND)
			{
//printf("\t-2-\n");
			result = 1;
			goto out;
			}
		}

out:
	if ((error = EAVLc_Context_Disassociate(&checkcontext)))
		{
		printf("ERROR: Context_Disassociate: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}

	destroy_cbData(checkcontext.common.cbdata);

	return result;
	}


void check_op(
		EAVLc_context_t *context,
		unsigned int index,
		int (*op)(EAVLc_context_t *context, unsigned int k),
		int expect,
		char *operation,
		unsigned int state
		)
	{
	int error;

	if (check_tree(context))
		{
		printf("Operation PRE check FAILED\n");
		printf("\tindex: %d  OpCount: %d\n", index, opcount);
		exit(1);
		}

	error = (*op)(context, index);
	if (error != expect)
		{
		printf("Operation \"%s\" FAILED\n", operation);
		printf("\tindex: %d  error: %d  expect: %d\n", index, error, expect);
		exit(1);
		}

	member[index] = (unsigned char)state;
	opcount++;

	if (check_tree(context))
		{
		printf("Operation POST check FAILED\n");
		printf("\tindex: %d  OpCount: %d\n", index, opcount);
		exit(1);
		}

printf("%10d\r", opcount); fflush(NULL);
	}


void EAVLc_Node_print(
		EAVLc_node_t *eavl_node
		)
	{
	struct node *node;

	node = container_of(eavl_node, struct node, node);
	printf("%d", node->val);
	}


void EAVLc_Tree_print(char *prefix, char *end, char *prefix2, int dir, EAVLc_node_t *root)
	{
	char *last;
	char *nend;

	if (!root)
		{
		printf("%s%c---(nil)\n", prefix, bend[dir]);

		return;
		}

	nend = stpcpy(end, prefix2);

	if (root && EAVLc_GET_CHILD(root, RIGHT))
		{
		if (dir != LEFT)
			{
			last = SAME;
			}
		else
			{
			last = DIFF;
			}
		EAVLc_Tree_print(prefix, nend, last, RIGHT, EAVLc_GET_CHILD(root, RIGHT));
		printf("%s%s%s\n", prefix, last, DIFF);
		}

	printf("%s%c---(", prefix, bend[dir]);
	EAVLc_Node_print(root);
	printf(")[a:%p  L:%p  R:%p  r:%lu  b:%u]\n",
			(void *)root,
			(void *)EAVLc_GET_CHILD(root, 0),
			(void *)EAVLc_GET_CHILD(root, 1),
			EAVLc_GET_REFS(root),
			EAVLc_GET_BAL(root)
			);

	if (root && EAVLc_GET_CHILD(root, LEFT))
		{
		if (dir != RIGHT)
			{
			last = SAME;
			}
		else
			{
			last = DIFF;
			}
		printf("%s%s%s\n", prefix, last, DIFF);
		EAVLc_Tree_print(prefix, nend, last, LEFT, EAVLc_GET_CHILD(root, LEFT));
		}

	*end = '\0';
	}


EAVL_dir_t Node_CMP(
		void *ref_value,
		EAVLc_node_t *ref_node,
		EAVLc_node_t *node,
		void *data
		)
	{
	UNUSED(data);

	unsigned int *valp = &container_of(node, struct node, node)->val;
	unsigned int *refp = (unsigned int *)ref_value;

	if (ref_node)
		{
		refp = &container_of(ref_node, struct node, node)->val;
		}

	return (*valp == *refp) ? EAVL_CMP_SAME : (*valp < *refp) ? EAVL_CMP_LEFT : EAVL_CMP_RIGHT;
	}


#define CBNODEPRINT(NODE, PREFIX)					\
	do								\
		{							\
		if ((NODE))						\
			{						\
			printf("%s%5d", (PREFIX), container_of((NODE), struct node, node)->val);	\
			}						\
		else							\
			{						\
			printf("%s     ", (PREFIX));			\
			}						\
		} while (0)


#define CBNODEPRINTHW(NODE)						\
	do								\
		{							\
		printf("(%d:%d:%d:%d)",					\
			((NODE) ? container_of((NODE), struct node, node)->height : 0),	\
			((NODE) ? container_of((NODE), struct node, node)->weight : 0),	\
			((NODE) ? container_of((NODE), struct node, node)->sum : 0),	\
			((NODE) ? container_of((NODE), struct node, node)->count : 0)	\
			);						\
		} while (0)


int Node_verify(
		EAVLc_node_t*		eavl_node,
		EAVLc_node_t*		childL,
		EAVLc_node_t*		childR,
		void*			data
		)
	{
	UNUSED(data);

//	printf("Verify::  ");
//	CBNODEPRINT(eavl_node, "");
//	CBNODEPRINTHW(eavl_node);
//	CBNODEPRINT(childL, "  L:");
//	CBNODEPRINTHW(childL);
//	CBNODEPRINT(childR, "  R:");
//	CBNODEPRINTHW(childR);
//	printf("\n");

	if (container_of(eavl_node, struct node, node)->height !=
			1 + MAX(
				((childL) ? container_of(childL, struct node, node)->height : 0),
				((childR) ? container_of(childR, struct node, node)->height : 0)
				)
			)
		{
		return EAVL_ERROR;
		}
	if (container_of(eavl_node, struct node, node)->weight !=
			1
			+ ((childL) ? container_of(childL, struct node, node)->weight : 0)
			+ ((childR) ? container_of(childR, struct node, node)->weight : 0)
			)
		{
		return EAVL_ERROR;
		}
	if (container_of(eavl_node, struct node, node)->sum !=
			container_of(eavl_node, struct node, node)->count
			+ ((childL) ? container_of(childL, struct node, node)->sum : 0)
			+ ((childR) ? container_of(childR, struct node, node)->sum : 0)
			)
		{
		return EAVL_ERROR;
		}

	return EAVL_CB_OK;
	return EAVL_CB_FINISHED;
	}


int Node_fixup(
		EAVLc_node_t*		eavl_node,
		EAVLc_node_t*		childL,
		EAVLc_node_t*		childR,
		void*			data
		)
	{
	UNUSED(data);

//	printf("Fixup::  ");
//	CBNODEPRINT(eavl_node, "");
//	CBNODEPRINTHW(eavl_node);
//	CBNODEPRINT(childL, "  L:");
//	CBNODEPRINTHW(childL);
//	CBNODEPRINT(childR, "  R:");
//	CBNODEPRINTHW(childR);
//	printf("\n");

	container_of(eavl_node, struct node, node)->height =
			1 + MAX(
				((childL) ? container_of(childL, struct node, node)->height : 0),
				((childR) ? container_of(childR, struct node, node)->height : 0)
				)
			;
	container_of(eavl_node, struct node, node)->weight =
			1
			+ ((childL) ? container_of(childL, struct node, node)->weight : 0)
			+ ((childR) ? container_of(childR, struct node, node)->weight : 0)
			;
	container_of(eavl_node, struct node, node)->sum =
			container_of(eavl_node, struct node, node)->count
			+ ((childL) ? container_of(childL, struct node, node)->sum : 0)
			+ ((childR) ? container_of(childR, struct node, node)->sum : 0)
			;

	return EAVL_OK;
	}


static int Node_release(
		EAVLc_node_t*		node,
		void*			data
		)
	{
	UNUSED(node);
	UNUSED(data);

//	printf("Release::  ");
//	CBNODEPRINT(node, "");
//	printf("\n");

	/* Do nothing */

	return EAVL_OK;
	}


static EAVLc_node_t* Ndup(EAVLc_node_t* node, void* cbdata)
	{
	UNUSED(node);
	UNUSED(cbdata);

	printf("NodeDupe::  %p\n", (void*)node);

	return NULL;
	}


EAVLc_cbset_t	cbset =
		{
		&Node_CMP,
		&Ndup,
		&Node_fixup,
		&Node_verify
		};


void Init_nodes(
		struct node fnodes[],
		EAVLc_node_t *fnodep[],
		unsigned int count
		)
	{
	unsigned int i;

	for (i=0; i<count; i++)
		{
		fnodes[i].val = i+100;
		fnodes[i].height = -1u;
		fnodes[i].weight = -1u;
		fnodes[i].sum = -1u;
		fnodes[i].count = 0;
		fnodep[i] = &fnodes[i].node;
		}
	}


void init_tree_context(
		EAVLc_tree_t *tree,
		EAVLc_context_t *context
		)
	{
	int error;

	UNUSED(tree);

//	if ((error = EAVLc_Tree_Init(tree, NULL, &cbset)))
//		{
//		printf("ERROR: Tree_Init: %d\n", error);
//		printf("\t%s:%u\n", __FILE__, __LINE__);
//		exit(1);
//		}
//
//	if ((error = EAVLc_Context_Init(context, NULL, NULL)))
//		{
//		printf("ERROR: Context_Init: %d\n", error);
//		printf("\t%s:%u\n", __FILE__, __LINE__);
//		exit(1);
//		}
//
//	if ((error = EAVLc_Context_Associate(context, tree)))
//		{
//		printf("ERROR: Context_Associate: %d\n", error);
//		printf("\t%s:%u\n", __FILE__, __LINE__);
//		exit(1);
//		}

	do
		{
		error = EAVLc_Clear(context, &Node_release);
		} while (error == EAVL_CALLBACK);
	if (error)
		{
		printf("ERROR: Tree_Clear: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}

//	printf("init_tree_context:: finis\n");fflush(NULL);
	}


void build_tree(
		EAVLc_tree_t *tree,
		EAVLc_context_t *context,
		unsigned int count
		)
	{
	int error;

	init_tree_context(tree, context);
	if ((error = EAVLc_Load(context, count, nodep)))
		{
		printf("ERROR: Tree_Load: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);

//		printf("\n");
//		*buffer = '\0';
//		EAVLc_Tree_print(buffer, buffer, "", 2, context->tree->root);
//		printf("\n");

		exit(1);
		}
	}


int insert(
		EAVLc_context_t *context,
		unsigned int k
		)
	{
	EAVLc_node_t *existing;
	int error;

//	printf("%d\n\n", k);

	do
		{
		error = EAVLc_Insert(context, nodep[k], &existing);
		} while (error == EAVL_CALLBACK);

//	printf("\n");
//	*buffer = '\0';
//	EAVLp_Tree_print(buffer, buffer, "", 2, context->tree->root);
//	printf("\n");

	if (error)
		{
		printf("ERROR: Node_Insert: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);

		printf("\n");
		*buffer = '\0';
		EAVLc_Tree_print(buffer, buffer, "", 2, context->tree->root);
		printf("\n");

		printf("Recent::  %p\n", (void*)context->recent);
		printf("PathLen::  %3u\n", context->pathlen);
		print_pathestore(((cbData_t*)(context->common.cbdata))->pathestore);

		exit(1);
		}

	return error;
	}


int Eremove(
		EAVLc_context_t *context,
		unsigned int k
		)
	{
	int error = 0;
	EAVLc_node_t *dummy;

//	printf("\n");
//	*buffer = '\0';
//	EAVLc_Tree_print(buffer, buffer, "", 2, context->tree->root);
//	printf("\n");

//	printf("%d\n\n", k);

	do
		{
		error = EAVLc_Find(context, EAVL_FIND_EQ, NULL, NULL, nodep[k], &dummy);
		} while (error == EAVL_CALLBACK);
	if (error)
		{
		printf("ERROR: Node_Find: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);

		printf("\n");
		*buffer = '\0';
		EAVLc_Tree_print(buffer, buffer, "", 2, context->tree->root);
		printf("\n");

		printf("Recent::  %p\n", (void*)context->recent);
		printf("PathLen::  %3u\n", context->pathlen);

		exit(1);
		}
	do
		{
		error = EAVLc_Remove(context, NULL);
		} while (error == EAVL_CALLBACK);

//	printf("\n");
//	*buffer = '\0';
//	EAVLc_Tree_print(buffer, buffer, "", 2, context->tree->root);
//	printf("\n");

//	print_pathestore(((cbData_t*)(context->common.cbdata))->pathestore);

	if (error)
		{
		printf("ERROR: Node_Remove: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);

		printf("\n");
		*buffer = '\0';
		EAVLc_Tree_print(buffer, buffer, "", 2, context->tree->root);
		printf("\n");

		printf("Recent::  %p\n", (void*)context->recent);
		printf("PathLen::  %3u\n", context->pathlen);
		print_pathestore(((cbData_t*)(context->common.cbdata))->pathestore);

		exit(1);
		}

	return error;
	}


int Nfixup(
		EAVLc_context_t*	context,
		unsigned int		k
		)
	{
	EAVLc_node_t*		dummy;
	int			error = 0;

	do
		{
		error = EAVLc_Find(context, EAVL_FIND_EQ, NULL, NULL, nodep[k], &dummy);
		} while (error == EAVL_CALLBACK);

	if (!error)
		{
		container_of(nodep[k], struct node, node)->count++;
		error = EAVLc_Fixup(context);
//printf("\n");
		}

	return error;
	}


int traverse(
		EAVLc_context_t*	context,
		EAVL_dir_t		dir,
		EAVL_order_t		order
		)
	{
	EAVLc_node_t*	result;
	int		error;

	do
		{
		error = EAVLc_First(context, dir, order, &result);
		} while (error == EAVL_CALLBACK);

	while (error == EAVL_OK)
		{
		printf("Found: (%d)[n:%p a:%p]\n",
				container_of(result, struct node, node)->val,
				(void *)container_of(result, struct node, node),
				(void *)result
				);

		do
			{
			error = EAVLc_Next(context, dir, order, &result);
			} while (error == EAVL_CALLBACK);
		}

	if (error != EAVL_NOTFOUND)
		{
		printf("ERROR: Node_First/Next{d:%u o:%u}: %d\n", dir, order, error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		}

	return error;
	}


int main(int argc, char **argv)
	{
	unsigned int count = 0;
	EAVLc_context_t	context;
	EAVLc_tree_t	tree;
	int error;
	unsigned int k;
	unsigned int i;
/*
	INFO("void *:", void *);
	printf("\n");

	INFO("char:", char);
	INFO("short:", short);
	INFO("int:", int);
	INFO("long:", long);
	INFO("long long:", long long);
	printf("\n");

	INFO("float:", float);
	INFO("double:", double);
	printf("\n");

	INFO("EAVLc_tree_t:", EAVLc_tree_t);
	INFO("EAVLc_node_t:", EAVLc_node_t);
	INFO("EAVLc_context_t:", EAVLc_context_t);
	printf("\n");
*/

	EAVLc_Checks_Enabled = EAVLc_Checks_Available;

	if (argc > 1)
		{
		count = (unsigned int)strtol(argv[1], NULL, 10);
		}

	if (!(1 <= count && count <= NODES))
		{
		exit(1);
		}

	Init_nodes(nodes, nodep, NODES-1);

	if ((error = EAVLc_Tree_Init(&tree, NULL, &cbset)))
		{
		printf("ERROR: Tree_Init: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}

	if ((error = EAVLc_Context_Init(&context, &cb_pathe, create_cbData(1))))
		{
		printf("ERROR: Context_Init: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}

	if ((error = EAVLc_Context_Associate(&context, &tree)))
		{
		printf("ERROR: Context_Associate: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}

	if ((error = EAVLc_Load(&context, count, nodep)))
		{
		printf("ERROR: Tree_Load: %d\n", error);
		printf("\t%s:%u\n", __FILE__, __LINE__);
		exit(1);
		}


	*buffer = '\0';

	printf("\n");
	EAVLc_Tree_print(buffer, buffer, "", 2, tree.root);
	printf("\n");


	for (k=EAVL_DIR_LEFT; k<=EAVL_DIR_RIGHT; k++)
		{
		EAVLc_node_t *result = NULL;

		do
			{
			error = EAVLc_First(
					&context,
					k,
					EAVL_ORDER_IN,
					&result
				);
			} while (error == EAVL_CALLBACK);
		if (error)
			{
			printf("ERROR: Node_First{u:%d}: %d\n", k, error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			printf("Recent::  %p\n", (void*)context.recent);
			printf("PathLen::  %3u\n", context.pathlen);
			print_pathestore(((cbData_t*)context.common.cbdata)->pathestore);

			exit(1);
			}

		if (result)
			{
			printf("Found: (%d)[n:%p a:%p]{d:%u}\n",
					container_of(result, struct node, node)->val,
					(void *)container_of(result, struct node, node),
					(void *)result,
					k
					);
			}
		}


	if (argc > 2)
		{
		int val = atoi(argv[2]);
		EAVLc_node_t *result;

		printf("\n");

		if ((error = EAVLc_Find(
				&context,
				EAVL_FIND_EQ,
				NULL,
				&val,
				NULL,
				&result
				)))
			{
			printf("ERROR: Node_Find: %d\n", error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			exit(1);
			}

		if (result)
			{
			printf("Found: (%d)[n:%p a:%p]\n",
					container_of(result, struct node, node)->val,
					(void *)container_of(result, struct node, node),
					(void *)result
					);
			}
		else
			{
			printf("NOT found: (%d)\n", val);
			}
		}


	if (argc > 3)
		{
		unsigned int val = (unsigned int)strtol(argv[2], NULL, 10);
		unsigned int dir = (unsigned int)strtol(argv[3], NULL, 10);
		EAVLc_node_t *result;

		printf("\n");

		if ((error = EAVLc_Find(
				&context,
				EAVL_FIND_EQ,
				NULL,
				&val,
				NULL,
				&result
				)))
			{
			printf("ERROR: Node_Find: %d\n", error);
			printf("\t%s:%u\n", __FILE__, __LINE__);
			exit(1);
			}

		if (!result)
			{
			printf("NOT found: (%d)\n", val);
			}

		while (result)
			{
			printf("Found: (%d)[n:%p a:%p]\n",
					container_of(result, struct node, node)->val,
					(void *)container_of(result, struct node, node),
					(void *)result
					);

			error = EAVLc_Next(
					&context,
					dir,
					EAVL_ORDER_IN,
					&result
					);
			if (error == EAVL_NOTFOUND)
				{
				break;
				}
			else if (error != EAVL_OK)
				{
				printf("ERROR: Node_Next{d:%d}: %d\n", dir, error);
				printf("\t%s:%u\n", __FILE__, __LINE__);
				printf("%p\n", (void*)context.recent);
				exit(1);
				}
			}
		}

//  traverse RIGHT IN
	printf("\n== traverse RIGHT IN\n");
	(void) traverse(&context, EAVL_DIR_RIGHT, EAVL_ORDER_IN);

//  traverse LEFT IN
	printf("\n== traverse LEFT IN\n");
	(void) traverse(&context, EAVL_DIR_LEFT, EAVL_ORDER_IN);

//  traverse RIGHT PRE
	printf("\n== traverse RIGHT PRE\n");
	(void) traverse(&context, EAVL_DIR_RIGHT, EAVL_ORDER_PRE);

//  traverse LEFT POST
	printf("\n== traverse LEFT POST\n");
	(void) traverse(&context, EAVL_DIR_LEFT, EAVL_ORDER_POST);

//  traverse RIGHT POST
	printf("\n== traverse RIGHT POST\n");
	(void) traverse(&context, EAVL_DIR_RIGHT, EAVL_ORDER_POST);

//  traverse LEfT PRE
	printf("\n== traverse LEFT PRE\n");
	(void) traverse(&context, EAVL_DIR_LEFT, EAVL_ORDER_PRE);

//  insert LEFT
	printf("\n== insert LEFT\n");
	init_tree_context(&tree, &context);
	check_reset(count, 0);
	for (k=0; k<count; k++)
		{
		check_op(&context, k, &insert, EAVL_OK, "Insert LEFT", 1);
		}

//  insert RIGHT
	printf("\n== insert RIGHT\n");
	init_tree_context(&tree, &context);
	check_reset(count, 0);
	for (k=count-1; k<count; k--)
		{
		check_op(&context, k, &insert, EAVL_OK, "Insert RIGHT", 1);
		}

// insert inside-out
	printf("\n== insert inside-out\n");
	init_tree_context(&tree, &context);
	check_reset(count, 0);
	for (k=((count+1)>>1)-1; k<((count+1)>>1); k-=1)
		{
		check_op(&context, k, &insert, EAVL_OK, "Insert inside-out", 1);
		if (k != count-1-k)
			{
			check_op(&context, count-1-k, &insert, EAVL_OK, "Insert inside-out", 1);
			}
		}

//  insert outside-in
	printf("\n== insert outside-in\n");
	init_tree_context(&tree, &context);
	check_reset(count, 0);
	for (k=0; k<((count+1)>>1); k+=1)
		{
		check_op(&context, k, &insert, EAVL_OK, "Insert outside-in", 1);
		if (k != count-1-k)
			{
			check_op(&context, count-1-k, &insert, EAVL_OK, "Insert outside-in", 1);
			}
		}

//  remove LEFT
	printf("\n== remove LEFT\n");
	build_tree(&tree, &context, count);
	check_reset(count, 1);
	for (k=0; k<count; k++)
		{
		check_op(&context, k, &Eremove, EAVL_OK, "Remove LEFT", 0);
		}

//  remove RIGHT
	printf("\n== remove RIGHT\n");
	build_tree(&tree, &context, count);
	check_reset(count, 1);
	for (k=count-1; k<count; k--)
		{
		check_op(&context, k, &Eremove, EAVL_OK, "Remove RIGHT", 0);
		}

//  remove inside-out
	printf("\n== remove inside-out\n");
	build_tree(&tree, &context, count);
	check_reset(count, 1);
	for (k=((count+1)>>1)-1; k<((count+1)>>1); k-=1)
		{
		check_op(&context, k, &Eremove, EAVL_OK, "Remove inside-out", 0);
		if (k != count-1-k)
			{
			check_op(&context, count-1-k, &Eremove, EAVL_OK, "Remove inside-out", 0);
			}
		}

//  remove outside-in
	printf("\n== remove outside-in\n");
	build_tree(&tree, &context, count);
	check_reset(count, 1);
	for (k=0; k<((count+1)>>1); k+=1)
		{
		check_op(&context, k, &Eremove, EAVL_OK, "Remove outside-in", 0);
		if (k != count-1-k)
			{
			check_op(&context, count-1-k, &Eremove, EAVL_OK, "Remove outside-in", 0);
			}
		}

// insert-remove
	printf("\n== insert-remove\n");
	init_tree_context(&tree, &context);
	check_reset(count, 0);
	srandom(2);
	for (i=0; i<count; i++)
		{
		k = (unsigned int)random()%count;
		check_op(&context, k, &insert, EAVL_OK, "Insert-Remove insert", 1);
		check_op(&context, k, &Eremove, EAVL_OK, "Insert-Remove remove", 0);
		}

//  random
	printf("\n== Random\n");
	init_tree_context(&tree, &context);
	check_reset(count, 0);
	srandom(2);
	while (1)
		{
		EAVLc_node_t *dummy;

		k = (unsigned int)random()%count;
		do
			{
			error = EAVLc_Find(
					&context,
					EAVL_FIND_EQ,
					NULL,
					NULL,
					nodep[k],
					&dummy
					);
			} while (error == EAVL_CALLBACK);
		switch (error)
			{
			case EAVL_OK:
				if (random() & 0x3)
					{
					check_op(&context, k, &Nfixup, EAVL_OK, "Random Fixup", 1);
					}
				else
					{
					check_op(&context, k, &Eremove, EAVL_OK, "Random Remove", 0);
					}
				break;

			case EAVL_NOTFOUND:
				check_op(&context, k, &insert, EAVL_OK, "Random Insert", 1);
				break;

			default:
				printf("ERROR: Node_Find: %d\n", error);
				printf("\t%s:%u\n", __FILE__, __LINE__);
				exit(1);
			}
		}

	destroy_cbData(context.common.cbdata);

	return 0;
	}
