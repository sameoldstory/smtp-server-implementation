#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <arpa/inet.h>
#include <sys/time.h>
#include "queueManager.h"

class TCPSession;

struct ReadWriteDescriptors {
    int max_fd;
    fd_set readfds;
    fd_set writefds;
    ReadWriteDescriptors();
    void AddListeningSock(int sock);
    void AddSessionSock(int sock);
    void DeleteSessionSock(int sock);
    void ClearWritefds(int sock);
    void SetWritefds(int sock);
};

class Server {
    int listening_sock;
    int port;
    TCPSession** sessions;
protected:
    TCPSession* AddSession(sockaddr_in* addr, int fd);
    void DeleteSession(TCPSession**);
    void ProcessSession(TCPSession*& s_ptr, fd_set* readfds, fd_set* writefds);
    int AcceptConnection(sockaddr_in* cl_addr);
    virtual TCPSession* NewConnection();
public:
    ReadWriteDescriptors fdsets;
    virtual void Init();
    void Run(fd_set* readfds, fd_set* writefds);
    Server(int _port);
    virtual ~Server();
};

#endif