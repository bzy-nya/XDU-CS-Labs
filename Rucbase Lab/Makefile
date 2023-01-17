copy:
	cd build; \
	cmake .. -DCMAKE_BUILD_TYPE=Debug

t1_1: copy
	cd build; \
	make disk_manager_test; \
	./bin/disk_manager_test

t1_2: copy
	cd build; \
	make lru_replacer_test; \
	./bin/lru_replacer_test

t1_3: copy
	cd build; \
	make buffer_pool_manager_test; \
	./bin/buffer_pool_manager_test

t1_4: copy
	cd build; \
	make rm_gtest; \
	./bin/rm_gtest

t2_1: copy
	cd build; \
	make b_plus_tree_insert_test; \
	./bin/b_plus_tree_insert_test

t2_2: copy
	cd build; \
	make b_plus_tree_delete_test; \
	./bin/b_plus_tree_delete_test

t2_3: copy
	cd build; \
	make b_plus_tree_concurrent_test; \
	./bin/b_plus_tree_concurrent_test

all: t1_1 t1_2 t1_3 t1_4 t2_1 t2_2 t2_3

clean:
	rm -rf build/*