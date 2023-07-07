str_mkfifo : str_mkfifo.c
	g++ -o str_mkfifo str_mkfifo.c -pthread

clean :
	rm str_mkfifo $(objects)

