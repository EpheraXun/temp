/*
 * Copyright (C), 2016-2017, Sourcebrella, Inc Ltd - All rights reserved.
 * Unauthorized copying, using, modifying of this file, via any medium is strictly prohibited.
 * Proprietary and confidential.
 *
 * Author: Qingkai Shi
 * File Description: SMT object, the base class of other smt classes
 * Creation Date:
 * Modification History:
 */

#ifndef SMT_SMTOBJECT_H
#define SMT_SMTOBJECT_H

class SMTFactory;

class SMTObject {
protected:
	SMTFactory* Factory;

	SMTObject(SMTFactory* F) : Factory(F) {
	}

	SMTObject(const SMTObject& Obj) : Factory(Obj.Factory) {
	}

	SMTObject& operator=(const SMTObject& Obj) {
		if (this != &Obj) {
			this->Factory = Obj.Factory;
		}
		return *this;
	}

public:
	virtual ~SMTObject() = 0;

	SMTFactory& getSMTFactory() const {
		return *Factory;
	}
};

#endif
