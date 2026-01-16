/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: global smt configurations
 * Creation Date:
 * Modification History:
 */

#ifndef SMT_SMTCONFIGURE_H
#define SMT_SMTCONFIGURE_H

class SMTConfigure {
public:
    static int Timeout;

    static std::string Tactic;

public:
    static void init(int Timeout);
};

#endif
