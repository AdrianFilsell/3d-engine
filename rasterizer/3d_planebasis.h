#pragma once

#include "3d.h"

namespace af3d
{

template <typename T=RAS_FLTTYPE> class planebasis
{
public:
	planebasis(){}
	planebasis(const planebasis<T>& o){*this=o;}
	planebasis(const vec3<T>& origin,const vec3<T>& U,const vec3<T>& V):m_O(origin),m_U(U),m_V(V){}
	
	const vec3<T>& getorigin(void)const{return m_O;}
	void getbasis(const vec3<T>& vFrom,vec2<T>& vTo)const
	{
		// origin relative
		const vec3<T> v=(vFrom-m_O);
		
		// project vFrom onto U basis vector
		vTo[0]=v.dot(m_U);

		// project vFrom onto V basis vector
		vTo[1]=v.dot(m_V);
	}
	T getbasisradians(const vec3<T>& vFrom)const
	{
		vec2<T> vTo;
		getbasis(vFrom,vTo);
		return atan2(vTo[1], vTo[0]);
	}
	void get3d(const vec2<T>& vFrom,vec3<T>& vTo)const
	{
		// scale basis vectors to reconstruct 3d point
		vTo=m_O+(m_U*vFrom[0] + m_V*vFrom[1]);
	}

	planebasis& operator =(const planebasis& o)
	{
		m_O=o.m_O;
		m_U=o.m_U;
		m_V=o.m_V;
		return *this;
	}
protected:
	vec3<T> m_O;
	vec3<T> m_U;
	vec3<T> m_V;
};

}
