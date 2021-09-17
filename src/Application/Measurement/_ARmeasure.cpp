/*
 * _ARmeasure.cpp
 *
 *  Created on: July 26, 2021
 *      Author: yankai
 */

#include "_ARmeasure.h"

namespace kai
{

	_ARmeasure::_ARmeasure()
	{
		m_pV = NULL;
		m_pD = NULL;
		m_pN = NULL;
		m_pW = NULL;

		m_bSave = false;

		m_minPoseConfidence = 0.5;
		m_d = 0.0;
		m_bValidDist = false;
		m_bValidPose = false;
		m_vKlaserSpot.init(0.7 / 100.0, -0.2 / 100.0);
		m_vRange.init(0.5, 330.0);
		m_vDorgP.init(0);
		m_vDptW = {0, 0, 0};
		m_vCorgP.init(0);
		m_vAxisIdx.init(0, 1, 2);

		m_vCircleSize.init(10, 20);
		m_crossSize = 20;
		m_drawCol = Scalar(0, 255, 0);
		m_drawMsg = "";
		m_pFt = NULL;
	}

	_ARmeasure::~_ARmeasure()
	{
	}

	bool _ARmeasure::init(void *pKiss)
	{
		IF_F(!this->_StateBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("minPoseConfidence", &m_minPoseConfidence);
		pK->v("vRange", &m_vRange);
		pK->v("vKlaserSpot", &m_vKlaserSpot);
		pK->v("vAxisIdx", &m_vAxisIdx);
		pK->v("vDorgP", &m_vDorgP);
		pK->v("vCorgP", &m_vCorgP);

		pK->v("vCircleSize", &m_vCircleSize);
		pK->v("crossSize", &m_crossSize);

		string n;

		n = "";
		pK->v("_VisionBase", &n);
		m_pV = (_VisionBase *)(pK->getInst(n));
		IF_Fl(!m_pV, n + " not found");

		n = "";
		pK->v("_DistSensorBase", &n);
		m_pD = (_DistSensorBase *)(pK->getInst(n));
		IF_Fl(!m_pD, n + " not found");

		n = "";
		pK->v("_NavBase", &n);
		m_pN = (_NavBase *)(pK->getInst(n));
		IF_Fl(!m_pN, n + " not found");

		n = "";
		pK->v("_WindowCV", &n);
		m_pW = (_WindowCV *)(pK->getInst(n));
		IF_Fl(!m_pW, n + " not found");

		m_pW->setCbBtn("Save", sOnBtnSave, this);

		return true;
	}

	bool _ARmeasure::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	int _ARmeasure::check(void)
	{
		NULL__(m_pV, -1);
		IF__(m_pV->BGR()->bEmpty(), -1);
		NULL__(m_pD, -1);
		NULL__(m_pN, -1);
		NULL__(m_pW, -1);

		return this->_StateBase::check();
	}

	void _ARmeasure::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPSfrom();

			this->_StateBase::update();
			updateSensor();

			m_pT->autoFPSto();
		}
	}

	bool _ARmeasure::updateSensor(void)
	{
		IF_F(check() < 0);

		if (!m_pD->bReady())
		{
			m_bValidDist = false;
		}
		else
		{
			m_d = m_pD->d((int)0);
			m_bValidDist = m_vRange.bInside(m_d);
		}

		if (!m_pN->bReady())
			m_bValidPose = false;
		else
			m_bValidPose = (m_pN->confidence() > m_minPoseConfidence);

		IF_F(!m_bValidDist);
		IF_F(!m_bValidPose);

		m_vDptP = {m_vDorgP.x, m_vDorgP.y + m_d, m_vDorgP.z};

		Matrix4f mTpose = m_pN->mT();
		m_aPose = mTpose;
		m_vDptW = m_aPose * m_vDptP;

		Matrix3f mRRpose = m_pN->mR().transpose();
		Matrix4f mTwc = Matrix4f::Identity();
		mTwc.block(0, 0, 3, 3) = mRRpose;
		Vector3f mRT = {mTpose(0, 3), mTpose(1, 3), mTpose(2, 3)};
		mRT = mRRpose * mRT;
		mTwc(0, 3) = -mRT(0); // - m_vCorgP.x;
		mTwc(1, 3) = -mRT(1); // - m_vCorgP.y;
		mTwc(2, 3) = -mRT(2); // - m_vCorgP.z;
		m_aW2C = mTwc;

		return true;
	}

	bool _ARmeasure::c2scr(const Vector3f &vPc,
						   const cv::Size &vSize,
						   const vFloat2 &vF,
						   const vFloat2 &vC,
						   cv::Point *pvPs)
	{
		NULL_F(pvPs);

		Vector3f vP = Vector3f(
			vPc[m_vAxisIdx.x],
			-vPc[m_vAxisIdx.y],
			vPc[m_vAxisIdx.z]);

		float ovZ = 1.0 / abs(vP.z());
		pvPs->x = vF.x * vP.x() * ovZ + vC.x;
		pvPs->y = vF.y * vP.y() * ovZ + vC.y;

		return (vP.z() > 0.0);
	}

	bool _ARmeasure::bInsideScr(const cv::Size &s, const cv::Point &p)
	{
		IF_F(p.x < 0);
		IF_F(p.x > s.width);
		IF_F(p.y < 0);
		IF_F(p.y > s.height);

		return true;
	}

	bool _ARmeasure::bValid(void)
	{
		return m_bValidDist & m_bValidPose;
	}

	Vector3f _ARmeasure::vDptW(void)
	{
		return m_vDptW;
	}

	Eigen::Affine3f _ARmeasure::aW2C(void)
	{
		return m_aW2C;
	}

	// UI handler
	void _ARmeasure::save(void)
	{
		// save screen shot to USB memory
		m_bSave = true;
	}

	// callbacks
	void _ARmeasure::sOnBtnSave(void *pInst, uint32_t f)
	{
		NULL_(pInst);
		((_ARmeasure *)pInst)->save();
	}

	// UI draw
	void _ARmeasure::drawCross(Mat *pM)
	{
		NULL_(pM);

		vFloat2 vF = m_pV->getFf();
		vFloat2 vC = m_pV->getCf();
		cv::Size s = m_pV->BGR()->size();

		// laser spot
		Vector3f vLSc = {m_vDorgP.x, m_vDorgP.y + m_d, m_vDorgP.z};
		Vector3f vLSl = {m_vDorgP.x + m_d * m_vKlaserSpot.y,
						 m_vDorgP.y + m_d,
						 m_vDorgP.z};
		Vector3f vLSt = {m_vDorgP.x,
						 m_vDorgP.y + m_d,
						 m_vDorgP.z + m_d * m_vKlaserSpot.x};

		Eigen::Affine3f aL2C = m_aW2C * m_aPose;
		vLSc = aL2C * vLSc;
		vLSl = aL2C * vLSl;
		vLSt = aL2C * vLSt;

		cv::Point vPc, vPl, vPt;
		c2scr(vLSc, s, vF, vC, &vPc);
		c2scr(vLSl, s, vF, vC, &vPl);
		c2scr(vLSt, s, vF, vC, &vPt);

		float rd = (m_vRange.y - m_d) / m_vRange.len();
		Scalar col = (m_bValidDist) ? Scalar(0, 255.0 * rd, 255.0 * (1.0 - rd)) : Scalar(0, 0, 255);

		if (m_bValidDist)
		{
			Rect2i r;
			r.x = vPl.x;
			r.y = vPt.y;
			r.width = abs(vPc.x - vPl.x) * 2 + 1;
			r.height = abs(vPc.y - vPt.y) * 2 + 1;
			rectangle(*pM, r, col, 1);
		}

		// target cross
		line(*pM,
			 Point(vPc.x - m_crossSize, vPc.y),
			 Point(vPc.x + m_crossSize, vPc.y),
			 col,
			 1);

		line(*pM,
			 Point(vPc.x, vPc.y - m_crossSize),
			 Point(vPc.x, vPc.y + m_crossSize),
			 col,
			 1);
	}

	void _ARmeasure::drawLidarRead(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		Rect r;
		r.x = 0;
		r.y = 440;
		r.width = 640;
		r.height = 40;
		(*pM)(r) = Scalar(0);

		Scalar col = (m_bValidDist) ? Scalar(255, 255, 255) : Scalar(0, 0, 255);
		string sD = "D = ";
		if (m_bValidDist)
			sD += f2str(m_d, 2) + "m";
		else
			sD += "Err";

		Point pt;
		pt.x = 20;
		pt.y = pM->rows - 45;

		m_pFt->putText(*pM, sD,
					   pt,
					   40,
					   col,
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasure::drawMsg(Mat *pM)
	{
		NULL_(pM);
		NULL_(m_pFt);

		m_drawMsg = "";

		if (m_pN->confidence() < m_minPoseConfidence)
			m_drawMsg = "Tracking lost";

		IF_(m_drawMsg.empty());

		int baseline = 0;
		Size ts = m_pFt->getTextSize(m_drawMsg,
									 40,
									 -1,
									 &baseline);

		Point pt;
		pt.x = constrain(320 - ts.width / 2, 0, pM->cols);
		pt.y = constrain(pM->rows / 2 - ts.height, 0, pM->rows);

		m_pFt->putText(*pM, m_drawMsg,
					   pt,
					   40,
					   Scalar(0, 0, 255),
					   -1,
					   cv::LINE_AA,
					   false);
	}

	void _ARmeasure::cvDraw(void *pWindow)
	{
		NULL_(pWindow);
		this->_ModuleBase::cvDraw(pWindow);
		IF_(check() < 0);

		_WindowCV *pWin = (_WindowCV *)pWindow;
		Frame *pF = pWin->getFrame();
		NULL_(pF);
		Mat *pMw = pF->m();
		IF_(pMw->empty());
		m_pFt = pWin->getFont();

		drawCross(pMw);
		drawLidarRead(pMw);
		drawMsg(pMw);

		if (m_bSave)
		{
			//TODO: save screenshot to USB memory
			m_bSave = false;
		}
	}

	void _ARmeasure::console(void *pConsole)
	{
		NULL_(pConsole);
		this->_ModuleBase::console(pConsole);
	}

}
