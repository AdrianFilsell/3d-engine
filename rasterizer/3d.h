#pragma once

#include "core.h"
#include "../Eigen/Dense"
#include <vector>
#include <memory>

// types
#define RAS_FLTTYPE double

// paradigms
#define RAS_DX_PARADIGM		1		// DirectX paradigm: left handed world coordinate system (positive x-axis goes right,positive y-axis goes up,positive z-axis goes forward)
									//					 camera looks down positive z axis
									//					 projection matrix transforms viewing frustrum into NDC cuboid
									//							NDC extents are X[-1,1], Y[-1,1], Z[0,1]
									//							NDC near plane maps to 0, far plane maps to 1
									//							NDC coordinate system is left handed
									//					row major matrices (row vectors/pre-multiplication)
									//					face vertices CW orientation
#define RAS_OGL_PARADIGM	2		// OpenGL paradigm: right handed world coordinate system (positive x-axis goes right,positive y-axis goes up,positive z-axis goes behind)
									//					camera looks down negative z axis
									//					projection matrix transforms viewing frustrum into NDC cube
									//							NDC extents are X[-1,1], Y[-1,1], Z[-1,1]
									//							NDC near plane maps to -1, far plane maps to 1
									//							NDC coordinate system is left handed
									//					column major matrices (column vectors/post-multiplication)
									//					face vertices CCW orientation
#define RAS_PARADIGM		RAS_DX_PARADIGM

// transform
// use w=0 when transforming a direction D by transform T ie T.mul({D[0],D[1],[2],0)
// use transposed inverse when transforming a normal N by transform T ie T.inverse().transpose().mul(N)

// matrix format
#if (RAS_PARADIGM == RAS_DX_PARADIGM)
	#define RAS_EIGENROWMAJOR Eigen::RowMajor
	#define RAS_EIGENVEC2ROWS 1
	#define RAS_EIGENVEC2COLS 2
	#define RAS_EIGENVEC3ROWS 1
	#define RAS_EIGENVEC3COLS 3
	#define RAS_EIGENVEC4ROWS 1
	#define RAS_EIGENVEC4COLS 4
#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
	#define RAS_EIGENROWMAJOR 0
	#define RAS_EIGENVEC2ROWS 2
	#define RAS_EIGENVEC2COLS 1
	#define RAS_EIGENVEC3ROWS 3
	#define RAS_EIGENVEC3COLS 1
	#define RAS_EIGENVEC4ROWS 4
	#define RAS_EIGENVEC4COLS 1
#endif

namespace af3d
{

template <typename T=RAS_FLTTYPE> __forceinline T getfwd(void)
{
	#if (RAS_PARADIGM == RAS_DX_PARADIGM)
		return T(1);
	#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
		return T(-1);
	#endif
}

template <typename T=RAS_FLTTYPE> class quantize_static_3
{
// this is a simple 3 threshold quantize class but could be expanded to hold any number of thresholds and threshold values
// for quantizing diffuse colour/specular highlight between 2 - 5 ( 3 is fine )
public:
	using t_flt=T;

	quantize_static_3(){m_bEmpty=true;}
	~quantize_static_3(){}
	__forceinline bool isempty(void)const{return m_bEmpty;}
	__forceinline void setempty(const bool b){m_bEmpty=b;}
	__forceinline T quantize(const T d)const
	{
		constexpr bool bFloor=false;
		if(bFloor)
		{
			if(d<0.5)return 0;
			if(d<1.0)return 0.5;
			return 1.0;
		}
		if(d>0.5)return 1.0;
		if(d>0.0)return 0.5;
		return 0.0;
	}
protected:
	bool m_bEmpty;
};

template <typename T=RAS_FLTTYPE> class vec2
{
public:
	using t_flt=T;

	vec2(){}
	vec2(const T x,const T y){m_v[0]=x;m_v[1]=y;}
	vec2(const vec2& o){*this=o;}
	__forceinline void lerp(const vec2& from,const vec2& to,const t_flt t){m_v[0]=from[0]+(to[0]-from[0])*t;m_v[1]=from[1]+(to[1]-from[1])*t;}
	template <bool REVERSE> __forceinline void barylerp(const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp,const vec2<T>& a,const vec2<T>& b,const vec2<T>& c)
	{
		m_v[REVERSE?1:0] = (a[0]*dAlphaWa + b[0]*dBetaWb + c[0]*dGammaWc)*dRecipWp;
		m_v[REVERSE?0:1] = (a[1]*dAlphaWa + b[1]*dBetaWb + c[1]*dGammaWc)*dRecipWp;
	}
	__forceinline void negate(void){m_v[0]=-m_v[0];m_v[1]=-m_v[1];}
	__forceinline T dot(const vec2& o)const{return (m_v[0]*o.m_v[0]+m_v[1]*o.m_v[1]);}
	__forceinline RAS_FLTTYPE getlengthsq(void)const{const RAS_FLTTYPE d = m_v[0]*m_v[0]+m_v[1]*m_v[1];return d;}
	__forceinline RAS_FLTTYPE getlength(void)const{const RAS_FLTTYPE d=getlengthsq();return d?sqrt(d):0;}
	__forceinline T operator[](const size_t n)const{return m_v[n];}
	__forceinline T& operator[](const size_t n){return m_v[n];}
	__forceinline vec2& operator=(const vec2& o){m_v[0]=o.m_v[0];m_v[1]=o.m_v[1];return *this;}
	__forceinline vec2 operator-(void)const{return {-m_v[0],-m_v[1]};}
	__forceinline vec2 operator-(const vec2& o)const{return {m_v[0]-o.m_v[0],m_v[1]-o.m_v[1]};}
	__forceinline vec2 operator+(const vec2& o)const{return {m_v[0]+o.m_v[0],m_v[1]+o.m_v[1]};}
	__forceinline bool operator==(const vec2& o)const{return m_v[0]==o.m_v[0]&&m_v[1]==o.m_v[1];}
protected:
	T m_v[2];
};

class rect
{
public:
	enum vertex {v_tl=0,v_br=1};
	rect(){clear();}
	rect(const vec2<int>& tl,const vec2<int>& br){set(v_tl,tl);set(v_br,br);}
	~rect(){}
	__forceinline bool isempty(void)const{return getwidth() <= 0 || getheight() <= 0;}
	__forceinline int getwidth(void)const{return get(v_br)[0]-get(v_tl)[0];}
	__forceinline int getheight(void)const{return get(v_br)[1]-get(v_tl)[1];}
	__forceinline const vec2<int>& get(const vertex v)const{return m_pts[v];}
	__forceinline bool ishorznormalised(void)const{return get(v_tl)[0]<=get(v_br)[0];}
	__forceinline bool isvertnormalised(void)const{return get(v_tl)[1]<=get(v_br)[1];}
	__forceinline bool isinside(const vec2<int>& pt)const{return pt[0]>=get(v_tl)[0] && pt[0]<get(v_br)[0] && pt[1]>=get(v_tl)[1] && pt[1]<get(v_br)[1];}
	__forceinline rect getinflate(const rect& r)const
	{
		if( isempty() )
			return (*this);

		return {{m_pts[v_tl][0]-r.get(v_tl)[0],m_pts[v_tl][1]-r.get(v_tl)[0]},
				{m_pts[v_br][0]+r.get(v_br)[0],m_pts[v_br][1]+r.get(v_br)[1]}};
	}
	__forceinline rect getnormalised(void)const
	{
		if( isempty() )
			return (*this);

		return {{af::minval<int>(get(v_tl)[0], get(v_br)[0] ),af::minval<int>( get(v_tl)[1],get(v_br)[1])},
				{af::maxval<int>(get(v_tl)[0], get(v_br)[0] ),af::maxval<int>( get(v_tl)[1],get(v_br)[1])}};
	}
	__forceinline rect getunion(const rect& other)const
	{
		if( isempty() )
			return other;
		if( other.isempty() )
			return (*this);
		
		const bool bHorzNormalisedA = ishorznormalised();
		const bool bVertNormalisedA = isvertnormalised();
		const bool bHorzNormalisedB = other.ishorznormalised();
		const bool bVertNormalisedB = other.isvertnormalised();

		if( bHorzNormalisedA && bHorzNormalisedB && bVertNormalisedA && bVertNormalisedB )
			return {{af::minval( get(v_tl)[0], other.get(v_tl)[0] ), af::minval( get(v_tl)[1], other.get(v_tl)[1] )},{af::maxval( get(v_br)[0], other.get(v_br)[0] ), af::maxval( get(v_br)[1], other.get(v_br)[1] )}};

		rect res = ( bHorzNormalisedA && bVertNormalisedA ) ? getunion(bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised()) :
															  getnormalised().getunion(bHorzNormalisedB && bVertNormalisedB ? other : other.getnormalised());
		if( !bHorzNormalisedA )
		{
			auto n = res.get(v_tl)[0];
			res.set(v_tl,{res.get(v_br)[0],res.get(v_tl)[1]});
			res.set(v_br,{n,res.get(v_br)[1]});
		}
		if( !bVertNormalisedA )
		{
			auto n = res.get(v_tl)[1];
			res.set(v_tl,{res.get(v_tl)[0],res.get(v_br)[1]});
			res.set(v_br,{res.get(v_br)[0],n});
		}
		return res;
	}
	__forceinline rect getintersect(const rect& other)const
	{
		rect r;
		intersect(other,*this,r);
		return r;
	}
	__forceinline void clear(void){m_pts[0]={0,0};m_pts[1]={-1,-1};}
	__forceinline void set(const vertex v,const vec2<int>& p){m_pts[v]=p;}
	__forceinline rect& operator =(const rect& o){m_pts[0]=o.m_pts[0];m_pts[1]=o.m_pts[1];return *this;}

	static void getrectscale(const double dSrcTLX,const double dSrcTLY,const double dSrcBRX,const double dSrcBRY,
							 const double dDstTLX,const double dDstTLY,const double dDstBRX,const double dDstBRY,
							 const bool bLetterBox,double& dS)
	{
		const double dDstWidth = dDstBRX-dDstTLX;
		const double dDstHeight = dDstBRY-dDstTLY;
		const double dSrcWidth = dSrcBRX-dSrcTLX;
		const double dSrcHeight = dSrcBRY-dSrcTLY;
		const double x = dDstWidth / dSrcWidth, y = dDstHeight / dSrcHeight;
		if( bLetterBox )
		{
			// make src as large as possible while keeping both sides of src within dst
			const double min = x < y ? x : y;
			dS=min;
		}
		else
		{
			// make src as large as possible while keeping smallest side of src within dst
			const double max = x < y ? y : x;
			dS=max;
		}
	}
	__forceinline static void intersect(const rect& a,const rect& b,rect& c)
	{
		if( a.isempty() || b.isempty() )
		{
			c.clear();
			return;
		}

		const bool bHorzNormalisedA = a.ishorznormalised();
		const bool bVertNormalisedA = a.isvertnormalised();
		const bool bHorzNormalisedB = b.ishorznormalised();
		const bool bVertNormalisedB = b.isvertnormalised();

		if( bHorzNormalisedA && bHorzNormalisedB && bVertNormalisedA && bVertNormalisedB )
		{
			// if inclusive then checking for an intersect uses the 'interior' therfore this will NOT detect a 'shared' edge
			bool bIntersect = true;
			if( b.get(v_tl)[1] >= a.get(v_br)[1] )
				bIntersect = false;
			else
			if( a.get(v_tl)[1] >= b.get(v_br)[1] )
				bIntersect = false;
			else
			if( b.get(v_tl)[0] >= a.get(v_br)[0] )
				bIntersect = false;
			else
			if( a.get(v_tl)[0] >= b.get(v_br)[0] )
				bIntersect = false;
			if( bIntersect )
			{
				c.set(v_tl,{af::maxval<int>( a.get(v_tl)[0], b.get(v_tl)[0]),af::maxval<int>( a.get(v_tl)[1], b.get(v_tl)[1])});
				c.set(v_br,{af::minval<int>( a.get(v_br)[0], b.get(v_br)[0]),af::minval<int>( a.get(v_br)[1], b.get(v_br)[1])});
			}
			else
				c.clear();
			return;
		}

		if( bHorzNormalisedA && bVertNormalisedA )
			intersect( a, bHorzNormalisedB && bVertNormalisedB ? b : b.getnormalised(), c );
		else
		{
			intersect(a.getnormalised(),bHorzNormalisedB && bVertNormalisedB ? b : b.getnormalised(), c );
			vec2<int> tl=c.get(v_tl);
			vec2<int> br=c.get(v_br);
			if( !bHorzNormalisedA )
				std::swap(tl[0],br[0]);
			if( !bVertNormalisedA )
				std::swap(tl[1],br[1]);
			c.set(v_tl,tl);
			c.set(v_br,br);
		}
	}
	__forceinline vec2<int>& get(const vertex v){return m_pts[v];}
protected:
	vec2<int> m_pts[2];
};

template <typename T=RAS_FLTTYPE> class crect
{
public:
	enum vertex {v_tl=0,v_br=1};
	crect(){}
	crect(const vec2<T>& tl,const vec2<T>& br){m_pts[v_tl]=tl;m_pts[v_br]=br;}
	crect(const rect& o){m_pts[v_tl][0]=o.get(rect::v_tl)[0];m_pts[v_tl][1]=o.get(rect::v_tl)[1];m_pts[v_br][0]=o.get(rect::v_br)[0];m_pts[v_br][1]=o.get(rect::v_br)[1];}
	~crect(){}
	__forceinline const vec2<T>& get(const vertex v)const{return m_pts[v];}
	__forceinline T getwidth(void)const{return get(v_br)[0]-get(v_tl)[0];}
	__forceinline T getheight(void)const{return get(v_br)[1]-get(v_tl)[1];}
	__forceinline vec2<T> getcentre(void)const{return {get(v_tl)[0]+(getwidth()/2.0),get(v_tl)[1]+(getheight()/2.0)};}
	__forceinline vec2<T>& get(const vertex v){return m_pts[v];}
	__forceinline void set(const vertex v,const vec2<T>& p){m_pts[v]=p;}
	__forceinline crect& operator =(const crect& o){m_pts[0]=o.m_pts[0];m_pts[1]=o.m_pts[1];return *this;}
	template <typename V> static __forceinline void postodiscrete(const V *p,const int n,rect& r)
	{
		T minx=p[0][0];
		T maxx=minx;
		T miny=p[0][1];
		T maxy=miny;
		for(int nP=1;nP<n;++nP)
		{
			if(p[nP][0]<minx)
				minx=p[nP][0];
			else
			if(p[nP][0]>maxx)
				maxx=p[nP][0];
			if(p[nP][1]<miny)
				miny=p[nP][1];
			else
			if(p[nP][1]>maxy)
				maxy=p[nP][1];
		}
		vec2<int> tl={af::posround<T,int>(minx),af::posround<T,int>(miny)};
		vec2<int> br={af::posround<T,int>(maxx),af::posround<T,int>(maxy)};
		if(br[0]<maxx)
			br[0]++;
		if(br[1]<maxy)
			br[1]++;
		r.set(rect::v_tl,tl);
		r.set(rect::v_br,br);
	}
protected:
	vec2<T> m_pts[2];
};

template <typename T=RAS_FLTTYPE> class quad
{
public:
	enum vertex {v_tl=0,v_tr=1,v_br=2,v_bl=3};
	quad(){}
	quad(const vec2<T>& tl,const vec2<T>& tr,const vec2<T>& br,const vec2<T>& bl){m_pts[v_tl]=tl;m_pts[v_br]=br;m_pts[v_tr]=tr;m_pts[v_bl]=bl;}
	~quad(){}
	__forceinline const vec2<T>& get(const vertex v)const{return m_pts[v];}
	__forceinline void set(const vertex v,const vec2<T>& p){m_pts[v]=p;}
	__forceinline quad& operator =(const quad& o){m_pts[0]=o.m_pts[0];m_pts[1]=o.m_pts[1];m_pts[2]=o.m_pts[2];m_pts[3]=o.m_pts[3];return *this;}
protected:
	vec2<T> m_pts[4];
};

template <typename T=RAS_FLTTYPE> class vec3
{
public:
	using t_flt=T;

	vec3(){}
	vec3(const T x,const T y,const T z){m_v[0]=x;m_v[1]=y;m_v[2]=z;}
	vec3(const vec3& o){*this=o;}
	__forceinline void lerp(const vec3& from,const vec3& to,const t_flt t){m_v[0]=from[0]+(to[0]-from[0])*t;m_v[1]=from[1]+(to[1]-from[1])*t;m_v[2]=from[2]+(to[2]-from[2])*t;}
	template <bool REVERSE> __forceinline void barylerp(const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp,const vec3<T>& a,const vec3<T>& b,const vec3<T>& c)
	{
		m_v[REVERSE?2:0] = (a[0]*dAlphaWa + b[0]*dBetaWb + c[0]*dGammaWc)*dRecipWp;
		m_v[1] = (a[1]*dAlphaWa + b[1]*dBetaWb + c[1]*dGammaWc)*dRecipWp;
		m_v[REVERSE?0:2] = (a[2]*dAlphaWa + b[2]*dBetaWb + c[2]*dGammaWc)*dRecipWp;
	}
	__forceinline void negate(void){m_v[0]=-m_v[0];m_v[1]=-m_v[1];m_v[2]=-m_v[2];}
	__forceinline T getlengthsq(void)const{return (m_v[0]*m_v[0]+m_v[1]*m_v[1]+m_v[2]*m_v[2]);}
	__forceinline T getlength(void)const{const T d=getlengthsq();return d?sqrt(d):0;}
	__forceinline vec3 normalized(void)const{const T dL=getlength();if(dL)return (*this)*(1.0/dL); return *this;}
	__forceinline vec3 cross(const vec3& o)const{return {m_v[1]*o.m_v[2]-m_v[2]*o.m_v[1],m_v[2]*o.m_v[0]-m_v[0]*o.m_v[2],m_v[0]*o.m_v[1]-m_v[1]*o.m_v[0]};}
	__forceinline T dot(const vec3& o)const{return (m_v[0]*o.m_v[0]+m_v[1]*o.m_v[1]+m_v[2]*o.m_v[2]);}
	__forceinline T dot(const T dX,const T dY,const T dZ)const{return (m_v[0]*dX+m_v[1]*dY+m_v[2]*dZ);}
	__forceinline vec3& normalize(void){const T dL=getlength();if(dL){const T dRecip=1/dL;m_v[0]*=dRecip;m_v[1]*=dRecip;m_v[2]*=dRecip;}return *this;}
	__forceinline T operator[](const size_t n)const{return m_v[n];}
	__forceinline T& operator[](const size_t n){return m_v[n];}
	__forceinline vec3& operator=(const vec3& o){m_v[0]=o.m_v[0];m_v[1]=o.m_v[1];m_v[2]=o.m_v[2];return *this;}
	__forceinline vec3 operator*(const T d)const{return {m_v[0]*d,m_v[1]*d,m_v[2]*d};}
	__forceinline vec3 operator-(const vec3& o)const{return {m_v[0]-o.m_v[0],m_v[1]-o.m_v[1],m_v[2]-o.m_v[2]};}
	__forceinline vec3& operator -=(const vec3& o){m_v[0]-=o[0];m_v[1]-=o[1];m_v[2]-=o[2];return *this;}
	__forceinline vec3& operator -=(const T d){m_v[0]-=d;m_v[1]-=d;m_v[2]-=d;return *this;}
	__forceinline vec3& operator +=(const T d){m_v[0]+=d;m_v[1]+=d;m_v[2]+=d;return *this;}
	__forceinline vec3& operator +=(const vec3& o){m_v[0]+=o.m_v[0];m_v[1]+=o.m_v[1];m_v[2]+=o.m_v[2];return *this;}
	__forceinline vec3& operator *=(const T d){m_v[0]*=d;m_v[1]*=d;m_v[2]*=d;return *this;}
	__forceinline vec3& operator *=(const vec3& o){m_v[0]*=o[0];m_v[1]*=o[1];m_v[2]*=o[2];return *this;}
	__forceinline vec3 operator-()const{return {-m_v[0],-m_v[1],-m_v[2]};}
	__forceinline vec3 operator+(const vec3& o)const{return {m_v[0]+o.m_v[0],m_v[1]+o.m_v[1],m_v[2]+o.m_v[2]};}
	__forceinline vec3 operator*(const vec3& o)const{return {m_v[0]*o.m_v[0],m_v[1]*o.m_v[1],m_v[2]*o.m_v[2]};}
	__forceinline bool operator==(const vec3& o)const{return m_v[0]==o.m_v[0]&&m_v[1]==o.m_v[1]&&m_v[2]==o.m_v[2];}
protected:
	T m_v[3];
};

template <typename T=RAS_FLTTYPE> class vec4
{
public:
	using t_flt=T;

	vec4(){}
	vec4(const T x,const T y,const T z,const T w){m_v[0]=x;m_v[1]=y;m_v[2]=z;m_v[3]=w;}
	vec4(const vec4& o){*this=o;}
	__forceinline vec4 getdehomogenise(void)const{if(m_v[3])return (*this)*(1.0/m_v[3]);return (*this);}
	__forceinline void getdevice(const crect<T>& rDstNDC,vec3<T>& dev)const
	{
		const T dW=(*this)[3];

		T dX=((m_v[0]/dW)+1.0)*0.5;		// [0,1]
		dX*=rDstNDC.getwidth();
		dev[0]=rDstNDC.get(crect<T>::v_tl)[0]+dX;

		T dY=((m_v[1]/-dW)+1.0)*0.5;	// [0,1]
		dY*=rDstNDC.getheight();
		dev[1]=rDstNDC.get(crect<T>::v_tl)[1]+dY;

		const T dZ=getdeviceZ(m_v[2],dW);
		dev[2]=dZ;
	}
	static __forceinline T getdeviceZ(const T dClipspaceZ,const T dClipspaceW)
	{
		const T dZ=dClipspaceZ/dClipspaceW;
		#if(RAS_PARADIGM==RAS_DX_PARADIGM)
			return dZ;
		#elif(RAS_PARADIGM==RAS_OGL_PARADIGM)
			return (dZ+1.0)*0.5;
		#endif
	}
	__forceinline void lerp(const vec4& from,const vec4& to,const t_flt t){m_v[0]=from[0]+(to[0]-from[0])*t;m_v[1]=from[1]+(to[1]-from[1])*t;m_v[2]=from[2]+(to[2]-from[2])*t;m_v[3]=from[3]+(to[3]-from[3])*t;}
	template <bool REVERSE> __forceinline void barylerp(const T dAlphaWa,const T dBetaWb,const T dGammaWc,const T dRecipWp,const vec4<T>& a,const vec4<T>& b,const vec4<T>& c)
	{
		m_v[REVERSE?3:0] = (a[0]*dAlphaWa + b[0]*dBetaWb + c[0]*dGammaWc)*dRecipWp;
		m_v[REVERSE?2:1] = (a[1]*dAlphaWa + b[1]*dBetaWb + c[1]*dGammaWc)*dRecipWp;
		m_v[REVERSE?1:2] = (a[2]*dAlphaWa + b[2]*dBetaWb + c[2]*dGammaWc)*dRecipWp;
		m_v[REVERSE?0:3] = (a[3]*dAlphaWa + b[3]*dBetaWb + c[3]*dGammaWc)*dRecipWp;
	}
	__forceinline void negate(void){m_v[0]=-m_v[0];m_v[1]=-m_v[1];m_v[2]=-m_v[2];m_v[3]=-m_v[3];}
	__forceinline void dehomogenise(void){if(m_v[3]){const T d=1.0/m_v[3];m_v[0]*=d;m_v[1]*=d;m_v[2]*=d;m_v[3]=1.0;}}
	__forceinline T operator[](const size_t n)const{return m_v[n];}
	__forceinline T& operator[](const size_t n){return m_v[n];}
	__forceinline vec4& operator=(const vec4& o){m_v[0]=o.m_v[0];m_v[1]=o.m_v[1];m_v[2]=o.m_v[2];m_v[3]=o.m_v[3];return *this;}
	__forceinline vec4 operator*(const T d)const{return {m_v[0]*d,m_v[1]*d,m_v[2]*d,m_v[3]*d};}
	__forceinline vec4 operator-(const vec4& o)const{return {m_v[0]-o.m_v[0],m_v[1]-o.m_v[1],m_v[2]-o.m_v[2],m_v[3]-o.m_v[3]};}
	__forceinline vec4 operator+(const vec4& o)const{return {m_v[0]+o.m_v[0],m_v[1]+o.m_v[1],m_v[2]+o.m_v[2],m_v[3]+o.m_v[3]};}
	__forceinline bool operator==(const vec4& o)const{return m_v[0]==o.m_v[0]&&m_v[1]==o.m_v[1]&&m_v[2]==o.m_v[2]&&m_v[3]==o.m_v[3];}
protected:
	T m_v[4];
};

template <typename T=RAS_FLTTYPE> class plane
{
public:
	using t_flt=T;

	enum axistype {at_xy,at_zy,at_xz};

	plane(){}
	plane(const vec3<T>& origin,const vec3<T>& normal):m_Origin(origin)
	{
		m_v[0] = normal[0];
		m_v[1] = normal[1];
		m_v[2] = normal[2];
		m_v[3] = -origin.dot( normal );
	}
	__forceinline const vec3<T>& getorigin(void)const{return m_Origin;}
	__forceinline const vec3<T> getnormal(void)const{return {m_v[0], m_v[1], m_v[2]};}
	__forceinline const vec4<T>& get(void)const{return m_v;}
	__forceinline bool getintersect(const vec3<T>& from,const vec3<T>& to, vec3<T>& res) const
	{
		const vec3<T> direction( to - from ), normal( m_v[0], m_v[1], m_v[2] );
		const T dot = normal.dot( direction );
		if( 0 == dot )
			return false;
		T temp = ( m_v[3] + normal.dot( from ) ) / dot;
		res[0]=( from[0] - temp * direction[0] );
		res[1]=( from[1] - temp * direction[1] );
		res[2]=( from[2] - temp * direction[2] );
		return true;
	}
	plane& operator=(const plane& o){m_Origin=o.m_Origin;m_v=o.m_v;return *this;}
protected:
	vec3<T> m_Origin;
	vec4<T> m_v;
};

template <typename T=RAS_FLTTYPE> class translate3
{
public:
	using t_flt=T;

	translate3(const vec3<T>& t):m_V(t){}
	__forceinline const vec3<T>& get(void)const{return m_V;}
protected:
	vec3<T> m_V;
};

template <typename T=RAS_FLTTYPE> class scale3
{
public:
	using t_flt=T;
	
	scale3(const vec3<T>& t):m_V(t){}
	__forceinline const vec3<T>& get(void)const{return m_V;}
protected:
	vec3<T> m_V;
};

template <typename T=RAS_FLTTYPE> class rotation3
{
public:
	using t_flt=T;
	
	rotation3(const vec3<T>& normalisedaxis,const T dRadianRot):m_V(normalisedaxis),m_dT(dRadianRot){}
	__forceinline const vec3<T>& getnormalisedaxis(void)const{return m_V;}
	__forceinline const T getradianrot(void)const{return m_dT;}
protected:
	vec3<T> m_V;
	T m_dT;
};

template <typename T=RAS_FLTTYPE> class mat4:public Eigen::Matrix<T,4,4,RAS_EIGENROWMAJOR>
{
public:
	using t_flt=T;
	
	mat4(){}
	template <typename R> mat4(const R& o):Eigen::Matrix<T,4,4,RAS_EIGENROWMAJOR>(o){}
	mat4(const Eigen::Matrix<T,4,4,RAS_EIGENROWMAJOR>& o):Eigen::Matrix<T,4,4,RAS_EIGENROWMAJOR>(o){}
	mat4(const translate3<T> t)
	{
		#if (RAS_PARADIGM == RAS_DX_PARADIGM)
			*this << 1,0,0,0,
					 0,1,0,0,
					 0,0,1,0,
					 t.get()[0],t.get()[1],t.get()[2],1;
		#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
			*this << 1,0,0,t.get()[0],
					 0,1,0,t.get()[1],
					 0,0,1,t.get()[2],
					 0,0,0,1;
		#endif
	}
	mat4(const scale3<T> t)
	{
		*this << t.get()[0],0,0,0,
				 0,t.get()[1],0,0,
				 0,0,t.get()[2],0,
				 0,0,0,1;
	}
	mat4(const rotation3<T> r)
	{
		const T halfrot=r.getradianrot()/2.0;
		const T s=::sin(halfrot);
		const vec4<T> q(r.getnormalisedaxis()[0]*s,r.getnormalisedaxis()[1]*s,r.getnormalisedaxis()[2]*s,::cos(halfrot));

		#if (RAS_PARADIGM == RAS_DX_PARADIGM)
			*this << (1-(2*(q[1]*q[1]+q[2]*q[2]))),(2*(q[0]*q[1]+q[2]*q[3])),(2*(q[0]*q[2]-q[1]*q[3])),0,
					 (2*(q[0]*q[1]-q[2]*q[3])),(1-(2*(q[0]*q[0]+q[2]*q[2]))),(2*(q[1]*q[2]+q[0]*q[3])),0,
					 (2*(q[0]*q[2]+q[1]*q[3])),(2*(q[1]*q[2]-q[0]*q[3])),(1-(2*(q[0]*q[0]+q[1]*q[1]))),0,
					 0,0,0,1;
		#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
			*this << (1-(2*(q[1]*q[1]+q[2]*q[2]))),(2*(q[0]*q[1]-q[2]*q[3])),(2*(q[0]*q[2]+q[1]*q[3])),0,
					 (2*(q[0]*q[1]+q[2]*q[3])),(1-(2*(q[0]*q[0]+q[2]*q[2]))),(2*(q[1]*q[2]-q[0]*q[3])),0,
					 (2*(q[0]*q[2]-q[1]*q[3])),(2*(q[1]*q[2]+q[0]*q[3])),(1-(2*(q[0]*q[0]+q[1]*q[1]))),0,
					 0,0,0,1;
		#endif
	}
	__forceinline bool isinvertable(void)const
	{
		const T dTol=1e-6;
		const T d=determinant();
		return d>=dTol?true:d<=dTol;
	}
	__forceinline void mul(const mat4& a)
	{
		// perform 'this' followed by 'a' and assign to this
		mat4 b;
		mul(*this,a,b);
		*this=b;
	}
	__forceinline void mul(const mat4& a,mat4& b)const
	{
		// perform 'this' followed by 'a' and assign to b
		mul(*this,a,b);
	}
	__forceinline void mul(const vec3<T>& a,vec4<T>& b)const
	{
		// apply 'this' transform to a and assign to b
		mul(*this,a,b);
	}
	__forceinline void mul(const vec3<T>& a,vec3<T>& b)const
	{
		// apply 'this' transform to a and assign to b
		mul(*this,a,b);
	}
	__forceinline void mul(const vec4<T>& a,vec4<T>& b)const
	{
		// apply 'this' transform to a and assign to b
		mul(*this,a,b);
	}
	__forceinline void mul(const vec4<T>& a,vec3<T>& b)const
	{
		// apply 'this' transform to a and assign to b
		mul(*this,a,b);
	}
	__forceinline void mul(const crect<T>& a,crect<T>& b)const
	{
		// apply 'this' transform to a and assign to b
		mul(*this,a,b);
	}
	static __forceinline void mul(const mat4& a,const mat4& b,mat4& c)
	{
		// perform 'a' followed by 'b' ie if 'a' is translate by 10 and 'b' is scale by 2 then 'c' applied to 5 will give 30 NOT 20
		constexpr bool bEigenCall=false;
		#if (RAS_PARADIGM == RAS_DX_PARADIGM)
			if(bEigenCall)
				c = (a * b);
			else
				for(int nR=0;nR<4;++nR)
					for(int nC=0;nC<4;++nC)
						c.coeffRef(nR,nC) = a.coeff(nR,0)*b.coeff(0,nC) + a.coeff(nR,1)*b.coeff(1,nC) + a.coeff(nR,2)*b.coeff(2,nC) + a.coeff(nR,3)*b.coeff(3,nC);
		#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
			if(bEigenCall)
				c = (b * a);
			else
				for(int nR=0;nR<4;++nR)
					for(int nC=0;nC<4;++nC)
						c.coeffRef(nR,nC) = b.coeff(nR,0)*a.coeff(0,nC) + b.coeff(nR,1)*a.coeff(1,nC) + b.coeff(nR,2)*a.coeff(2,nC) + b.coeff(nR,3)*a.coeff(3,nC);
		#endif
	}
	static __forceinline void mul(const mat4& a,const vec3<T>& b,vec3<T>& c)
	{
		// apply 'a' transform to 'b' and assign to 'c'

		// assume b[3] == 1
		// assume c[3] is not required
		
		#if (RAS_PARADIGM == RAS_DX_PARADIGM)
			for(int n=0;n<3;++n)
				c[n] = a.coeff(0,n)*b[0] + a.coeff(1,n)*b[1] + a.coeff(2,n)*b[2] + a.coeff(3,n);
		#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
			for(int n=0;n<3;++n)
				c[n] = a.coeff(n,0)*b[0] + a.coeff(n,1)*b[1] + a.coeff(n,2)*b[2] + a.coeff(n,3);
		#endif
	}
	static __forceinline void mul(const mat4& a,const vec3<T>& b,vec4<T>& c)
	{
		// apply 'a' transform to 'b' and assign to 'c'

		// assume b[3] == 1

		#if (RAS_PARADIGM == RAS_DX_PARADIGM)
			for(int n=0;n<4;++n)
				c[n] = a.coeff(0,n)*b[0] + a.coeff(1,n)*b[1] + a.coeff(2,n)*b[2] + a.coeff(3,n);
		#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
			for(int n=0;n<4;++n)
				c[n] = a.coeff(n,0)*b[0] + a.coeff(n,1)*b[1] + a.coeff(n,2)*b[2] + a.coeff(n,3);
		#endif
	}
	static __forceinline void mul(const mat4& a,const vec4<T>& b,vec4<T>& c)
	{
		// apply 'a' transform to 'b' and assign to 'c'
		#if (RAS_PARADIGM == RAS_DX_PARADIGM)
			for(int n=0;n<4;++n)
				c[n] = a.coeff(0,n)*b[0] + a.coeff(1,n)*b[1] + a.coeff(2,n)*b[2] + a.coeff(3,n)*b[3];
		#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
			for(int n=0;n<4;++n)
				c[n] = a.coeff(n,0)*b[0] + a.coeff(n,1)*b[1] + a.coeff(n,2)*b[2] + a.coeff(n,3)*b[3];
		#endif
	}
	static __forceinline void mul(const mat4& a,const vec4<T>& b,vec3<T>& c)
	{
		// apply 'a' transform to 'b' and assign to 'c'

		// assume c[3] == 1
		#if (RAS_PARADIGM == RAS_DX_PARADIGM)
			for(int n=0;n<3;++n)
				c[n] = a.coeff(0,n)*b[0] + a.coeff(1,n)*b[1] + a.coeff(2,n)*b[2] + a.coeff(3,n)*b[3];
		#elif (RAS_PARADIGM == RAS_OGL_PARADIGM)
			for(int n=0;n<3;++n)
				c[n] = a.coeff(n,0)*b[0] + a.coeff(n,1)*b[1] + a.coeff(n,2)*b[2] + a.coeff(n,3)*b[3];
		#endif
	}
	static __forceinline void mul(const mat4& a,const crect<T>& b,crect<T>& c)
	{
		// apply 'a' transform to 'b' and assign to 'c'
		vec4<T> v;
		mul(a,{b.get(crect<T>::v_tl)[0],b.get(crect<T>::v_tl)[1],0,1},v);
		c.get(crect<T>::v_tl)={v[0],v[1]};
		mul(a,{b.get(crect<T>::v_br)[0],b.get(crect<T>::v_br)[1],0,1},v);
		c.get(crect<T>::v_br)={v[0],v[1]};
	}
};

}
