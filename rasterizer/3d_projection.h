#pragma once

#include "3d_projectionclipper.h"

namespace af3d
{

// maps viewing frustrum to canonical viewing volume

template <typename T=RAS_FLTTYPE> class perspectiveprojection
{
public:
	using t_flt=T;
	using t_clipper=projectionclipper<true>;

	perspectiveprojection(){}
	~perspectiveprojection(){}
	
	__forceinline const mat4<T>& gettrns(void)const{return m_Trns;}

	void setaspect(const T d){m_dAspect=d;}
	void setnear(const T d){m_dNear=d;}
	void setfar(const T d){m_dFar=d;}
	void setfov(const T d){m_dFov=d;}
	void settrns(void)
	{
		// symmetric frustrum

		// https://www.songho.ca/opengl/gl_projectionmatrix.html
		// describes side view of viewing frustrum i.e in 2d cartesian coordinate system
		// eye 'e' at (0,0), near plane (n,0) and far place (f,0)
		// top Y value 't' of near viewing plane positive Y axis
		// bottom Y value 'b' of near viewing plane negative Y axis
		// angle between vectors (t-e) and (b-e) is the fov
		// 
		// t/n = tan(fovY/2)
		// t = tan(fovY/2).n
		const T t = tan(m_dFov/2.0)*m_dNear;
		const T b = -t;
		const T r = t*m_dAspect; // use aspect ratio of viewport (width/height)
		const T l = -r;

		// now we have the extents of the near plane in camera coordinates setup transform
		#if (RAS_PARADIGM==RAS_DX_PARADIGM || RAS_PARADIGM==RAS_OGL_PARADIGM)
			mat4<T> trns;
			trns.setZero();
			trns.coeffRef(0,0)=(2*m_dNear)/(r-l);
			trns.coeffRef(1,1)=(2*m_dNear)/(t-b);

			#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
				// viewing frustrum mapped to cube [-1,1],[-1,1],[-1,1]
				// front plane becomes -1, far plane becomes 1
				// viewing frustrum mapped to left hand coordinate system
				trns.coeffRef(0,2)=(r+l)/(r-l);
				trns.coeffRef(1,2)=(t+b)/(t-b);
				trns.coeffRef(2,2)=(-(m_dFar+m_dNear))/(m_dFar-m_dNear);
				trns.coeffRef(2,3)=(-(2.0*m_dFar*m_dNear))/(m_dFar-m_dNear);
				trns.coeffRef(3,2)=-1;
			#else
				// viewing frustrum mapped to cube [-1,1],[-1,1],[0,1]
				// front plane becomes 0, far plane becomes 1
				// viewing frustrum mapped to left hand coordinate system
				trns.coeffRef(2,0)=(r+l)/(r-l);
				trns.coeffRef(2,1)=(t+b)/(t-b);
				trns.coeffRef(2,2)=m_dFar/(m_dFar-m_dNear);
				trns.coeffRef(3,2)=-(m_dFar*m_dNear)/(m_dFar-m_dNear);
				trns.coeffRef(2,3)=1;
			#endif
	
			m_Trns=trns;
		#endif
	}

	perspectiveprojection& operator =(const perspectiveprojection& o)
	{
		projection::operator =(o);
		m_dAspect=o.m_dAspect;
		m_dNear=o.m_dNear;
		m_dFar=o.m_dFar;
		m_dFov=o.m_dFov;
		m_spTrns=o.m_spTrns;
		return *this;
	}

	static void getunprojectworldpos(const vec2<T>& devPos,const crect<T>& rDevice,const mat4<T>& cliptoworld,vec3<T>& worldFront,vec3<T>& worldBack,vec3<T>& worldDir)
	{
		const T dFarPlane = 1;
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			const T dNearPlane = 0;
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			const T dNearPlane = -1;
		#endif

		const vec4<T> vNearPlane(( ( (devPos[0] - rDevice.get(crect<T>::v_tl)[0]) / rDevice.getwidth() ) * 2.0 ) - 1.0,
								 ( ( 1.0-((devPos[1] - rDevice.get(crect<T>::v_tl)[1]) / rDevice.getheight()) ) * 2.0 ) - 1.0,
								 dNearPlane,
								 1);
		const vec4<T> vFarPlane(vNearPlane[0],vNearPlane[1],1,1);

		vec4<T> vWorldNearPlane,vWorldFarPlane;
		cliptoworld.mul(vNearPlane,vWorldNearPlane);
		cliptoworld.mul(vFarPlane,vWorldFarPlane);
		vWorldNearPlane.dehomogenise();
		vWorldFarPlane.dehomogenise();

		worldFront= {vWorldNearPlane[0],vWorldNearPlane[1],vWorldNearPlane[2]};
		worldBack= {vWorldFarPlane[0],vWorldFarPlane[1],vWorldFarPlane[2]};
		worldDir = {vWorldFarPlane[0] - vWorldNearPlane[0],vWorldFarPlane[1] - vWorldNearPlane[1],vWorldFarPlane[2] - vWorldNearPlane[2]};
		worldDir.normalize();
	}
	void unproject(const vec2<T>& devPos,const vec3<T>& cameraPos,const mat4<T>& cameraTrns,const crect<T>& rDevice,vec3<>& vRayOrigin,vec3<>& vRayDir)const
	{		
		vRayOrigin=cameraPos;
		vRayDir=(getunprojectworldpos(devPos,cameraTrns,*m_spTrns,rDevice)-vRayOrigin).normalized();
	}
protected:
	mat4<T> m_Trns;
	T m_dAspect;
	T m_dNear;
	T m_dFar;
	T m_dFov;
};

template <typename T=RAS_FLTTYPE> class orthographicprojection
{
public:
	using t_flt=T;
	using t_clipper=projectionclipper<false>;

	enum subtype {st_none,st_isometric,st_dimetric,st_trimetric};
	orthographicprojection(){m_dT=0;m_dB=480;m_dL=0;m_dB=640;m_dSubTypeXAxisRotRadians=0;m_dSubTypeYAxisRotRadians=0;}
	~orthographicprojection(){}
	
	const mat4<T>& gettrns(void)const{return m_Trns;}
	T getsubtypexaxisrot(const subtype t)
	{
		switch(t)
		{
			case st_none:return 0;
			case st_isometric:return ::atan(::sqrt(2.0/3.0));
			case st_dimetric:return af::getradian<T>(20);
			case st_trimetric:return af::getradian<T>(30);
			default:return 0;
		}
	}
	T getsubtypeyaxisrot(const subtype t)
	{
		switch(t)
		{
			case st_none:return 0;
			case st_isometric:
			case st_dimetric:
			case st_trimetric:return af::getradian<T>(45);
			default:return 0;
		}
	}

	void setsubtypexaxisrot(const T d){m_dSubTypeXAxisRotRadians=d;}
	void setsubtypeyaxisrot(const T d){m_dSubTypeYAxisRotRadians=d;}
	void setvolume(const T dL,const T dT,const T dR,const T dB){m_dL=dL;m_dR=dR;m_dT=dT;m_dB=dB;}
	void setnear(const T d){m_dNear=d;}
	void setfar(const T d){m_dFar=d;}
	void settrns(void)
	{
		// symmetric frustrum

		// https://www.songho.ca/opengl/gl_projectionmatrix.html
		const T t = m_dT;
		const T b = m_dB;
		const T r = m_dR;
		const T l = m_dL;
		
		// now we have the extents of the near plane in camera coordinates setup transform
		#if (RAS_PARADIGM==RAS_DX_PARADIGM || RAS_PARADIGM==RAS_OGL_PARADIGM)
			mat4<T> orthographicTrns;
			orthographicTrns.setZero();
			orthographicTrns.coeffRef(0,0)=2.0/(r-l);
			orthographicTrns.coeffRef(1,1)=2.0/(t-b);

			#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
				// viewing frustrum mapped to cube [-1,1],[-1,1],[-1,1]
				// front plane becomes -1, far plane becomes 1
				// viewing frustrum mapped to left hand coordinate system
				orthographicTrns.coeffRef(0,3)=-((r+l)/(r-l));
				orthographicTrns.coeffRef(1,3)=-((t+b)/(t-b));
				orthographicTrns.coeffRef(2,2)=-(2.0/(m_dFar-m_dNear));
				orthographicTrns.coeffRef(2,3)=-((m_dFar+m_dNear)/(m_dFar-m_dNear));
				orthographicTrns.coeffRef(3,3)=1;
			#else
				// viewing frustrum mapped to cube [-1,1],[-1,1],[0,1]
				// front plane becomes 0, far plane becomes 1
				// viewing frustrum mapped to left hand coordinate system
				orthographicTrns.coeffRef(3,0)=-((r+l)/(r-l));
				orthographicTrns.coeffRef(3,1)=-((t+b)/(t-b));
				orthographicTrns.coeffRef(2,2)=(1.0/(m_dFar-m_dNear));
				orthographicTrns.coeffRef(3,2)=-((m_dNear)/(m_dFar-m_dNear));
				orthographicTrns.coeffRef(3,3)=1;
			#endif

			mat4<T> trns(rotation3<T>(vec3<T>(1,0,0),m_dSubTypeXAxisRotRadians));
			trns.mul(rotation3<T>(vec3<T>(0,1,0),m_dSubTypeYAxisRotRadians),trns);
			trns.mul(orthographicTrns,trns);
			m_Trns=trns;
		#endif
	}

	orthographicprojection& operator =(const orthographicprojection& o)
	{
		projection::operator =(o);
		m_dNear=o.m_dNear;
		m_dFar=o.m_dFar;
		m_dL=o.m_dL;
		m_dR=o.m_dR;
		m_dT=o.m_dT;
		m_dB=o.m_dB;
		m_dSubTypeXAxisRotRadians=o.m_dSubTypeXAxisRotRadians;
		m_dSubTypeYAxisRotRadians=o.m_dSubTypeYAxisRotRadians;
		m_spTrns=o.m_spTrns;
		return *this;
	}

	void unproject(const vec2<T>& devPos,const mat4<T>& cameraTrns,const crect<T>& rDevice,vec3<>& vRayOrigin,vec3<>& vRayDir)const
	{
		vRayOrigin=perspectiveprojection<T>::getunprojectworldpos(devPos,cameraTrns,*m_spTrns,rDevice);
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			vRayDir=vec3<T>(cameraTrns.coeff(2,0),cameraTrns.coeff(2,1),cameraTrns.coeffRef(2,2)).normalized();
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			vRayDir=vec3<T>(cameraTrns.coeff(0,2),cameraTrns.coeff(1,2),cameraTrns.coeffRef(2,2)).normalized();
		#endif
	}
protected:
	mat4<T> m_Trns;
	T m_dSubTypeXAxisRotRadians;
	T m_dSubTypeYAxisRotRadians;
	T m_dL,m_dR,m_dT,m_dB;
	T m_dNear;
	T m_dFar;
};

}
