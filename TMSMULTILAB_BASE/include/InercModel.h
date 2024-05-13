/*
 * InercModel.h
 *
 *  Created on: 27 mar 2022
 *      Author: Czarek
 */

#ifndef SRC_INERCMODEL_H_
#define SRC_INERCMODEL_H_
#include "UAR.h"
class InercModel:public UAR {
	float m_pole;
public:
	InercModel(float pole);
	virtual ~InercModel();
	Calculate();
};

#endif /* SRC_INERCMODEL_H_ */
