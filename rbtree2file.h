#ifndef __TREEFILE_H__
#define __TREEFILE_H__

#include "rbtree_augmented.h"

void tree2diskfile(const char *path, struct rb_node *root_node, long rbnode_offset, long each_node_size);
struct rb_node *diskfile2tree(const char *file);

#endif
