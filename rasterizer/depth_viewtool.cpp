
#include "pch.h"
#include "depth_viewtool.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "hint.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool depth_viewtool::render(void) const
{
	return m_pView && !!m_pView->getselection();
}

void depth_viewtool::selectionchanged( void )
{
}

void depth_viewtool::render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const
{
	renderbbox(pDst,rDevice,pSurface,0);
}

void depth_viewtool::getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht )const
{
	ht=nullptr;
}

void depth_viewtool::cancel(void)
{
	if(m_InDrag==td_frame_transform)
	{
		m_pView->getselection()->settrns(m_BeginTrns);
		m_pView->getselection()->setcomposite();
		m_pView->getselection()->invalidatecompositebbox();

		synclights(m_vLights);
		
		theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getselection()));
	}

	viewtool::cancel();
}

void depth_viewtool::begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove )
{
	af3d::vertexattsframe<>::get(m_pView->getselection(),af3d::vertexattsframe<>::t_light_mesh,m_vLights);

	af3d::mat4<> mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m_WorldToClipSpaceTrns);
	m_ClipToWorldSpaceTrns=m_WorldToClipSpaceTrns.inverse();

	af3d::vec3<> worldpos,worldback,worlddir;
	m_pView->getsurface()->getworldray(rtptStart,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);

	m_BeginTrns=m_pView->getselection()->gettrns();
	m_BeginCompositeTrns=m_pView->getselection()->getcompositetrns();
	m_WorldToParentSpaceTrns=m_pView->getselection()->getparent()->getcompositetrns().inverse();

	std::shared_ptr<hittest<>> docht;
	getdocht( rtptStart, docht );
	if(docht && docht->getvertexframe())
	{
		// unbound
		m_pView->getselection()->getcompositetrns().mul(docht->getmodelspacefacepos(),m_CameraWorldPlaneIntersect);
		m_CameraWorldPlane=af3d::plane<>(m_CameraWorldPlaneIntersect,worlddir/*m_pView->getsurface()->getcamera()->getdir()*/);

		m_CameraWorldPlaneBasis=af3d::planebasis<>(m_CameraWorldPlaneIntersect,
												   m_pView->getsurface()->getcamera()->getup().cross(worlddir/*m_pView->getsurface()->getcamera()->getdir()*/),
												   m_pView->getsurface()->getcamera()->getup());
		m_DepthWorldPlaneBasis=af3d::planebasis<>(m_CameraWorldPlaneIntersect,
												  m_pView->getsurface()->getcamera()->getup().cross(worlddir/*m_pView->getsurface()->getcamera()->getdir()*/),
												  worlddir/*m_pView->getsurface()->getcamera()->getdir()*/);

		m_InDrag=td_frame_transform;
		captureinput();
		
		translate(rtptMove);
	}
}

void depth_viewtool::movedrag( const af3d::vec2<int>& rtpt )
{
	translate(rtpt);
}

void depth_viewtool::enddrag( const af3d::vec2<int>& rtpt )
{
	translate(rtpt);

	stop();
}

void depth_viewtool::translate( const af3d::vec2<int>& rtpt )
{
	if(!m_pView || !m_pView->getsurface() || !m_pView->getselection())
		return;

	af3d::vec3<> worldpos,worlddir,worldback;
	m_pView->getsurface()->getworldray(rtpt,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);
	
	af3d::vec3<> worldintersect;
	if(!m_CameraWorldPlane.getintersect(worldpos,worldpos+worlddir,worldintersect))
		return;
	
	af3d::vec2<> basis;
	m_CameraWorldPlaneBasis.getbasis(worldintersect,basis);

	af3d::vec3<> worldto;
	m_DepthWorldPlaneBasis.get3d({0,basis[1]},worldto); // only use 2d vertical delta

	af3d::mat4<> trns;
	trns.mul(m_BeginCompositeTrns,af3d::translate3<>(worldto-m_CameraWorldPlaneIntersect),trns);
	trns.mul(m_WorldToParentSpaceTrns);

	m_pView->getselection()->settrns(trns);
	m_pView->getselection()->setcomposite();
	m_pView->getselection()->invalidatecompositebbox();

	synclights(m_vLights);

	theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getselection()));
}
