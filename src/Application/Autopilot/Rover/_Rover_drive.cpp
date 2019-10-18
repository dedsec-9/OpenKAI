#include "_Rover_drive.h"

namespace kai
{

_Rover_drive::_Rover_drive()
{
	m_pAB = NULL;
	m_pCMD = NULL;
	m_pPID = NULL;

	m_ctrl.init();
}

_Rover_drive::~_Rover_drive()
{
}

bool _Rover_drive::init(void* pKiss)
{
	IF_F(!this->_AutopilotBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	Kiss** ppD = pK->getChildItr();
	int i = 0;
	while (ppD[i])
	{
		Kiss* pD = ppD[i++];

		ROVER_DRIVE d;
		d.init();
		pD->v("kDir",&d.m_kDir);
		pD->v("sDir",&d.m_sDir);
		pD->v("pwmH",&d.m_pwmH);
		pD->v("pwmM",&d.m_pwmM);
		pD->v("pwmL",&d.m_pwmL);

		m_vDrive.push_back(d);
	}

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_RoverCMD", &iName));
	m_pCMD = (_Rover_CMD*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pCMD, iName+": not found");

	iName = "";
	pK->v("PID", &iName);
	m_pPID = (PIDctrl*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pPID, iName+": not found");

	return true;
}

int _Rover_drive::check(void)
{
	NULL__(m_pAB, -1);
	NULL__(m_pCMD, -1);
	NULL__(m_pPID, -1);

	return 0;
}

void _Rover_drive::update(void)
{
	this->_AutopilotBase::update();
	IF_(check()<0);
	IF_(!bActive());

	m_ctrl = *((ROVER_CTRL*)m_pAB->m_pCtrl);

}

void _Rover_drive::updatePWM(void)
{
	IF_(m_ctrl.m_hdg < 0.0);
	IF_(m_ctrl.m_targetHdg < 0.0);

	float dSpeed = m_pPID->update(0.0, dHdg(m_ctrl.m_hdg, m_ctrl.m_targetHdg), m_tStamp);

	vector<uint16_t> vPWM;
	for(int i=0; i<m_vDrive.size(); i++)
		vPWM.push_back(m_vDrive[i].updatePWM(m_ctrl.m_nSpeed, dSpeed));

	m_pCMD->setPWM(vPWM);
}

void _Rover_drive::draw(void)
{
	this->_AutopilotBase::draw();

}

}
