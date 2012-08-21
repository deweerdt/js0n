all:
	gcc -O3 -Wall -o js0n_test_goto js0n_test.c js0n.c 
	gcc -O3 -Wall -o js0n_test_cb js0n_test.c js0n_fn_callbacks.c 
