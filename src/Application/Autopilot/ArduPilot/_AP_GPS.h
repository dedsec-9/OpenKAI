#ifndef OpenKAI_src_Autopilot_AP__AP_GPS_H_
#define OpenKAI_src_Autopilot_AP__AP_GPS_H_

#include "../../../Filter/Median.h"
#include "../../../Navigation/Coordinate.h"
#include "../../../SLAM/_SlamBase.h"
#include "../ArduPilot/_AP_base.h"

namespace kai
{

class _AP_GPS: public _AutopilotBase
{
public:
	_AP_GPS();
	~_AP_GPS();

	bool init(void* pKiss);
	bool start(void);
	int check(void);
	void update(void);
	void draw(void);

	bool reset(void);

protected:
	void updateGPS(void);
	static void* getUpdateThread(void* This)
	{
		((_AP_GPS *) This)->update();
		return NULL;
	}

public:
	_AP_base* m_pAP;
	_SlamBase* m_pSB;
	Coordinate m_GPS;

	STATE_CHANGE m_scApArm;
	double	m_yaw;
	bool	m_bYaw;

	vInt3 m_vAxisIdx;
	vFloat3 m_vAxisK;

	LL_POS m_llPos;
	UTM_POS m_utmPos;
	LL_POS m_llOrigin;
	UTM_POS m_utmOrigin;
	bool	m_bUseApOrigin;

	mavlink_gps_input_t m_D;

};

}
#endif
