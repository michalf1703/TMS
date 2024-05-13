/*
 * UAR.h
 *
 *  Created on: 27 mar 2022
 *      Author: Czarek
 */

#ifndef SRC_UAR_H_
#define SRC_UAR_H_

class UAR {
public:
	float m_out;
	float m_in;
	UAR();
	virtual ~UAR();
	void setInput(float input){m_in=input;}
	void Calculate(){m_out=m_in;};
	float getOutput() {return m_out;}
};

#endif /* SRC_UAR_H_ */
