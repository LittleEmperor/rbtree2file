
#include <stdio.h>
#include "buildrbtree.h"
#include "rbtree2file.h"

int main()
{
	struct rb_root *root = NULL;
	/**
	 * build rbtree and print tree
	 * */
	root = init_treeroot_and_treenode(10);
	print_tree(root);

	/**
	 * save rbtree to a diskfile and free the rbtree
	 * */
	tree2diskfile("./tree.bin", root->rb_node, (long)&(((struct mytype *)0)->node), sizeof(struct mytype));
	free_all_treenodes(root->rb_node);

	/**
	 * restore the rbtree from the diskfile and print tree
	 * 
	 * Note that diskfile2tree's return value is [struct rb_node *]
	 * Not [struct rb_root *]
	 * */
	root->rb_node = diskfile2tree("./tree.bin");
	printf("\nafter rebuild:\n");
	print_tree(root);

	/**
	 * free all treenodes and free treeroot
	 * */
	free_all_treenodes(root->rb_node);
	free_treeroot(root);
	return 0;
}
