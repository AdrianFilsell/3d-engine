
#include "pch.h"
#include "camerarotate_viewtool.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "hint.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

camerarotate_viewtool::camerarotate_viewtool(CrasterizerView *pV):viewtool(pV)
{
}

bool camerarotate_viewtool::render(void) const
{
	return m_pView && !!m_pView->getselection();
}

void camerarotate_viewtool::selectionchanged( void )
{
}

void camerarotate_viewtool::render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const
{
	renderbbox(pDst,rDevice,pSurface,0);
}

void camerarotate_viewtool::getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht )const
{
	ht=nullptr;
}

void camerarotate_viewtool::cancel(void)
{
	if(m_InDrag==td_camera_transform)
	{
		m_pView->getsurface()->getcamera()->setdir(m_BeginDir);

		m_pView->getsurface()->getcamera()->setup(m_BeginUp);

		m_pView->getsurface()->getcamera()->settrns();
		
		theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getsurface()->getcamera()));
	}

	viewtool::cancel();
}

void camerarotate_viewtool::begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove )
{
	af3d::mat4<> mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m_WorldToClipSpaceTrns);
	m_ClipToWorldSpaceTrns=m_WorldToClipSpaceTrns.inverse();

	m_rtptBeginDrag=rtptStart;
	m_BeginUp=m_pView->getsurface()->getcamera()->getup();
	m_BeginDir=m_pView->getsurface()->getcamera()->getdir();
	
	af3d::vec3<> worldpos,worlddir,worldback;
	m_pView->getsurface()->getworldray(rtptStart,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);

	m_CameraWorldPlaneIntersect=worldpos;

	m_rtptPrev=rtptStart;

	// unbound
	m_CameraWorldPlane=af3d::plane<>(m_CameraWorldPlaneIntersect,m_pView->getsurface()->getcamera()->getdir());

	af3d::vertexattsframe<>::getaatrns({1,0,0},{0,1,0},{0,0,af3d::getfwd()},
									   m_BeginUp.cross(m_BeginDir),m_BeginUp,m_BeginDir,
									   m_AggregateUnBoundTrns);
	
	m_InDrag=td_camera_transform;
	captureinput();
		
	rotate(rtptMove);
}

void camerarotate_viewtool::movedrag( const af3d::vec2<int>& rtpt )
{
	rotate(rtpt);
}

void camerarotate_viewtool::rotate( const af3d::vec2<int>& rtpt )
{
	unbound_rotate(rtpt);
	m_rtptPrev=rtpt;
}

void camerarotate_viewtool::unbound_rotate( const af3d::vec2<int>& rtpt )
{
	if(!m_pView || !m_pView->getsurface())
		return;

	af3d::vec3<> worldpos,worlddir,worldback;
	m_pView->getsurface()->getworldray(rtpt,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);
	
	af3d::vec3<> worldintersect;
	if(!m_CameraWorldPlane.getintersect(worldpos,worldpos+worlddir,worldintersect))
		return;
	
	af3d::vec3<> rotspaceaxisfrom=m_CameraWorldPlaneIntersect,rotspaceaxisto=worldintersect;

	m_CameraWorldPlaneIntersect=worldintersect;

	const RAS_FLTTYPE dDrag = (rtpt - m_rtptPrev).getlength();
	const RAS_FLTTYPE dFullDrag = 360/0.25; // lets say 0.25 device pixel is 1 degree of rotation
	const RAS_FLTTYPE dRadians = (dDrag/dFullDrag) * 2.0 * af::getpi<RAS_FLTTYPE>();

	af3d::vec3<> rotspacecrossdir=m_CameraWorldPlane.getnormal(),rotspaceorigin={0,0,0};

	const af3d::vec3<> rotspaceaxis=(rotspaceaxisto-rotspaceaxisfrom).cross(rotspacecrossdir-rotspaceorigin).normalize();
	addrotation(m_AggregateUnBoundTrns,af3d::rotation3<>(rotspaceaxis,dRadians),m_AggregateUnBoundTrns);

	af3d::vec3<> up,dir;
	m_AggregateUnBoundTrns.mul({0,1,0},up);
	m_AggregateUnBoundTrns.mul({0,0,af3d::getfwd()},dir);

	m_pView->getsurface()->getcamera()->setup(up);

	m_pView->getsurface()->getcamera()->setdir(dir);

	m_pView->getsurface()->getcamera()->settrns();
	
	theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getsurface()->getcamera()));
}

void camerarotate_viewtool::enddrag( const af3d::vec2<int>& rtpt )
{
	rotate(rtpt);

	stop();
}

void camerarotate_viewtool::addrotation(const af3d::mat4<>& from,const af3d::rotation3<>& r,af3d::mat4<>& to)const
{
	af3d::mat4<> tmp=from;
	
	tmp.mul(r);
	
	to=tmp;
}
