
#include "pch.h"
#include "rotate_viewtool.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "hint.h"
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

rotate_viewtool::rotate_viewtool(CrasterizerView *pV):viewtool(pV)
{
	m_bBound=false;
	m_dRotationBBoxSweepDeg=12;
}

bool rotate_viewtool::render(void) const
{
	return m_pView && !!m_pView->getselection();
}

void rotate_viewtool::selectionchanged( void )
{
}

void rotate_viewtool::render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const
{
	renderrotatebbox(pDst,rDevice,pSurface,af3d::vertexattsframe<>::t_all);
}

void rotate_viewtool::getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht )const
{
	ht=nullptr;

	if(m_pView->getselection())
	{
		af3d::mat4<> m,mProj;
		m_pView->getsurface()->getprojmat(mProj);
		af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m);

		std::vector<af3d::vec3<>> vWorld;
		getselectionrotatebboxhandles(vWorld);
		int nHandle;
		if(m_pView->getsurface()->htcircle(rtpt,vWorld,m_nHandleRadius,m,nHandle))
		{
			std::shared_ptr<hittest<>> sp(std::make_unique<hittest<>>());
			sp->setworldhandle(nHandle,vWorld[nHandle]);
			sp->settype(hittest<>::t_rotate_handle);
			ht=sp;
		}
	}
}

void rotate_viewtool::cancel(void)
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

void rotate_viewtool::begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove )
{
	af3d::vertexattsframe<>::get(m_pView->getselection(),af3d::vertexattsframe<>::t_light_mesh,m_vLights);

	af3d::mat4<> mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m_WorldToClipSpaceTrns);
	m_ClipToWorldSpaceTrns=m_WorldToClipSpaceTrns.inverse();

	m_rtptBeginDrag=rtptStart;
	
	m_BeginTrns=m_pView->getselection()->gettrns();
	m_BeginCompositeTrns=m_pView->getselection()->getcompositetrns();
	m_WorldToParentSpaceTrns=m_pView->getselection()->getparent()->getcompositetrns().inverse();

	m_pView->getselection()->validatecompositebbox();

	m_BeginWorldBBox=af3d::facetrnsbbox<>(m_pView->getselection()->getbbox(true),m_pView->getselection()->getcompositetrns());

	m_rtptPrev=rtptStart;

	m_pView->getselection()->getcompositetrns().mul(m_pView->getselection()->getbbox(true).getorigin(),m_WorldBBoxCentre);
	
	std::shared_ptr<hittest<>> viewht;
	getviewht( rtptStart, viewht );
	if(viewht && viewht->gettype()!=hittest<>::t_null)
	{
		// bound
		m_CameraWorldPlane=af3d::plane<>(viewht->getworldhandle(),m_pView->getsurface()->getcamera()->getdir());

		af3d::vec3<> worldOrigin;
		m_pView->getselection()->getcompositetrns().mul(m_pView->getselection()->getbbox(true).getorigin(),worldOrigin);

		af3d::vec3<> worldU,worldV,worldO;
		m_pView->getselection()->getcompositetrns().mul({0,0,0},worldO);
		const std::pair<af3d::plane<>::axistype,bool> p=getrotplanefromhandle(viewht->gethandle());
		switch(p.first)
		{
			case af3d::plane<>::at_xy:
			{
				m_pView->getselection()->getcompositetrns().mul({1,0,0},worldU);
				m_pView->getselection()->getcompositetrns().mul({0,1,0},worldV);
			}
			break;
			case af3d::plane<>::at_xz:
			{
				m_pView->getselection()->getcompositetrns().mul({1,0,0},worldU);
				m_pView->getselection()->getcompositetrns().mul({0,0,1},worldV);
			}
			break;
			case af3d::plane<>::at_zy:
			{
				m_pView->getselection()->getcompositetrns().mul({0,0,1},worldU);
				m_pView->getselection()->getcompositetrns().mul({0,1,0},worldV);
			}
			break;
		}
		worldU=worldU-worldO;
		worldV=worldV-worldO;
		worldU.normalize();
		worldV.normalize();
		m_BoundWorldPlaneNormal=worldU.cross(worldV).normalize();
		
		m_BoundWorldPlaneBasis=af3d::planebasis<>(worldOrigin,worldU,worldV);
		m_dBoundWorldPlaneBasisRadians=m_BoundWorldPlaneBasis.getbasisradians(viewht->getworldhandle());
		m_bBoundPlaneColinearHandles=isboundplanecolinearhandles(p);
		
		getselectionrotatebbox(m_BoundClipPlaneSegments,m_dRotationBBoxSweepDeg,p.first);
		
		m_bBound=true;
		m_InDrag=td_frame_transform;
		captureinput();
		
		rotate(rtptMove);
	}
	else
	{
		std::shared_ptr<hittest<>> docht;
		getdocht( rtptStart, docht );
		if(docht)
		{
			// unbound
			m_pView->getselection()->getcompositetrns().mul(docht->getmodelspacefacepos(),m_CameraWorldPlaneIntersect);
			m_CameraWorldPlane=af3d::plane<>(m_CameraWorldPlaneIntersect,m_pView->getsurface()->getcamera()->getdir());
						
			m_bBound=false;
			m_InDrag=td_frame_transform;
			captureinput();
		
			rotate(rtptMove);
		}
	}
}

void rotate_viewtool::movedrag( const af3d::vec2<int>& rtpt )
{
	rotate(rtpt);
}

void rotate_viewtool::rotate( const af3d::vec2<int>& rtpt )
{
	if(m_bBound)
		bound_rotate(rtpt);
	else
		unbound_rotate(rtpt);
	m_rtptPrev=rtpt;
}

void rotate_viewtool::bound_rotate( const af3d::vec2<int>& rtpt )
{
	if(!m_pView || !m_pView->getsurface() || !m_pView->getselection())
		return;

	RAS_FLTTYPE dRadians;
	af3d::vec3<> worldintersect;
	if(!getboundplaneworldintersect(rtpt,worldintersect))
		return;
	dRadians=m_BoundWorldPlaneBasis.getbasisradians(worldintersect)-m_dBoundWorldPlaneBasisRadians;
	
	af3d::mat4<> trns;
	addrotation(m_BeginCompositeTrns,af3d::rotation3<>(m_BoundWorldPlaneNormal,dRadians),trns);
	trns.mul(m_WorldToParentSpaceTrns);

	m_pView->getselection()->settrns(trns);
	m_pView->getselection()->setcomposite();
	m_pView->getselection()->invalidatecompositebbox();

	synclights(m_vLights);

	theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getselection()));
}

void rotate_viewtool::unbound_rotate( const af3d::vec2<int>& rtpt )
{
	if(!m_pView || !m_pView->getsurface() || !m_pView->getselection())
		return;

	af3d::vec3<> worldpos,worlddir,worldback;
	m_pView->getsurface()->getworldray(rtpt,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);
	
	af3d::vec3<> worldintersect;
	if(!m_CameraWorldPlane.getintersect(worldpos,worldpos+worlddir,worldintersect))
		return;
	
	const RAS_FLTTYPE dDrag = (rtpt - m_rtptPrev).getlength();
	const RAS_FLTTYPE dFullDrag = 360; // lets say 1 device pixel is 1 degree of rotation
	const RAS_FLTTYPE dRadians = (dDrag/dFullDrag) * 2.0 * af::getpi<RAS_FLTTYPE>();

	af3d::mat4<> trns;
	const af3d::vec3<> worldspaceaxis=(worldintersect-m_CameraWorldPlaneIntersect).cross(m_CameraWorldPlane.getnormal()).normalize();
	addrotation(m_pView->getselection()->getcompositetrns(),af3d::rotation3<>(worldspaceaxis,dRadians),trns);
	trns.mul(m_WorldToParentSpaceTrns);

	m_CameraWorldPlaneIntersect=worldintersect;

	m_pView->getselection()->settrns(trns);
	m_pView->getselection()->setcomposite();
	m_pView->getselection()->invalidatecompositebbox();

	synclights(m_vLights);

	theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getselection()));
}

void rotate_viewtool::enddrag( const af3d::vec2<int>& rtpt )
{
	rotate(rtpt);

	stop();
}

void rotate_viewtool::getselectionrotatebboxhandles(std::vector<af3d::vec3<>>& vWorld)const
{
	vWorld.clear();
	
	if(m_pView->getselection())
	{
		m_pView->getselection()->validatecompositebbox();

		const af3d::facetrnsbbox<> bb(m_pView->getselection()->getbbox(true),m_pView->getselection()->getcompositetrns());
		
		const af3d::vec3<> up=bb.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_front_tl,false);
		const af3d::vec3<> right=bb.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_front_br,false);
		const af3d::vec3<> fwd=bb.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_back_bl,false);
		const af3d::vec3<> rightnorm=right.normalized();
		const af3d::vec3<> upnorm=up.normalized();
		const af3d::vec3<> fwdnorm=fwd.normalized();

		const RAS_FLTTYPE dUpLength=up.getlength();
		const RAS_FLTTYPE dRightLength=right.getlength();
		const RAS_FLTTYPE dFwdLength=fwd.getlength();
		const RAS_FLTTYPE dRadius=std::max<>(std::max<>(dUpLength,dRightLength),dFwdLength)*0.5;

		const af3d::vec3<> origin=bb.getcentre();

		// (pos/neg)
		vWorld.resize(6);
		vWorld[gethandlefromrotplane({af3d::plane<>::at_zy,true})]=(origin+(upnorm*dRadius));
		vWorld[gethandlefromrotplane({af3d::plane<>::at_zy,false})]=(origin-(upnorm*dRadius));
		vWorld[gethandlefromrotplane({af3d::plane<>::at_xz,true})]=(origin+(fwdnorm*dRadius));
		vWorld[gethandlefromrotplane({af3d::plane<>::at_xz,false})]=(origin-(fwdnorm*dRadius));
		vWorld[gethandlefromrotplane({af3d::plane<>::at_xy,true})]=(origin+(rightnorm*dRadius));
		vWorld[gethandlefromrotplane({af3d::plane<>::at_xy,false})]=(origin-(rightnorm*dRadius));
	}
}

void rotate_viewtool::getselectionrotatebbox(std::vector<af3d::vec4<>>& vClip,const RAS_FLTTYPE dRotationBBoxSweepDeg,const af3d::plane<>::axistype at)const
{
	std::vector<af3d::vec3<>> vWorld;
	getselectionrotatebbox(vWorld,dRotationBBoxSweepDeg,at);
	vClip.resize(vWorld.size());
	auto i=vWorld.cbegin(),end=vWorld.cend();
	auto j=vClip.begin();
	for(;i!=end;++i,++j)
		m_WorldToClipSpaceTrns.mul(*i,*j);
}

void rotate_viewtool::getselectionrotatebbox(std::vector<af3d::vec3<>>& vWorld,const RAS_FLTTYPE dRotationBBoxSweepDeg,const af3d::plane<>::axistype at)const
{
	vWorld.clear();
	
	if(m_pView->getselection())
	{
		m_pView->getselection()->validatecompositebbox();

		const af3d::facetrnsbbox<> worldbb(m_pView->getselection()->getbbox(true),m_pView->getselection()->getcompositetrns());
				
		const af3d::vec3<> worldup=worldbb.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_front_tl,false);
		const af3d::vec3<> worldright=worldbb.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_front_br,false);
		const af3d::vec3<> worldfwd=worldbb.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_back_bl,false);
		const af3d::vec3<> worldrightnorm=worldright.normalized();
		const af3d::vec3<> worldupnorm=worldup.normalized();
		const af3d::vec3<> worldfwdnorm=worldfwd.normalized();

		const RAS_FLTTYPE dWorldUpLength=worldup.getlength();
		const RAS_FLTTYPE dWorldRightLength=worldright.getlength();
		const RAS_FLTTYPE dWorldFwdLength=worldfwd.getlength();
		const RAS_FLTTYPE dWorldRadius=std::max<>(std::max<>(dWorldUpLength,dWorldRightLength),dWorldFwdLength)*0.5;
		
		const af3d::vec3<> worldorigin=worldbb.getcentre();

		int nSegments = static_cast<int>(0.5 + (360.0/(dRotationBBoxSweepDeg<0?-dRotationBBoxSweepDeg:(dRotationBBoxSweepDeg?dRotationBBoxSweepDeg:1.0))));
		nSegments = nSegments<3?3:nSegments;
		
		const RAS_FLTTYPE dSweepRad = (af::getpi<RAS_FLTTYPE>()*2.0)/RAS_FLTTYPE(nSegments);

		af3d::vec3<> worldrotaxis,worldrotpos;
		switch(at)
		{
			case af3d::plane<>::at_xy:
			{
				worldrotaxis=worldfwdnorm;
				worldrotpos=(worldorigin+(worldrightnorm*dWorldRadius));
			}
			break;
			case af3d::plane<>::at_zy:
			{
				worldrotaxis=worldrightnorm;
				worldrotpos=(worldorigin+(worldupnorm*dWorldRadius));
			}
			break;
			case af3d::plane<>::at_xz:
			{
				worldrotaxis=worldupnorm;
				worldrotpos=(worldorigin+(worldfwdnorm*dWorldRadius));
			}
			break;
		}
		
		const af3d::translate3<> trns(worldorigin),invtrns(-worldorigin);
		const af3d::mat4<> mTrns(trns),mInvTrns(invtrns);
		af3d::mat4<> m,mProj,mRot;
		m_pView->getsurface()->getprojmat(mProj);
		af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m);
		for(int n=0;n<nSegments;++n)
		{
			mInvTrns.mul(af3d::mat4<>(af3d::rotation3<>(worldrotaxis,n*dSweepRad)),mRot);
			mRot.mul(mTrns);
		
			af3d::vec3<> ptWorld;
			mRot.mul(worldrotpos,ptWorld);
			vWorld.push_back(ptWorld);
		}
	}
}

void rotate_viewtool::renderrotatebbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const int nHandleTypes)const
{
	af3d::mat4<> m,mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m);

	m_pView->getselection()->validatecompositebbox();

	const af3d::facetrnsbbox<> bb(m_pView->getselection()->getbbox(true),m_pView->getselection()->getcompositetrns());

	const std::vector<af3d::plane<>::axistype> axistypes={af3d::plane<>::at_xy,af3d::plane<>::at_xz,af3d::plane<>::at_zy};
	auto i=axistypes.cbegin(),end=axistypes.cend();
	for(;i!=end;++i)
	{
		std::vector<af3d::vec3<>> vWorld;
		getselectionrotatebbox(vWorld,m_dRotationBBoxSweepDeg,*i);

		for(int n=0;n<static_cast<int>(vWorld.size())-1;++n)
		{
			const af3d::line_pos_vertex_data<af3d::vec3<>> l({vWorld[0+n],vWorld[1+n]});
			pSurface->renderline(pDst,l,m_LineCol,m);
		}
		if(vWorld.size()>1)
		{
			const af3d::line_pos_vertex_data<af3d::vec3<>> l({vWorld[0],vWorld[vWorld.size()-1]});			
			pSurface->renderline(pDst,l,m_LineCol,m);
		}
	}
	
	if(m_pView->getselection()->gettype()&nHandleTypes)
	{
		std::vector<af3d::vec3<>> vWorld;
		getselectionrotatebboxhandles(vWorld);
		for(int n=0;n<static_cast<int>(vWorld.size());++n)
			pSurface->rendercircle(pDst,vWorld[n],m_nHandleRadius,m_HandleCol,m);
	}
}

bool rotate_viewtool::getboundplaneworldintersect(const af3d::vec2<int>& rtpt,af3d::vec3<>& worldintersect)const
{
	if(m_BoundClipPlaneSegments.size()<2)
		return false;

	std::function<RAS_FLTTYPE(const af3d::vec2<>& from,const af3d::vec2<>& to,af3d::vec2<>& p)> param=[](const af3d::vec2<>& from,const af3d::vec2<>& to,af3d::vec2<>& p) -> RAS_FLTTYPE
	{
		const RAS_FLTTYPE dDot=(to-from).dot(p-from);
		if(dDot>0)
		{
			// moving along line
			const RAS_FLTTYPE dLineLenSq=(to-from).getlengthsq();
			const RAS_FLTTYPE dPLenSq=(p-from).getlengthsq();
		
			const RAS_FLTTYPE d=sqrt(dPLenSq)/sqrt(dLineLenSq);
			if(d>1)
			{
				p=to;
				return 1;
			}
			return d;
		}
		else
		if(dDot<0)
		{
			// moving in opposite direction
			p=from;
			return 0;
		}
		
		// p is perpendicular to ( to - from ), lets call it 0
		p=from;
		return 0;
	};
	std::function<bool(const af3d::vec3<>& l0,const af3d::vec3<>& l1,af3d::vec2<>& i)> intersect=[](const af3d::vec3<>& l0,const af3d::vec3<>& l1,af3d::vec2<>& i) -> bool
	{
		const RAS_FLTTYPE divisor = ( ( l0[0] * l1[1] ) - ( l1[0] * l0[1]) );
		if( fabs(divisor-0)<1e-3) return false;		// parallel
		const RAS_FLTTYPE recip = ( 1 / divisor );
		i={( ( l0[1] * l1[2] ) - ( l1[1] * l0[2] ) ) * recip,( ( l1[0] * l0[2] ) - ( l0[0] * l1[2] ) ) * recip };
		return true;
	};
	std::function<af3d::vec2<>(const af3d::vec3<>& abc)> perpdelta=[](const af3d::vec3<>& abc) -> af3d::vec2<>
	{
		af3d::vec2<> delta(0,0);
		if( fabs(abc[0]-0)<1e-3) return {0,1};		// horz
		if( fabs(abc[1]-0)<1e-3) return {1,0};		// vert
		return {1,1 / -( -abc[0] / abc[1] )};
	};
	std::function<af3d::vec3<>(const af3d::vec2<>& from,const af3d::vec2<>& to)> abc=[](const af3d::vec2<>& from,const af3d::vec2<>& to) -> af3d::vec3<>
	{
		return {( from[1] - to[1] ),( to[0] - from[0] ),( ( from[0] * to[1] ) - ( to[0] * from[1] ) )};
	};
	std::function<bool(const af3d::vec2<int>& rtpt,const af3d::vec4<>& from,const af3d::vec4<>& to,RAS_FLTTYPE& dDist,RAS_FLTTYPE& dT)> dist=[&](const af3d::vec2<int>& rtpt,const af3d::vec4<>& from,const af3d::vec4<>& to,RAS_FLTTYPE& dDist,RAS_FLTTYPE& dT) -> bool
	{
		// screen line
		af3d::vec3<> l[2];
		from.getdevice(m_pView->getsurface()->getdstndc(),l[0]);
		to.getdevice(m_pView->getsurface()->getdstndc(),l[1]);
		const af3d::vec3<> l0=(abc)({l[0][0],l[0][1]},{l[1][0],l[1][1]});

		// perpendicular line ( rtpt, rtpt + delta )
		const af3d::vec2<> delta=(perpdelta)(l0);
		const af3d::vec3<> l1=(abc)({rtpt[0]+0.0,rtpt[1]+0.0},{rtpt[0]+delta[0],rtpt[1]+delta[1]});
		
		// intersect between lines l0 and l1
		af3d::vec2<> i;
		if(!intersect(l0,l1,i))
			return false;
		
		// clamp i onto finite line and get param 't'
		dT=param({l[0][0],l[0][1]},{l[1][0],l[1][1]},i);

		// distance
		dDist=(af3d::vec2<>(rtpt[0],rtpt[1])-i).getlength();

		return true;
	};

	// [), then last
	RAS_FLTTYPE d,dT;
	af3d::vec4<> clippos;
	struct segment {int nSegmentFrom,nSegmentTo;RAS_FLTTYPE dDistance,dParametric;};
	std::vector<segment> vCandidates;
	vCandidates.reserve(m_BoundClipPlaneSegments.size());
	for(int n=0;n<static_cast<int>(m_BoundClipPlaneSegments.size())-1;++n)
		if((dist)(rtpt,m_BoundClipPlaneSegments[n+0],m_BoundClipPlaneSegments[n+1],d,dT))
		{
			if(m_bBoundPlaneColinearHandles || !vCandidates.size())
				vCandidates.push_back({n,n+1,d,dT});
			else
			if(d<vCandidates[0].dDistance)
				vCandidates[0]={n,n+1,d,dT};
		}
	if((dist)(rtpt,m_BoundClipPlaneSegments[m_BoundClipPlaneSegments.size()-1],m_BoundClipPlaneSegments[0],d,dT))
	{
		if(m_bBoundPlaneColinearHandles || !vCandidates.size())
			vCandidates.push_back({static_cast<int>(m_BoundClipPlaneSegments.size()-1),0,d,dT});
		else
		if(d<vCandidates[0].dDistance)
			vCandidates[0]={static_cast<int>(m_BoundClipPlaneSegments.size()-1),0,d,dT};
	}
	int nCandidate=0;
	if(m_bBoundPlaneColinearHandles)
	{
		// use best candidate closest to camera
		const RAS_FLTTYPE dCoincidentTol=3;
		std::sort(vCandidates.begin(),vCandidates.end(),[](const segment& a,const segment& b)->bool{return a.dDistance<b.dDistance;});
		if(vCandidates.size()>1 && fabs(vCandidates[0].dDistance-vCandidates[1].dDistance)<=dCoincidentTol)
		{
			const int nA=nCandidate;
			const int nB=nA+1;
			const segment& sA=vCandidates[nA];
			const segment& sB=vCandidates[nB];
			af3d::vec4<> clipposA,clipposB;
			clipposA.lerp(m_BoundClipPlaneSegments[sA.nSegmentFrom],m_BoundClipPlaneSegments[sA.nSegmentTo],sA.dParametric);
			clipposB.lerp(m_BoundClipPlaneSegments[sB.nSegmentFrom],m_BoundClipPlaneSegments[sB.nSegmentTo],sB.dParametric);
			nCandidate=(clipposA[2]/clipposA[3])<(clipposB[2]/clipposB[3])?nA:nB;
		}
	}
	const segment& s=vCandidates[nCandidate];
	clippos.lerp(m_BoundClipPlaneSegments[s.nSegmentFrom],m_BoundClipPlaneSegments[s.nSegmentTo],s.dParametric);
	m_ClipToWorldSpaceTrns.mul(clippos,worldintersect);
	return true;
}

std::pair<af3d::plane<>::axistype,bool> rotate_viewtool::getrotplanefromhandle(const int nHandle )const
{
	switch(nHandle)
	{
		case 0:
		case 1:return {af3d::plane<>::at_xy,nHandle==0};
		case 2:
		case 3:return {af3d::plane<>::at_xz,nHandle==2};
		default:return {af3d::plane<>::at_zy,nHandle==4};
	}
}

int rotate_viewtool::gethandlefromrotplane(const std::pair<af3d::plane<>::axistype,bool> p)const
{
	switch(p.first)
	{
		case af3d::plane<>::at_xy:return p.second?0:1;
		case af3d::plane<>::at_xz:return p.second?2:3;
		case af3d::plane<>::at_zy:return p.second?4:5;
		default:return 0;
	}
}

void rotate_viewtool::addrotation(const af3d::mat4<>& from,const af3d::rotation3<>& r,af3d::mat4<>& to)const
{
	af3d::mat4<> tmp=from;
	
	af3d::vec3<> o;
	tmp.mul(af3d::translate3<>(-m_BeginWorldBBox.getcentre()));
	tmp.mul(r);
	tmp.mul(af3d::translate3<>(m_BeginWorldBBox.getcentre()));

	to=tmp;
}

bool rotate_viewtool::isboundplanecolinearhandles(const std::pair<af3d::plane<>::axistype,bool> p)const
{
	std::vector<af3d::vec3<>> vWorld;
	getselectionrotatebboxhandles(vWorld);

	const int nHandle=gethandlefromrotplane(p);
	std::vector<af3d::vec3<>> vWorldHandles;
	switch(p.first)
	{
		case af3d::plane<>::at_xz:
		{
			vWorldHandles={vWorld[gethandlefromrotplane({p.first,!p.second})],
						   vWorld[gethandlefromrotplane({af3d::plane<>::at_xy,true})],		
						   vWorld[gethandlefromrotplane({af3d::plane<>::at_xy,false})]};
		}
		break;
		case af3d::plane<>::at_zy:
		{
			vWorldHandles={vWorld[gethandlefromrotplane({p.first,!p.second})],
						   vWorld[gethandlefromrotplane({af3d::plane<>::at_xz,true})],		
						   vWorld[gethandlefromrotplane({af3d::plane<>::at_xz,false})]};
		}
		break;
		case af3d::plane<>::at_xy:
		{
			vWorldHandles={vWorld[gethandlefromrotplane({p.first,!p.second})],
						   vWorld[gethandlefromrotplane({af3d::plane<>::at_zy,true})],		
						   vWorld[gethandlefromrotplane({af3d::plane<>::at_zy,false})]};
		}
		break;
	}
	vWorldHandles.push_back(vWorld[nHandle]);

	af3d::mat4<> m,mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m);

	std::vector<af3d::vec2<>> vScreenHandles;
	vScreenHandles.resize(vWorldHandles.size());
	auto i=vWorldHandles.cbegin(),end=vWorldHandles.cend();
	auto j=vScreenHandles.begin();
	for(;i!=end;++i,++j)
	{
		af3d::vec3<> d;
		af3d::vec4<> c;
		m.mul(*i,c);
		c.getdevice(m_pView->getsurface()->getdstndc(),d);
		*j={d[0],d[1]};
	}
	
	std::function<RAS_FLTTYPE(const af3d::vec2<>& a,const af3d::vec2<>& b,const af3d::vec2<>& c)> triarea=[](const af3d::vec2<>& a,const af3d::vec2<>& b,const af3d::vec2<>& c) -> RAS_FLTTYPE
	{
		return 0.5 * fabs(a[0]*(b[1]-c[1]) + b[0]*(c[1]-a[1]) + c[0]*(a[1]-b[1]));
	};

	const RAS_FLTTYPE dTol=1e-3;
	const RAS_FLTTYPE dArea=(triarea)(vScreenHandles[0],vScreenHandles[1],vScreenHandles[2]);
	if(!(dArea<dTol))
		return false;
	if(!(fabs(dArea-(triarea)(vScreenHandles[0],vScreenHandles[1],vScreenHandles[3]))<dTol))
		return false;
	if(!(fabs(dArea-(triarea)(vScreenHandles[0],vScreenHandles[2],vScreenHandles[3]))<dTol))
		return false;
	if(!(fabs(dArea-(triarea)(vScreenHandles[1],vScreenHandles[2],vScreenHandles[3]))<dTol))
		return false;
	return true;
}
