/*

	How to save a tree to a disk file ?
		-- by LittleEmperor on 2018.08.24
	
	
思考：
	如何把一棵二叉树保存在磁盘上：
难点：
	指针保存在磁盘上是无意义的，需要保存的不是指针，而是节点之间的关系

方案：	
	核心思想：利用二叉树后序遍历的性质
	按后序遍历保存各个节点，子节点的id一定小于父节点的id，当修正第X个节点时，他的子树一定修正完毕了
             9
           /   \
          5     8
         / \    /\
        3   4  6  7
       / \
      1   2

限制：
	当前解决方案的前提是：每个节点的大小都是相同的.

I. 存储过程：
	1. 构造磁盘文件头结构，头结构保存2个变量：节点总个数,每个节点的大小
	2. 递归后序遍历二叉树，因为后序遍历可以保证每个节点的id号都比他的子节点大
	3. 对于被遍历到的每个节点，id号从1开始编号，每遍历到一个有效节点，id自增
	4. 对于每个被遍历到的节点，记录其左右子树id，NULL节点id为0
	5. 把当前被遍历到的节点的左右子树指针替换成左右子树id，整个节点存入磁盘
	6. 释放当前结点
	
	磁盘文件存储格式：
	头信息 + N个节点的详细信息
	---------------------------------------------------------------------------
	|                   |             |             |           |             |
	|节点个数,每节点长度| ++ 节点1 ++ | ++ 节点2 ++ | ********* | ++ 节点N ++ |
	|                   |             |             |           |             |
	---------------------------------------------------------------------------
		
II. 构建过程：
	1. 读取文件头，申请一片连续的空间，大小为 each_node_size * number_of_nodes
	2. 取出第一个节点，写入缓冲区buff[0]位置，取第二个，放在buf[1]位置 *****
	3. 对于每个取出的节点，指针域修改为基址+偏移量。即 buff + each_node_size*(id-1)
	4. 特殊地，如果id为0，则指针域为NULL

	如果这棵树被重新构建后仍需要继续进行执行增删改查等动作，需要确保每个节点都是可以
	随时释放的，此时对于每个节点都需要调用一次malloc分配单独的chunk。
	
	如果这棵二叉树在重新构建后不需要改变，我们可以一次性申请一片很大的空间，像填充
	数组一样恢复每个节点，免去了大量的malloc调用,即快速又节省了内存
	即便恢复过程像填充数组元素一样，但经过指针修正后，逻辑上仍然是一棵树

END

=========================================================================================================
扩展：
		考虑一个复杂的情况，如果每个节点的内容长度不一样大，节点中既包含变长数组，又包含数据的指针。
		
		对于包含数据指针的，可以把数据保存在文件末尾。在指针中保存文件偏移量。
		对于包含变长数组，可直接把数据保存在当前节点后面。这样做会使每个节点的大小不同
		
=========================================================================================================
*/

/*
	Demo:
		2个核心函数: 
			__save_rbnode 递归遍历二叉树，存入文件
			rebuild_tree 读取文件，修正指针，重新构建二叉树
			 或者
			rebuild_tree_readonly 同样的功能，更高的性能
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "rbtree_augmented.h"
#include "buildrbtree.h"

struct file_header{
	char file_magic[8];   /**< file header magic */
	long rbnode_offset;   /**< rbnode's offset in a user's struct */
	long total_count;     /**< total nodes in an rbtree */
	long each_node_size;  /**< each node size */
};
#define FILEMAGIC "TREEFILE"

static int __save_rbnode(struct file_header *fh ,struct rb_node *rbnode, int fd)
{
	long id_l, id_r;
	struct rb_node *pbak_l, *pbak_r;
	char *p = (char *)rbnode;

	if( !rbnode ){
		return 0;/* 如果节点为空，id返回0 */
	}

	/**
	 * backup subtree pointers
	 * */
	pbak_l = rbnode->rb_left;
	pbak_r = rbnode->rb_right;

	/**
	 * save a node to the file
	 * */
	id_l = __save_rbnode(fh, rbnode->rb_left, fd);
	id_r = __save_rbnode(fh, rbnode->rb_right, fd);
	rbnode->rb_left = (struct rb_node *)id_l;
	rbnode->rb_right = (struct rb_node *)id_r;
	write(fd, p - fh->rbnode_offset, fh->each_node_size);

	/**
	 * restore subtree pointers
	 * */
	rbnode->rb_left = pbak_l;
	rbnode->rb_right = pbak_r;

	fh->total_count++;
	
	return fh->total_count; /* 每遍历到一个节点，id自增1， id从1开始，id==0标识一个空节点 */
}

/**
 * @desc:
 * @prarm:
 *
 *
 * */
void tree2diskfile(const char *path, struct rb_node *root_node, long rbnode_offset, long each_node_size)
{
	int fd = open(path, O_RDWR|O_CREAT);
	if(fd < 0){
		perror("open");
		return ;
	}
	
	struct file_header fh;
	memcpy(fh.file_magic, FILEMAGIC, 8);
	fh.each_node_size = each_node_size;
	fh.rbnode_offset = rbnode_offset;
	fh.total_count = 0;

	/**
	 * 先向文件中写入头结构，节点总数设成0. 
	 * 待__save_rbnode完成便可获得节点总数，重新写入即可
	*/
	write(fd, &fh, sizeof(struct file_header));

	__save_rbnode(&fh, root_node, fd);

	lseek(fd, 0, SEEK_SET);
	write(fd, &fh, sizeof(struct file_header));

	close(fd);
}

/**
 * rebuild_tree_readonly can reduce the call of malloc
 *
 * if the tree is not going to change once it is built,
 * we can alloc a large size of mem in one time, and 
 * build the tree just like walking an array, it is more
 * faster than func [rebuild_tree]
 *
 * */
static struct rb_node *rebuild_tree_readonly(const char *file)
{
	int fd = open(file, O_RDONLY);
	if(fd < 0){
		perror("open");
		return NULL;
	}

	struct file_header fh;
	read(fd, &fh, sizeof(struct file_header));
	if(*(long *)fh.file_magic != *(long *)FILEMAGIC){
	       	printf("%s FILEMAGIC check failed\n", file);
	       	return NULL;
	}
	if(!fh.each_node_size || !fh.total_count){ 
		printf("%s there is no rbtree in this file\n", file); 
		return NULL;
	}

	const char *p_base = (char *)malloc(fh.total_count * fh.each_node_size);
	char *raw_node = (char *)p_base;
	struct rb_node *rbnode = NULL;
	
	while(fh.total_count--){
		read(fd, raw_node, fh.each_node_size);
		rbnode = (struct rb_node *)(raw_node + fh.rbnode_offset);

		if(rbnode->rb_left){
			unsigned long id = (unsigned long)rbnode->rb_left;
			unsigned long p = (long)p_base + fh.each_node_size * (id - 1);
			rbnode->rb_left = (struct rb_node *)(p + fh.rbnode_offset);
		}
		if(rbnode->rb_right){
			unsigned long id = (unsigned long)rbnode->rb_right;
			unsigned long p = (long)p_base + fh.each_node_size * (id - 1);
			rbnode->rb_right = (struct rb_node *)(p + fh.rbnode_offset);
		}
		
		raw_node += fh.each_node_size;
	}
	close(fd);
	return rbnode;
}

static struct rb_node *rebuild_tree(const char *file)
{
	int fd = open(file, O_RDONLY);
	if(fd < 0){
		perror("open");
		return NULL;
	}

	struct file_header fh;
	read(fd, &fh, sizeof(struct file_header));
	if(*(long *)fh.file_magic != *(long *)FILEMAGIC){
		printf("%s FILEMAGIC check failed\n", file);
		return NULL;
	}
	if(!fh.each_node_size || !fh.total_count){
		printf("%s there is no rbtree in this file\n", file);
		return NULL;
	}

	/**
	 * p_slots is a tmp buffer for saving node pointers
	 *
	 * once there is a node has a subnode, the subnode's id is 
	 * indecated by node->left or node->right, which can be the 
	 * index of p_slots to get the real node pointer of its subnode
	 *
	 * */
	void **p_slots = (void **)malloc((fh.total_count+1) * fh.each_node_size);
	//memset(p_slots, 0, total_count * sizeof(bnode *));

	char *raw_node = NULL;
	struct rb_node *rbnode= NULL;
	unsigned long index = 0, id_l = 0, id_r = 0;
	/**
	 * let p_slots[0] == NULL
	 * */
	p_slots[index++] = NULL;
	while(fh.total_count--){

		raw_node = (char *)malloc(fh.each_node_size);
		read(fd, raw_node, fh.each_node_size);

		rbnode = (struct rb_node *)(raw_node + fh.rbnode_offset);
		p_slots[index++] = rbnode;

		id_l = (unsigned long)rbnode->rb_left;
		rbnode->rb_left = (struct rb_node *)p_slots[id_l];
		id_r = (unsigned long)rbnode->rb_right;
		rbnode->rb_right = (struct rb_node *)p_slots[id_r];
	}
	free(p_slots);
	close(fd);

	return rbnode;
}


struct rb_node *diskfile2tree(const char *file)
{
	return rebuild_tree(file);
	return rebuild_tree_readonly(file);
}

