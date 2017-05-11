#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "exceptions.h"
#include "server.h"
#include "TCPSession.h"
#include <netdb.h>
#include <fcntl.h>
#include "timeval.h"
// TODO: get rid of this include later, when queueManager class is ready
#include "SMTPServerSession.h"

class SMTPServerSession;

#define MAX_SESSIONS 16
#define DEFAULT_BACKLOG 5

ReadWriteDescriptors::ReadWriteDescriptors()
{
    max_fd = 0;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
}

void ReadWriteDescriptors::AddListeningSock(int sock)
{
    FD_SET(sock, &readfds);
    max_fd = sock;
}

void ReadWriteDescriptors::AddSessionSock(int sock)
{
    FD_SET(sock, &readfds);
    FD_SET(sock, &writefds);
    if (sock > max_fd)
        max_fd = sock;
}

void ReadWriteDescriptors::DeleteSessionSock(int sock)
{
    FD_CLR(sock, &readfds);
    FD_CLR(sock, &writefds);
}

void ReadWriteDescriptors::ClearWritefds(int sock)
{
    FD_CLR(sock, &writefds);
}

void ReadWriteDescriptors::SetWritefds(int sock)
{
    FD_SET(sock, &writefds);
}


Server::Server(int _port):
    listening_sock(-1), port(_port), sessions(NULL), fdsets()
{
    sessions = new TCPSession*[MAX_SESSIONS];
    for (int i = 0; i < MAX_SESSIONS; i++)
        sessions[i] = NULL;
}

TCPSession* Server::NewConnection()
{
    sockaddr_in addr;
    int fd = AcceptConnection(&addr);
    TCPSession* s = AddSession(&addr, fd);
    return s;
}

void Server::Run(fd_set* readfds, fd_set* writefds)
{
    if (FD_ISSET(listening_sock, readfds)) {
        NewConnection();
    }

    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i]) {
            printf("process session with sock %d\n", sessions[i]->GetSocketDesc());
            ProcessSession(sessions[i], readfds, writefds);
        }
    }
}

void Server::Init()
{
    sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

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

    fdsets.AddListeningSock(listening_sock);
}


int Server::AcceptConnection(sockaddr_in* addr)
{
    socklen_t size = INET_ADDRSTRLEN;
    int fd = accept(listening_sock, (sockaddr*) addr, &size);
    if (fd == -1)
        throw "Accept Failed";
    return fd;
}

TCPSession* Server::AddSession(sockaddr_in* addr, int fd)
{
    int i;
    for (i = 0; sessions[i]; i++) {

    }
    if (i == MAX_SESSIONS) {
// TODO: handle situation when number of sessions is exceeded
    } else {
        sessions[i] = new TCPSession(fd, *addr);
        fdsets.AddSessionSock(fd);
    }
    return sessions[i];
}

void Server::DeleteSession(TCPSession** ptr)
{
    int fd = (*ptr)->GetSocketDesc();
    delete *ptr;
    *ptr = NULL;
    fdsets.DeleteSessionSock(fd);
}

void Server::ProcessSession(TCPSession* & s_ptr, fd_set*readfds, fd_set* writefds)
{
    int fd = s_ptr->GetSocketDesc();

    if (s_ptr->NeedsToWrite() && FD_ISSET(fd, writefds)) {
        printf("Process Write %d\n", s_ptr->GetSocketDesc());
        s_ptr->ProcessWriteOperation();
        if (!s_ptr->NeedsToWrite()) {
            fdsets.ClearWritefds(fd);
        }
        if (s_ptr->NeedsToBeClosed()) {
// TODO: get rid of this block of code when class queueManager is ready
// here I need to check what kind of driver is hidden inside TcpSession
            SMTPServerSession* tmp_ptr =
                dynamic_cast<SMTPServerSession*>(s_ptr->GetSessionDriverPtr());
            if (tmp_ptr) {
// TODO: here I should create SMTPClientSession

            #ifdef CLIENT_SMTP_SESSION_TEST
                int port = 25;
                char buf[1024];
                const char* host = "alt4.aspmx.l.google.com";

                const char* filename = tmp_ptr->GetFilename();
                sprintf(buf, "../mail_queue/%s.env", filename);
                int env = open(buf, O_RDONLY);
                sprintf(buf, "../mail_queue/%s.msg", filename);
                int msg = open(buf, O_RDONLY);

                char sender[64], rcpt[64], *left, *right;
                int n = read(env, buf, sizeof(buf)-1);
                buf[n] = '\0';
                left = strstr(buf, " ");
                left += 1;
                left = strstr(left, " ");
                left += 1;
                right = strstr(left, " ");
                strncpy(sender, left, right-left);
                sender[right-left] = '\0';
                left = ++right;
                strcpy(rcpt, left);
                int len = strlen(rcpt);
                rcpt[len-1] ='\0';
                close(env);

                // ConnectToHost

                int sock = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in addr;
                struct hostent* host_ptr = gethostbyname(host);
                addr.sin_family = AF_INET;
                addr.sin_port = htons(port);
                addr.sin_addr.s_addr = * (int32_t*) host_ptr->h_addr_list[0];
                connect(sock, (struct sockaddr*)&addr, sizeof(addr));

                TCPSession* s = AddSession(&addr, sock);
                char ehlo[] = "ceres.intelib.org";
                s->ServeAsSMTPClientSession(ehlo, sender, rcpt, msg);
                fdsets.ClearWritefds(sock);
            #endif

            }
            DeleteSession(&s_ptr);
        }
    }

    if (s_ptr && FD_ISSET(fd, readfds)) {
        printf("Process Read %d\n", s_ptr->GetSocketDesc());
        int res = s_ptr->ProcessReadOperation();
        if (s_ptr->NeedsToWrite()) {
            fdsets.SetWritefds(fd);
        } else {
            fdsets.ClearWritefds(fd);
        }
        if (res == 0) {
            DeleteSession(&s_ptr);
            return;
        }
    }

}

Server::~Server()
{
    if (listening_sock != -1) {
        shutdown(listening_sock, 2);
        close(listening_sock);
    }
    for (int i = 0; i < MAX_SESSIONS; i++)
        delete sessions[i];
    delete[] sessions;
}




