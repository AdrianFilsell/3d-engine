
#include "pch.h"
#include "cameratranslate_viewtool.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "hint.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool cameratranslate_viewtool::render(void) const
{
	return m_pView && !!m_pView->getselection();
}

void cameratranslate_viewtool::selectionchanged( void )
{
}

void cameratranslate_viewtool::render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const
{
	renderbbox(pDst,rDevice,pSurface,0);
}

void cameratranslate_viewtool::getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht )const
{
	ht=nullptr;
}

void cameratranslate_viewtool::cancel(void)
{
	if(m_InDrag==td_camera_transform)
	{
		m_pView->getsurface()->getcamera()->setorigin(m_BeginTrns);
		m_pView->getsurface()->getcamera()->settrns();

		theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getsurface()->getcamera()));
	}

	viewtool::cancel();
}

void cameratranslate_viewtool::begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove )
{
	af3d::mat4<> mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m_WorldToClipSpaceTrns);
	m_ClipToWorldSpaceTrns=m_WorldToClipSpaceTrns.inverse();

	m_BeginTrns=m_pView->getsurface()->getcamera()->getorigin();

	af3d::vec3<> worldpos,worlddir,worldback;
	m_pView->getsurface()->getworldray(rtptStart,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);

	m_CameraWorldPlaneIntersect=worldpos+((worldback-worldpos)*0.01);

	m_CameraWorldPlane=af3d::plane<>(m_CameraWorldPlaneIntersect,m_pView->getsurface()->getcamera()->getdir());

	m_CameraWorldPlaneBasis=af3d::planebasis<>(m_CameraWorldPlaneIntersect,
												m_pView->getsurface()->getcamera()->getup().cross(m_pView->getsurface()->getcamera()->getdir()),
												m_pView->getsurface()->getcamera()->getup());
	m_TranslateWorldPlaneBasis=af3d::planebasis<>(m_pView->getsurface()->getcamera()->getorigin(),
												  m_pView->getsurface()->getcamera()->getup().cross(m_pView->getsurface()->getcamera()->getdir()),
												  m_pView->getsurface()->getcamera()->getup());

	m_InDrag=td_camera_transform;
	captureinput();
		
	translate(rtptMove);
}

void cameratranslate_viewtool::movedrag( const af3d::vec2<int>& rtpt )
{
	translate(rtpt);
}

void cameratranslate_viewtool::enddrag( const af3d::vec2<int>& rtpt )
{
	translate(rtpt);

	stop();
}

void cameratranslate_viewtool::translate( const af3d::vec2<int>& rtpt )
{
	if(!m_pView || !m_pView->getsurface())
		return;

	af3d::vec3<> worldpos,worldback,worlddir;
	m_pView->getsurface()->getworldray(rtpt,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);
	
	af3d::vec3<> worldintersect;
	if(!m_CameraWorldPlane.getintersect(worldpos,worldpos+worlddir,worldintersect))
		return;

	af3d::vec2<> basis;
	m_CameraWorldPlaneBasis.getbasis(worldintersect,basis);

	af3d::vec3<> worldto;
	m_TranslateWorldPlaneBasis.get3d(basis,worldto);

	m_pView->getsurface()->getcamera()->setorigin(worldto);
	m_pView->getsurface()->getcamera()->settrns();

	theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getsurface()->getcamera()));
}
