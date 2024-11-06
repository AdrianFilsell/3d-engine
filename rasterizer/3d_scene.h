#pragma once

#include "3d_mesh.h"
#include "3d_light.h"

namespace af3d
{

template <typename T=RAS_FLTTYPE> class scene : public vertexattsframe<T>
{
public:
	scene():vertexattsframe(vertexattsframe<T>::t_scene,0)
	{
		m_spDirLight=std::shared_ptr<light<T>>(new light<T>);
		m_spDirLight->settype(light<T>::t_directional);
		setname("scene");
	}
	virtual ~scene(){}

	__forceinline const std::vector<vertexattsframe<T>*> *getlights(const bool bFilterOn)const{return bFilterOn?m_spOnLights.get():m_spLights.get();}
	__forceinline light<T> *getdirlight(void)const{return m_spDirLight.get();}
	
	void cache_visible_lights(void)
	{
		if(!m_spLights)
			m_spOnLights=nullptr;
		else
		{
			std::shared_ptr<std::vector<vertexattsframe<T>*>> spOnLights=m_spOnLights?m_spOnLights:std::shared_ptr<std::vector<vertexattsframe<T>*>>(new std::vector<vertexattsframe<T>*>);
			spOnLights->clear();
			spOnLights->reserve(m_spLights->size());
			auto i = m_spLights->cbegin(),end=m_spLights->cend();
			for(;i!=end;++i)
				if((*i)->getvisible())
					spOnLights->push_back(*i);
			m_spOnLights=spOnLights;
		}
	}
	void cache_lights(void)
	{
		m_spLights=std::shared_ptr<std::vector<vertexattsframe<T>*>>(new std::vector<vertexattsframe<T>*>);
		getlights(this,*m_spLights);

		cache_visible_lights();
	}
	void erase_lights(vertexattsframe *p)
	{
		if(!m_spLights) return;

		std::vector<vertexattsframe<T>*> vLights;
		getlights(p,vLights);
		
		std::sort(vLights.begin(),vLights.end());
		for(int n=static_cast<int>(m_spLights->size()-1);n>=0 && vLights.size();--n)
		{
			// get first iterator representing position >= val
			auto i=std::lower_bound(vLights.cbegin(),vLights.cend(),(*m_spLights)[n]);
			if(i==vLights.cend())
				continue;
			if((*i)!=(*m_spLights)[n])
				continue;
			m_spLights->erase(m_spLights->cbegin()+n);
			vLights.erase(i);
		}

		cache_visible_lights();
	}
	bool reparent(vertexattsframe *p,vertexattsframe *pParent,vertexattsframe *pTarget,const bool bAbove)
	{
		if(!p || !p->getparent() || !p->getparent()->reparent(p,pParent,pTarget,bAbove))
			return false;
		cache_lights();
				
		return true;
	}
	
	scene& operator =(const scene& o)
	{
		vertexattsframe::operator=(o);
		m_spLights=o.m_spLights;
		m_spOnLights=o.m_spOnLights;
		m_spDirLight=o.m_spDirLight;
		return *this;
	}
protected:
	std::shared_ptr<std::vector<vertexattsframe<T>*>> m_spLights;
	std::shared_ptr<std::vector<vertexattsframe<T>*>> m_spOnLights;
	std::shared_ptr<light<T>> m_spDirLight;
	virtual vertexattsframe* clone(vertexattsframe *p)const override
	{
		scene *pF(new scene);
		(*pF)=*this;
		pF->m_pParent=p;
		return pF;
	}
	static void getlights(vertexattsframe *p,std::vector<vertexattsframe<T>*>& v)
	{
		get(p,vertexattsframe<T>::t_light_mesh,v);
	}
};

}
