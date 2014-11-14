#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

void encode(char* target, char* key, int len);

int main(int argc, char** argv)
{
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    char user[100];
    printf("What's your username: ");
    gets(user);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* TCP */

    s = getaddrinfo(NULL, "2000", &hints, &result);
    if (s != 0) {
            exit(1);
    }

    connect(sock_fd, result->ai_addr, result->ai_addrlen);
    char welcome[100];
    strcpy(welcome, "User ");
    strcat(welcome, user);
    strcat(welcome, " just joined, say hello!\n");
    int messagesize = strlen(welcome);
    encode(welcome,argv[1], messagesize);
    write(sock_fd, welcome, messagesize);
    int done = 0;
    char* exit = "exit";
    char* message;
    pid_t childid = fork();
    int size;
    while (!done)
    {
        if (childid > 0)
	{
	    	char* buffer;
	    	message = getpass("");
		if (strcmp(message,exit) == 0)
	    	{
			done = 1;
			welcome[5] = '\0';
			strcat(welcome,user);
			kill(childid, SIGTERM);
			buffer = message;
			size = strlen(buffer);
	    	}

	    	else
	    	{
			char del[100];
			strcpy(del,"User ");
			strcat(del,user);
			strcat(del, ": ");
			strcat(del,message);
			size = strlen(del);
			encode(del,argv[1],size);
			buffer = del;
	    	}
	    	write(sock_fd, buffer, size);
	}
	
	else
	{
	    char resp[100];
	    int len = read(sock_fd, resp, 99);
    	    resp[len] = '\0';
	    int i;
	    encode(resp,argv[1],len);
	    if (resp[0] == 'U')
	    	printf("%s\n", resp);
	}
    }
    return 0;
}

void encode(char* target, char* key, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
        	target[i] = target[i] ^ key[i % strlen(key)];
	}
}
