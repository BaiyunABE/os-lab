all: msg_queue share_mem

msg_queue: msg_queue.c
	gcc -o msg_queue msg_queue.c

share_mem: share_mem.c
	gcc -o share_mem share_mem.c

clean:
	rm -f msg_queue share_mem