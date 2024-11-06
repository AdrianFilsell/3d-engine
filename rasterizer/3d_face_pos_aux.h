#pragma once

#include "3d_face.h"

namespace af3d
{

template <typename T=RAS_FLTTYPE> class line_pos3_xform
{
public:
	using t_flt=T;

	enum clippostype {cpt_inside=0x1,cpt_outside=0x2,cpt_unknown=0x4};

	struct vertex
	{
		line_pos_vertex_data<vec4<T>>::template vertex clippos;
		__forceinline void set(const line_pos3_xform& src,const size_t n){clippos.set(src.getclippos(),n);}
		__forceinline void lerp(const line_pos3_xform&src,const vertex& from,const vertex& to,const T t){clippos.lerp(src.getclippos(),from.clippos,to.clippos,t);}
	};

	line_pos3_xform():m_ClipPosType(cpt_unknown){}
	clippostype getclippostype(void)const{return m_ClipPosType;}
	template <int EXTENTS,typename CLIPPER> __forceinline void validateclipspace(void)
	{
		bool bAllInside,bAllOutside;
		CLIPPER::getinsideoutside<EXTENTS>(*this,bAllInside,bAllOutside);
		if(bAllOutside)
			m_ClipPosType=cpt_outside;
		else
		if(bAllInside)
			m_ClipPosType=cpt_inside;
		else
			m_ClipPosType=cpt_unknown;
	}
	__forceinline const line_pos_vertex_data<vec4<T>>& getclippos(void)const{return m_ClipPos;}
	__forceinline line_pos_vertex_data<vec4<T>>& getclippos(void){return m_ClipPos;}
	__forceinline static int size(void){return line_pos_vertex_data<vec4<T>>::size();}
protected:
	clippostype m_ClipPosType;
	line_pos_vertex_data<vec4<T>> m_ClipPos;
};

template <typename T=RAS_FLTTYPE> class line_pos3
{
public:
	using t_flt=T;
	using t_xform=line_pos3_xform<T>;

	line_pos3(){}
	line_pos3(const line_pos_vertex_data<vec3<T>>& p):m_Pos(p){}
	__forceinline const line_pos_vertex_data<vec3<T>>& getpos(void)const{return m_Pos;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const{m_Pos.xform(modeltoworld,modeltoclipspace,dst.getclippos());}
protected:
	line_pos_vertex_data<vec3<T>> m_Pos;
};

template <typename T=RAS_FLTTYPE> class face_pos3_xform
{
public:
	using t_flt=T;

	enum clippostype {cpt_inside=0x1,cpt_outside=0x2,cpt_unknown=0x4};

	struct vertex
	{
		face_pos_vertex_data<vec4<T>>::template vertex clippos;
		__forceinline void set(const face_pos3_xform& src,const size_t n){clippos.set(src.getclippos(),n);}
		__forceinline void lerp(const face_pos3_xform&src,const vertex& from,const vertex& to,const T t){clippos.lerp(src.getclippos(),from.clippos,to.clippos,t);}
		__forceinline void barylerp(const face_pos3_xform&src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp){}
	};

	face_pos3_xform():m_ClipPosType(cpt_unknown){}
	clippostype getclippostype(void)const{return m_ClipPosType;}
	template <int EXTENTS,typename CLIPPER> __forceinline void validateclipspace(void)
	{
		bool bAllInside,bAllOutside;
		CLIPPER::getinsideoutside<EXTENTS>(*this,bAllInside,bAllOutside);
		if(bAllOutside)
			m_ClipPosType=cpt_outside;
		else
		if(bAllInside)
			m_ClipPosType=cpt_inside;
		else
			m_ClipPosType=cpt_unknown;
	}
	__forceinline const face_pos_vertex_data<vec4<T>>& getclippos(void)const{return m_ClipPos;}
	__forceinline face_pos_vertex_data<vec4<T>>& getclippos(void){return m_ClipPos;}
	__forceinline static bool xform_normal(void){return false;}
	__forceinline void set(const face_pos3_xform& src,const vertex& a,const vertex& b,const vertex& c){m_ClipPos.set(src.m_ClipPos,a.clippos,b.clippos,c.clippos);}
	__forceinline static int size(void){return face_pos_vertex_data<vec4<T>>::size();}
protected:
	clippostype m_ClipPosType;
	face_pos_vertex_data<vec4<T>> m_ClipPos;
};

template <typename T=RAS_FLTTYPE> class face_pos3
{
public:
	using t_flt=T;
	using t_xform=face_pos3_xform<T>;

	face_pos3(){}
	face_pos3(const face_pos_vertex_data<vec3<T>>& p):m_Pos(p){}
	__forceinline face_pos_vertex_data<vec3<T>>& getpos(void){return m_Pos;}
	__forceinline const face_pos_vertex_data<vec3<T>>& getpos(void)const{return m_Pos;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const{}
	template <int EXTENTS,typename CLIPPER> __forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		m_Pos.xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst.getclippos());
		dst.validateclipspace<EXTENTS,CLIPPER>();
	}
	static __forceinline int getatts(void){return face_pos_vertex_data<vec3<T>>::getatt();}
protected:
	face_pos_vertex_data<vec3<T>> m_Pos;
};

template <typename T=RAS_FLTTYPE> class face_col_xform
{
public:
	struct vertex
	{
		face_col_vertex_data<T>::template vertex col;
		__forceinline void set(const face_col_xform& src,const size_t n){col.set(src.getcol(),n);}
		__forceinline void lerp(const face_col_xform& src,const vertex& from,const vertex& to,const T t){col.lerp(src.getcol(),from.col,to.col,t);}
		__forceinline void barylerp(const face_col_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp){col.barylerp(src.getcol(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);}
	};

	face_col_xform(){}
	__forceinline const face_col_vertex_data<T>& getcol(void)const{return m_Col;}
	__forceinline face_col_vertex_data<T>& getcol(void){return m_Col;}
	__forceinline void set(const face_col_xform& src,const vertex& a,const vertex& b,const vertex& c){m_Col.set(src.m_Col,a.col,b.col,c.col);}
protected:
	face_col_vertex_data<T> m_Col;
};

template <typename T=RAS_FLTTYPE> class face_col
{
public:
	using t_xform=face_col_xform<T>;

	face_col(){}
	face_col(const face_col_vertex_data<T>& c):m_Col(c){}
	__forceinline const face_col_vertex_data<T>& getcol(void)const{return m_Col;}
	__forceinline face_col_vertex_data<T>& getcol(void){return m_Col;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const{m_Col.xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst.getcol());}
	static __forceinline int getatts(void){return face_col_vertex_data<T>::getatt();}
protected:
	face_col_vertex_data<T> m_Col;
};

template <typename T=RAS_FLTTYPE> class face_tex_xform
{
public:
	struct vertex
	{
		face_tex_vertex_data<T>::template vertex tex;
		__forceinline void set(const face_tex_xform& src,const size_t n){tex.set(src.gettex(),n);}
		__forceinline void lerp(const face_tex_xform& src,const vertex& from,const vertex& to,const T t){tex.lerp(src.gettex(),from.tex,to.tex,t);}
		__forceinline void barylerp(const face_tex_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp){tex.barylerp(src.gettex(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);}
	};

	face_tex_xform(){}
	__forceinline const face_tex_vertex_data<T>& gettex(void)const{return m_Tex;}
	__forceinline face_tex_vertex_data<T>& gettex(void){return m_Tex;}
	 __forceinline void set(const face_tex_xform& src,const vertex& a,const vertex& b,const vertex& c){m_Tex.set(src.gettex(),a.tex,b.tex,c.tex);}
protected:
	face_tex_vertex_data<T> m_Tex;
};

template <typename T=RAS_FLTTYPE> class face_tex
{
public:
	using t_xform=face_tex_xform<T>;

	face_tex(){}
	face_tex(const face_tex_vertex_data<T>& t):m_Tex(t){}
	__forceinline face_tex_vertex_data<T>& gettex(void){return m_Tex;}
	__forceinline const face_tex_vertex_data<T>& gettex(void)const{return m_Tex;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const{m_Tex.xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst.gettex());}
	static __forceinline int getatts(void){return face_tex_vertex_data<T>::getatt();}
protected:
	face_tex_vertex_data<T> m_Tex;
};

template <typename T=RAS_FLTTYPE> class face_bump_xform
{
public:
	struct vertex
	{
		face_bump_vertex_data<T>::template vertex bump;
		__forceinline void set(const face_bump_xform& src,const size_t n){bump.set(src.getbump(),n);}
		__forceinline void lerp(const face_bump_xform& src,const vertex& from,const vertex& to,const T t){bump.lerp(src.getbump(),from.bump,to.bump,t);}
		__forceinline void barylerp(const face_bump_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp){bump.barylerp(src.getbump(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);}
	};

	face_bump_xform(){}
	__forceinline const face_bump_vertex_data<T>& getbump(void)const{return m_Bump;}
	__forceinline face_bump_vertex_data<T>& getbump(void){return m_Bump;}
	__forceinline void set(const face_bump_xform& src,const vertex& a,const vertex& b,const vertex& c){m_Bump.set(src.getbump(),a.bump,b.bump,c.bump);}
protected:
	face_bump_vertex_data<T> m_Bump;
};

template <typename T=RAS_FLTTYPE> class face_bump
{
public:
	using t_xform=face_bump_xform<T>;

	face_bump(){}
	face_bump(const face_bump_vertex_data<T>& t):m_Bump(t){}
	__forceinline face_bump_vertex_data<T>& getbump(void){return m_Bump;}
	__forceinline const face_bump_vertex_data<T>& getbump(void)const{return m_Bump;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const{m_Bump.xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst.getbump());}
	static __forceinline int getatts(void){return face_bump_vertex_data<T>::getatt();}
protected:
	face_bump_vertex_data<T> m_Bump;
};

template <typename T=RAS_FLTTYPE> class face_norm_xform
{
public:
	struct vertex
	{
		face_norm_vertex_data<T>::template vertex worldnorm;
		face_pos_vertex_data<vec3<T>>::template vertex worldpos;
		__forceinline void set(const face_norm_xform& src,const size_t n)
		{
			worldnorm.set(src.getworldnorm(),n);
			worldpos.set(src.getworldpos(),n);
		}
		__forceinline void lerp(const face_norm_xform& src,const vertex& from,const vertex& to,const T t)
		{
			worldnorm.lerp(src.getworldnorm(),from.worldnorm,to.worldnorm,t);
			worldpos.lerp(src.getworldpos(),from.worldpos,to.worldpos,t);
		}
		__forceinline void barylerp(const face_norm_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			worldnorm.barylerp(src.getworldnorm(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			worldpos.barylerp(src.getworldpos(),dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_norm_xform(){}
	__forceinline const face_pos_vertex_data<vec3<T>>& getworldpos(void)const{return m_WorldPos;}
	__forceinline const face_norm_vertex_data<T>& getworldnorm(void)const{return m_WorldNorm;}
	__forceinline face_pos_vertex_data<vec3<T>>& getworldpos(void){return m_WorldPos;}
	__forceinline face_norm_vertex_data<T>& getworldnorm(void){return m_WorldNorm;}
	__forceinline void set(const face_norm_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		m_WorldNorm.set(src.getworldnorm(),a.worldnorm,b.worldnorm,c.worldnorm);
		m_WorldPos.set(src.getworldpos(),a.worldpos,b.worldpos,c.worldpos);
	}
protected:
	face_norm_vertex_data<T> m_WorldNorm;
	face_pos_vertex_data<vec3<T>> m_WorldPos;
};

template <typename T=RAS_FLTTYPE> class face_norm
{
public:
	using t_xform=face_norm_xform<T>;

	face_norm(){}
	face_norm(const face_norm_vertex_data<T>& n):m_Norm(n){}
	__forceinline face_norm_vertex_data<T>& getnorm(void){return m_Norm;}
	__forceinline const face_norm_vertex_data<T>& getnorm(void)const{return m_Norm;}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const{m_Norm.xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst.getworldnorm());}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,const face_pos_vertex_data<vec3<T>>& src,t_xform& dst)const
	{
		modeltoworld.mul(src.getpos()[0],dst.getworldpos().getpos()[0]);
		modeltoworld.mul(src.getpos()[1],dst.getworldpos().getpos()[1]);
		modeltoworld.mul(src.getpos()[2],dst.getworldpos().getpos()[2]);
	}
	static __forceinline int getatts(void){return face_norm_vertex_data<T>::getatt();}
protected:
	face_norm_vertex_data<T> m_Norm;
};

template <typename T=RAS_FLTTYPE> class face_pos3_col_xform:public face_pos3_xform<T>,public face_col_xform<T>
{
public:
	struct vertex:public face_pos3_xform<T>::template vertex,public face_col_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_col_xform& src,const size_t n)
		{
			face_pos3_xform<T>::template vertex::set(src,n);
			face_col_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_col_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_xform<T>::template vertex::lerp(src,from,to,t);
			face_col_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_col_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_col_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_col_xform(){}
	__forceinline void set(const face_pos3_col_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_xform<T>::set(src,a,b,c);
		face_col_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_col:public face_pos3<T>,public face_col<T>
{
public:
	using t_xform=face_pos3_col_xform<T>;

	face_pos3_col(){}
	face_pos3_col(const face_pos_vertex_data<vec3<T>>& p,const face_col_vertex_data<T>& c):face_pos3<T>(p),face_col<T>(c){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_col<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3<T>::getatts()|face_col<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_tex_xform:public face_pos3_xform<T>,public face_tex_xform<T>
{
public:
	struct vertex:public face_pos3_xform<T>::template vertex,public face_tex_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_tex_xform& src,const size_t n)
		{
			face_pos3_xform<T>::template vertex::set(src,n);tex.set(src.gettex(),n);
			face_tex_xform<T>::template vertex::set(src,n);tex.set(src.gettex(),n);
		}
		__forceinline void lerp(const face_pos3_tex_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_xform<T>::template vertex::lerp(src,from,to,t);
			face_tex_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_tex_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_tex_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_tex_xform(){}
	__forceinline void set(const face_pos3_tex_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_xform<T>::set(src,a,b,c);
		face_tex_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_tex:public face_pos3<T>,public face_tex<T>
{
public:
	using t_xform=face_pos3_tex_xform<T>;

	face_pos3_tex(){}
	face_pos3_tex(const face_pos_vertex_data<vec3<T>>& p,const face_tex_vertex_data<T>& t):face_pos3<T>(p),face_tex<T>(t){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_tex<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3<T>::getatts()|face_tex<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_xform:public face_pos3_xform<T>,public face_norm_xform<T>
{
public:
	struct vertex:public face_pos3_xform<T>::template vertex,public face_norm_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_xform& src,const size_t n)
		{
			face_pos3_xform<T>::template vertex::set(src,n);
			face_norm_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_xform<T>::template vertex::lerp(src,from,to,t);
			face_norm_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_norm_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_xform(){}
	__forceinline void set(const face_pos3_norm_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_xform<T>::set(src,a,b,c);
		face_norm_xform<T>::set(src,a,b,c);
	}
	static __forceinline bool xform_normal(void){return true;}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm:public face_pos3<T>,public face_norm<T>
{
public:
	using t_xform=face_pos3_norm_xform<T>;

	face_pos3_norm(){}
	face_pos3_norm(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n):face_pos3<T>(p),face_norm<T>(n){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_norm<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_norm<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,getpos(),dst);
	}
	static __forceinline int getatts(void){return face_pos3<T>::getatts()|face_norm<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col_xform:public face_pos3_norm_xform<T>,public face_col_xform<T>
{
public:
	struct vertex:public face_pos3_norm_xform<T>::template vertex,public face_col_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_col_xform& src,const size_t n)
		{
			face_pos3_norm_xform<T>::template vertex::set(src,n);
			face_col_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_col_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_norm_xform<T>::template vertex::lerp(src,from,to,t);
			face_col_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_col_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_norm_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_col_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_col_xform(){}
	__forceinline void set(const face_pos3_norm_col_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_norm_xform<T>::set(src,a,b,c);
		face_col_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col:public face_pos3_norm<T>,public face_col<T>
{
public:
	using t_xform=face_pos3_norm_col_xform<T>;

	face_pos3_norm_col(){}
	face_pos3_norm_col(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n,const face_col_vertex_data<T>& c):face_pos3_norm<T>(p,n),face_col<T>(c){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_norm<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_col<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_norm<T>::getatts()|face_col<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col_bump_xform:public face_pos3_norm_col_xform<T>,public face_bump_xform<T>
{
public:
	struct vertex:public face_pos3_norm_col_xform<T>::template vertex,public face_bump_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_col_bump_xform& src,const size_t n)
		{
			face_pos3_norm_col_xform<T>::template vertex::set(src,n);
			face_bump_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_col_bump_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_norm_col_xform<T>::template vertex::lerp(src,from,to,t);
			face_bump_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_col_bump_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_norm_col_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_bump_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_col_bump_xform(){}
	__forceinline void set(const face_pos3_norm_col_bump_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_norm_col_xform<T>::set(src,a,b,c);
		face_bump_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col_bump:public face_pos3_norm_col<T>,public face_bump<T>
{
public:
	using t_xform=face_pos3_norm_col_bump_xform<T>;

	face_pos3_norm_col_bump(){}
	face_pos3_norm_col_bump(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n,const face_col_vertex_data<T>& c,const face_bump_vertex_data<T>& b):face_pos3_norm_col<T>(p,n,c),face_bump<T>(b){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_norm_col<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_bump<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_norm_col<T>::getatts()|face_bump<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_tex_xform:public face_pos3_norm_xform<T>,public face_tex_xform<T>
{
public:
	struct vertex:public face_pos3_norm_xform<T>::template vertex,public face_tex_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_tex_xform& src,const size_t n)
		{
			face_pos3_norm_xform<T>::template vertex::set(src,n);
			face_tex_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_tex_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_norm_xform<T>::template vertex::lerp(src,from,to,t);
			face_tex_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_tex_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_norm_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_tex_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_tex_xform(){}
	__forceinline void set(const face_pos3_norm_tex_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_norm_xform<T>::set(src,a,b,c);
		face_tex_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_tex:public face_pos3_norm<T>,public face_tex<T>
{
public:
	using t_xform=face_pos3_norm_tex_xform<T>;

	face_pos3_norm_tex(){}
	face_pos3_norm_tex(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n,const face_tex_vertex_data<T>& t):face_pos3_norm<T>(p,n),face_tex<T>(t){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_norm<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_tex<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_norm<T>::getatts()|face_tex<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_tex_bump_xform:public face_pos3_norm_tex_xform<T>,public face_bump_xform<T>
{
public:
	struct vertex:public face_pos3_norm_tex_xform<T>::template vertex,public face_bump_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_tex_bump_xform& src,const size_t n)
		{
			face_pos3_norm_tex_xform<T>::template vertex::set(src,n);
			face_bump_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_tex_bump_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_norm_tex_xform<T>::template vertex::lerp(src,from,to,t);
			face_bump_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_tex_bump_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_norm_tex_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_bump_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_tex_bump_xform(){}
	__forceinline void set(const face_pos3_norm_tex_bump_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_norm_tex_xform<T>::set(src,a,b,c);
		face_bump_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_tex_bump:public face_pos3_norm_tex<T>,public face_bump<T>
{
public:
	using t_xform=face_pos3_norm_tex_bump_xform<T>;

	face_pos3_norm_tex_bump(){}
	face_pos3_norm_tex_bump(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n,const face_tex_vertex_data<T>& t,const face_bump_vertex_data<T>& b):face_pos3_norm_tex<T>(p,n,t),face_bump<T>(b){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_norm_tex<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_bump<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_norm_tex<T>::getatts()|face_bump<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col_tex_xform:public face_pos3_norm_col_xform<T>,public face_tex_xform<T>
{
public:
	struct vertex:public face_pos3_norm_col_xform<T>::template vertex,public face_tex_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_col_tex_xform& src,const size_t n)
		{
			face_pos3_norm_col_xform<T>::template vertex::set(src,n);
			face_tex_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_col_tex_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_norm_col_xform<T>::template vertex::lerp(src,from,to,t);
			face_tex_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_col_tex_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_norm_col_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_tex_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_col_tex_xform(){}
	__forceinline void set(const face_pos3_norm_col_tex_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_norm_col_xform<T>::set(src,a,b,c);
		face_tex_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col_tex:public face_pos3_norm_col<T>,public face_tex<T>
{
public:
	using t_xform=face_pos3_norm_col_tex_xform<T>;

	face_pos3_norm_col_tex(){}
	face_pos3_norm_col_tex(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n,const face_col_vertex_data<T>& c,const face_tex_vertex_data<T>& t):face_pos3_norm_col<T>(p,n,c),face_tex<T>(t){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_norm_col<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_tex<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_norm_col<T>::getatts()|face_tex<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col_tex_bump_xform:public face_pos3_norm_col_tex_xform<T>,public face_bump_xform<T>
{
public:
	struct vertex:public face_pos3_norm_col_tex_xform<T>::template vertex,public face_bump_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_col_tex_bump_xform& src,const size_t n)
		{
			face_pos3_norm_col_tex_xform<T>::template vertex::set(src,n);
			face_bump_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_col_tex_bump_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_norm_col_tex_xform<T>::template vertex::lerp(src,from,to,t);
			face_bump_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_col_tex_bump_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_norm_col_tex_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_bump_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_col_tex_bump_xform(){}
	__forceinline void set(const face_pos3_norm_col_tex_bump_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_norm_col_tex_xform<T>::set(src,a,b,c);
		face_bump_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_col_tex_bump:public face_pos3_norm_col_tex<T>,public face_bump<T>
{
public:
	using t_xform=face_pos3_norm_col_tex_bump_xform<T>;

	face_pos3_norm_col_tex_bump(){}
	face_pos3_norm_col_tex_bump(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n,const face_col_vertex_data<T>& c,const face_tex_vertex_data<T>& t,const face_bump_vertex_data<T>& b):face_pos3_norm_col_tex<T>(p,n,c,t),face_bump<T>(b){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_norm_col_tex<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_bump<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_norm_col_tex<T>::getatts()|face_bump<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_col_tex_xform:public face_pos3_col_xform<T>,public face_tex_xform<T>
{
public:
	struct vertex:public face_pos3_col_xform<T>::template vertex,public face_tex_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_col_tex_xform& src,const size_t n)
		{
			face_pos3_col_xform<T>::template vertex::set(src,n);
			face_tex_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_col_tex_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_col_xform<T>::template vertex::lerp(src,from,to,t);
			face_tex_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_col_tex_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_col_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_tex_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_col_tex_xform(){}
	__forceinline void set(const face_pos3_col_tex_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_col_xform<T>::set(src,a,b,c);
		face_tex_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_col_tex:public face_pos3_col<T>,public face_tex<T>
{
public:
	using t_xform=face_pos3_col_tex_xform<T>;

	face_pos3_col_tex(){}
	face_pos3_col_tex(const face_pos_vertex_data<vec3<T>>& p,const face_col_vertex_data<T>& c,const face_tex_vertex_data<T>& t):face_pos3_col<T>(p,c),face_tex<T>(t){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_col<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_tex<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_col<T>::getatts()|face_tex<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_bump_xform:public face_pos3_norm_xform<T>,public face_bump_xform<T>
{
public:
	struct vertex:public face_pos3_norm_xform<T>::template vertex,public face_bump_xform<T>::template vertex
	{
		__forceinline void set(const face_pos3_norm_bump_xform& src,const size_t n)
		{
			face_pos3_norm_xform<T>::template vertex::set(src,n);
			face_bump_xform<T>::template vertex::set(src,n);
		}
		__forceinline void lerp(const face_pos3_norm_bump_xform& src,const vertex& from,const vertex& to,const T t)
		{
			face_pos3_norm_xform<T>::template vertex::lerp(src,from,to,t);
			face_bump_xform<T>::template vertex::lerp(src,from,to,t);
		}
		__forceinline void barylerp(const face_pos3_norm_bump_xform& src,const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp)
		{
			face_pos3_norm_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
			face_bump_xform<T>::template vertex::barylerp(src,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);
		}
	};

	face_pos3_norm_bump_xform(){}
	__forceinline void set(const face_pos3_norm_bump_xform& src,const vertex& a,const vertex& b,const vertex& c)
	{
		face_pos3_norm_xform<T>::set(src,a,b,c);
		face_bump_xform<T>::set(src,a,b,c);
	}
};

template <typename T=RAS_FLTTYPE> class face_pos3_norm_bump:public face_pos3_norm<T>,public face_bump<T>
{
public:
	using t_xform=face_pos3_norm_bump_xform<T>;

	face_pos3_norm_bump(){}
	face_pos3_norm_bump(const face_pos_vertex_data<vec3<T>>& p,const face_norm_vertex_data<T>& n,const face_bump_vertex_data<T>& t):face_pos3_norm<T>(p,n),face_bump<T>(t){}
	__forceinline void xform(const mat4<T>& modeltoworld,const mat4<T>& transposedinversemodeltoworld,const mat4<T>& modeltoclipspace,t_xform& dst)const
	{
		face_pos3_norm<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
		face_bump<T>::xform(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,dst);
	}
	static __forceinline int getatts(void){return face_pos3_norm<T>::getatts()|face_bump<T>::getatts();}
};

template <typename T=RAS_FLTTYPE> union face_xform_union
{
	using t_flt=T;

	face_xform_union(){}
	face_xform_union(const face_xform_union& o){/*not required*/}
	~face_xform_union(){}

	face_pos3_xform<T> a;
	face_pos3_col_xform<T> b;
	face_pos3_tex_xform<T> c;
	face_pos3_norm_xform<T> d;
	face_pos3_norm_col_xform<T> e;
	face_pos3_norm_tex_xform<T> f;
	face_pos3_col_tex_xform<T> g;
	face_pos3_norm_col_tex_xform<T> h;
	face_pos3_norm_bump_xform<T> i;
	face_pos3_norm_tex_bump_xform<T> j;
	face_pos3_norm_col_bump_xform<T> k;
	face_pos3_norm_col_tex_bump_xform<T> l;

	template <typename F> F& get(void);
	template <> __forceinline face_pos3_xform<T>& get(void){return a;}
	template <> __forceinline face_pos3_col_xform<T>& get(void){return b;}
	template <> __forceinline face_pos3_tex_xform<T>& get(void){return c;}
	template <> __forceinline face_pos3_norm_xform<T>& get(void){return d;}
	template <> __forceinline face_pos3_norm_col_xform<T>& get(void){return e;}
	template <> __forceinline face_pos3_norm_tex_xform<T>& get(void){return f;}
	template <> __forceinline face_pos3_col_tex_xform<T>& get(void){return g;}
	template <> __forceinline face_pos3_norm_col_tex_xform<T>& get(void){return h;}
	template <> __forceinline face_pos3_norm_bump_xform<T>& get(void){return i;}
	template <> __forceinline face_pos3_norm_tex_bump_xform<T>& get(void){return j;}
	template <> __forceinline face_pos3_norm_col_bump_xform<T>& get(void){return k;}
	template <> __forceinline face_pos3_norm_col_tex_bump_xform<T>& get(void){return l;}

	template <typename F> const F& get(void)const;
	template <> __forceinline const face_pos3_xform<T>& get(void)const{return a;}
	template <> __forceinline const face_pos3_col_xform<T>& get(void)const{return b;}
	template <> __forceinline const face_pos3_tex_xform<T>& get(void)const{return c;}
	template <> __forceinline const face_pos3_norm_xform<T>& get(void)const{return d;}
	template <> __forceinline const face_pos3_norm_col_xform<T>& get(void)const{return e;}
	template <> __forceinline const face_pos3_norm_tex_xform<T>& get(void)const{return f;}
	template <> __forceinline const face_pos3_col_tex_xform<T>& get(void)const{return g;}
	template <> __forceinline const face_pos3_norm_col_tex_xform<T>& get(void)const{return h;}
	template <> __forceinline const face_pos3_norm_bump_xform<T>& get(void)const{return i;}
	template <> __forceinline const face_pos3_norm_tex_bump_xform<T>& get(void)const{return j;}
	template <> __forceinline const face_pos3_norm_col_bump_xform<T>& get(void)const{return k;}
	template <> __forceinline const face_pos3_norm_col_tex_bump_xform<T>& get(void)const{return l;}
};

template <typename F> class facebuffer
{
public:
	using t_flt=F::template t_flt;
	using t_face=F;

	facebuffer(){}
	facebuffer(const facebuffer& o){m_Faces=o.m_Faces;}
	~facebuffer(){}
	__forceinline const std::vector<F>& get(void)const{return m_Faces;}
	__forceinline void push_back(const F& v){m_Faces.push_back(v);}
	__forceinline std::vector<F>& get(void){return m_Faces;}
	template <int EXTENTS,typename VST> static __forceinline void xform(const afthread::taskscheduler *pSched,
															const mat4<t_flt>& modeltoworld,
															const mat4<t_flt>& transposedinversemodeltoworld,
															const mat4<t_flt>& modeltoclipspace,
															const VST::template t_base_types::template t_fb *pB,
															facebuffer<face_xform_union<t_flt>> *pC)
	{
		if(pC->get().capacity()<pB->get().size())
			pC->get().reserve(pB->m_Faces.size());
		if(pC->get().size()!=pB->get().size())
			pC->get().resize(pB->get().size());

		if(pSched)
			pSched->parallel_for(0,static_cast<unsigned int>(pB->get().size()),pSched->getcores(),op<EXTENTS,VST>(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,pB->get(),pC->get()));
		else
			op<EXTENTS,VST>(modeltoworld,transposedinversemodeltoworld,modeltoclipspace,pB->get(),pC->get())(0,0+static_cast<unsigned int>(pB->get().size())-1,nullptr);
	}
protected:
	std::vector<F> m_Faces;

	template <int EXTENTS,typename VST> class op
	{
	public:
		op(const mat4<t_flt>& modeltoworld,const mat4<t_flt>& transposedinversemodeltoworld,const mat4<t_flt>& modeltoclipspace,
			const std::vector<t_face>& b,std::vector<face_xform_union<t_flt>>& c):
			m_modeltoworld(modeltoworld),m_transposedinversemodeltoworld(transposedinversemodeltoworld),m_modeltoclipspace(modeltoclipspace),m_b(b),m_c(c){}
		void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
		{
			for(int n=nFrom;n<=nInclusiveTo;++n)
			{
				// xform clipspace
				VST::template t_base_types::template t_xform_face& dst = m_c[n].get<VST::template t_base_types::template t_xform_face>();
				m_b[n].face_pos3<t_flt>::xform<EXTENTS,VST::template t_proj::template t_clipper>(m_modeltoworld,m_transposedinversemodeltoworld,m_modeltoclipspace,dst);
				if(dst.getclippostype()==face_pos3_xform<t_flt>::cpt_outside)
					continue; // no point continuing

				// xform remaining
				m_b[n].xform(m_modeltoworld,m_transposedinversemodeltoworld,m_modeltoclipspace,dst);
			}
		}
	protected:
		const mat4<t_flt>& m_modeltoworld;
		const mat4<t_flt>& m_transposedinversemodeltoworld;
		const mat4<t_flt>& m_modeltoclipspace;
		const std::vector<t_face>& m_b;
		std::vector<face_xform_union<t_flt>>& m_c;
	};
};

template <typename FT> class face_types
{
public:
	using t_flt=FT::template t_flt;							// float
	
	using t_face=FT;										// face
	using t_fb=facebuffer<t_face>;							// face buffer

	using t_xform_face=t_face::template t_xform;			// clipspace face
	using t_xform_vertex=t_xform_face::template vertex;		// clipspace vertex

	static const int s_nClipMax=10;																// max 6 per face so 10 is more than enough
	using t_xform_maxvertexbuffer_clipped=maxvertexbuffer<t_xform_vertex,s_nClipMax>;			// clipped
	using t_xform_maxvertexbuffer_triangulated=maxvertexbuffer<t_xform_face,s_nClipMax-2>;		// triangulated
};

}
