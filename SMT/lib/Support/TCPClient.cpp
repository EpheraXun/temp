/* Copyright (C), 2016-2018, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 * Author: Yongchao WANG, wangyongchao@sbrella.com
 * File Description: This file implements TCPClient class.
 * Creation Date: 2018.5.29
 * Modification History:
 */
#ifdef __linux__
#include "Support/TCPClient.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

TCPClient::TCPClient(){
	Sock = -1;
	Port = 0;
	Address = "";
}

bool TCPClient::Setup(std::string Address, int Port){
    if(Sock == -1){
        Sock = socket(AF_INET, SOCK_STREAM, 0);
        if(Sock == -1){
            errs() << "Could not create socket\n";
        }
    }

    if(inet_addr(Address.c_str()) == -1){
        struct hostent *he;
        struct in_addr **addr_list;
        if((he = gethostbyname(Address.c_str())) == NULL){
            herror("gethostbyname");
            errs() << "Failed to resolve hostname\n";
            return false;
        }
        addr_list = (struct in_addr **) he->h_addr_list;
		
        for(int i = 0; addr_list[i] != NULL; i++){
            Server.sin_addr = *addr_list[i];
            break;
        }
    }
    else{
        Server.sin_addr.s_addr = inet_addr(Address.c_str());
    }

    Server.sin_family = AF_INET;
    Server.sin_port = htons(Port);

    if(connect(Sock, (struct sockaddr*)&Server, sizeof(Server)) < 0){
        perror("Connect failed. Error");
        return false;
    }
    return true;
}

bool TCPClient::Send(string Data){
    if(Sock != -1){
        if(send(Sock, Data.c_str(), strlen(Data.c_str()), 0) < 0){
            errs() << "Send failed : " << Data << "\n";
            return false;
        }
    }
    else{
        return false;
    }
    return true;
}

std::string TCPClient::Receive(int Size){
    char Buffer[Size];
    memset(&Buffer[0], 0, sizeof(Buffer));

    std::string Reply;

    if(recv(Sock, Buffer, Size, 0) < 0){
        errs() << "Receive failed!\n";
        return nullptr;
    }

    Buffer[Size - 1] = '\0';
    Reply = Buffer;
    return Reply;
}

void TCPClient::Exit(){
    close(Sock);
}

#endif // __linux__