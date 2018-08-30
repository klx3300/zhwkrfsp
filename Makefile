__fake_main__: server

zhwkre/zhwkre_bubble_sort.o: zhwkre/algorithm/bubble_sort.c
	@echo "   gcc   zhwkre/zhwkre_bubble_sort.o"
	@gcc -c  -o zhwkre/zhwkre_bubble_sort.o zhwkre/algorithm/bubble_sort.c
zhwkre/zhwkre_bss.o: zhwkre/bss/bss.c
	@echo "   gcc   zhwkre/zhwkre_bss.o"
	@gcc -c  -o zhwkre/zhwkre_bss.o zhwkre/bss/bss.c
zhwkre/zhwkre_mutex.o: zhwkre/concurrent/mutex.c
	@echo "   gcc   zhwkre/zhwkre_mutex.o"
	@gcc -c  -o zhwkre/zhwkre_mutex.o zhwkre/concurrent/mutex.c
zhwkre/zhwkre_threading.o: zhwkre/concurrent/threading.c
	@echo "   gcc   zhwkre/zhwkre_threading.o"
	@gcc -c  -o zhwkre/zhwkre_threading.o zhwkre/concurrent/threading.c
zhwkre/zhwkre_list.o: zhwkre/containers/list.c
	@echo "   gcc   zhwkre/zhwkre_list.o"
	@gcc -c  -o zhwkre/zhwkre_list.o zhwkre/containers/list.c
zhwkre/zhwkre_unordered_map.o: zhwkre/containers/unordered_map.c
	@echo "   gcc   zhwkre/zhwkre_unordered_map.o"
	@gcc -c  -o zhwkre/zhwkre_unordered_map.o zhwkre/containers/unordered_map.c
zhwkre/zhwkre_vector.o: zhwkre/containers/vector.c
	@echo "   gcc   zhwkre/zhwkre_vector.o"
	@gcc -c  -o zhwkre/zhwkre_vector.o zhwkre/containers/vector.c
zhwkre/zhwkre_btree.o: zhwkre/containers/btree.c
	@echo "   gcc   zhwkre/zhwkre_btree.o"
	@gcc -c -g -o zhwkre/zhwkre_btree.o zhwkre/containers/btree.c
zhwkre/zhwkre_avltree.o: zhwkre/containers/AVLTree.c
	@echo "   gcc   zhwkre/zhwkre_avltree.o"
	@gcc -c -g -o zhwkre/zhwkre_avltree.o zhwkre/containers/AVLTree.c
zhwkre/zhwkre_set.o: zhwkre/containers/set.c
	@echo "   gcc   zhwkre/zhwkre_set.o"
	@gcc -c -g -o zhwkre/zhwkre_set.o zhwkre/containers/set.c
zhwkre/zhwkre_socket.o: zhwkre/network/socket.c
	@echo "   gcc   zhwkre/zhwkre_socket.o"
	@gcc -c  -o zhwkre/zhwkre_socket.o zhwkre/network/socket.c
zhwkre/zhwkre_tcp.o: zhwkre/network/tcp.c
	@echo "   gcc   zhwkre/zhwkre_tcp.o"
	@gcc -c  -o zhwkre/zhwkre_tcp.o zhwkre/network/tcp.c
zhwkre/zhwkre_udp.o: zhwkre/network/udp.c
	@echo "   gcc   zhwkre/zhwkre_udp.o"
	@gcc -c  -o zhwkre/zhwkre_udp.o zhwkre/network/udp.c
zhwkre/zhwkre_serialization.o: zhwkre/serialization/serialization.c
	@echo "   gcc   zhwkre/zhwkre_serialization.o"
	@gcc -c  -o zhwkre/zhwkre_serialization.o zhwkre/serialization/serialization.c
zhwkre/zhwkre_utils.o: zhwkre/utils/utils.c
	@echo "   gcc   zhwkre/zhwkre_utils.o"
	@gcc -c  -o zhwkre/zhwkre_utils.o zhwkre/utils/utils.c
zhwkre/zhwkre_error.o: zhwkre/error/error.c
	@echo "   gcc   zhwkre/zhwkre_error.o"
	@gcc -c  -o zhwkre/zhwkre_error.o zhwkre/error/error.c
zhwkre/zhwkre_fmt.o: zhwkre/format/format.c
	@echo "   gcc   zhwkre/zhwkre_fmt.o"
	@gcc -c  -o zhwkre/zhwkre_fmt.o zhwkre/format/format.c
zhwkre/zhwkre_btree_test.o: zhwkre/tests/btree_test.c
	@echo "   gcc   zhwkre/zhwkre_btree_test.o"
	@gcc -c -g -o zhwkre/zhwkre_btree_test.o zhwkre/tests/btree_test.c
zhwkre/zhwkre_bss_test.o: zhwkre/tests/bss_test.c
	@echo "   gcc   zhwkre/zhwkre_bss_test.o"
	@gcc -c -g -o zhwkre/zhwkre_bss_test.o zhwkre/tests/bss_test.c
zhwkre/zhwkre_avltree_test.o: zhwkre/tests/avltree_test.c
	@echo "   gcc   zhwkre/zhwkre_avltree_test.o"
	@gcc -c -g -o zhwkre/zhwkre_avltree_test.o zhwkre/tests/avltree_test.c
zhwkre/zhwkre_vector_test.o: zhwkre/tests/vector_test.c
	@echo "   gcc   zhwkre/zhwkre_vector_test.o"
	@gcc -c -g -o zhwkre/zhwkre_vector_test.o zhwkre/tests/vector_test.c
zhwkre/btree_test: zhwkre/zhwkre/zhwkre_btree_test.o zhwkre_btree.o zhwkre_error.o zhwkre_fmt.o 
	@echo "   LD   zhwkre/btree_test"
	@gcc  -o zhwkre/btree_test zhwkre/zhwkre_btree_test.o zhwkre_btree.o zhwkre_error.o zhwkre_fmt.o 
zhwkre/bss_test: zhwkre/zhwkre/zhwkre_bss_test.o zhwkre_bss.o zhwkre_error.o zhwkre_fmt.o 
	@echo "   LD   zhwkre/bss_test"
	@gcc  -o zhwkre/bss_test zhwkre/zhwkre_bss_test.o zhwkre_bss.o zhwkre_error.o zhwkre_fmt.o 
zhwkre/avltree_test: zhwkre/zhwkre/zhwkre_avltree_test.o zhwkre_avltree.o zhwkre_error.o zhwkre_fmt.o 
	@echo "   LD   zhwkre/avltree_test"
	@gcc  -o zhwkre/avltree_test zhwkre/zhwkre_avltree_test.o zhwkre_avltree.o zhwkre_error.o zhwkre_fmt.o 
zhwkre/vector_test: zhwkre/zhwkre/zhwkre_vector_test.o zhwkre_vector.o zhwkre_error.o zhwkre_fmt.o 
	@echo "   LD   zhwkre/vector_test"
	@gcc  -o zhwkre/vector_test zhwkre/zhwkre_vector_test.o zhwkre_vector.o zhwkre_error.o zhwkre_fmt.o 
fuse_server.o: fuse_server.c
	@echo "   gcc   fuse_server.o"
	@gcc -c -g -o fuse_server.o fuse_server.c
netwrap.o: netwrap.c
	@echo "   gcc   netwrap.o"
	@gcc -c -g -o netwrap.o netwrap.c
ustruct.o: ustruct.c
	@echo "   gcc   ustruct.o"
	@gcc -c -g -o ustruct.o ustruct.c
prot.o: prot.c
	@echo "   gcc   prot.o"
	@gcc -c -g -o prot.o prot.c
server.o: server.c
	@echo "   gcc   server.o"
	@gcc -c -g -o server.o server.c
server: server.o fuse_server.o netwrap.o ustruct.o prot.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_tcp.o zhwkre/zhwkre_utils.o zhwkre/zhwkre_bss.o zhwkre/zhwkre_error.o zhwkre/zhwkre_fmt.o 
	@echo "   LD   server"
	@gcc -pthread -lfuse3 -lbsd -o server server.o fuse_server.o netwrap.o ustruct.o prot.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_tcp.o zhwkre/zhwkre_utils.o zhwkre/zhwkre_bss.o zhwkre/zhwkre_error.o zhwkre/zhwkre_fmt.o 
clean:
	@echo "  CLEAN   all files."
	-@rm zhwkre/zhwkre_bubble_sort.o zhwkre/zhwkre_bss.o zhwkre/zhwkre_mutex.o zhwkre/zhwkre_threading.o zhwkre/zhwkre_list.o zhwkre/zhwkre_unordered_map.o zhwkre/zhwkre_vector.o zhwkre/zhwkre_btree.o zhwkre/zhwkre_avltree.o zhwkre/zhwkre_set.o zhwkre/zhwkre_socket.o zhwkre/zhwkre_tcp.o zhwkre/zhwkre_udp.o zhwkre/zhwkre_serialization.o zhwkre/zhwkre_utils.o zhwkre/zhwkre_error.o zhwkre/zhwkre_fmt.o zhwkre/zhwkre_btree_test.o zhwkre/zhwkre_bss_test.o zhwkre/zhwkre_avltree_test.o zhwkre/zhwkre_vector_test.o zhwkre/btree_test zhwkre/bss_test zhwkre/avltree_test zhwkre/vector_test fuse_server.o netwrap.o ustruct.o prot.o server.o server  || true
