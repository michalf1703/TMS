/*
 * PID.h
 *
 *  Created on: 27 mar 2022
 *      Author: Czarek
 */

#ifndef SRC_PID_H_
#define SRC_PID_H_
#include "UAR.h"
class PID:public UAR {
	float m_Tp, m_Kp, m_Ki, m_Kd;
	float m_sum, m_Smin, m_Smax;
	float m_prevIn;
public:
	PID(float Kp, float Ti, float Td,float Tp,float Smin,float Smax);
	virtual ~PID();
	Calculate();
};

#endif /* SRC_PID_H_ */
