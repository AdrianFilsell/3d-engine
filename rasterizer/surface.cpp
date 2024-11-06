
#include "pch.h"
#include "surface.h"
#include "rasterizer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const RAS_FLTTYPE surface::s_dDefDistance=80;
const cameraorient surface::s_DefFront={af3d::vec3<>(0,1,0),af3d::vec3<>(0,0,af3d::getfwd<>()),af3d::vec3<>(0,0,-af3d::getfwd<>()*s_dDefDistance)};
const cameraorient surface::s_DefRight={af3d::vec3<>(0,1,0),af3d::vec3<>(-1,0,0),af3d::vec3<>(s_dDefDistance,0,0)};
const cameraorient surface::s_DefBack={af3d::vec3<>(0,1,0),af3d::vec3<>(0,0,-af3d::getfwd<>()),af3d::vec3<>(0,0,af3d::getfwd<>()*s_dDefDistance)};
const cameraorient surface::s_DefLeft={af3d::vec3<>(0,1,0),af3d::vec3<>(1,0,0),af3d::vec3<>(-s_dDefDistance,0,0)};
const cameraorient surface::s_DefAbove={af3d::vec3<>(0,0,af3d::getfwd<>()),af3d::vec3<>(0,-1,0),af3d::vec3<>(0,s_dDefDistance,0)};
const cameraorient surface::s_DefBelow={af3d::vec3<>(0,0,-af3d::getfwd<>()),af3d::vec3<>(0,1,0),af3d::vec3<>(0,-s_dDefDistance,0)};

const RAS_FLTTYPE surface::s_dDefNear=1;
const RAS_FLTTYPE surface::s_dDefFar=5000;
const RAS_FLTTYPE surface::s_dDefPerspectiveFov=af::getradian<RAS_FLTTYPE>(60);	// unity default is 60 degrees
const RAS_FLTTYPE surface::s_dDefSceneAspect=1;
const af3d::orthographicprojection<>::subtype surface::s_dDefOrthographicSubType=af3d::orthographicprojection<>::st_none;

const COLORREF surface::s_grey=RGB(200,200,200);

std::shared_ptr<const af3d::mat4<>> surface::s_spIdentity=[]{std::shared_ptr<af3d::mat4<>> sp=std::make_unique<af3d::mat4<>>();sp->setIdentity();return sp;}();

surface::~surface()
{
	// Tidy up
	stop();
}

void surface::tidyup( void )
{
	// Tidy up
	m_spZBuffer=nullptr;
	m_spGBuffer=nullptr;
	m_spDev=nullptr;
	m_spCamera=nullptr;
	m_spPerspectiveProjection=nullptr;
	m_spVSScratchCache=nullptr;
}

void surface::stop( void )
{
	tidyup();
}

void surface::start(void)
{
	stop();

	m_spVSScratchCache=std::shared_ptr<af3d::vertexshaderscratch<>>(new af3d::vertexshaderscratch<>);
	m_dSceneAspect=s_dDefSceneAspect;
	
	createcamera();
	createprojection();
}

void surface::createcamera(void)
{
	std::shared_ptr<af3d::camera<>> spCamera(new af3d::camera<>);
	m_spCamera=spCamera;
	setcameraorient(s_DefFront);
}

void surface::createprojection(void)
{
	std::shared_ptr<af3d::perspectiveprojection<>> spProjection(new af3d::perspectiveprojection<>);
	spProjection->setnear(s_dDefNear);
	spProjection->setfar(s_dDefFar);
	spProjection->setfov(s_dDefPerspectiveFov);
	spProjection->setaspect(m_dSceneAspect);
	spProjection->settrns();
	m_spPerspectiveProjection=spProjection;
}

void surface::resize(const af3d::scene<> *pScene,const int nW,const int nH)
{
	if(nW<1 || nH<1)
		return;

	RAS_FLTTYPE dS;
	af3d::rect::getrectscale(0,0,m_dSceneAspect,1,0,0,nW,nH,true,dS);

	const af3d::vec2<> centre(nW/2.0,nH/2.0);
	const af3d::vec2<> offset=centre - af3d::vec2<>((m_dSceneAspect*dS)/2.0,dS/2.0);
	const af3d::crect<> rDst(af3d::vec2<>(0,0)+offset,af3d::vec2<>(m_dSceneAspect*dS,dS)+offset);

	m_rDstNDC.set(af3d::rect::v_tl,{af::posfloor<RAS_FLTTYPE>(rDst.get(af3d::crect<>::v_tl)[0]),af::posfloor<RAS_FLTTYPE>(rDst.get(af3d::crect<>::v_tl)[1])});
	m_rDstNDC.set(af3d::rect::v_br,{af::posfloor<RAS_FLTTYPE>(rDst.get(af3d::crect<>::v_br)[0]),af::posfloor<RAS_FLTTYPE>(rDst.get(af3d::crect<>::v_br)[1])});
	
	if(!(m_spZBuffer && m_spZBuffer->getwidth() == m_rDstNDC.getwidth() && m_spZBuffer->getheight() == m_rDstNDC.getheight() ))
	{
		m_spZBuffer=nullptr;
		std::shared_ptr<af3d::zbuffer<>> spZBuffer(new af3d::zbuffer<>);
		if(spZBuffer->create(m_rDstNDC.getwidth(),m_rDstNDC.getheight()))
			m_spZBuffer=spZBuffer;
	}

	if(!(m_spGBuffer && m_spGBuffer->getwidth() == m_rDstNDC.getwidth() && m_spGBuffer->getheight() == m_rDstNDC.getheight() ))
	{
		m_spGBuffer=nullptr;
		std::shared_ptr<af3d::gbuffer<>> spGBuffer(new af3d::gbuffer<>);
		if(spGBuffer->create(m_rDstNDC.getwidth(),m_rDstNDC.getheight()))
			m_spGBuffer=spGBuffer;
	}

	if(!(m_spDev && m_spDev->getwidth() == m_rDstNDC.getwidth() && m_spDev->getheight() == m_rDstNDC.getheight() ))
	{
		m_spDev=nullptr;
		std::shared_ptr<afdib::dib> spDev(new afdib::dib);
		if(spDev->create(nW,nH,afdib::dib::pt_b8g8r8))
			m_spDev=spDev;
	}
}

cameraorient surface::getcameraorient(void)const
{
	cameraorient o;
	o.up=m_spCamera->getup();
	o.origin=m_spCamera->getorigin();
	o.dir=m_spCamera->getdir();
	return o;
}

void surface::setcameraorient(const cameraorient& o)
{
	m_spCamera->setup(o.up);
	m_spCamera->setdir(o.dir);
	m_spCamera->setorigin(o.origin);
	m_spCamera->settrns();
}

void surface::getworldray(const af3d::vec2<int>& ptDevice,const af3d::mat4<>& cliptoworld,af3d::vec3<>& worldposFront,af3d::vec3<>& worldposBack,af3d::vec3<>& worlddir)const
{
	const af3d::vec2<> dbl(ptDevice[0],ptDevice[1]);
	af3d::perspectiveprojection<>::getunprojectworldpos(dbl,m_rDstNDC,cliptoworld,worldposFront,worldposBack,worlddir);
}

void surface::ht(af3d::scene<> *pScene,const af3d::vec2<int>& ptDevice,const af3d::mat4<>& cliptoworld,const int nTypes,hittest<>& res)const
{
	af3d::vec3<> worldpos,worlddir,worldback;
	if(m_rDstNDC.isinside(ptDevice) && pScene && pScene->getchildren() && pScene->getchildren()->size())
	{
		const af3d::plane<> worldcameraplane(getcamera()->getorigin(),getcamera()->getdir());

		getworldray(ptDevice,cliptoworld,worldpos,worldback,worlddir);
		ht(pScene,worldcameraplane,worldpos,worlddir,nTypes,res);
	}
}

void surface::ht(af3d::vertexattsframe<> *pFrame,const af3d::plane<>& worldcameraplane,const af3d::vec3<>& worldpos,const af3d::vec3<>& worlddir,const int nTypes,hittest<>& res)const
{
	if(pFrame && pFrame->getvisible() && pFrame->gettype()&nTypes)
		switch(pFrame->getvertexatts())
		{
			case (af3d::face_vertex_att::t_pos):
				if(faceht<af3d::face_pos3<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;

			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm):
				if(faceht<af3d::face_pos3_norm<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_bump):
				if(faceht<af3d::face_pos3_norm_bump<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex):
				if(faceht<af3d::face_pos3_norm_tex<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):
				if(faceht<af3d::face_pos3_norm_tex_bump<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_bump):
				if(faceht<af3d::face_pos3_norm_col_bump<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col):
				if(faceht<af3d::face_pos3_norm_col<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):
				if(faceht<af3d::face_pos3_norm_col_tex<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_norm|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex|af3d::face_vertex_att::t_bump):
				if(faceht<af3d::face_pos3_norm_col_tex_bump<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;

			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col):
				if(faceht<af3d::face_pos3_col<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_col|af3d::face_vertex_att::t_tex):
				if(faceht<af3d::face_pos3_col_tex<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;

			case (af3d::face_vertex_att::t_pos|af3d::face_vertex_att::t_tex):
				if(faceht<af3d::face_pos3_tex<>>(pFrame,worldcameraplane,worldpos,worlddir,res))
					res.setvertexframe(pFrame);
			break;
				
			default:break;
		}
	if(pFrame->getchildren() && pFrame->getvisible() && pFrame->gettype()&nTypes)
	{
		auto i = pFrame->getchildren()->cbegin(),end=pFrame->getchildren()->cend();
		for(;i!=end;++i)
			ht((*i).get(),worldcameraplane,worldpos,worlddir,nTypes,res);
	}
}

template <typename F> bool surface::faceht(af3d::vertexattsframe<> *pFrame,const af3d::plane<>& worldcameraplane,const af3d::vec3<>& worldpos,const af3d::vec3<>& worlddir,hittest<>& res)const
{
	using t_base_types=af3d::face_types<F>;

	const af3d::mesh<t_base_types::template t_fb> *pMesh=static_cast<const af3d::mesh<t_base_types::template t_fb>*>(pFrame);
	return pMesh->ht(worldcameraplane,worldpos,worlddir,res);
}

void surface::composeshadowmaps(const af3d::scene<> *pScene,const af3d::vertexattsframe<>* pLight,const af3d::rect& rDeviceClip,const int nTypes)
{
	const af3d::rect rDstNDCClip=m_rDstNDC.getintersect(rDeviceClip);
	const af3d::vec2<int>& tl=m_rDstNDC.get(af3d::rect::v_tl);
	const af3d::rect rNormDstNDCClip={rDstNDCClip.get(af3d::rect::v_tl)-tl,rDstNDCClip.get(af3d::rect::v_br)-tl};
	
	if(!m_spCamera || !pScene || rDstNDCClip.isempty())
	{
		clearshadowmaps();
		return;
	}

	const std::vector<af3d::vertexattsframe<>*> *pOnLights = pScene->getlights(true);
	if(!pOnLights || pOnLights->size()==0)
	{
		clearshadowmaps();
		return;
	}

	bool bMutatedVectorFrom=pOnLights->size() != m_vShadowMaps.size();
	if(!bMutatedVectorFrom)
	{
		auto i=pOnLights->cbegin(),end=pOnLights->cend();
		auto j=m_vShadowMaps.cbegin();
		for(;i!=end && !bMutatedVectorFrom;++i,++j)
			if((*i)!=(*j).first)
				bMutatedVectorFrom=true;
	}

	if(bMutatedVectorFrom)
	{
		using E=std::pair<af3d::vertexattsframe<>*,std::shared_ptr<af3d::shadowmap<>>>;
		std::function<bool(const E& a,const E& b)> fn = [](const E& a,const E& b)->bool{return a.first<b.first;};

		std::vector<std::pair<af3d::vertexattsframe<>*,std::shared_ptr<af3d::shadowmap<>>>> vSortedShadowMaps=m_vShadowMaps;
		std::sort(vSortedShadowMaps.begin(),vSortedShadowMaps.end(),fn);
		
		m_vShadowMaps.resize(pOnLights->size());
		auto i=pOnLights->cbegin(),end=pOnLights->cend();
		auto j=m_vShadowMaps.begin();
		for(;i!=end;++i,++j)
		{
			const E e=std::make_pair(*i,nullptr);
			auto k=std::lower_bound(vSortedShadowMaps.cbegin(),vSortedShadowMaps.cend(),e,fn);
			(*j)=(k==vSortedShadowMaps.cend() || (*k).first!=(*i)) ? e : (*j)=(*k);
		}
	}
	
	const afthread::taskscheduler *pSched=theApp.getsched();

	const af3d::mat4<> t=m_spCamera->gettrns();
	const af3d::vec3<> o=m_spCamera->getorigin();
	const af3d::vec3<> u=m_spCamera->getup();
	const af3d::vec3<> d=m_spCamera->getdir();

	const RAS_FLTTYPE dCamDist=o.getlength();
	
	const int nShadowMapTypes=nTypes&~(af3d::vertexattsframe<>::t_shadow|af3d::vertexattsframe<>::t_light_mesh);
	const af3d::lightcache<RAS_FLTTYPE> lm_lights(pScene->getdirlight());
	const af3d::lightcache<RAS_FLTTYPE> lm(pScene->getlights(true),nullptr);

	af3d::mat4<> cameratoclipspace;
	if(m_spPerspectiveProjection)
		cameratoclipspace=m_spPerspectiveProjection->gettrns();
			
	af3d::mat4<> worldtoclipspace;
	auto i=pOnLights->cbegin(),end=pOnLights->cend();
	auto j=m_vShadowMaps.begin();
	for(;i!=end;++i,++j)
	{
		if(!pLight || pLight==*i)
		{
			bool bRender=(*j).first->getvisible();
			if(bRender)
			{
				af3d::lightmeshcache<> *pL=dynamic_cast<af3d::lightmeshcache<>*>(*i);
				switch(pL->getlight()->gettype())
				{
					case af3d::light<>::t_point:
					case af3d::light<>::t_directional:
					case af3d::light<>::t_spot:m_spCamera->setorigin(-pL->getlight()->getworlddir().normalized() * dCamDist);break;//m_spCamera->setorigin(pL->getlight()->getworldpos());break;
					default:bRender=false;
				}

				std::shared_ptr<af3d::shadowmap<>> sp=(*j).second;
				if(!sp || sp->getwidth()!=m_rDstNDC.getwidth() || sp->getheight()!=m_rDstNDC.getheight())
				{
					std::shared_ptr<af3d::shadowmap<>> spZBuffer(new af3d::shadowmap<>);
					spZBuffer->create(m_rDstNDC.getwidth(),m_rDstNDC.getheight());
					sp=spZBuffer;
				}
			
				// this is a bit naive:
				//		- point light should render a cubic shadow map
				//		- directional light should use an orthographic projection
				m_spCamera->setup(pL->getlight()->getworldup());
				m_spCamera->setdir(pL->getlight()->getworlddir());
				m_spCamera->settrns();

				m_spCamera->gettrns().mul(cameratoclipspace,worldtoclipspace);
				sp->setworldtoclipspace(worldtoclipspace);

				sp->clear(pSched,rNormDstNDCClip.getunion(sp->getdstndcclip()),2);
				sp->setdstndcclip(rDstNDCClip);

				if(m_spPerspectiveProjection)
					af3d::rendercontext::renderscene<af3d::perspectiveprojection<>,afdib::b8g8r8>(pSched,nShadowMapTypes,pScene,lm_lights,lm,m_spCamera.get(),m_spPerspectiveProjection.get(),m_rDstNDC,rDstNDCClip,-tl,m_spVSScratchCache.get(),sp.get(),nullptr,nullptr);
				
				(*j).second=sp;
			}
			if(pLight)
				break;
		}
	}

	m_spCamera->settrns(t);
	m_spCamera->setorigin(o);
	m_spCamera->setup(u);
	m_spCamera->setdir(d);
}

void surface::compose(const af3d::scene<> *pScene,const af3d::rect& rDeviceClip,const int nTypes)
{
	if(!m_spDev)
		return;
	
	const afthread::taskscheduler *pSched=theApp.getsched();

	const af3d::rect margins[4]={ {{0,0},{m_rDstNDC.get(af3d::rect::v_tl)[0],m_spDev->getheight()}},
								  {{m_rDstNDC.get(af3d::rect::v_tl)[0],0},{m_rDstNDC.get(af3d::rect::v_br)[0],m_rDstNDC.get(af3d::rect::v_tl)[1]}},
								  {{m_rDstNDC.get(af3d::rect::v_tl)[0],m_rDstNDC.get(af3d::rect::v_br)[1]},{m_rDstNDC.get(af3d::rect::v_br)[0],m_spDev->getheight()}},
								  {{m_rDstNDC.get(af3d::rect::v_br)[0],0},{m_spDev->getwidth(),m_spDev->getheight()}} };
	
	const afdib::b8g8r8a8 bgraMargin(GetBValue(s_grey),GetGValue(s_grey),GetRValue(s_grey),255);
	for(int n=0;n<4;++n)
	{
		const af3d::rect rClip=margins[n].getintersect(rDeviceClip);
		m_spDev->clear(pSched,rClip,bgraMargin);
	}
	
	const af3d::rect rDstNDCClip=m_rDstNDC.getintersect(rDeviceClip);

	const afdib::b8g8r8a8 bgraWhite(255,255,255,255);
	m_spDev->clear(pSched,m_rDstNDC.get(af3d::rect::v_tl),rDstNDCClip,bgraMargin,bgraWhite,10);

	if(!rDstNDCClip.isempty() && pScene)
	{
		const af3d::vec2<int>& tl=m_rDstNDC.get(af3d::rect::v_tl);
		const af3d::rect rNormDstNDCClip={rDstNDCClip.get(af3d::rect::v_tl)-tl,rDstNDCClip.get(af3d::rect::v_br)-tl};

		m_spZBuffer->clear(pSched,rNormDstNDCClip,2);
		if(m_spPerspectiveProjection)
			af3d::rendercontext::renderscene<af3d::perspectiveprojection<>,afdib::b8g8r8>(pSched,nTypes,pScene,af3d::lightcache<RAS_FLTTYPE>(pScene->getdirlight()),af3d::lightcache<RAS_FLTTYPE>(pScene->getlights(true),&m_vShadowMaps),m_spCamera.get(),m_spPerspectiveProjection.get(),m_rDstNDC,rDstNDCClip,-tl,m_spVSScratchCache.get(),m_spZBuffer.get(),m_spGBuffer.get(),m_spDev.get());
	}
}

void surface::flip(CDC *pDC,const af3d::rect& rDeviceClip) const
{
	if(m_spDev)
		m_spDev->blt(*pDC,rDeviceClip,rDeviceClip);
}

void surface::flip(afdib::dib *pDst,const af3d::rect& rDeviceClip) const
{
	if(m_spDev && pDst)
	{
		const afthread::taskscheduler *pSched=theApp.getsched();
		m_spDev->blt(pSched,pDst,rDeviceClip);
	}
}

af3d::rect surface::getdevbbox(const af3d::facetrnsbbox<>& bbox)const
{
	af3d::mat4<> m;
	if(m_spPerspectiveProjection)
		m_spCamera->gettrns().mul(m_spPerspectiveProjection->gettrns(),m);
	const RAS_FLTTYPE l=m_rDstNDC.get(af3d::rect::v_tl)[0],t=m_rDstNDC.get(af3d::rect::v_tl)[1],r=m_rDstNDC.get(af3d::rect::v_br)[0],b=m_rDstNDC.get(af3d::rect::v_br)[1];
	const af3d::crect<> rDstNDC({l,t},{r,b});
	af3d::vec3<> dstNDCspace[8];
	for(int n=0;n<8;++n)
	{
		af3d::vec4<> v;
		m.mul(bbox[n],v);
		v.getdevice(rDstNDC,dstNDCspace[n]);
	}
	af3d::rect rDiscrete;
	af3d::crect<>::postodiscrete(dstNDCspace,8,rDiscrete);
	return rDiscrete;
}

void surface::rendercircle(afdib::dib *pDst,const af3d::vec3<>& origin,const int nR,const af3d::vec4<>& bgra,const af3d::mat4<>& worldtoclip)const
{
	switch(pDst->getpixeltype())
	{
		case afdib::dib::pt_b8g8r8:rendercircle<afdib::b8g8r8>(pDst,origin,nR,bgra,worldtoclip);break;
		case afdib::dib::pt_b8g8r8a8:rendercircle<afdib::b8g8r8a8>(pDst,origin,nR,bgra,worldtoclip);break;
	}
}

bool surface::htcircle(const af3d::vec2<int>& rtpt,std::vector<af3d::vec3<>>& v,const int nR,const af3d::mat4<>& worldtoclip,int& nHandle)const
{
	// clip to front and back 4d clip space cuboid
	constexpr int nExtents=af3d::projectionclipper<>::et_far|af3d::projectionclipper<>::et_near;
	
	// clip to front and back 4d clip space cuboid
	nHandle=-1;
	const int nRSq=nR*nR;
	auto i = v.cbegin(),end=v.cend();
	std::vector<std::pair<RAS_FLTTYPE,int>> vCandidates;
	for(int n = 0;i!=end;++i,++n)
	{
		af3d::vec4<> clipOrigin;
		worldtoclip.mul(*i,clipOrigin);

		if(getperspectiveproj() &&
		   (af3d::perspectiveprojection<>::template t_clipper::getinvalidoutcodes<nExtents>(clipOrigin)!=0))
			continue;

		af3d::vec3<> dstNDCspace;
		clipOrigin.getdevice(m_rDstNDC,dstNDCspace);

		const RAS_FLTTYPE dX=rtpt[0]-dstNDCspace[0];
		const RAS_FLTTYPE dY=rtpt[1]-dstNDCspace[1];
		const RAS_FLTTYPE dDistSq=(dX*dX)+(dY*dY);
		if(dDistSq>nRSq)
			continue;

		vCandidates.push_back({clipOrigin[2]/clipOrigin[3],n});
	}
	if(vCandidates.size()==0)
		return false;
	if(vCandidates.size()>1)
	{
		std::sort(vCandidates.begin(),vCandidates.end(),[](const std::pair<RAS_FLTTYPE,int>& a,const std::pair<RAS_FLTTYPE,int>& b)->bool{return a.first<b.first;});
		nHandle=vCandidates[0].second;
	}
	else
		nHandle=vCandidates[0].second;
	return nHandle!=-1;
}

void surface::renderline(afdib::dib *pDst,const af3d::line_pos3<>& l,const af3d::vec4<>& bgra,const af3d::mat4<>& worldtoclip)const
{
	// clip to front and back 4d clip space cuboid
	constexpr int nExtents=af3d::projectionclipper<>::et_far|af3d::projectionclipper<>::et_near;

	af3d::line_pos3<>::t_xform xfm;
	l.xform(*s_spIdentity,worldtoclip,xfm);
	if(getperspectiveproj())
		xfm.validateclipspace<nExtents,af3d::perspectiveprojection<>::template t_clipper>();
	
	switch(xfm.getclippostype())
	{
		case af3d::line_pos3_xform<>::cpt_outside:break;
		case af3d::line_pos3_xform<>::cpt_inside:
			renderline(pDst,xfm.getclippos().getpos()[0],xfm.getclippos().getpos()[1],bgra);
		break;
		case af3d::line_pos3_xform<>::cpt_unknown:
		{
			af3d::maxvertexbuffer<af3d::line_pos3<>::t_xform::vertex,2> clipped,local;

			if(getperspectiveproj())
				af3d::perspectiveprojection<>::template t_clipper::clip<nExtents>(xfm,&clipped,&local);

			if(clipped.size()==2)
				renderline(pDst,clipped.get()[0].clippos.pos.t,clipped.get()[1].clippos.pos.t,bgra);
		}
		break;
	}
}

void surface::renderline(afdib::dib *pDst,const af3d::vec4<>& l0,const af3d::vec4<>& l1,const af3d::vec4<>& bgra)const
{
	af3d::vec3<> dstNDCspace0,dstNDCspace1;
	l0.getdevice(m_rDstNDC,dstNDCspace0);
	l1.getdevice(m_rDstNDC,dstNDCspace1);

	const af3d::vec2<int> il0={static_cast<int>(dstNDCspace0[0]+0.5),static_cast<int>(dstNDCspace0[1]+0.5)};
	const af3d::vec2<int> il1={static_cast<int>(dstNDCspace1[0]+0.5),static_cast<int>(dstNDCspace1[1]+0.5)};

	const af3d::vec2<> zl0={l0[2],l0[3]};
	const af3d::vec2<> zl1={l1[2],l1[3]};

	switch(pDst->getpixeltype())
	{
		case afdib::dib::pt_b8g8r8:
			bresenham_line<afdib::b8g8r8>(pDst,il0,il1,zl0,zl1,bgra);
		break;
		case afdib::dib::pt_b8g8r8a8:
			bresenham_line<afdib::b8g8r8a8>(pDst,il0,il1,zl0,zl1,bgra);
		break;
	}
}

template <typename PT> void surface::rendercircle(afdib::dib *pDst,const af3d::vec3<>& origin,const int nR,const af3d::vec4<>& bgra,const af3d::mat4<>& worldtoclip)const
{
	// clip to front and back 4d clip space cuboid
	constexpr int nExtents=af3d::projectionclipper<>::et_far|af3d::projectionclipper<>::et_near;

	af3d::vec4<> clipOrigin;
	worldtoclip.mul(origin,clipOrigin);

	if(getperspectiveproj() &&
	   (af3d::perspectiveprojection<>::template t_clipper::getinvalidoutcodes<nExtents>(clipOrigin)!=0))
		return;

	af3d::vec3<> dstNDCspace;
	clipOrigin.getdevice(m_rDstNDC,dstNDCspace);

	const af3d::vec2<int> o(static_cast<int>(dstNDCspace[0]),static_cast<int>(dstNDCspace[1]));
	const af3d::vec2<int> tl(o[0]-nR,o[1]-nR);
	const af3d::vec2<int> br(o[0]-tl[0]+o[0]+1,o[1]-tl[1]+o[1]+1);

	if(br[0]<=0 || tl[0]>=pDst->getwidth() || br[1]<=0 || tl[1]>=pDst->getheight())
		return;
	
	const RAS_FLTTYPE dRadSq = (nR*nR);

	for(int nY=tl[1];nY<=o[1];++nY)
		for(int nX=tl[0];nX<=o[0];++nX)
		{
			const RAS_FLTTYPE dDX=((nX+0.5)-(dstNDCspace[0]));
			const RAS_FLTTYPE dDY=((nY+0.5)-(dstNDCspace[1]));
			const RAS_FLTTYPE dDistSq=( dDX*dDX + dDY*dDY );
			if(dDistSq>dRadSq)
				continue;

			int yaxis[2];
			yaxis[0]=nY;
			yaxis[1]=o[1]-nY+o[1];

			for(int n=0;n<2;++n)
				if(n==0 || yaxis[n]!=yaxis[n-1])
				{
					PT *pScanline=reinterpret_cast<PT*>(pDst->getscanline(yaxis[n]));
					if(pScanline)
					{
						const bool bZBufY=(yaxis[n]>=m_rDstNDC.get(af3d::rect::v_tl)[1]) && (yaxis[n]<m_rDstNDC.get(af3d::rect::v_br)[1]);
						const af3d::zvertex<> *pZScanline=bZBufY?m_spZBuffer->getscanline(yaxis[n]-m_rDstNDC.get(af3d::rect::v_tl)[1]):nullptr;
						
						int xaxis[2];
						xaxis[0]=nX;
						xaxis[1]=o[0]-nX+o[0];
						for(int n=0;n<2;++n)
							if(n==0 || xaxis[n]!=xaxis[n-1])
								if(xaxis[n]>=0 && xaxis[n]<pDst->getwidth())
								{
									const bool bZBufXY=bZBufY && (xaxis[n]>=m_rDstNDC.get(af3d::rect::v_tl)[0]) && (xaxis[n]<m_rDstNDC.get(af3d::rect::v_br)[0]);
									if(!bZBufXY || dstNDCspace[2]<pZScanline[xaxis[n]-m_rDstNDC.get(af3d::rect::v_tl)[0]].get())
										afdib::pixel<RAS_FLTTYPE>::blend<PT>(pScanline[xaxis[n]],bgra[0],bgra[1],bgra[2],bgra[3]);
								}
					}
				}
		}
}

template <typename PT> void surface::bresenham_line(afdib::dib *pDst,const af3d::vec2<int>& a,const af3d::vec2<int>& b,const af3d::vec2<RAS_FLTTYPE>& clipspaceA,const af3d::vec2<RAS_FLTTYPE>& clipspaceB,const af3d::vec4<>& bgra)const
{
	if(a[0]<0 && b[0]<0)
		return;
	if(a[1]<0 && b[1]<0)
		return;
	if(a[0]>=pDst->getwidth() && b[0]>=pDst->getwidth())
		return;
	if(a[1]>=pDst->getheight() && b[1]>=pDst->getheight())
		return;

	const af3d::vec4<> sqrbgra = {bgra[0],bgra[1],bgra[2],bgra[3]*0.35};

	int x0(a[0]);
	const int x1(b[0]);
	int y0(a[1]);
	const int y1(b[1]);

	const int dx = std::abs(x1 - x0);
	const int dy = std::abs(y1 - y0);
	int sx = x0 < x1 ? 1 : -1;
	int sy = y0 < y1 ? 1 : -1;
	int err = dx - dy;

	int nScanlineY=-1;
	PT *pScanline=nullptr;
	PT *pScanlineBelow=nullptr;
	af3d::zvertex<> *pZScanline=nullptr;
	af3d::zvertex<> *pZScanlineBelow=nullptr;

	bresenhamClipSpace clipspace;
	clipspace.pA=&a;
	clipspace.pB=&b;
	clipspace.pClipSpaceA=&clipspaceA;
	clipspace.pClipSpaceB=&clipspaceB;
	{
		const RAS_FLTTYPE dDX = (a[0]+0.5) - (b[0]+0.5);
		const RAS_FLTTYPE dDY = (a[1]+0.5) - (b[1]+0.5);
		clipspace.dLengthSq = ((dDX*dDX)+(dDY*dDY));
	}
	
	bool bContinue=true;
	while(bContinue)
	{
		clipspace.bValidate=true;

		if(nScanlineY!=y0)
		{
			nScanlineY=y0;
			pScanline=reinterpret_cast<PT*>(pDst->getscanline(nScanlineY));
			pScanlineBelow=reinterpret_cast<PT*>(pDst->getscanline(nScanlineY+1));

			const bool bZBufY=(nScanlineY>=m_rDstNDC.get(af3d::rect::v_tl)[1]) && (nScanlineY<m_rDstNDC.get(af3d::rect::v_br)[1]);
			pZScanline=bZBufY?m_spZBuffer->getscanline(nScanlineY-m_rDstNDC.get(af3d::rect::v_tl)[1]):nullptr;
			const bool bZBufYBelow=(nScanlineY+1>=m_rDstNDC.get(af3d::rect::v_tl)[1]) && (nScanlineY+1<m_rDstNDC.get(af3d::rect::v_br)[1]);
			pZScanlineBelow=bZBufYBelow?m_spZBuffer->getscanline(nScanlineY-m_rDstNDC.get(af3d::rect::v_tl)[1]+1):nullptr;
		}

		bresenham_line_plot(pDst,pZScanline,pScanline,x0,y0,clipspace,bgra);

		const bool bSquare=true;
		if(bSquare)
		{
			bresenham_line_plot(pDst,pZScanline,pScanline,x0+1,y0+1,clipspace,sqrbgra);
			if(pScanlineBelow)
			{
				bresenham_line_plot(pDst,pZScanlineBelow,pScanlineBelow,x0,y0+1,clipspace,sqrbgra);
				bresenham_line_plot(pDst,pZScanlineBelow,pScanlineBelow,x0+1,y0+1,clipspace,sqrbgra);
			}
		}

		bContinue=bresenham_line<PT>(x0,y0,x1,y1,dx,dy,sx,sy,err);
	}
}

void surface::validate(const int nX,const int nY,bresenhamClipSpace& ws)const
{
	if(!ws.bValidate)
		return;
	ws.bValidate=false;

	const RAS_FLTTYPE dDX = (nX+0.5) - ((*ws.pA)[0]+0.5);
	const RAS_FLTTYPE dDY = (nY+0.5) - ((*ws.pA)[1]+0.5);
	const RAS_FLTTYPE dLengthSq = ((dDX*dDX)+(dDY*dDY));
	const RAS_FLTTYPE dT = dLengthSq ? dLengthSq/ws.dLengthSq : 0;

	const RAS_FLTTYPE dZ = (*ws.pClipSpaceA)[0] + ( (*ws.pClipSpaceB)[0] - (*ws.pClipSpaceA)[0] ) * dT;
	const RAS_FLTTYPE dW = (*ws.pClipSpaceA)[1] + ( (*ws.pClipSpaceB)[1] - (*ws.pClipSpaceA)[1] ) * dT;
	
	ws.z = af3d::vec4<>::getdeviceZ(dZ,dW);
}

template <typename PT> void surface::bresenham_line_plot(afdib::dib *pDst,const af3d::zvertex<> *pZScanline,PT *pScanline,const int x, const int y,bresenhamClipSpace& clipspace,const af3d::vec4<>& bgra)const
{
	if(x<0 || x>=pDst->getwidth() || !pScanline)
		return;
	
	if(pZScanline)
	{
		const int nZX=x-m_rDstNDC.get(af3d::rect::v_tl)[0];
		if(nZX>=0 && nZX<m_rDstNDC.getwidth())
		{
			validate(x,y,clipspace);
			if(clipspace.z<pZScanline[nZX].get())
				afdib::pixel<RAS_FLTTYPE>::blend<PT>(pScanline[x],bgra[0],bgra[1],bgra[2],bgra[3]);
			return;
		}
	}
	afdib::pixel<RAS_FLTTYPE>::blend<PT>(pScanline[x],bgra[0],bgra[1],bgra[2],bgra[3]);
}

template <typename PT> bool surface::bresenham_line(int& x, int& y,const int x1, const int y1,const int dx, const int dy,const int sx, const int sy,int& err)const
{
	if(x == x1 && y == y1)
		return false;
	int e2 = 2 * err;
	if(e2 > -dy)
	{
		err -= dy;
		x += sx;
	}
	if(e2 < dx)
	{
		err += dx;
		y += sy;
	}
	return true;
}
