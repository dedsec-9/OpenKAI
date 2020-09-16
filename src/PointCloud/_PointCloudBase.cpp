/*
 * _PointCloudBase.cpp
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#include "_PointCloudBase.h"

#ifdef USE_OPEN3D

namespace kai
{

_PointCloudBase::_PointCloudBase()
{
	m_bOpen = false;
	m_pViewer = NULL;

	m_vT.init(0);
	m_vR.init(0);
	m_vRz.init(0.0, FLT_MAX);
}

_PointCloudBase::~_PointCloudBase()
{
}

bool _PointCloudBase::init(void *pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("vT", &m_vT);
	pK->v("vR", &m_vR);
	pK->v("vRz", &m_vRz);

	int nPCreserve = 0;
	pK->v("nPCreserve", &nPCreserve);
	if(nPCreserve > 0)
	{
		m_sPC.prev()->points_.reserve(nPCreserve);
		m_sPC.prev()->colors_.reserve(nPCreserve);
		m_sPC.next()->points_.reserve(nPCreserve);
		m_sPC.next()->colors_.reserve(nPCreserve);
	}

	string iName = "";
	pK->v("_PointCloudViewer", &iName);
	m_pViewer = (_PointCloudViewer*) (pK->getInst(iName));

	return true;
}

int _PointCloudBase::size(void)
{
	return -1;
}

bool _PointCloudBase::open(void)
{
	return true;
}

bool _PointCloudBase::isOpened(void)
{
	return m_bOpen;
}

void _PointCloudBase::close(void)
{
	m_bOpen = false;
}

int _PointCloudBase::check(void)
{
	IF__(!m_bOpen, -1);

	return 0;
}

void _PointCloudBase::setTranslation(vFloat3& vT)
{
	m_vT = vT;
}

void _PointCloudBase::setRotation(vFloat3& vR)
{
	m_vR = vR;
}

void _PointCloudBase::transform(void)
{
	IF_(check()<0);

	Eigen::Matrix4d mT = Eigen::Matrix4d::Identity();
	Eigen::Vector3d vR(m_vR.x, m_vR.y, m_vR.z);
	mT.block(0,0,3,3) = m_sPC.next()->GetRotationMatrixFromXYZ(vR);
	mT(0,3) = m_vT.x;
	mT(1,3) = m_vT.y;
	mT(2,3) = m_vT.z;

	m_sPC.next()->Transform(mT);
}

void _PointCloudBase::draw(void)
{
	this->_ThreadBase::draw();

	NULL_(m_pViewer);
//	m_pViewer->render(m_sPC.prev());
}

}
#endif
