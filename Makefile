shell: shell.c utils.c
	gcc -Wall -o shell shell.c utils.c

clean:
	rm -f shell
