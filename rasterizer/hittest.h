#pragma once

#include "3d_frame.h"

template <typename T=RAS_FLTTYPE> class hittest
{
public:
	using t_flt=T;
	
	enum type{t_null,t_mesh,t_bbox_handle,t_rotate_handle};
	
	hittest() {m_p=nullptr;m_nHandle=-1;m_Type=t_null;m_rtpt = {0,0}; }	
	hittest( const hittest &other ):hittest(){(*this)=other;}
	~hittest(){}
	
	type gettype(void)const{return m_Type;}
	const af3d::vec2<int>& getrtpt( void ) const { return m_rtpt; }
	af3d::vertexattsframe<T> *getvertexframe(void)const{return m_p;}
	int getface(void)const{return m_nFace;}
	T getbaryU(void)const{return m_dBaryU;}
	T getbaryV(void)const{return m_dBaryV;}
	T getworlddist(void)const{return m_dWorldDist;}
	const af3d::vec3<T>& getmodelspacefacepos(void)const{return m_ModelSpaceFacePos;}
	const af3d::vec3<T>& getworldhandle(void)const {return m_WorldSpaceHandle;}
	int gethandle(void)const {return m_nHandle;}
	
	void settype(const type t){m_Type=t;}
	void setrtpt( const af3d::vec2<int>& p ) { m_rtpt = p; }
	void setface(const int n){m_nFace=n;}
	void setvertexframe(af3d::vertexattsframe<T> *p){m_p=p;}
	void setbary(const T dU,const T dV){m_dBaryU=dU;m_dBaryV=dV;}
	void setworlddist(const T d){m_dWorldDist=d;}
	void setmodelspacefacepos(const af3d::vec3<T>& p){m_ModelSpaceFacePos=p;}
	void setworldhandle(const int n,const af3d::vec3<T>& pt){m_WorldSpaceHandle=pt;m_nHandle=n;}
		
	LPCTSTR getcursor(void)const
	{
		switch(m_Type)
		{
			case t_mesh:
			case t_bbox_handle:
			case t_rotate_handle:return IDC_HAND;
		}
		return IDC_ARROW;
	}

	hittest& operator =( const hittest& o)
	{
		m_nHandle=o.m_nHandle;
		m_WorldSpaceHandle=o.m_WorldSpaceHandle;
		m_ModelSpaceFacePos=o.m_ModelSpaceFacePos;
		m_Type=o.m_Type;
		m_rtpt=o.m_rtpt;
		m_p=o.m_p;
		m_nFace=o.m_nFace;
		m_dBaryU=o.m_dBaryU;
		m_dBaryV=o.m_dBaryV;
		m_dWorldDist=o.m_dWorldDist;
		return *this;
	}
protected:
	type m_Type;
	af3d::vec2<int> m_rtpt;
	af3d::vertexattsframe<T> *m_p;
	int m_nFace;
	T m_dBaryU;
	T m_dBaryV;
	T m_dWorldDist;
	af3d::vec3<T> m_ModelSpaceFacePos;
	af3d::vec3<T> m_WorldSpaceHandle;
	int m_nHandle;
};
