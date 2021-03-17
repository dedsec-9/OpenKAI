/*
 * PCbase.h
 *
 *  Created on: May 24, 2020
 *      Author: yankai
 */

#ifndef OpenKAI_src_PointCloud__PCframe_H_
#define OpenKAI_src_PointCloud__PCframe_H_

#ifdef USE_OPEN3D
#include "_PCbase.h"
using namespace open3d;
using namespace open3d::geometry;
using namespace open3d::visualization;

namespace kai
{

class _PCframe: public _PCbase
{
public:
	_PCframe();
	virtual ~_PCframe();

	virtual bool init(void* pKiss);
	virtual int size(void);
	virtual int check(void);
	virtual void draw(void);

    //frame
	virtual void getPC(PointCloud* pPC);
	virtual void updatePC(void);
    
protected:
    virtual void updateTransformMatrix(void);
    virtual void paintPC(PointCloud* pPC);
    
    //frame buf
	_PCframe* m_pPCB;
	vSwitch<PointCloud> m_sPC;
    pthread_mutex_t m_mutexPC;

	vFloat3 m_vColOvrr;    
};

}
#endif
#endif