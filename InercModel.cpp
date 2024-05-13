#include "InercModel.h"

InercModel::InercModel(float pole) {
m_pole = pole;
m_in = 0;
m_out = 0;
}

InercModel::~InercModel() {
	// TODO Auto-generated destructor stub
}

InercModel::Calculate()
{
m_out = m_out * m_pole + (1 - m_pole) * m_in;
return 0;
}
