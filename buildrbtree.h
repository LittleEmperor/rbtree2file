#ifndef __BUILDTREE_H__
#define __BUILDTREE_H__

#include "rbtree_augmented.h"
struct mytype {
	long data;
	char *keystring;
	struct rb_node node;
};
struct rb_root *init_treeroot_and_treenode(long nr_nodes);
void free_treeroot(struct rb_root *root);
void free_all_treenodes(struct rb_node *root);
void print_tree(struct rb_root *root);

#endif

