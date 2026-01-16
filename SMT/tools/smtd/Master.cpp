/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: The master of smtd
 * Creation Date: 04/2017
 * Modification History:
 *  Lai Wei: make it as an independent class
 */

#include <llvm/Support/Debug.h>
#include <llvm/Support/ErrorHandling.h>

#include <sys/wait.h> // waitpid
#include <signal.h> // kill
#include <error.h>
#include <unistd.h> // fork

#include <map>
#include <set>

#include "Support/MessageQueue.h"
#include "UserIDAllocator.h"
#include "SMT/SMTFactory.h"
#include "Master.h"

#define DEBUG_TYPE "smtd-master"

using namespace llvm;

Master::Master(MessageQueue* CommandMSQ, MessageQueue* CommunicateMSQ) {
    this->ClientID = -1;
    this->CommandMSQ = CommandMSQ;
    this->CommunicateMSQ = CommunicateMSQ;
}

void Master::requestID() {
    UserIDAllocator* IDAllocator = UserIDAllocator::getUserAllocator();
    ClientID = IDAllocator->allocate();
    CommunicateMSQ->sendMessage(std::to_string(ClientID), 12);
}

int Master::open() {
    auto It = ClientSlaveMap.find(ClientID);
    if (It == ClientSlaveMap.end()) {
        if (!IdleSlaveVec.empty()) {
            // try to reuse
            auto& Slave = IdleSlaveVec.back();
            IdleSlaveVec.pop_back();
            bindClientSlave(Slave.first, Slave.second);
            return 0;
        }
    } else {
        throw std::runtime_error("[Master] Existent user requests open!");
    }
    return 1;
}

int Master::close() {
    auto It = ClientSlaveMap.find(ClientID);
    if (It != ClientSlaveMap.end()) {
        ClientSlaveMap.erase(It);
        IdleSlaveVec.push_back(It->second);

        UserIDAllocator* IDAllocator = UserIDAllocator::getUserAllocator();
        IDAllocator->recycle(ClientID);
        return 0;
    } else {
        throw std::runtime_error("[Master] Non-existent user requests close!");
    }
    return 1;
}

int Master::reopen() {
    // reopen means the client detects the exception of
    // the slave, and request a new one.
    auto It = ClientSlaveMap.find(ClientID);
    if (It != ClientSlaveMap.end()) {
        auto ChildPID = It->second.first;
        ClientSlaveMap.erase(It);
        kill(ChildPID, SIGINT);
        waitpid(ChildPID, 0, 0);
        // try reuse
        if (!IdleSlaveVec.empty()) {
            auto& Slave = IdleSlaveVec.back();
            IdleSlaveVec.pop_back();
            bindClientSlave(Slave.first, Slave.second);
            return 0;
        }
    } else {
        throw std::runtime_error("[Master] Non-existent user requests reopen!");
    }
    return 1;
}

Master::SMTDCommandType Master::listen() {
    if (-1 == CommandMSQ->recvMessage(Command)) {
        llvm_unreachable("[Master] fail to recv command");
    }
    ClientID = -1;
    try {
        if (Command == "requestid") {
            return SMTDCT_RequestID;
        }

        size_t M = Command.find_first_of(':');
        if (M == 0 || M == std::string::npos || M + 1 == Command.length()) {
            throw std::runtime_error(": is not found or the last character!\n");
        }

        // request message: "user_id:request_content";
        ClientID = std::stol(Command.substr(0, M));
        if (ClientID < 100) {
            throw std::runtime_error("[Master] UserID cannot be less than 100!");
        }

        std::string ClientRequest = Command.substr(M + 1);

        if (ClientRequest == "open") {
            return SMTDCT_Open;
        } else if (ClientRequest == "close") {
            return SMTDCT_Close;
        } else if (ClientRequest == "reopen") {
            return SMTDCT_Reopen;
        } else {
            throw std::runtime_error("[Master] Unknown request: " + ClientRequest);
        }
    } catch (std::exception& Ex) {
        errs() << "[Master] Invalid request message: " << Command << "\n";
        errs() << Ex.what() << "\n";
        abort();
    }

    return SMTDCT_Invalid;
}

void Master::bindClientSlave(pid_t SlavePid, int SlaveMSQKey) {
    if (CommunicateMSQ->sendMessage(std::to_string(SlaveMSQKey), ClientID) == -1) {
        throw std::runtime_error("[Master] Fail to send slave id to user after (re)open without reuse!");
    }
    ClientSlaveMap[ClientID] = {SlavePid, SlaveMSQKey};

    // Here we need one guarantee:
    // client has got the sent msg
    if (-1 == CommunicateMSQ->recvMessage(CtrlMsg, 11)) {
        llvm_unreachable("[Master] Fail to receive confirmation!");
    }
}
