#pragma once

#include "3d.h"

namespace af3d
{

// maps points fromn world coordinate system to camera coordinate system

template <typename T=RAS_FLTTYPE> class camera
{
public:
	camera(){}
	virtual ~camera(){}

	virtual std::shared_ptr<camera>clone(void)const{std::shared_ptr<camera> sp(new camera);(*sp)=*this;return sp;}

	__forceinline const mat4<T>& gettrns(void)const{return m_Trns;}
	__forceinline const vec3<T>& getorigin(void)const{return m_Origin;}
	__forceinline const vec3<T>& getup(void)const{return m_Up;}
	__forceinline const vec3<T>& getdir(void)const{return m_Dir;}
	
	void setup(const vec3<T>& p){m_Up=p;}
	void setdir(const vec3<T>& p){m_Dir=p;}
	void setorigin(const vec3<T>& p){m_Origin=p;}
	void settrns(const mat4<T>& t){m_Trns=t;}
	void settrns(void)
	{
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			const vec3<T> f=m_Dir.normalized();
			const vec3<T> s=m_Up.cross(f).normalized();
			const vec3<T> u=f.cross(s);
			
			mat4<T> trns;
			trns.setZero();
			trns.coeffRef(3,3) = 1.0f;

			trns.coeffRef(0,0) = s[0];
			trns.coeffRef(1,0) = s[1];
			trns.coeffRef(2,0) = s[2];
			trns.coeffRef(0,1) = u[0];
			trns.coeffRef(1,1) = u[1];
			trns.coeffRef(2,1) = u[2];
			trns.coeffRef(0,2) = f[0];
			trns.coeffRef(1,2) = f[1];
			trns.coeffRef(2,2) = f[2];
			
			trns.coeffRef(3,0) =-s.dot(m_Origin);
			trns.coeffRef(3,1) =-u.dot(m_Origin);
			trns.coeffRef(3,2) =-f.dot(m_Origin);

			m_Trns=trns;
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			const vec3<T> f=m_Dir.normalized();
			const vec3<T> s=f.cross(m_Up).normalized();
			const vec3<T> u=s.cross(f);
			
			mat4<T> trns;
			trns.setZero();
			trns.coeffRef(3,3) = 1.0f;

			trns.coeffRef(0,0) = s[0];
			trns.coeffRef(0,1) = s[1];
			trns.coeffRef(0,2) = s[2];
			trns.coeffRef(1,0) = u[0];
			trns.coeffRef(1,1) = u[1];
			trns.coeffRef(1,2) = u[2];
			trns.coeffRef(2,0) =-f[0];
			trns.coeffRef(2,1) =-f[1];
			trns.coeffRef(2,2) =-f[2];
			
			trns.coeffRef(0,3) =-s.dot(m_Origin);
			trns.coeffRef(1,3) =-u.dot(m_Origin);
			trns.coeffRef(2,3) = f.dot(m_Origin);
			
			m_Trns=trns;
		#endif
	}

	camera& operator =(const camera& o)
	{
		m_Up=o.m_Up;
		m_Dir=o.m_Dir;
		m_Origin=o.m_Origin;
		m_Trns=o.m_Trns;
		return *this;
	}
protected:
	vec3<T> m_Up;
	vec3<T> m_Dir;
	vec3<T> m_Origin;
	mat4<T> m_Trns;
};

}
