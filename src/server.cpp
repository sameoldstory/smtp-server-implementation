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
#define MAX_SESSIONS 8

ReadyIndicators::ReadyIndicators()
{
	max_fd = 0;
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
}

void ReadyIndicators::AddListeningSock(int sock)
{
	FD_SET(sock, &readfds);
	max_fd = sock;
}

void ReadyIndicators::AddClientSock(int sock)
{
	FD_SET(sock, &readfds);
	FD_SET(sock, &writefds);
	if (sock > max_fd)
		max_fd = sock;
}

void ReadyIndicators::DeleteClientSock(int sock)
{
	FD_CLR(sock, &readfds);
	FD_CLR(sock, &writefds);
}

void ReadyIndicators::ClearWritefds(int sock)
{
	FD_CLR(sock, &writefds);
}

void ReadyIndicators::SetWritefds(int sock)
{
	FD_SET(sock, &writefds);
}

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
	clients_array(NULL), config(conf_path), fdsets()
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
	sockaddr_in* cl_addr = NULL;
	socklen_t cl_addrlen = INET_ADDRSTRLEN;
	int cl_fd = accept(listening_sock, (sockaddr*) cl_addr, &cl_addrlen);
	if (cl_fd == -1) {
		perror("accept");
		// some error handling is needed here
		return;
	}
	int i;
	for (i = 0; clients_array[i]; i++) {

	}
	if (i == MAX_SESSIONS) {
		// handle situation when number of sessions is exceeded
	} else {
		clients_array[i] = new Client(cl_fd, cl_addr, BUF_SIZE_SERV, &config);
		fdsets.AddClientSock(cl_fd);
		puts("Client added");
	}
}

void Server::DeleteClient(Client** client_ptr)
{
	int fd = (*client_ptr)->GetSocketDesc();
	delete *client_ptr;
	*client_ptr = NULL;
	shutdown(fd, SHUT_RDWR);
	close(fd);
	fdsets.DeleteClientSock(fd);
	puts("Client deleted");
}

void Server::PrepareSetsForSelect(fd_set* read, fd_set* write)
{
	memcpy(read, &(fdsets.readfds), sizeof(fd_set));
	memcpy(write, &(fdsets.writefds), sizeof(fd_set));
}

void Server::MainLoop()
{
	fd_set readfds, writefds;

	for(;;) {

		PrepareSetsForSelect(&readfds, &writefds);

		int res = select(fdsets.max_fd + 1, &readfds, &writefds, NULL, NULL);
		if (res < 1) {
			perror("select");
			return;
		}

		if (FD_ISSET(listening_sock, &readfds))
			AddClient();

		for (int i = 0; i < MAX_SESSIONS; i++) {
			if (!clients_array[i])
				continue;
			int fd = clients_array[i]->GetSocketDesc();

			if (clients_array[i]->NeedsToWrite() && FD_ISSET(fd, &writefds)) {
				clients_array[i]->ProcessWriteOperation();
				fdsets.ClearWritefds(fd);
				if (clients_array[i]->NeedsToBeClosed())
					DeleteClient(&(clients_array[i]));
			}

			if (clients_array[i] && FD_ISSET(fd, &readfds)) {
				clients_array[i]->ProcessReadOperation();
				fdsets.SetWritefds(fd);
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
		fdsets.AddListeningSock(listening_sock);
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
	for (int i = 0; i < MAX_SESSIONS; i++)
		delete clients_array[i];
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