#include <pthread.h>
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

struct line_info {
	int* users;
	int pos;
	int* max;
};
void add(int client_fd, int* users, int* pos, int* max);
void* line(void* argument);
void* server(void* argument);

void add(int client, int* users, int* pos, int* max)
{
	if (*pos >= *max)
	{
		int* newusers = malloc(2 * (*max) * sizeof(int));
		int i;
		for (i = 0; i < *max; i++)
			newusers[i] = users[i];
		free(users);
		*max = (*max) * 2;
		users = newusers; 
	}

	users[*pos] = client;
	(*pos)++;
}

void* line(void* argument)
{
	int pos = ((struct line_info*) argument)->pos;
	int* users = ((struct  line_info*) argument)->users;
	int* max = ((struct line_info*) argument)->max;
	int i;
	char resp[100];
	int len;
	char* exit = "exit";
	while(1)
	{
		len = read(users[pos], resp, 99);
		resp[len] = '\0';
		printf("%s\n",resp);
		if (strcmp(resp,exit)==0)
		{
			users[pos] = -1;
			pthread_exit(NULL);
		}
		for (i=0; i<(*max); i++)
		{
			if (users[i] > 0)
				write(users[i],resp,len);
		}	 			
	}
	return NULL;
}

int main(int argc, char** argv)
{
    if (strcmp(argv[1],"127.0.0.1") == 0)
    {
		pthread_t serve;
		pthread_create(& serve, NULL, server, NULL);
    }

    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    char user[100];
    printf("What's your username: ");
    gets(user);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* IPv4 only */
    hints.ai_socktype = SOCK_STREAM; /* TCP */



    s = getaddrinfo(argv[1], "2000", &hints, &result);
    

    if (s != 0) {
            exit(1);
    }

    connect(sock_fd, result->ai_addr, result->ai_addrlen);
    char welcome[100];
    strcpy(welcome, "User ");
    strcat(welcome, user);
    strcat(welcome, " just joined, say hello!\n");
    int messagesize = strlen(welcome);
    encode(welcome,argv[2], messagesize);
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
			encode(del,argv[2],size);
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
	    encode(resp,argv[2],len);
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

void* server(void* argument)
{
    int* users = malloc(10 * sizeof(int));
    int position = 0;
    int max_users = 10;
    int s;
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, "2000", &hints, &result);

    if (s != 0) {
            exit(1);
    }

    if ( bind(sock_fd, result->ai_addr, result->ai_addrlen) != 0 )
    {
        exit(1);
    }

    if ( listen(sock_fd, 10) != 0 )
    {
        exit(1);
    }

    while (1)
    {
        int client_fd = accept(sock_fd, NULL, NULL);
		struct line_info package;
		package.users = users;
		package.pos = position;
		package.max = &position;
		add(client_fd,users,&position,&max_users);
		pthread_t id;
		pthread_create(& id, NULL, line, &package);
    }
    return NULL;
}
