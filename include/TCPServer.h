#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H

#include <arpa/inet.h>
#include <sys/time.h>

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

class TCPServer {
    int listening_sock;
    int port;
    TCPSession** sessions;
    TCPSession* AddSession(sockaddr_in* addr, int fd);
    void DeleteSession(TCPSession**);
    void ProcessSession(TCPSession*& s_ptr, fd_set* readfds, fd_set* writefds);
    int AcceptConnection(sockaddr_in* cl_addr);
protected:
    virtual TCPSession* NewConnection();
    TCPSession* ConnectToRemoteHost(char* host);
public:
    ReadWriteDescriptors fdsets;

    TCPServer(int _port);
    virtual ~TCPServer();

    virtual void Init();
    void Run(fd_set* readfds, fd_set* writefds);
    virtual void HandleEvent() {};
};

#endif