
#include "pch.h"
#include "translate_scale_viewtool.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "hint.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool translate_scale_viewtool::render(void) const
{
	return m_pView && !!m_pView->getselection();
}

void translate_scale_viewtool::selectionchanged( void )
{
}

void translate_scale_viewtool::render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const
{
	renderbbox(pDst,rDevice,pSurface,af3d::vertexattsframe<>::t_all&~af3d::vertexattsframe<>::t_light_mesh);
}

void translate_scale_viewtool::getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht )const
{
	ht=nullptr;

	if(m_pView->getselection() && !(m_pView->getselection()->gettype()&af3d::vertexattsframe<>::t_light_mesh))
	{
		af3d::mat4<> m,mProj;
		m_pView->getsurface()->getprojmat(mProj);
		af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m);

		std::vector<af3d::vec3<>> vWorld;
		getselectionbbox(vWorld);
		int nHandle;
		if(m_pView->getsurface()->htcircle(rtpt,vWorld,m_nHandleRadius,m,nHandle))
		{
			std::shared_ptr<hittest<>> sp(std::make_unique<hittest<>>());
			sp->setworldhandle(nHandle,vWorld[nHandle]);
			sp->settype(hittest<>::t_bbox_handle);
			ht=sp;
		}
	}
}

void translate_scale_viewtool::cancel(void)
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

void translate_scale_viewtool::begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove )
{
	af3d::vertexattsframe<>::get(m_pView->getselection(),af3d::vertexattsframe<>::t_light_mesh,m_vLights);
	
	af3d::mat4<> mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m_WorldToClipSpaceTrns);
	m_ClipToWorldSpaceTrns=m_WorldToClipSpaceTrns.inverse();
	
	m_BeginTrns=m_pView->getselection()->gettrns();
	m_BeginCompositeTrns=m_pView->getselection()->getcompositetrns();
	m_WorldToParentSpaceTrns=m_pView->getselection()->getparent()->getcompositetrns().inverse();

	m_pView->getselection()->validatecompositebbox();

	m_BeginWorldBBox=af3d::facetrnsbbox<>(m_pView->getselection()->getbbox(true),m_pView->getselection()->getcompositetrns());

	m_bScale=false;

	std::shared_ptr<hittest<>> viewht;
	getviewht( rtptStart, viewht );
	if(viewht && viewht->gettype()!=hittest<>::t_null)
	{
		m_nHandle=viewht->gethandle();

		af3d::mat4<> trns;
		af3d::vertexattsframe<>::getaatrns(m_BeginWorldBBox.get(af3d::facetrnsbbox<>::v_front_tl,af3d::facetrnsbbox<>::v_front_tr,true),
										   m_BeginWorldBBox.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_front_tl,true),
										   m_BeginWorldBBox.get(af3d::facetrnsbbox<>::v_front_tl,af3d::facetrnsbbox<>::v_back_tl,true),
										   {1,0,0},{0,1,0},{0,0,af3d::getfwd()},trns);

		m_WorldSpaceAxisAlignTrns=af3d::translate3<>(-m_BeginWorldBBox.getcentre());
		m_WorldSpaceAxisAlignTrns.mul(trns);
		m_InvWorldSpaceAxisAlignTrns=m_WorldSpaceAxisAlignTrns.inverse();

		m_BeginCompositeTrns.mul(m_WorldSpaceAxisAlignTrns,m_ModelToAxisAlignedWorldSpaceTrns);
		
		m_ScaleWorldPlaneIntersect=viewht->getworldhandle();
		getscaleplane();

		m_InDrag=td_frame_transform;
		captureinput();
		
		m_bScale=true;

		scale(rtptMove);
	}
	else
	{
		std::shared_ptr<hittest<>> docht;
		getdocht( rtptStart, docht );
		if(docht && docht->getvertexframe())
		{
			ASSERT(docht->getvertexframe()==m_pView->getselection());

			m_pView->getselection()->getcompositetrns().mul(docht->getmodelspacefacepos(),m_CameraWorldPlaneIntersect);
			m_CameraWorldPlane=af3d::plane<>(m_CameraWorldPlaneIntersect,m_pView->getsurface()->getcamera()->getdir());

			m_InDrag=td_frame_transform;
			captureinput();
		
			translate(rtptMove);
		}
	}
}

void translate_scale_viewtool::movedrag( const af3d::vec2<int>& rtpt )
{
	if(m_bScale)
		scale(rtpt);
	else
		translate(rtpt);
}

void translate_scale_viewtool::enddrag( const af3d::vec2<int>& rtpt )
{
	if(m_bScale)
		scale(rtpt);
	else
		translate(rtpt);

	stop();
}

void translate_scale_viewtool::translate( const af3d::vec2<int>& rtpt )
{
	if(!m_pView || !m_pView->getsurface() || !m_pView->getselection())
		return;

	af3d::vec3<> worldpos,worlddir,worldback;
	m_pView->getsurface()->getworldray(rtpt,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);
	
	af3d::vec3<> worldintersect;
	if(!m_CameraWorldPlane.getintersect(worldpos,worldpos+worlddir,worldintersect))
		return;

	af3d::mat4<> trns;
	trns.mul(m_BeginCompositeTrns,af3d::translate3<>(worldintersect-m_CameraWorldPlaneIntersect),trns);
	trns.mul(m_WorldToParentSpaceTrns);

	m_pView->getselection()->settrns(trns);
	m_pView->getselection()->setcomposite();
	m_pView->getselection()->invalidatecompositebbox();

	synclights(m_vLights);

	theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getselection()));
}

void translate_scale_viewtool::scale( const af3d::vec2<int>& rtpt )
{
	if(!m_pView || !m_pView->getsurface() || !m_pView->getselection())
		return;

	af3d::vec3<> worldpos,worlddir,worldback;
	m_pView->getsurface()->getworldray(rtpt,m_ClipToWorldSpaceTrns,worldpos,worldback,worlddir);
	
	af3d::vec3<> worldintersect;
	if(!m_ScaleWorldPlane.getintersect(worldpos,worldpos+worlddir,worldintersect))
		return;

	m_pView->getselection()->validatecompositebbox();

	const af3d::facetrnsbbox<> worldbbto=getscalebbox(worldintersect);
	const af3d::facetrnsbbox<> axisalignedworldspacebbto(worldbbto,m_WorldSpaceAxisAlignTrns);
	const af3d::facetrnsbbox<> axisalignedworldspacebbfrom(m_pView->getselection()->getbbox(true),m_ModelToAxisAlignedWorldSpaceTrns);
	
	af3d::mat4<> trns;
	addscale(m_BeginCompositeTrns,getscale(axisalignedworldspacebbfrom,axisalignedworldspacebbto),trns);
	addtranslate(af3d::facetrnsbbox<>(m_pView->getselection()->getbbox(true),trns),trns);
	trns.mul(m_WorldToParentSpaceTrns);

	m_pView->getselection()->settrns(trns);
	m_pView->getselection()->setcomposite();
	m_pView->getselection()->invalidatecompositebbox();

	synclights(m_vLights);

	theApp.UpdateAllViews(&hint(m_pView,getdoc(),m_InDrag,m_pView->getselection()));
}

void translate_scale_viewtool::addtranslate(const af3d::facetrnsbbox<>& worldto,af3d::mat4<>& trns)const
{
	af3d::vec3<> vFrom=m_BeginWorldBBox[m_nHandle],vTo=worldto[m_nHandle];
	trns.mul(af3d::translate3<>(vTo-vFrom));
}

void translate_scale_viewtool::addscale(const af3d::mat4<>& from,const af3d::scale3<>& r,af3d::mat4<>& to)const
{
	af3d::mat4<> tmp;
	
	from.mul(m_WorldSpaceAxisAlignTrns,tmp);
	tmp.mul(r);
	tmp.mul(m_InvWorldSpaceAxisAlignTrns,to);
}

af3d::vec3<> translate_scale_viewtool::getscale(const af3d::facetrnsbbox<>& worldfrom,const af3d::facetrnsbbox<>& worldto)const
{
	RAS_FLTTYPE d;
	af3d::vec3<> s(1,1,1), v[2];

	v[0]=worldfrom.get(af3d::facetrnsbbox<>::v_front_tl,af3d::facetrnsbbox<>::v_front_tr,false);
	v[1]=worldto.get(af3d::facetrnsbbox<>::v_front_tl,af3d::facetrnsbbox<>::v_front_tr,false);
	
	d=v[0].getlength();
	s[0]=d?v[1].getlength()/d:s[0];
	if(v[0].dot(v[1])<0)
		s[0]*=-1;

	v[0]=worldfrom.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_front_tl,false);
	v[1]=worldto.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_front_tl,false);

	d=v[0].getlength();
	s[1]=d?v[1].getlength()/d:s[1];
	if(v[0].dot(v[1])<0)
		s[1]*=-1;

	v[0]=worldfrom.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_back_bl,false);
	v[1]=worldto.get(af3d::facetrnsbbox<>::v_front_bl,af3d::facetrnsbbox<>::v_back_bl,false);

	d=v[0].getlength();
	s[2]=d?v[1].getlength()/d:s[2];
	if(v[0].dot(v[1])<0)
		s[2]*=-1;
	
	return s;
}

void translate_scale_viewtool::getscaleplane(void)
{
	af3d::facetrnsbbox<>::planetype arrPrimaryCamdidates[3];
	std::vector<af3d::facetrnsbbox<>::planetype> vP={af3d::facetrnsbbox<>::p_front,af3d::facetrnsbbox<>::p_back,af3d::facetrnsbbox<>::p_top,af3d::facetrnsbbox<>::p_bottom,af3d::facetrnsbbox<>::p_left,af3d::facetrnsbbox<>::p_right};
	auto i=vP.cbegin(),end=vP.cend();
	for(int n=0;i!=end && n<3;++i)
		if(af3d::facetrnsbbox<>::isplanevertex(*i,static_cast<af3d::facetrnsbbox<>::vertextype>(m_nHandle)))
			arrPrimaryCamdidates[n++]=*i;
	
	af3d::vec3<> arrPrimaryCamdidateDirs[3][2];
	arrPrimaryCamdidateDirs[0][0]=m_BeginWorldBBox.getdir(arrPrimaryCamdidates[0],true,true);
	arrPrimaryCamdidateDirs[1][0]=m_BeginWorldBBox.getdir(arrPrimaryCamdidates[1],true,true);
	arrPrimaryCamdidateDirs[2][0]=m_BeginWorldBBox.getdir(arrPrimaryCamdidates[2],true,true);
	arrPrimaryCamdidateDirs[0][1]=m_BeginWorldBBox.getdir(arrPrimaryCamdidates[0],false,true);
	arrPrimaryCamdidateDirs[1][1]=m_BeginWorldBBox.getdir(arrPrimaryCamdidates[1],false,true);
	arrPrimaryCamdidateDirs[2][1]=m_BeginWorldBBox.getdir(arrPrimaryCamdidates[2],false,true);

	RAS_FLTTYPE arrPrimaryCamdidateDots[3][2];
	arrPrimaryCamdidateDots[0][0]=m_pView->getsurface()->getcamera()->getdir().dot(arrPrimaryCamdidateDirs[0][0]);
	arrPrimaryCamdidateDots[1][0]=m_pView->getsurface()->getcamera()->getdir().dot(arrPrimaryCamdidateDirs[1][0]);
	arrPrimaryCamdidateDots[2][0]=m_pView->getsurface()->getcamera()->getdir().dot(arrPrimaryCamdidateDirs[2][0]);
	arrPrimaryCamdidateDots[0][1]=m_pView->getsurface()->getcamera()->getdir().dot(arrPrimaryCamdidateDirs[0][1]);
	arrPrimaryCamdidateDots[1][1]=m_pView->getsurface()->getcamera()->getdir().dot(arrPrimaryCamdidateDirs[1][1]);
	arrPrimaryCamdidateDots[2][1]=m_pView->getsurface()->getcamera()->getdir().dot(arrPrimaryCamdidateDirs[2][1]);
	
	RAS_FLTTYPE arrPrimaryCamdidateRadians[3][2];
	arrPrimaryCamdidateRadians[0][0]=acos(arrPrimaryCamdidateDots[0][0]);
	arrPrimaryCamdidateRadians[1][0]=acos(arrPrimaryCamdidateDots[1][0]);
	arrPrimaryCamdidateRadians[2][0]=acos(arrPrimaryCamdidateDots[2][0]);
	arrPrimaryCamdidateRadians[0][1]=acos(arrPrimaryCamdidateDots[0][1]);
	arrPrimaryCamdidateRadians[1][1]=acos(arrPrimaryCamdidateDots[1][1]);
	arrPrimaryCamdidateRadians[2][1]=acos(arrPrimaryCamdidateDots[2][1]);

	int nMin=0;
	RAS_FLTTYPE dMin=arrPrimaryCamdidateRadians[0][0]<arrPrimaryCamdidateRadians[0][1]?arrPrimaryCamdidateRadians[0][0]:arrPrimaryCamdidateRadians[0][1];
	for(int n=1;n<3;++n)
	{
		const RAS_FLTTYPE d = arrPrimaryCamdidateRadians[n][0]<arrPrimaryCamdidateRadians[n][1]?arrPrimaryCamdidateRadians[n][0]:arrPrimaryCamdidateRadians[n][1];
		if(d<dMin){	dMin=d; nMin=n; }
	}
	m_WorldBBoxScalePrimary=arrPrimaryCamdidates[nMin];
	
	const af3d::vec3<> normal=m_BeginWorldBBox.getdir(m_WorldBBoxScalePrimary,true,true);
	const af3d::vec3<> up=m_BeginWorldBBox.getup(m_WorldBBoxScalePrimary,true);
	const af3d::vec3<> right=normal.cross(up);
	
	m_ScaleWorldPlane=af3d::plane<>(m_ScaleWorldPlaneIntersect,normal);
	m_ScaleWorldPlaneBasis=af3d::planebasis<>(m_ScaleWorldPlaneIntersect,right,up);

	m_ScaleU.clear();
	m_ScaleV.clear();
	m_ScaleUV.clear();

	switch(m_nHandle)
	{
		case af3d::facetrnsbbox<>::v_front_tr:getscalebbox_tr(true);break;
		case af3d::facetrnsbbox<>::v_back_tr:getscalebbox_tr(false);break;

		case af3d::facetrnsbbox<>::v_front_br:getscalebbox_br(true);break;
		case af3d::facetrnsbbox<>::v_back_br:getscalebbox_br(false);break;

		case af3d::facetrnsbbox<>::v_front_tl:getscalebbox_tl(true);break;
		case af3d::facetrnsbbox<>::v_back_tl:getscalebbox_tl(false);break;

		case af3d::facetrnsbbox<>::v_front_bl:getscalebbox_bl(true);break;
		case af3d::facetrnsbbox<>::v_back_bl:getscalebbox_bl(false);break;
	}
}

void translate_scale_viewtool::getscalebbox_tr(const bool bFront)
{
	if(bFront)
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_front:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tl);
			}
			break;
			case af3d::facetrnsbbox<>::p_right:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tr);
			}
			break;
			case af3d::facetrnsbbox<>::p_top:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tr);
			}
			break;
		}
	}
	else
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_back:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tl);
			}
			break;
			case af3d::facetrnsbbox<>::p_right:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tr);
			}
			break;
			case af3d::facetrnsbbox<>::p_top:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tr);
			}
			break;
		}
	}
}

void translate_scale_viewtool::getscalebbox_bl(const bool bFront)
{
	if(bFront)
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_front:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_br);
			}
			break;
			case af3d::facetrnsbbox<>::p_left:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_br);
			}
			break;
			case af3d::facetrnsbbox<>::p_bottom:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_bl);
			}
			break;
		}
	}
	else
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_back:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_br);
			}
			break;
			case af3d::facetrnsbbox<>::p_left:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_br);
			}
			break;
			case af3d::facetrnsbbox<>::p_bottom:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_bl);
			}
			break;
		}
	}
}

void translate_scale_viewtool::getscalebbox_tl(const bool bFront)
{
	if(bFront)
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_front:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tr);
			}
			break;
			case af3d::facetrnsbbox<>::p_left:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tr);
			}
			break;
			case af3d::facetrnsbbox<>::p_top:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tl);
			}
			break;
		}
	}
	else
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_back:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tr);
			}
			break;
			case af3d::facetrnsbbox<>::p_left:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tr);
			}
			break;
			case af3d::facetrnsbbox<>::p_top:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tl);
			}
			break;
		}
	}
}

void translate_scale_viewtool::getscalebbox_br(const bool bFront)
{
	if(bFront)
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_front:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_bl);
			}
			break;
			case af3d::facetrnsbbox<>::p_right:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_br);
			}
			break;
			case af3d::facetrnsbbox<>::p_bottom:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_br);
			}
			break;
		}
	}
	else
	{
		switch(m_WorldBBoxScalePrimary)
		{
			case af3d::facetrnsbbox<>::p_back:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_front_br);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_back_bl);
			}
			break;
			case af3d::facetrnsbbox<>::p_right:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_bl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_br);
			}
			break;
			case af3d::facetrnsbbox<>::p_bottom:
			{
				m_ScaleUV.push_back(af3d::facetrnsbbox<>::v_back_tr);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_bl);
				m_ScaleU.push_back(af3d::facetrnsbbox<>::v_back_tl);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_tr);
				m_ScaleV.push_back(af3d::facetrnsbbox<>::v_front_br);
			}
			break;
		}
	}
}

af3d::facetrnsbbox<> translate_scale_viewtool::getscalebbox(const af3d::vec3<>& worldpos)const
{
	af3d::vec2<> toUV;
	m_ScaleWorldPlaneBasis.getbasis(worldpos,toUV);

	af3d::vec3<> worldDeltaU,worldDeltaV;
	m_ScaleWorldPlaneBasis.get3d({toUV[0],0},worldDeltaU);
	worldDeltaU=worldDeltaU-m_BeginWorldBBox[m_nHandle];
	m_ScaleWorldPlaneBasis.get3d({0,toUV[1]},worldDeltaV);
	worldDeltaV=worldDeltaV-m_BeginWorldBBox[m_nHandle];

	af3d::facetrnsbbox<> bbto=m_BeginWorldBBox;
	bbto[m_nHandle]=worldpos;
	{
		auto i=m_ScaleU.cbegin(),end=m_ScaleU.cend();
		for(;i!=end;++i) bbto[*i]+=worldDeltaU;
	}
	{
		auto i=m_ScaleV.cbegin(),end=m_ScaleV.cend();
		for(;i!=end;++i) bbto[*i]+=worldDeltaV;
	}
	{
		auto i=m_ScaleUV.cbegin(),end=m_ScaleUV.cend();
		for(;i!=end;++i) bbto[*i]+=(worldDeltaU+worldDeltaV);
	}
	bbto[m_nHandle]=worldpos;
	return bbto;
}
