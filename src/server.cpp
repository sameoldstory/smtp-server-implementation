#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include "exceptions.h"
#include "server.h"
#include "SMTPsession.h"
#include "string.h"

#define DEFAULT_BACKLOG 5

Server::Server(int port_, char* conf_addr): listening_sock(-1), port(port_),
	clients(NULL), config(conf_addr)
{
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;
}

void Server::CreateListeningSocket()
{
	listening_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_sock == -1) {
		perror("socket");
		throw FatalException();
	}
	int opt = 1;
	setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(listening_sock, (sockaddr*) &address, sizeof(address))) {
		perror("bind");
		throw FatalException();
	}
	if (listen(listening_sock, DEFAULT_BACKLOG) == -1) {
		perror("listen");
		throw FatalException();
	}
}

void Server::AddClient(int new_fd, sockaddr_in* addr)
{
	if (!clients) {
		clients = new SMTPsessionList(new_fd, addr);
	} else {
		SMTPsessionList* tmp = new SMTPsessionList(new_fd, addr, clients);
		clients = tmp;
	}
	clients->session.Start();
}

void Server::DeleteClient(int fd)
{
	SMTPsessionList* tmp, *prev = NULL;
	for (tmp = clients; tmp; tmp = tmp->next) {
		if (fd == tmp->session.GetSocketDesc()) {
			if (prev == NULL)
				clients = tmp->next;
			else
				prev->next = tmp->next;
			delete tmp;
			if (shutdown(fd, 2) == -1)
				perror("shutdown");
			close(fd);
			return;
		}
		prev = tmp;
	}
}

void Server::ListeningModeOn()
{
	for(;;) {
		fd_set readfds;
		int max_d = listening_sock;
		FD_ZERO(&readfds);
		FD_SET(listening_sock, &readfds);
		SMTPsessionList* tmp;
		int fd;
		for (tmp = clients; tmp; tmp = tmp->next) {
			fd = tmp->session.GetSocketDesc();
			FD_SET(fd, &readfds);
			if (fd > max_d)
				max_d = fd;
		}
		int res = select(max_d+1, &readfds, NULL, NULL, NULL);
		if (res < 1) {
			perror("select");
			throw FatalException();
		}
		// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		if (FD_ISSET(listening_sock, &readfds)) {
			sockaddr_in* cl_addr = new sockaddr_in;
			socklen_t cl_addrlen = INET_ADDRSTRLEN;
			int new_fd = accept(listening_sock, (sockaddr*) cl_addr, &cl_addrlen);
			if (new_fd == -1) {
				perror("accept");
				// some error handling is needed here
			} else {
				AddClient(new_fd, cl_addr);
			}
		}
		for (tmp = clients; tmp; tmp = tmp->next) {
			fd = tmp->session.GetSocketDesc();
			if (FD_ISSET(fd, &readfds)) {
				if (false == tmp->session.Resume()) {
					// disconnection; get rid of the client
					DeleteClient(fd);
				}
			}
		}
	}
}

void Server::ConfigureServer()
{
	if (!config.GetConfigPath()) {
		printf("Server can't be launched: specify path for configuration file with -c key\n");
		throw FatalException();
	}
	if (!config.OpenConfig())
		throw FatalException();
	if (!config.InitializeBuffer())
		throw FatalException();
	config.ExtractInfoFromConfig();
	config.CloseConfig();
	config.PrintMailboxes();
}

void Server::Run()
{
	try {
		ConfigureServer();
		CreateListeningSocket();
		ListeningModeOn();

	} catch(const char* s) {
		printf("%s\n", s);
	}
	catch(FatalException e) {
		printf("Fatal exception caught\n");
	}
	catch(...){
		printf("Unknown error occured\n");
	}
}

void Server::EmptyAllocatedMemory()
{
	SMTPsessionList* tmp;
	while(clients) {
		tmp = clients->next;
		delete clients;
		clients = tmp;
	}
}

Server::~Server()
{
	EmptyAllocatedMemory();
	if (listening_sock != -1) {
		shutdown(listening_sock, 2);
		close(listening_sock);
	}
}