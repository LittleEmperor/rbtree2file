
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "rbtree_augmented.h"
#include "buildrbtree.h"

#define myoffsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define mycontainer_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - myoffsetof(type,member) );})


int my_insert(struct rb_root *root, struct mytype *data)
{
	struct rb_node **new_node = &(root->rb_node), *parent = NULL;

	/* Figure out where to put new node */
	while (*new_node) {
		struct mytype *this_nod = mycontainer_of(*new_node, struct mytype, node);

		parent = *new_node;
		if (data->data <= this_nod->data)
			new_node = &((*new_node)->rb_left);
		else if (data->data > this_nod->data)
			new_node = &((*new_node)->rb_right);
		else
			return 0;
	}

	/* Add new node and rebalance tree. */
	rb_link_node(&data->node, parent, new_node);
	rb_insert_color(&data->node, root);

	return 1;
}

struct mytype *my_search(struct rb_root *root, char *string)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct mytype *data = mycontainer_of(node, struct mytype, node);
		int result;

		result = strcmp(string, data->keystring);

		if (result < 0)
			node = node->rb_left;
		else if (result > 0)
			node = node->rb_right;
		else
			return data;
	}
	return NULL;
}


struct rb_root *init_treeroot_and_treenode(long nr_nodes)
{
	struct rb_root *mytree = (struct rb_root *)malloc(sizeof(struct rb_root));
	if(!mytree){
		perror("malloc");
		return NULL;
	}
	mytree->rb_node = NULL;

	int i;
	for(i = 0; i < nr_nodes; i++){
		struct mytype *p = (struct mytype *)malloc(sizeof(struct mytype));
		if(!p){
			perror("malloc");
			return NULL;
		}
		p->data = i + 1;

		if(my_insert(mytree, p) != 1){
			printf("insert node err\n");
			return NULL;
		}
	}
	return mytree;
}

void __free_treenode(struct rb_node *node)
{
	if(!node)
		return ;
	struct mytype *this_nod = mycontainer_of(node, struct mytype, node);
	__free_treenode(node->rb_left);
	__free_treenode(node->rb_right);
	free(this_nod);
}

void free_all_treenodes(struct rb_node *rootnode)
{
	if(rootnode)
		__free_treenode(rootnode);
}

void free_treeroot(struct rb_root *root)
{
	if(root)
		free(root);
}

void __print_tree(struct rb_node *node)
{
	if(!node)
		return;

	struct mytype *this_nod = mycontainer_of(node, struct mytype, node);

	__print_tree(node->rb_left);
	printf("node:%p, data:%lu, node->l:%p, node->r:%p\n", 
				node, this_nod->data, node->rb_left, node->rb_right);
	__print_tree(node->rb_right);

}

void print_tree(struct rb_root *root)
{
	__print_tree(root->rb_node);
}
