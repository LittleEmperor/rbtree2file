
lib:
	gcc -o t main.c rbtree.c buildrbtree.c rbtree2file.c -g -Wall
clean:
	rm *.o t tree.bin -f
