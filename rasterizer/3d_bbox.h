
#include "3d.h"

namespace af3d
{

template <typename T=RAS_FLTTYPE> class facetrnsbbox
{
public:
	using t_flt=T;

	enum planetype {p_front=0x1,p_back=0x2,p_right=0x4,p_left=0x8,p_top=0x10,p_bottom=0x20};
	enum vertextype {v_front_tl,v_front_tr,v_front_br,v_front_bl,v_back_tl,v_back_tr,v_back_br,v_back_bl};

	facetrnsbbox(){}
	template <typename MB> facetrnsbbox(const MB& o,const mat4<T>& trns)
	{
		const vec3<T>& frontminV=o.getfrontmin();
		const vec3<T>& backmaxV=o.getbackmax();
		
		trns.mul({frontminV[0],backmaxV[1],frontminV[2]},m_Vertices[v_front_tl]);
		trns.mul({backmaxV[0],backmaxV[1],frontminV[2]},m_Vertices[v_front_tr]);
		trns.mul({frontminV[0],frontminV[1],frontminV[2]},m_Vertices[v_front_bl]);
		trns.mul({backmaxV[0],frontminV[1],frontminV[2]},m_Vertices[v_front_br]);

		trns.mul({frontminV[0],backmaxV[1],backmaxV[2]},m_Vertices[v_back_tl]);
		trns.mul({backmaxV[0],backmaxV[1],backmaxV[2]},m_Vertices[v_back_tr]);
		trns.mul({frontminV[0],frontminV[1],backmaxV[2]},m_Vertices[v_back_bl]);
		trns.mul({backmaxV[0],frontminV[1],backmaxV[2]},m_Vertices[v_back_br]);
	}
	facetrnsbbox(const facetrnsbbox& o,const mat4<T>& trns){for(int n=0;n<8;++n)trns.mul(o.m_Vertices[n],m_Vertices[n]);}
	facetrnsbbox(const facetrnsbbox& o){*this=o;}
	~facetrnsbbox(){}

	const vec3<T>* getvertices(void)const{return m_Vertices;}
	vec3<T> get(const vertextype from,const vertextype to,const bool bNormalise)const{vec3<T> d=(*this)[to]-(*this)[from];if(bNormalise)return d.normalize();return d;}
	vec3<T> getcentre(void)const{vec3<T> dir=get(v_front_bl,v_back_tr,false);const T dLength = dir.getlength();return (*this)[v_front_bl]+(dir.normalize())*dLength*0.5;}
	vec3<T> getdir(const planetype p,const bool bIn,const bool bNormalise)const
	{
		vec3<T> d;
		switch(p)
		{
			case p_front:d=get(v_front_tl,v_back_tl,bNormalise);break;
			case p_back:d=get(v_back_tl,v_front_tl,bNormalise);break;
			case p_right:d=get(v_front_tr,v_front_tl,bNormalise);break;
			case p_left:d=get(v_front_tl,v_front_tr,bNormalise);break;
			case p_top:d=get(v_front_tl,v_front_bl,bNormalise);break;
			case p_bottom:d=get(v_front_bl,v_front_tl,bNormalise);break;
		}
		if(!bIn)d=-d;
		return d;
	}
	vec3<T> getup(const planetype p,const bool bNormalise)const
	{
		vec3<T> d;
		switch(p)
		{
			case p_front:d=get(v_front_bl,v_front_tl,bNormalise);break;
			case p_back:d=get(v_front_bl,v_front_tl,bNormalise);break;
			case p_right:d=get(v_front_br,v_front_tr,bNormalise);break;
			case p_left:d=get(v_front_bl,v_front_tl,bNormalise);break;
			case p_top:d=get(v_front_tl,v_front_tr,bNormalise);break;
			case p_bottom:d=get(v_front_bl,v_front_br,bNormalise);break;
		}
		return d;
	}		
	__forceinline const t_flt getmaxextent(void)const
	{
		t_flt d0,d1;
		d0=get(v_front_bl,v_front_br,false).getlength();
		d1=get(v_front_bl,v_front_tl,false).getlength();
		if(d1>d0) d0=d1;
		d1=get(v_front_bl,v_back_bl,false).getlength();;
		if(d1<0) { if(-d1>d0) d0=-d1; }
		else if(d1>d0) d0=d1;
		return d0;
	}

	static bool isplanevertex(const planetype p,const vertextype v)
	{
		switch(p)
		{
			case p_front:return v<v_back_tl;
			case p_back:return v>v_front_bl;
			case p_right:return v==v_front_br || v==v_front_tr || v==v_back_br || v==v_back_tr;
			case p_left:return v==v_front_bl || v==v_front_tl || v==v_back_bl || v==v_back_tl;
			case p_top:return v==v_front_tr || v==v_front_tl || v==v_back_tr || v==v_back_tl;
			case p_bottom:return v==v_front_br || v==v_front_bl || v==v_back_br || v==v_back_bl;
		}
		return false;
	}

	__forceinline vec3<t_flt> operator[](const size_t n)const{return m_Vertices[n];}
	__forceinline vec3<t_flt>& operator[](const size_t n){return m_Vertices[n];}
	
	facetrnsbbox& operator=(const facetrnsbbox& o){for(int n=0;n<8;++n)m_Vertices[n]=o.m_Vertices[n];return *this;}
protected:
	vec3<t_flt> m_Vertices[8];
};

template <typename T=RAS_FLTTYPE> class facemodelbbox
{
public:
	using t_flt=T;

	facemodelbbox(){m_FrontMin={0,0,0};m_BackMax={0,0,0};m_Origin={0,0,0};}
	facemodelbbox(const facemodelbbox& o){*this=o;}
	facemodelbbox(const facetrnsbbox<T>& o)
	{
		const vec3<t_flt> *pV=o.getvertices();
		vec3<t_flt> minV(0,0,0),maxV(0,0,0);
		{
			minV=pV[0];
			maxV=pV[0];
			for(int n=1;n<8;++n)
				for(int nC=0;nC<3;++nC)
				{
					const t_flt dComponent=pV[n][nC];
					if(dComponent<minV[nC]) minV[nC]=dComponent;
					if(dComponent>maxV[nC]) maxV[nC]=dComponent;
				}
		}
		#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
			m_FrontMin={minV[0],minV[1],maxV[2]};
			m_BackMax={maxV[0],maxV[1],minV[2]};
		#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
			m_FrontMin=minV;
			m_BackMax=maxV;
		#endif
		m_Origin={m_FrontMin[0]+((m_BackMax[0]-m_FrontMin[0])*0.5),m_FrontMin[1]+((m_BackMax[1]-m_FrontMin[1])*0.5),m_FrontMin[2]+((m_BackMax[2]-m_FrontMin[2])*0.5)};
	}
	facemodelbbox(const vec3<t_flt>& minV,const vec3<t_flt>& maxV)
	{
		#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
			m_FrontMin={minV[0],minV[1],maxV[2]};
			m_BackMax={maxV[0],maxV[1],minV[2]};
		#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
			m_FrontMin=minV;
			m_BackMax=maxV;
		#endif
		m_Origin={m_FrontMin[0]+((m_BackMax[0]-m_FrontMin[0])*0.5),m_FrontMin[1]+((m_BackMax[1]-m_FrontMin[1])*0.5),m_FrontMin[2]+((m_BackMax[2]-m_FrontMin[2])*0.5)};
	}
	facemodelbbox(const std::vector<vec3<T>>& v)
	{
		vec3<t_flt> minV(0,0,0),maxV(0,0,0);
		if(v.size())
		{
			minV=v[0];
			maxV=v[0];
			auto i=v.cbegin(),end=v.cend();
			for(++i;i!=end;++i)
				for(int nC=0;nC<3;++nC)
				{
					const t_flt dComponent=(*i)[nC];
					if(dComponent<minV[nC]) minV[nC]=dComponent;
					if(dComponent>maxV[nC]) maxV[nC]=dComponent;
				}
		}
		#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
			m_FrontMin={minV[0],minV[1],maxV[2]};
			m_BackMax={maxV[0],maxV[1],minV[2]};
		#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
			m_FrontMin=minV;
			m_BackMax=maxV;
		#endif
		m_Origin={m_FrontMin[0]+((m_BackMax[0]-m_FrontMin[0])*0.5),m_FrontMin[1]+((m_BackMax[1]-m_FrontMin[1])*0.5),m_FrontMin[2]+((m_BackMax[2]-m_FrontMin[2])*0.5)};
	}
	template <typename F> facemodelbbox(const std::vector<F>& v)
	{
		vec3<t_flt> minV(0,0,0),maxV(0,0,0);
		if(v.size())
		{
			minV=v[0].getpos().getminpos();
			maxV=v[0].getpos().getmaxpos();
			auto i=v.cbegin(),end=v.cend();
			for(++i;i!=end;++i)
			{
				const auto faceminV=(*i).getpos().getminpos();
				for(int nC=0;nC<3;++nC)
				{
					const t_flt dComponent=faceminV[nC];
					if(dComponent<minV[nC]) minV[nC]=dComponent;
				}
				const auto facemaxV=(*i).getpos().getmaxpos();
				for(int nC=0;nC<3;++nC)
				{
					const t_flt dComponent=facemaxV[nC];
					if(dComponent>maxV[nC]) maxV[nC]=dComponent;
				}
			}
		}
		#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
			m_FrontMin={minV[0],minV[1],maxV[2]};
			m_BackMax={maxV[0],maxV[1],minV[2]};
		#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
			m_FrontMin=minV;
			m_BackMax=maxV;
		#endif
		m_Origin={m_FrontMin[0]+((m_BackMax[0]-m_FrontMin[0])*0.5),m_FrontMin[1]+((m_BackMax[1]-m_FrontMin[1])*0.5),m_FrontMin[2]+((m_BackMax[2]-m_FrontMin[2])*0.5)};
	}
	~facemodelbbox(){}
	__forceinline const t_flt getwidth(void)const{return m_BackMax[0]-m_FrontMin[0];}
	__forceinline const t_flt getheight(void)const{return m_BackMax[1]-m_FrontMin[1];}
	__forceinline const t_flt getdepth(void)const
	{
		#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
			return m_FrontMin[2]-m_BackMax[2];
		#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
			return m_BackMax[2]-m_FrontMin[2];
		#endif
	}
	__forceinline const vec3<t_flt>& getfrontmin(void)const{return m_FrontMin;}
	__forceinline const vec3<t_flt>& getbackmax(void)const{return m_BackMax;}
	__forceinline const vec3<t_flt>& getorigin(void)const{return m_Origin;}
	__forceinline const t_flt getminextent(void)const
	{
		t_flt d0,d1;
		d0=getwidth(); 
		d1=getheight();
		if(d1<d0) d0=d1;
		d1=getdepth();
		if(d1<0) { if(-d1<d0) d0=-d1; }
		else if(d1<d0) d0=d1;
		return d0;
	}
	__forceinline const t_flt getmaxextent(void)const
	{
		t_flt d0,d1;
		d0=getwdth(); 
		d1=getheight();
		if(d1>d0) d0=d1;
		d1=getdepth();
		if(d1<0) { if(-d1>d0) d0=-d1; }
		else if(d1>d0) d0=d1;
		return d0;
	}
	facemodelbbox getunion(const facemodelbbox<T>& o)const
	{
		facemodelbbox u;
		u.m_FrontMin[0]=std::min<>(m_FrontMin[0],o.m_FrontMin[0]);
		u.m_FrontMin[1]=std::min<>(m_FrontMin[1],o.m_FrontMin[1]);
		u.m_BackMax[0]=std::max<>(m_BackMax[0],o.m_BackMax[0]);
		u.m_BackMax[1]=std::max<>(m_BackMax[1],o.m_BackMax[1]);
		#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
			u.m_FrontMin[2]=std::max<>(m_FrontMin[2],o.m_FrontMin[2]);
			u.m_BackMax[2]=std::min<>(m_BackMax[2],o.m_BackMax[2]);
		#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
			u.m_FrontMin[2]=std::min<>(m_FrontMin[2],o.m_FrontMin[2]);
			u.m_BackMax[2]=std::max<>(m_BackMax[2],o.m_BackMax[2]);
		#endif
		u.m_Origin={m_FrontMin[0]+((m_BackMax[0]-m_FrontMin[0])*0.5),m_FrontMin[1]+((m_BackMax[1]-m_FrontMin[1])*0.5),m_FrontMin[2]+((m_BackMax[2]-m_FrontMin[2])*0.5)};
		return u;
	}
	
	facemodelbbox& operator=(const facemodelbbox& other){m_Origin=other.m_Origin;m_FrontMin=other.m_FrontMin;m_BackMax=other.m_BackMax;return *this;}
protected:
	vec3<t_flt> m_FrontMin;
	vec3<t_flt> m_BackMax;
	vec3<t_flt> m_Origin;
};

}
