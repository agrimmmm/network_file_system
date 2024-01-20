all: naming_server storage_server client

naming_server: tries.c lru.c ss_info.c readwritelock.c nm_functions.c nm_modular.c NM.c
	gcc -o naming_server tries.c lru.c ss_info.c readwritelock.c nm_functions.c nm_modular.c NM.c 

storage_server: ss_client_orders.c ss_nm_orders.c storage_server.c ss_copydir.c ss_tries.c ss_backup.c
	gcc -o storage_server ss_client_orders.c ss_nm_orders.c storage_server.c ss_copydir.c ss_tries.c ss_backup.c

client: client.c
	gcc -o client client.c	

clean:
	rm -f naming_server storage_server client


