/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Lai Wei
 * File Description: The slave of smtd
 * Creation Date: 04/2017
 * Modification History:
 */

#ifndef TOOLS_SMTD_SLAVE
#define TOOLS_SMTD_SLAVE

class Slave {
private:
    /// the process id of this slave
    pid_t SlavePid;

    /// the message queue of this slave
    MessageQueue* SlaveMSQ;

    /// using incremental solving
    bool Incremental;

public:
    Slave(pid_t Pid, MessageQueue* MSQ, bool Incremental):SlavePid(Pid), SlaveMSQ(MSQ), Incremental(Incremental) {}

    /// all jobs are done in the function
    void listen();
};
#endif
