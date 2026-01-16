/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Lai Wei
 * File Description: The master of smtd
 * Creation Date: 04/2017
 * Modification History:
 */

#ifndef TOOLS_SMTD_MASTER
#define TOOLS_SMTD_MASTER

class Master {
public:
    enum SMTDCommandType {
        SMTDCT_RequestID,
        SMTDCT_Open,
        SMTDCT_Close,
        SMTDCT_Reopen,

        SMTDCT_Invalid
    };

private:
    std::vector<std::pair<pid_t, int>> IdleSlaveVec;
    std::map<long, std::pair<pid_t, int>> ClientSlaveMap;

    /// the MessageQueue for Command
    MessageQueue* CommandMSQ;

    /// the MessageQueue for Communicate
    MessageQueue* CommunicateMSQ;

    /// buffer that can be reused
    std::string Command, CtrlMsg;

    /// the ID of the client in current session
    long ClientID;

public:
    Master(MessageQueue* CommandMSQ, MessageQueue* CommunicateMSQ);

    /// the listener that get command message and return the type of command
    SMTDCommandType listen();

    /// assign a new client an ID
    void requestID();

    /// to open a Client-Slave link, assign Slave to Client
    int open();

    /// close a Client-Slave link, recycle the resources
    int close();

    /// when Slave down, handle the failure and reopen new link
    int reopen();

    /// Master records which client is associated to this slave
    void bindClientSlave(pid_t SlavePid,int SlaveMSQKey);

    /// wait for a confirmation message from the client
    void waitConfirmation();
};

#endif
