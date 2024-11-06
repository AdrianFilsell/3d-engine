#pragma once

#include "3d.h"
#include "3d_bbox.h"
#include "thread_taskscheduler.h"

namespace af3d
{

class face_vertex_att
{
public:
	enum type {t_pos=0x1,t_norm=0x2,
			   t_col=0x4,
			   t_tex=0x8,t_env_cubic=0x10,
			   t_bump=0x20,
			   
			   t_visible=(t_col|t_tex|t_env_cubic),
			   t_material=(t_col|t_tex|t_env_cubic|t_bump),
			   t_all=(t_pos|t_norm|t_col|t_tex|t_env_cubic|t_bump)};
};

template <typename T=RAS_FLTTYPE,int ELEMENTS=3> class vertex_data
{
public:
	using t_data=T;

	struct vertex
	{
		T t;
		__forceinline void set(const vertex_data& src,const size_t n){t=(src[n]);}
		__forceinline void lerp(const vertex& from,const vertex& to,const T::template t_flt dT){t.lerp(from.t,to.t,dT);}
		__forceinline void barylerp(const vertex_data& src,const T::template t_flt dAlphaWa,const T::template t_flt dBetaWb,const T::template t_flt dGammaWc,const T::template t_flt dRecipWp){t.barylerp(dAlphaWa,dBetaWb,dGammaWc,dRecipWp,src[0],src[1],src[2]);}
		__forceinline void normalize(void){t.normalize();}
	};

	vertex_data(){}
	vertex_data(const T& a,const T& b){m_v[0]=a;m_v[1]=b;}
	vertex_data(const T& a,const T& b,const T& c){m_v[0]=a;m_v[1]=b;m_v[2]=c;}
	__forceinline static int size(void){return ELEMENTS;}
	__forceinline void set(const vertex& a,const vertex& b,const vertex& c){m_v[0]=a.t;m_v[1]=b.t;m_v[2]=c.t;}
	__forceinline const T& operator[](const size_t n)const{return m_v[n];}
	__forceinline T& operator[](const size_t n){return m_v[n];}
	__forceinline vertex_data& operator =(const vertex_data& o){m_v[0]=o.m_v[0];m_v[1]=o.m_v[1];m_v[2]=o.m_v[2];return *this;}
protected:
	T m_v[ELEMENTS];
};

class vertex_data_singular // ie are all 3 face vertices have colour/normal/etc
{
public:
	enum type{t_unknown=2,t_true=1,t_false=0};
	vertex_data_singular(){m_s=t_unknown;}
	template <typename T> vertex_data_singular(const T& a,const T& b,const T& c){m_s=(a==b&&a==c)?t_true:t_false;}
	~vertex_data_singular(){}
	__forceinline type get(void)const{return m_s;}	
	__forceinline vertex_data_singular& operator =(const vertex_data_singular& o){m_s=o.m_s;return *this;}
protected:
	type m_s;
};

template <typename T> class line_pos_vertex_data
{
public:
	using t_pos=T;
	using t_flt=T::template t_flt;

	struct vertex
	{
		vertex_data<T,2>::template vertex pos;
		__forceinline void set(const line_pos_vertex_data& src,const size_t n){pos.set(src.getpos(),n);}
		__forceinline void lerp(const line_pos_vertex_data& src,const vertex& from,const vertex& to,const T::template t_flt t){pos.lerp(from.pos,to.pos,t);}
	};
	
	line_pos_vertex_data(){}
	line_pos_vertex_data(const vertex_data<T,2>& p):m_Pos(p){}
	__forceinline const vertex_data<T,2>& getpos(void)const{return m_Pos;}
	__forceinline vertex_data<T,2>& getpos(void){return m_Pos;}
	__forceinline void xform(const mat4<t_flt>& modeltoworld,const mat4<t_flt>& modeltoclipspace,
							 line_pos_vertex_data<vec4<T::template t_flt>>& dst)const{xform_pos4(modeltoclipspace,dst.getpos()[0],dst.getpos()[1]);}
	__forceinline void set(const line_pos_vertex_data& src,const vertex& a,const vertex& b){m_Pos.set(a.pos,b.pos);}
	__forceinline line_pos_vertex_data& operator =(const line_pos_vertex_data& o){m_Pos=o.m_Pos;return *this;}
	__forceinline static int size(void){return vertex_data<T,2>::size(); }

	__forceinline void xform_pos4(const mat4<T::template t_flt>& xfm,
								  vec4<T::template t_flt>& oA,vec4<T::template t_flt>& oB)const
	{
		xfm.mul(m_Pos[0],oA);
		xfm.mul(m_Pos[1],oB);
	}
protected:
	vertex_data<T,2> m_Pos;
};

template <typename T> class face_pos_vertex_data
{
public:
	using t_pos=T;
	using t_flt=T::template t_flt;

	struct vertex
	{
		vertex_data<T>::template vertex pos;
		__forceinline void set(const face_pos_vertex_data& src,const size_t n){pos.set(src.getpos(),n);}
		__forceinline void lerp(const face_pos_vertex_data& src,const vertex& from,const vertex& to,const T::template t_flt t){pos.lerp(from.pos,to.pos,t);}
		__forceinline void barylerp(const face_pos_vertex_data& src,const T::template t_flt dAlphaWa,const T::template t_flt dBetaWb,const T::template t_flt dGammaWc,const T::template t_flt dRecipWp){pos.barylerp(src.getpos(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);}
	};
	
	face_pos_vertex_data(){}
	face_pos_vertex_data(const vertex_data<T>& p):m_Pos(p){}
	__forceinline const vertex_data<T>& getpos(void)const{return m_Pos;}
	__forceinline t_pos getminpos(void)const
	{
		t_pos p;
		for(int nComp=0;nComp<3;++nComp)
		{
			p[nComp]=m_Pos[0][nComp];
			if(m_Pos[1][nComp]<p[nComp])
				p[nComp]=m_Pos[1][nComp];
			if(m_Pos[2][nComp]<p[nComp])
				p[nComp]=m_Pos[2][nComp];
		}
		return p;
	}
	__forceinline t_pos getmaxpos(void)const
	{
		t_pos p;
		for(int nComp=0;nComp<3;++nComp)
		{
			p[nComp]=m_Pos[0][nComp];
			if(m_Pos[1][nComp]>p[nComp])
				p[nComp]=m_Pos[1][nComp];
			if(m_Pos[2][nComp]>p[nComp])
				p[nComp]=m_Pos[2][nComp];
		}
		return p;
	}
	__forceinline vertex_data<T>& getpos(void){return m_Pos;}
	__forceinline void xform(const mat4<t_flt>& modeltoworld,const mat4<t_flt>& transposedinversemodeltoworld,const mat4<t_flt>& modeltoclipspace,
													 face_pos_vertex_data<vec4<T::template t_flt>>& dst)const{xform_pos4(modeltoclipspace,dst.getpos()[0],dst.getpos()[1],dst.getpos()[2]);}
	__forceinline void set(const face_pos_vertex_data& src,const vertex& a,const vertex& b,const vertex& c){m_Pos.set(a.pos,b.pos,c.pos);}
	__forceinline face_pos_vertex_data& operator =(const face_pos_vertex_data& o){m_Pos=o.m_Pos;return *this;}
	__forceinline static face_vertex_att::type getatt(void){return face_vertex_att::t_pos;}
	__forceinline static int size(void){return vertex_data<T>::size(); }

	__forceinline void xform_pos4(const mat4<T::template t_flt>& xfm,
								  vec4<T::template t_flt>& oA,vec4<T::template t_flt>& oB,vec4<T::template t_flt>& oC)const
	{
		xfm.mul(m_Pos[0],oA);
		xfm.mul(m_Pos[1],oB);
		xfm.mul(m_Pos[2],oC);
	}
protected:
	vertex_data<T> m_Pos;
};

template <typename T=RAS_FLTTYPE> class face_col_vertex_data
{
public:
	struct vertex
	{
		vertex_data<vec3<T>>::template vertex col;
		__forceinline void set(const face_col_vertex_data& src,const size_t n)
		{
			if(src.getsingular().get()!=vertex_data_singular::t_true)
				col.set(src.getcol(),n);
		}
		__forceinline void lerp(const face_col_vertex_data& src,const vertex& from,const vertex& to,const T t)
		{
			if(src.getsingular().get()!=vertex_data_singular::t_true)
				col.lerp(from.col,to.col,t);
		}
		 __forceinline void barylerp(const face_col_vertex_data& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			if(src.getsingular().get()!=vertex_data_singular::t_true)
				col.barylerp(src.getcol(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_col_vertex_data(){}
	face_col_vertex_data(const vertex_data<vec3<T>>& c):m_Col(c),m_Singular(c[0],c[1],c[2]){}
	__forceinline vertex_data<vec3<T>>& getcol(void){return m_Col;}
	__forceinline const vertex_data<vec3<T>>& getcol(void)const{return m_Col;}
	__forceinline const vertex_data_singular& getsingular(void)const{return m_Singular;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,face_col_vertex_data& dst)const
	{
		dst.m_Col=m_Col;
		dst.m_Singular=m_Singular;
	}
	 __forceinline void set(const face_col_vertex_data& src,const vertex& a,const vertex& b,const vertex& c)
	{
		if(src.getsingular().get()!=vertex_data_singular::t_true)
			m_Col.set(a.col,b.col,c.col);
		m_Singular=src.getsingular();
	}
	__forceinline void setsingular(void){m_Singular=vertex_data_singular(m_Col[0],m_Col[1],m_Col[2]);}
	__forceinline face_col_vertex_data& operator =(const face_col_vertex_data& o){m_Col=o.m_Col;m_Singular=o.m_Singular;return *this;}
	__forceinline static face_vertex_att::type getatt(void){return face_vertex_att::t_col;}
protected:
	vertex_data<vec3<T>> m_Col;
	vertex_data_singular m_Singular;
};

template <typename T=RAS_FLTTYPE> class face_norm_vertex_data
{
public:
	enum type {t_null,t_geometric,t_average};
	struct vertex
	{
		vertex_data<vec3<T>>::template vertex norm;
		__forceinline void set(const face_norm_vertex_data& src,const size_t n)
		{
			if(src.getsingular().get()!=vertex_data_singular::t_true)
				norm.set(src.getnorm(),n);
		}
		__forceinline void lerp(const face_norm_vertex_data& src,const vertex& from,const vertex& to,const T t)
		{
			if(src.getsingular().get()!=vertex_data_singular::t_true)
			{
				norm.lerp(from.norm,to.norm,t);
				norm.normalize();
			}
		}
		__forceinline void barylerp(const face_norm_vertex_data& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			if(src.getsingular().get()!=vertex_data_singular::t_true)
			{
				norm.barylerp(src.getnorm(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
				norm.normalize();
			}
		}
	};

	face_norm_vertex_data(){}
	face_norm_vertex_data(const vertex_data<vec3<T>>& n):m_Norm(n),m_Singular(n[0],n[1],n[2]){}
	__forceinline const vertex_data<vec3<T>>& getnorm(void)const{return m_Norm;}
	__forceinline vertex_data<vec3<T>>& getnorm(void){return m_Norm;}
	__forceinline const vertex_data_singular& getsingular(void)const{return m_Singular;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,face_norm_vertex_data& dst)const
	{
		transposedinversemodeltoworld.mul(m_Norm[0],dst.m_Norm[0]);
		dst.m_Norm[0].normalize();
		if(getsingular().get()!=vertex_data_singular::t_true)
		{
			transposedinversemodeltoworld.mul(m_Norm[1],dst.m_Norm[1]);
			transposedinversemodeltoworld.mul(m_Norm[2],dst.m_Norm[2]);
			dst.m_Norm[1].normalize();
			dst.m_Norm[2].normalize();
		}
		dst.m_Singular=m_Singular;
	}
	__forceinline void set(const face_norm_vertex_data& src,const vertex& a,const vertex& b,const vertex& c)
	{
		if(src.getsingular().get()!=vertex_data_singular::t_true)
			m_Norm.set(a.norm,b.norm,c.norm);
		else
			m_Norm[0]=src.getnorm()[0];
		m_Singular=src.getsingular();
	}
	__forceinline void setsingular(void){m_Singular=vertex_data_singular(m_Norm[0],m_Norm[1],m_Norm[2]);}
	__forceinline face_norm_vertex_data& operator =(const face_norm_vertex_data& o){m_Norm=o.m_Norm;return *this;}
	__forceinline static face_vertex_att::type getatt(void){return face_vertex_att::t_norm;}
	__forceinline static vec3<T> geometricfacenormal(const vertex_data<vec3<T>>& p){const vec3<T> btoa=p[0]-p[1];const vec3<T> btoc=p[2]-p[1];return btoc.cross(btoa).normalize();}
	__forceinline static vec3<T> averagefacenormal(const vertex_data<vec3<T>>& n){return (n[0]+n[1]+n[2]).normalize();}
protected:
	vertex_data<vec3<T>> m_Norm;
	vertex_data_singular m_Singular;
};

template <typename T=RAS_FLTTYPE> class face_tex_vertex_data
{
public:
	enum type {t_null,t_flat,t_cylindrical,t_spherical};

	struct vertex
	{
		vertex_data<vec2<T>>::template vertex tex;
		__forceinline void set(const face_tex_vertex_data& src,const size_t n){tex.set(src.gettex(),n);}
		__forceinline void lerp(const face_tex_vertex_data& src,const vertex& from,const vertex& to,const T t){tex.lerp(from.tex,to.tex,t);}
		__forceinline void barylerp(const face_tex_vertex_data& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp){tex.barylerp(src.gettex(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);}
	};

	face_tex_vertex_data(){}
	face_tex_vertex_data(const vertex_data<vec2<T>>& t):m_Tex(t){}
	__forceinline vertex_data<vec2<T>>& gettex(void){return m_Tex;}
	__forceinline const vertex_data<vec2<T>>& gettex(void)const{return m_Tex;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,face_tex_vertex_data& dst)const{dst.m_Tex=m_Tex;}
	__forceinline void set(const face_tex_vertex_data& src,const vertex& a,const vertex& b,const vertex& c){m_Tex.set(a.tex,b.tex,c.tex);}
	__forceinline face_tex_vertex_data& operator =(const face_tex_vertex_data& o){m_Tex=o.m_Tex;return *this;}
	__forceinline static face_vertex_att::type getatt(void){return face_vertex_att::t_tex;}

	template <type UV> __forceinline static void getuv(const facemodelbbox<T>& bbox,const vec3<T>& p,vec2<T>& uv)
	{
		switch(UV)
		{
			case t_flat:
			{
				const vec3<T> norm=p-bbox.getfrontmin();
				uv[0] = norm[0]/bbox.getwidth();
				uv[1] = 1.0-(norm[1]/bbox.getheight());
			}
			break;
			case t_cylindrical:
			{
				const vec3<T> norm=p-bbox.getorigin();
				#if (RAS_PARADIGM==RAS_DX_PARADIGM)
					T dAngle = atan2(-norm[0],norm[2]);
				#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
					T dAngle = atan2(-norm[0],-norm[2]);
				#endif
				if(dAngle < 0)
					dAngle = (af::getpi<T>()+af::getpi<T>()+dAngle);
				uv[0] = dAngle / (2*af::getpi<T>());
				
				const T dHeight = bbox.getbackmax()[1] - bbox.getfrontmin()[1];
				uv[1] = 1 - ((p[1] - bbox.getfrontmin()[1])/dHeight);
			}
			break;
			case t_spherical:
			{
				const vec3<T> norm=p-bbox.getorigin();
				#if (RAS_PARADIGM==RAS_DX_PARADIGM)
					T dAngle = atan2(-norm[0],norm[2]);
				#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
					T dAngle = atan2(-norm[0],-norm[2]);
				#endif
				if(dAngle < 0)
					dAngle = (af::getpi<T>()+af::getpi<T>()+dAngle);
				uv[0] = dAngle / (2*af::getpi<T>());
				
				const T dPhi = acos(norm[1] / norm.getlength());  // polar angle from the Y-axis
				uv[1] = dPhi / af::getpi<T>();
			}
			break;
		}
	}
protected:
	vertex_data<vec2<T>> m_Tex;
};

template <typename T=RAS_FLTTYPE> class face_bump_vertex_data
{
public:
	struct vertex
	{
		vertex_data<vec2<T>>::template vertex bump;
		__forceinline void set(const face_bump_vertex_data& src,const size_t n){bump.set(src.getbump(),n);}
		__forceinline void lerp(const face_bump_vertex_data& src,const vertex& from,const vertex& to,const T t){bump.lerp(from.bump,to.bump,t);}
		__forceinline void barylerp(const face_bump_vertex_data& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp){bump.barylerp(src.getbump(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);}
	};

	face_bump_vertex_data(){}
	face_bump_vertex_data(const vertex_data<vec2<T>>& t):m_Bump(t){}
	__forceinline vertex_data<vec2<T>>& getbump(void){return m_Bump;}
	__forceinline const vertex_data<vec2<T>>& getbump(void)const{return m_Bump;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,face_bump_vertex_data& dst)const{dst.m_Bump=m_Bump;}
	__forceinline void set(const face_bump_vertex_data& src,const vertex& a,const vertex& b,const vertex& c){m_Bump.set(a.bump,b.bump,c.bump);}
	__forceinline face_bump_vertex_data& operator =(const face_bump_vertex_data& o){m_Bump=o.m_Bump;return *this;}
	__forceinline static face_vertex_att::type getatt(void){return face_vertex_att::t_bump;}
protected:
	vertex_data<vec2<T>> m_Bump;
};

template <typename V,int MAX> class maxvertexbuffer
{
public:
	using t_vertex=V;
	
	maxvertexbuffer():m_nSize(0){}
	~maxvertexbuffer(){}
	__forceinline void clear(void){m_nSize=0;}
	__forceinline void push_back(const V& v){if(m_nSize<MAX)m_Vertices[m_nSize]=v;++m_nSize;}
	__forceinline V& grow(void){++m_nSize;return m_Vertices[m_nSize-1];}
	__forceinline size_t size(void)const{return m_nSize;}
	__forceinline const V *get(void)const{return m_Vertices;}
	__forceinline maxvertexbuffer& operator =(const maxvertexbuffer& o){m_nSize=o.m_nSize;for(int n=0;n<m_nSize;++n)m_Vertices[n]=o.m_Vertices[n];return *this;}
protected:
	size_t m_nSize;
	V m_Vertices[MAX];
};

}
