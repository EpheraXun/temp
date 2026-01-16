/* Copyright (C), 2016-2018, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 * Author: Yongchao WANG, wangyongchao@sbrella.com
 * File Description: This file describes the TCP client class
 * Creation Date: 2018.5.29
 * Modification History:
 */
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#ifdef __linux__
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>

#define MAX_RECEIVE_SIZE 4096

using namespace std;

class TCPClient {
private:
    int Sock;
    string Address; // IP address, usually is 127.0.0.1
    int Port; // listening port, provided by AI server(PyAID)
    struct sockaddr_in Server; // client server, used for communication

public:
    TCPClient();
    // setup tcp config
    bool Setup(string Address, int Port);
    // sent message
    bool Send(string Data);
    // receive message
    string Receive(int Size = MAX_RECEIVE_SIZE);
    //destroy
    void Exit();
};
#endif // __linux__
#endif // TCP_CLIENT_H