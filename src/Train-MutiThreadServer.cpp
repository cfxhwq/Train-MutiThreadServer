//============================================================================
// Name        : Train-MutiThreadServer.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <stdio.h>
#include <iostream>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
using namespace std;

#define PORT 1234
#define MAXDATASIZE 100
#define BACKLOG 2

void process_cli(int connectfd, struct sockaddr_in client);
void *start_routine(void *arg);
struct ARG {
	int connfd;
	sockaddr_in client;
};
struct ARG *arg;
int main() {

	int listenfd, connectfd;
	pthread_t tid;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t addrlen;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket() error.");
		exit(-1);
	}

	int opt = SO_REUSEADDR;
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))
			== -1) {
		perror("setsockopt() error ");
		exit(-1);
	}

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
		cout << server.sin_addr.s_addr << endl;
		perror("bind() error");
		exit(-1);
	}
	cout << "bind finish" << endl;

	if (listen(listenfd, BACKLOG) == -1) {
		perror("listen() error");
		exit(-1);
	}
	cout << "listen finish" << endl;

	addrlen = sizeof(client);
	while (1) {
		if ((connectfd = accept(listenfd, (struct sockaddr *) &client, &addrlen))
				== -1) {
			perror("accept() error");
			exit(-1);
		}
		arg = (struct ARG*) malloc(sizeof(struct ARG));
		arg->connfd = connectfd;
		memcpy((void*) &arg->client, &client, sizeof(client));

		if (pthread_create(&tid, NULL, start_routine, (void*) arg)) {
			perror("pthread_create() error");
			exit(-1);
		}
	}
	close(listenfd);
}

void process_cli(int connectfd, struct sockaddr_in client) {
	int num, i;
	char recvbuf[MAXDATASIZE], sendbuf[MAXDATASIZE], cli_name[MAXDATASIZE];
	printf("process from %s:%d\n", inet_ntoa(client.sin_addr),
			ntohs(client.sin_port));
	send(connectfd, "Welcome!\n", 9, 0);
	num = recv(connectfd, cli_name, MAXDATASIZE, 0);
	if (num == 0) {
		close(connectfd);
		printf("disconnect!\n");
	} else if (num < 0) {
		close(connectfd);
		printf("connect broken!\n");
		return;
	}
	cli_name[num] = '\0';
	printf("client name: %s,len:%d\n", cli_name, num);
	while (num = recv(connectfd, recvbuf, MAXDATASIZE, 0)) {
		recvbuf[num] = '\0';
		printf("client:%s Msg: %s,len:%d\n", cli_name, recvbuf, num);
		int i = 0;
		for (i = 0; i < num; ++i) {
			sendbuf[i] = recvbuf[num - i - 1];
		}
		sendbuf[num] = '\0';
		send(connectfd, sendbuf, strlen(sendbuf), 0);
	}
	printf("client name: %s disconnect!\n", cli_name);
	close(connectfd);
}

void *start_routine(void *arg) {
	struct ARG *info;
	info = (struct ARG*) arg;
	process_cli(info->connfd, info->client);
	free(arg);
	pthread_exit(NULL);
}
