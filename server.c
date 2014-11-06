#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

struct line_info {
	int* users;
	int pos;
	int* max;
};
void add(int client_fd, int* users, int* pos, int* max);
void* line(void* argument);

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
		if (strcmp(resp,exit)==0)
		{
			users[pos] = -1;
			pthread_exit(NULL);
		}
		for (i=0; i<(*max); i++)
		{
			if (users[i] > 0 && resp[0] == 'U')
				write(users[i],resp,strlen(resp));
		}	 			
	}
	return NULL;
}

int main(int argc, char** argv)
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
    return 0;
}
