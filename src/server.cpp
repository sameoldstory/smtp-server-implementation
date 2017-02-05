#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>
#include "exceptions.h"
#include "server.h"
#include "SMTPsession.h"
#include "string.h"

#define DEFAULT_BACKLOG 5
#define MAX_SESSIONS 4

//return values are just like in read system call

short int Client::ProcessReadOperation()
{
	int portion = read(fd, &(buf[0]), BUF_SIZE_SERV);
	if (portion == -1) {
		printf("Reading error: client %d\n", fd);
		return -1;
	}
	if (portion == 0) {
		printf("Session ended: client %d\n", fd);
		smtp.EndSession();
		return 0;
	}
	buf[portion] = '\0';
	puts(buf);
	if (smtp.HandleInput(portion, &(buf[0])))
		need_to_write = true;
	return 1; 
}

short int Client::ProcessWriteOperation()
{
	char* message = strdup(smtp.GetMessage());
	puts(message);
	return write(fd, message, strlen(message));
}

Server::Server(char* conf_path): listening_sock(-1), port(-1),
	clients_array(NULL), config(conf_path)
{
	clients_array = new Client*[MAX_SESSIONS];
	for (int i = 0; i < MAX_SESSIONS; i++)
		clients_array[i] = NULL;
}

void Server::CreateListeningSocket()
{
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	listening_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_sock == -1) {
		perror("socket");
		exit(1);
	}
	int opt = 1;
	setsockopt(listening_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(listening_sock, (sockaddr*) &address, sizeof(address))) {
		perror("bind");
		exit(1);
	}
	if (listen(listening_sock, DEFAULT_BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}
}

void Server::AddClient()
{	
	// do i need info about client's address?
	// if i do, addr in following line should not be NULL
	sockaddr_in* cl_addr = NULL;
	socklen_t cl_addrlen = INET_ADDRSTRLEN;
	int cl_fd = accept(listening_sock, (sockaddr*) cl_addr, &cl_addrlen);
	if (cl_fd == -1) {
		perror("accept");
		// some error handling is needed here
		return;
	}
	int i;
	for (i = 0; clients_array[i]; i++)
		{}
	if (i == MAX_SESSIONS) {
		// handle situation when number of sessions is exceeded
	} else {
		printf("Created new client, index in array equals to: %d\n", i);
		clients_array[i] = new Client(cl_fd, cl_addr, BUF_SIZE_SERV, &config);
		//initialize client
	}
}

void Server::DeleteClient(int fd)
{	
	puts("Server::DeleteClient\n");
	for (int i = 0; i < MAX_SESSIONS; i++) {
		if (fd == clients_array[i]->GetSocketDesc()) {
			printf("Trying to delete client with index: %d\n", i);
			puts("before delete\n");
			delete clients_array[i];
			puts("after delete\n");
			clients_array[i] = NULL;
			if (shutdown(fd, 2) == -1)
				perror("shutdown");
			close(fd);
			return;
		}
	}
}

//mainloop to be optimized later
//loop initializing sets of descriptors is ubiqitous

void Server::MainLoop()
{
	for(;;) {
		fd_set readfds, writefds;
		int max_d = listening_sock;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(listening_sock, &readfds);
		int fd;
		for (int i = 0; i < MAX_SESSIONS; i++) {
			if (clients_array[i]) {
				fd = clients_array[i]->GetSocketDesc();
				FD_SET(fd, &readfds);
				if (clients_array[i]->NeedsToWrite())
					FD_SET(fd, &writefds);
				if (fd + 1 > max_d)
					max_d = fd + 1;
			}
		}
		printf("before select\n");
		int res = select(max_d+1, &readfds, &writefds, NULL, NULL);
		printf("after select\n");
		if (res < 1) {
			perror("select");
			exit(1);
		}
		if (FD_ISSET(listening_sock, &readfds))
			AddClient();
		for (int i = 0; i < MAX_SESSIONS && clients_array[i]; i++) {
			fd = clients_array[i]->GetSocketDesc();
			printf("%d that is descriptor of the client\n",fd);
			// if we need to write something, that means we do not read from socket
			if (FD_ISSET(fd, &writefds)) {
				printf("Socket is ready for writing\n");
				if(clients_array[i]->ProcessWriteOperation() == -1)
					perror("write");
				else {
					clients_array[i]->FulfillNeedToWrite();
					puts("Fulfilled need to write");
				}
				if (clients_array[i]->NeedsToBeClosed()) {
					puts("Need to close connection");
					DeleteClient(clients_array[i]->GetSocketDesc());
				}
			}
			if (FD_ISSET(fd, &readfds)) {
				printf("Socket is ready for reading\n");
				int res = clients_array[i]->ProcessReadOperation();
				if (res == 0) 
					DeleteClient(fd);
				else if (res == -1)
					perror("read");
			}
		}
	}
}

void Server::ConfigureServer()
{
	if (!config.GetConfigPath()) {
		printf("Server can't be launched: specify path for configuration file with -c key\n");
		exit(1);
	}
	if (!config.OpenConfig()) {
		printf("Config file can not be opened\n");
		exit(1);
	}
	config.ExtractInfoFromConfig();
	config.CloseConfig();
}

void Server::Run()
{
	try {
		ConfigureServer();
		//config.PrintEverything();
		port = config.GetPort();
		CreateListeningSocket();
		MainLoop();

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
	for (int i = 0; i < MAX_SESSIONS; i++) {
		if (clients_array[i])
			delete clients_array[i];
	}
	delete[] clients_array;
}

Server::~Server()
{
	EmptyAllocatedMemory();
	if (listening_sock != -1) {
		shutdown(listening_sock, 2);
		close(listening_sock);
	}
}