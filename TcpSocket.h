
#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <string>
#include <netinet/ip.h>

/*
 * 封装socket的基本操作
 */

class TcpSocket {
public:
    explicit TcpSocket(int _maxWaiter = 5);

    ~TcpSocket();
    //bind() in socket
    bool bindPort(int _port);
    //set() 设置serv_addr
    bool setServerInfo(const std::string &ip, int _port);
    //listen() in socket
    bool listenOn();
    //connect() in socket
    bool connectToHost();
    
    unsigned int getPort() const;

    inline unsigned int getSockFD() const { return static_cast<unsigned int>(m_sockfd); }

private:
    int m_sockfd{-1};
    struct sockaddr_in serv_addr;  
    int m_maxWaiter{5};

    
};


#endif 
