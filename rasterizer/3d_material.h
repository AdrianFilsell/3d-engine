#pragma once

#include "3d_face.h"
#include "3d_tbuffer.h"
#include "dib.h"

namespace af3d
{

template <typename T=RAS_FLTTYPE> class materialcol
{
public:
	materialcol()
	{
		m_Diffuse=getdefdiffuse();
		m_Ambient=getdefambient();
		m_Specular=getdefspecular();
		m_Shininess=getdefshininess();
	}
	materialcol(const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const T dShininess):m_Diffuse(diffuse),m_Ambient(ambient),m_Specular(specular),m_Shininess(dShininess){}
	materialcol(const vec3<T>& diffuse,const vec3<T>& ambient,const vec3<T>& specular,const af::nonnegpowerexp<T>& shininess):m_Diffuse(diffuse),m_Ambient(ambient),m_Specular(specular),m_Shininess(shininess){}
	__forceinline const vec3<T>& getdiffuse(void)const{return m_Diffuse;}
	__forceinline const vec3<T>& getambient(void)const{return m_Ambient;}
	__forceinline const vec3<T>& getspecular(void)const{return m_Specular;}
	__forceinline const af::nonnegpowerexp<T>& getshininess(void)const{return m_Shininess;}
	__forceinline void setdiffuse(const vec3<T>& c){m_Diffuse=c;}
	__forceinline void setambient(const vec3<T>& c){m_Ambient=c;}
	__forceinline void setspecular(const vec3<T>& c){m_Specular=c;}
	__forceinline void setshininess(const T d){m_Shininess=d<0?0:d;}
	
	__forceinline bool getquantize(void)const{return m_DiffuseQuantize.isempty() && m_SpecularQuantize.isempty() ? false : true;}
	__forceinline const quantize_static_3<T>& getdiffusequantize(void)const{return m_DiffuseQuantize;}
	__forceinline const quantize_static_3<T>& getspecularquantize(void)const{return m_SpecularQuantize;}
	__forceinline void setdiffusequantize(const quantize_static_3<T>& q){m_DiffuseQuantize=q;}
	__forceinline void setspecularquantize(const quantize_static_3<T>& q){m_SpecularQuantize=q;}
	
	__forceinline static const vec3<T>& getdefdiffuse(void){static const vec3<T> c(1,1,1);return c;}
	__forceinline static const vec3<T>& getdefambient(void){static const vec3<T> c(1,1,1);return c;}
	__forceinline static const vec3<T>& getdefspecular(void){static const vec3<T> c(1,1,1);return c;}
	__forceinline static const af::nonnegpowerexp<T>& getdefshininess(void){return s_DefShininess;}

	materialcol<T>& operator =(const materialcol<T>& o)
	{
		m_Diffuse=o.m_Diffuse;
		m_Ambient=o.m_Ambient;
		m_Specular=o.m_Specular;
		m_Shininess=o.m_Shininess;
		m_DiffuseQuantize=o.m_DiffuseQuantize;
		m_SpecularQuantize=o.m_SpecularQuantize;
		return *this;
	}
protected:
	vec3<T> m_Diffuse;
	vec3<T> m_Ambient;
	vec3<T> m_Specular;
	// Specular exponent -	the lower the value the more fragments will be affected but the effect will less intense
	//						the higher the value the fewer fragments will be affected but the affect will be more intense and concentrated
	// low shininess		0-10		rough or matte surfaces
	// medium shininess		10-50		semi glossy ie plastic/matte metal
	// high shininess		50-100		glossy ie polished metals/shiny plastics
	// v.high				100+		mirror like surfaces
	af::nonnegpowerexp<T> m_Shininess;
	static const af::nonnegpowerexp<T> s_DefShininess;
	quantize_static_3<T> m_DiffuseQuantize;
	quantize_static_3<T> m_SpecularQuantize;
};

template <typename T> const af::nonnegpowerexp<T> materialcol<T>::s_DefShininess(5);

class materialdib
{
public:
	materialdib()
	{
		m_nBytesPerScanline=0;
		m_nBytesPerPixel=0;
		m_pScanlines=nullptr;
		m_nWidth=0;
		m_nHeight=0;
	}
	materialdib(std::shared_ptr<const afdib::dib> sp):materialdib()
	{
		if(sp)
		{
			m_sp=sp;
			m_nBytesPerScanline=sp->getbytesperscanline();
			m_nBytesPerPixel=sp->getbytesperpixel();
			m_pScanlines=sp->getscanline(0);
			m_nWidth=sp->getwidth();
			m_nHeight=sp->getheight();
		}
	}
	materialdib(const rect& rSrc,std::shared_ptr<const afdib::dib> sp):materialdib(sp?sp->cut(rSrc.get(rect::v_tl)[0],rSrc.get(rect::v_tl)[1],rSrc.getwidth(),rSrc.getheight()):nullptr){}
	__forceinline bool isempty(void)const{return !m_pScanlines;}
	__forceinline const unsigned char *getscanlines(void)const{return m_pScanlines;}
	__forceinline int getbytesperscanline(void)const{return m_nBytesPerScanline;}
	__forceinline int getbytesperpixel(void)const{return m_nBytesPerPixel;}
	__forceinline int getwidth(void)const{return m_nWidth;}
	__forceinline int getheight(void)const{return m_nHeight;}
	template <typename T,typename PT,typename TEX> __forceinline void getcolour(const vec2<T>& src,TEX& dst)const
	{
		const unsigned char *pScanlines=getscanlines();
		const int nX=af::postodiscrete(af::postodiscrete(src[0]*(getwidth()-1.0)));
		const int nY=af::postodiscrete(af::postodiscrete(src[1]*(getheight()-1.0)));
		const unsigned char *pBGR=pScanlines+(nY*getbytesperscanline())+(getbytesperpixel()*nX);
		dst[0]=pBGR[0]*PT::getmaxrecip<T>();
		dst[1]=pBGR[1]*PT::getmaxrecip<T>();
		dst[2]=pBGR[2]*PT::getmaxrecip<T>();
	}
protected:
	std::shared_ptr<const afdib::dib> m_sp;
	const unsigned char *m_pScanlines;
	int m_nBytesPerScanline;
	int m_nBytesPerPixel;
	int m_nWidth;
	int m_nHeight;
};

template <typename T=RAS_FLTTYPE> class materialnormalmap:public tbuffer<vec3<T>>
{
public:
	materialnormalmap(){}
	materialnormalmap(std::shared_ptr<const afdib::dib> sp)
	{
		m_bLeftHanded=true; //assumption
		if(sp)
			switch(sp->getpixeltype())
			{
				case afdib::dib::pt_b8g8r8:create<afdib::b8g8r8>(sp);break;
				case afdib::dib::pt_b8g8r8a8:create<afdib::b8g8r8a8>(sp);break;
			}
	}
	__forceinline void getdelta(const vec3<T>& src,vec3<T>& dst)const
	{
		const int nX=af::postodiscrete(af::postodiscrete(src[0]*(getwidth()-1.0)));
		const int nY=af::postodiscrete(af::postodiscrete(src[1]*(getheight()-1.0)));
		const vec3<T> *pScanline=getscanline(nY);
		dst[0]=pScanline[nX][0];
		dst[1]=pScanline[nX][1];
		dst[2]=pScanline[nX][2];
	}
	__forceinline void perturb(const vec2<T>& src,const vec3<T>& from,vec3<T>& dst)const
	{
		const int nX=af::postodiscrete(af::postodiscrete(src[0]*(getwidth()-1.0)));
		const int nY=af::postodiscrete(af::postodiscrete(src[1]*(getheight()-1.0)));
		const vec3<T> *pScanline=getscanline(nY);
		dst[0]=from[0] + pScanline[nX][0];
		dst[1]=from[1] + pScanline[nX][1];
		dst[2]=from[2] + pScanline[nX][2];
		dst.normalize();
	}
protected:
	bool m_bLeftHanded;
	template <typename P> void create(std::shared_ptr<const afdib::dib> sp)
	{
		tbuffer<vec3<T>>::create(sp->getwidth(),sp->getheight());
		for(int nY=0;nY<getheight();++nY)
		{
			vec3<T> *pDst=getscanline(nY);
			const P *pSrc=reinterpret_cast<const P*>(sp->getscanline(nY));
			for(int nX=0;nX<getwidth();++nX)
			{
				// perturb normal
				
				// [0,1] to [-1,1]
				pDst[nX][0] = ( pSrc[nX].b * P::getmaxrecip<T>() * 2.0 ) - 1.0;
				pDst[nX][1] = ( pSrc[nX].g * P::getmaxrecip<T>() * 2.0 ) - 1.0;
				pDst[nX][2] = ( pSrc[nX].r * P::getmaxrecip<T>() * 2.0 ) - 1.0;
				#if (RAS_PARADIGM == RAS_DX_PARADIGM)
					constexpr bool bLeftHanded=true;
				#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
					constexpr bool bLeftHanded=false;
				#endif
				const bool bHandedMismatch=bLeftHanded != m_bLeftHanded;
				if(bHandedMismatch)
				{
					// the author assumed right or left handed'ness usage and we do not use that handed'ness
					pDst[nX][2] = -pDst[nX][2];
				}
			}
		}
	}
};

class materialcubicenvmap
{
public:
	enum tiletype{tt_infront=0,tt_behind,tt_left,tt_right,tt_above,tt_below};
	materialcubicenvmap(const rect& rTile,std::shared_ptr<const afdib::dib> sp)
	{
		m_bEmpty=true;
		if(sp)
		{
			const int nTileDim=sp->getwidth()/4;
			if(nTileDim>0 && sp->getheight()>=(nTileDim*3))
			{
				m_Faces[tt_left]=materialdib({{0,nTileDim},{nTileDim,2*nTileDim}},sp);
				m_bEmpty=m_Faces[tt_left].isempty();
				if(!m_bEmpty){m_Faces[tt_infront]=materialdib({{nTileDim,nTileDim},{2*nTileDim,2*nTileDim}},sp);m_bEmpty=m_Faces[tt_infront].isempty();}
				if(!m_bEmpty){m_Faces[tt_right]=materialdib({{2*nTileDim,nTileDim},{3*nTileDim,2*nTileDim}},sp);m_bEmpty=m_Faces[tt_right].isempty();}
				if(!m_bEmpty){m_Faces[tt_behind]=materialdib({{3*nTileDim,nTileDim},{4*nTileDim,2*nTileDim}},sp);m_bEmpty=m_Faces[tt_behind].isempty();}
				if(!m_bEmpty){m_Faces[tt_above]=materialdib({{nTileDim,0},{2*nTileDim,nTileDim}},sp);m_bEmpty=m_Faces[tt_above].isempty();}
				if(!m_bEmpty){m_Faces[tt_below]=materialdib({{nTileDim,2*nTileDim},{2*nTileDim,3*nTileDim}},sp);m_bEmpty=m_Faces[tt_below].isempty();}
			}
		}
	}
	__forceinline bool isempty(void)const{return m_bEmpty;}
	__forceinline const materialdib& getface(const tiletype t)const{return m_Faces[t];}
	template <typename T,typename PT,typename TEX> __forceinline void getcolour(const vec3<T>& campos,const vec3<T>& fragpos,const vec3<T>& fragnorm,TEX& dst)const
	{
//		vec3<T> I;
//		I[0]=campos[0]-fragpos[0];I[1]=campos[1]-fragpos[1];I[2]=campos[2]-fragpos[2];
//		I.normalize();

		vec3<T> negI;
		negI[0]=fragpos[0]-campos[0];negI[1]=fragpos[1]-campos[1];negI[2]=fragpos[2]-campos[2];
		negI.normalize();

		vec3<T> R;
		light<T>::reflect(negI,fragnorm,R);
		R.normalize();

		vec3<T> absdir;
		const bool bNegX=R[0]<0.0;const bool bNegY=R[1]<0.0;const bool bNegZ=R[2]<0.0;
		absdir[0]=bNegX?-R[0]:R[0];absdir[1]=bNegY?-R[1]:R[1];absdir[2]=bNegZ?-R[2]:R[2];

		vec2<T> uv;tiletype face;T max=absdir[0];
		if(bNegX)
			face=tt_left;
		else
			face=tt_right;
		if(absdir[1]>max)
		{
			max=absdir[1];
			if(bNegY)
				face=tt_below;
			else
				face=tt_above;
		}
		if(absdir[2]>max)
		{
			#if(RAS_PARADIGM==RAS_DX_PARADIGM)
				if(bNegZ)
					face=tt_behind;
				else
					face=tt_infront;
			#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
				if(bNegZ)
					face=tt_infront;
				else
					face=tt_behind;
			#endif
		}

		if(face<tt_left)
		{
			if(face==tt_infront)
				uv[0]=R[0]/absdir[2];
			else
				uv[0]=-R[0]/absdir[2];
			uv[1]=-R[1]/absdir[2];
		}
		else
		if(face<tt_above)
		{
			#if(RAS_PARADIGM==RAS_DX_PARADIGM)
				if(face==tt_left)
					uv[0]=R[2]/absdir[0];
				else
					uv[0]=-R[2]/absdir[0];
			#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
				if(face==tt_left)
					uv[0]=-R[2]/absdir[0];
				else
					uv[0]=R[2]/absdir[0];
			#endif
			uv[1]=-R[1]/absdir[0];
		}
		else
		{
			#if(RAS_PARADIGM==RAS_DX_PARADIGM)
				if(face==tt_above)
					uv[1]=R[2]/absdir[1];
				else
					uv[1]=-R[2]/absdir[1];
			#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
				if(face==tt_above)
					uv[1]=-R[2]/absdir[1];
				else
					uv[1]=R[2]/absdir[1];
			#endif
			uv[0]=R[0]/absdir[1];
		}

		uv[0]=0.5*uv[0] + 0.5;
		uv[1]=0.5*uv[1] + 0.5;
		if(uv[0]<0)
			uv[0]=0;
		else
		if(uv[0]>1)
			uv[0]=1;
		if(uv[1]<0)
			uv[1]=0;
		else
		if(uv[1]>1)
			uv[1]=1;

		m_Faces[face].getcolour<T,PT>(uv,dst);
	}
protected:
	bool m_bEmpty;
	materialdib m_Faces[6];
};

template <typename T=RAS_FLTTYPE> class material
{
public:
	material():m_nAtts(0),m_nAttSize(0),m_nEnabledAttSize(0),m_nFrom(0),m_nInclusiveTo(-1),m_nEnabledAtts(face_vertex_att::t_material){}
	material(const int nFrom,const int nInclusiveTo):m_nAtts(0),m_nAttSize(0),m_nFrom(nFrom),m_nInclusiveTo(nInclusiveTo){}
	material(const material& o):material(){*this=o;}
	~material(){}

	std::shared_ptr<material<T>> clone(void)const{std::shared_ptr<material<T>> sp(new material<T>);*sp=*this;return sp;}

	__forceinline int getatts(const bool bEnabled)const{return bEnabled?(m_nAtts&m_nEnabledAtts):m_nAtts;}
	__forceinline int getenabledatts(void)const{return m_nEnabledAtts;}
	__forceinline int getattsize(const bool bEnabled)const{return bEnabled?m_nEnabledAttSize:m_nAttSize;}
	__forceinline int getfrom(void)const{return m_nFrom;}
	__forceinline int getinclusiveto(void)const{return m_nInclusiveTo;}

	__forceinline const materialcol<T>& getcol(void)const{return m_Col;}
	__forceinline std::shared_ptr<const materialdib> gettex(void)const{return m_spTex;}
	__forceinline std::shared_ptr<const materialnormalmap<T>> getbump(void)const{return m_spBump;}
	__forceinline std::shared_ptr<const materialcubicenvmap> getcubicenv(void)const{return m_spCubicEnv;}
	const std::wstring& gettexpath(void)const{return m_TexPath;}
	const std::wstring& getbumppath(void)const{return m_BumpPath;}
	const std::wstring& getcubicenvpath(void)const{return m_CubicEnvPath;}

	void setrange(const int nFrom,const int nInclusiveTo){m_nFrom=nFrom;m_nInclusiveTo=nInclusiveTo;}
	void enable(const int n,const bool b)
	{
		if(b)
			m_nEnabledAtts|=n&face_vertex_att::t_material;
		else
			m_nEnabledAtts&=~(n&face_vertex_att::t_material);
		updateenabledattsize();
	}
	void clear(const int n)
	{
		const int nClear=n&m_nAtts;
		if(nClear & face_vertex_att::t_col)
		{
			m_nAtts&=~face_vertex_att::t_col;
			m_Col=materialcol<T>();
		}
		if(nClear & face_vertex_att::t_env_cubic)
		{
			m_nAtts&=~face_vertex_att::t_env_cubic;
			m_spCubicEnv=nullptr;
		}
		if(nClear & face_vertex_att::t_tex)
		{
			m_nAtts&=~face_vertex_att::t_tex;
			m_spTex=nullptr;
		}
		if(nClear & face_vertex_att::t_bump)
		{
			m_nAtts&=~face_vertex_att::t_bump;
			m_spBump=nullptr;
		}
		updateenabledattsize();
	}
	void setcol(const materialcol<T>& c){m_Col=c;m_nAtts|=face_vertex_att::t_col;updateenabledattsize();}
	void settex(const std::wstring& path,std::shared_ptr<const materialdib> sp)
	{
		m_TexPath=L"";
		m_spTex=sp&&!sp->isempty()?sp:nullptr;
		if(!!sp&&!sp->isempty())
		{
			m_TexPath=path;
			m_nAtts|=face_vertex_att::t_tex;
		}
		else
			m_nAtts&=~face_vertex_att::t_tex;
		updateenabledattsize();
	}
	void setbump(const std::wstring& path,std::shared_ptr<const materialnormalmap<T>> sp)
	{
		m_BumpPath=L"";
		m_spBump=sp&&!sp->isempty()?sp:nullptr;
		if(!!sp&&!sp->isempty())
		{
			m_nAtts|=face_vertex_att::t_bump;
			m_BumpPath=path;
		}
		else
			m_nAtts&=~face_vertex_att::t_bump;
		updateenabledattsize();
	}
	void setcubicenv(const std::wstring& path,std::shared_ptr<const materialcubicenvmap> sp)
	{
		m_CubicEnvPath=L"";
		m_spCubicEnv=sp&&!sp->isempty()?sp:nullptr;
		if(!!sp&&!sp->isempty())
		{
			m_nAtts|=face_vertex_att::t_env_cubic;
			m_CubicEnvPath=path;
		}
		else
			m_nAtts&=~face_vertex_att::t_env_cubic;
		updateenabledattsize();
	}

	material<T>& operator =(const material<T>& o)
	{
		m_nAtts=o.m_nAtts;
		m_nEnabledAtts=o.m_nEnabledAtts;
		m_nAttSize=o.m_nAttSize;
		m_nEnabledAttSize=o.m_nEnabledAttSize;
		m_nFrom=o.m_nFrom;
		m_nInclusiveTo=o.m_nInclusiveTo;
		m_Col=o.m_Col;
		m_spTex=o.m_spTex;
		m_spBump=o.m_spBump;
		m_spCubicEnv=o.m_spCubicEnv;
		m_TexPath=o.m_TexPath;
		m_BumpPath=o.m_BumpPath;
		m_CubicEnvPath=o.m_CubicEnvPath;
		return *this;
	}
protected:
	int m_nAtts;
	int m_nEnabledAtts;
	int m_nAttSize;
	int m_nEnabledAttSize;
	int m_nFrom;
	int m_nInclusiveTo;
	materialcol<T> m_Col;
	std::shared_ptr<const materialdib> m_spTex;
	std::shared_ptr<const materialnormalmap<T>> m_spBump;
	std::shared_ptr<const materialcubicenvmap> m_spCubicEnv;
	std::wstring m_TexPath;
	std::wstring m_BumpPath;
	std::wstring m_CubicEnvPath;
	__forceinline void updateenabledattsize(void)
	{
		m_nAttSize=0;
		m_nEnabledAttSize=0;
		const std::vector<face_vertex_att::type> v={face_vertex_att::t_col,face_vertex_att::t_tex,face_vertex_att::t_bump,face_vertex_att::t_env_cubic};
		auto i=v.cbegin(),end=v.cend();
		for(;i!=end;++i)
			if(m_nAtts&(*i))
			{
				++m_nAttSize;
				if(m_nEnabledAtts&(*i))
					++m_nEnabledAttSize;
			}
	}
};

}
