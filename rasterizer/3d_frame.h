#pragma once

#include "3d.h"
#include "3d_face_pos_aux.h"
#include "3d_material.h"

namespace af3d
{

template <typename T=RAS_FLTTYPE> class vertexattsframe
{
public:
	enum type {t_scene=0x1,t_mesh=0x2,t_light_mesh=0x4,t_shadow=0x8,
			   t_all=(t_scene|t_mesh|t_light_mesh|t_shadow)};

	virtual ~vertexattsframe(){}

	template <typename R> static std::shared_ptr<R>clone(const R *p,const vertexattsframe *pParent) {return std::shared_ptr<R>(static_cast<R*>(static_cast<const vertexattsframe*>(p)->clone(pParent)));}

	__forceinline type gettype(void)const{return m_Type;}
	__forceinline int getvertexatts(void)const{return m_nVertexAtts;}
	
	__forceinline vertexattsframe *getparent(void)const{return m_pParent;}
	__forceinline const std::vector<std::shared_ptr<vertexattsframe>> *getchildren(void)const{return m_spChildren.get();}
	__forceinline std::shared_ptr<vertexattsframe> getchild(int n)const{if(!m_spChildren||n<0||n>=static_cast<int>(m_spChildren->size()))return nullptr;return (*m_spChildren)[n];}
	bool isdescendant(vertexattsframe<T> *p){if(!p)return false;return (p->getparent()==this || isdescendant(p->getparent()));}

	bool getindex(int& n)const{if(!m_pParent||!m_pParent->getchildren())return false;auto i=m_pParent->getchildren()->cbegin(),end=m_pParent->getchildren()->cend();for(n=0;i!=end;++i,++n)if((*i).get()==this)return true;return false;}
	bool reparent(vertexattsframe *pChild,vertexattsframe *pParent,vertexattsframe *pTarget,const bool bAbove)
	{
		int nChild;
		const int nChildren=getchildren()?static_cast<int>(getchildren()->size()):0;
		if(!pChild || !pChild->getindex(nChild) || pChild==pTarget)
			return false;

		vertexattsframe *pPreParent=pChild?pChild->getparent():nullptr;
		std::function<void(af3d::vertexattsframe<> *p)> invalidatecompositebbox=[pPreParent](af3d::vertexattsframe<> *p) -> void
		{
			if(!p || p->getparent()==pPreParent)return;
			if(pPreParent)
				pPreParent->invalidatecompositebbox();
			if(p)
				p->invalidatecompositebbox();
		};

		if(!pTarget)
		{
			if(!pParent || !pParent->getparent())
				return false;
			
			std::shared_ptr<vertexattsframe> sp=(*m_spChildren)[nChild];
			
			mat4<T> trns;
			const mat4<T> modeltoworld = sp->getcompositetrns();
			modeltoworld.mul(pParent->getcompositetrns().inverse(),trns);

			erase(sp.get());
			pParent->push_back(sp);

			sp->setparent(pParent);
			sp->settrns(trns);
			sp->setcomposite();

			invalidatecompositebbox(sp.get());
			
			return true;
		}

		int nTarget;
		if(!pTarget->getindex(nTarget))
			return false;

		int nPos;
		if(bAbove)
			nPos=nTarget>0?nTarget-1:0;
		else
			nPos=nTarget<nChildren?nTarget+1:nChildren;
		if(nPos<0 || nPos>nChildren)
			return false;

		std::shared_ptr<vertexattsframe> sp=(*m_spChildren)[nChild];

		if(pParent==this)
		{
			// reposition
			if(nPos==nChild)
				return false;
			
			pParent->insert(sp,nPos);
			erase((nPos<nChild)?nChild+1:nChild);
			
			invalidatecompositebbox(sp.get());
			
			return true;
		}

		if(pChild->isdescendant(pParent))
			return false;

		mat4<T> trns;
		const mat4<T> modeltoworld = sp->getcompositetrns();
		modeltoworld.mul(pParent->getcompositetrns().inverse(),trns);

		erase(sp.get());
		pParent->insert(sp,nPos);
			
		sp->setparent(pParent);
		sp->settrns(trns);
		sp->setcomposite();

		invalidatecompositebbox(sp.get());

		return true;
	}
	
	__forceinline const mat4<T>& gettrns(void)const{return m_Trns;}
	__forceinline const mat4<T>& getcompositetrns(void)const{return m_CompositeTrns;}
	
	__forceinline bool getvisible(void)const{return m_bVisible;}
	void setvisible(const bool b){m_bVisible=b;}

	__forceinline const facemodelbbox<T>& getbbox(const bool bComposite)const{return bComposite?m_CompositeBBox:m_BBox;}
	void setbbox(const facemodelbbox<T>& b){m_BBox=b;}
	void invalidatecompositebbox(void)
	{
		m_bCompositeBBoxInvalid=true;
		if(m_pParent && m_pParent->gettype() != t_scene)
			m_pParent->invalidatecompositebbox();
	}
	void validatecompositebbox(void)
	{
		if(!m_bCompositeBBoxInvalid)return;
		m_bCompositeBBoxInvalid=false;
		m_CompositeBBox=m_BBox;
		if(!m_spChildren)return;

		auto i=m_spChildren->cbegin(),end=m_spChildren->cend();
		for(;i!=end;++i)
		{
			(*i)->validatecompositebbox();
			m_CompositeBBox=m_CompositeBBox.getunion(facemodelbbox<T>(facetrnsbbox<T>((*i)->getbbox(true),(*i)->gettrns())));
		}
	}
	
	const std::string& getname(void)const{return m_Name;}
	void setname(const std::string& s){m_Name=s;}
	
	__forceinline T getopacity(void)const{return m_dOpacity;}
	void setopacity(const T d){m_dOpacity=d;}

	void clear(void){m_pParent=nullptr;cleartrns();clearchildren();m_BBox.clear();m_CompositeBBox.clear();}
	void cleartrns(void){m_spTrns=nullptr;m_spCompositeTrns=nullptr;}
	void clearchildren(void){m_vChildren.clear();}

	void setparent(vertexattsframe *p){m_pParent=p;}

	void settrns(const mat4<T>& p){m_Trns=p;}
	void setcomposite(void)
	{
		// current transform followed by parent composite transform
		if(m_pParent)
			m_Trns.mul(m_pParent->getcompositetrns(),m_CompositeTrns);
		else
			m_CompositeTrns=m_Trns;
		if(!m_spChildren)return;
		auto i=m_spChildren->begin(),end=m_spChildren->end();
		for(;i!=end;++i)
			(*i)->setcomposite();
	}

	void push_back(std::shared_ptr<vertexattsframe<T>> sp)
	{
		if(!m_spChildren)
			m_spChildren=std::shared_ptr<std::vector<std::shared_ptr<vertexattsframe>>>(new std::vector<std::shared_ptr<vertexattsframe>>);
		m_spChildren->push_back(sp);
	}
	bool insert(std::shared_ptr<vertexattsframe<T>> sp,const int nPos)
	{
		int nChildren=getchildren()?static_cast<int>(getchildren()->size()):0;
		if(nPos<0 || nPos>nChildren)
			return false;
		if(!m_spChildren)
			m_spChildren=std::shared_ptr<std::vector<std::shared_ptr<vertexattsframe>>>(new std::vector<std::shared_ptr<vertexattsframe>>);
		m_spChildren->insert(m_spChildren->cbegin()+nPos,sp);
		return true;
	}
	bool erase(vertexattsframe<T> *p)
	{
		int n;
		if(p && p->getparent()==this && p->getindex(n))
		{
			m_spChildren->erase(m_spChildren->cbegin()+n);
			return true;
		}
		return false;
	}

	void bind(const std::vector<std::shared_ptr<material<T>>>& v)
	{
		if(!m_spMaterials)
			m_spMaterials=std::shared_ptr<std::vector<std::shared_ptr<material<T>>>>(new std::vector<std::shared_ptr<material<T>>>);
		m_spMaterials->insert(m_spMaterials->end(),v.cbegin(),v.cend());
		sortmaterials();
	}
	bool release(const material<T> *p)
	{
		if(!m_spMaterials || !p)
			return false;
		auto i=m_spMaterials->cbegin(),end=m_spMaterials->cend();
		for(;i!=end;++i)
			if((*i).get()==p)
			{
				m_spMaterials->erase(i);
				if(m_spMaterials->size()==0)
					m_spMaterials=nullptr;
				return true;
			}
		return false;
	}
	__forceinline const std::vector<std::shared_ptr<material<T>>> *getmaterials(void)const{return m_spMaterials.get();}
	void sortmaterials(void){std::sort(m_spMaterials->begin(),m_spMaterials->end(),[](const std::shared_ptr<material<T>>& a,std::shared_ptr<material<T>>& b)->bool{return a->getfrom()<b->getfrom();});}
	bool getmaterialindex(const material<T> *p,int& n)const{if(!m_spMaterials)return false;auto i=m_spMaterials->cbegin(),end=m_spMaterials->cend();for(n=0;i!=end;++i,++n)if((*i).get()==p)return true;return false;}
	
	vertexattsframe& operator =(const vertexattsframe& o)
	{
		m_nVertexAtts=o.m_nVertexAtts;
		m_pParent=o.m_pParent;
		m_Trns=o.m_Trns;
		m_CompositeTrns=o.m_CompositeTrns;
		m_spChildren=o.m_spChildren;
		m_BBox=o.m_BBox;
		m_CompositeBBox=o.m_CompositeBBox;
		m_bVisible=o.m_bVisible;
		m_Type=o.m_Type;
		m_Name=o.m_Name;
		m_dOpacity=o.m_dOpacity;
		m_spMaterials=o.m_spMaterials;
		m_bCompositeBBoxInvalid=o.m_bCompositeBBoxInvalid;
		return *this;
	}

	static void getaatrns(const vec3<T>& xfrom,const vec3<T>& yfrom,const vec3<T>& zfrom,const vec3<T>& ofrom,
						  const vec3<T>& xto,const vec3<T>& yto,const vec3<T>& zto,const vec3<T>& oto,
						  mat4<T>& res)
	{
		mat4<T> trnsAA;
		getaatrns(xfrom,yfrom,zfrom,xto,yto,zto,trnsAA);
		mat4<T>::mul(translate3<T>(-ofrom),trnsAA,res);
		res.mul(translate3<T>(oto));
	}
	static void getaatrns(const vec3<T>& xfrom,const vec3<T>& yfrom,const vec3<T>& zfrom,
						  const vec3<T>& xto,const vec3<T>& yto,const vec3<T>& zto,
						  mat4<T>& res)
	{
		// assuming all vec3's normalised
		mat4<T> from,to;
		from.setIdentity();to.setIdentity();
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			for(int n=0;n<3;++n) { from.coeffRef(0,n)=xfrom[n]; to.coeffRef(0,n)=xto[n]; }
			for(int n=0;n<3;++n) { from.coeffRef(1,n)=yfrom[n]; to.coeffRef(1,n)=yto[n]; }
			for(int n=0;n<3;++n) { from.coeffRef(2,n)=zfrom[n]; to.coeffRef(2,n)=zto[n]; }
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			for(int n=0;n<3;++n) { from.coeffRef(n,0)=xfrom[n]; to.coeffRef(n,0)=xto[n]; }
			for(int n=0;n<3;++n) { from.coeffRef(n,1)=yfrom[n]; to.coeffRef(n,1)=yto[n]; }
			for(int n=0;n<3;++n) { from.coeffRef(n,2)=zfrom[n]; to.coeffRef(n,2)=zto[n]; }
		#endif
		if(from.isinvertable())
			mat4<T>(from.inverse()).mul(to,res);
		else
			res.setIdentity();
		#ifdef _DEBUG
			vec3<T> tmpx,tmpy,tmpz;
			res.mul(xfrom,tmpx);
			res.mul(yfrom,tmpy);
			res.mul(zfrom,tmpz);
		#endif
	}

	static void get(vertexattsframe<T> *pFrom,const int nTypes,std::vector<vertexattsframe<T>*>& vFrames)
	{
		vFrames.clear();
		get_ex(pFrom,nTypes,vFrames);
	}
protected:
	type m_Type;
	int m_nVertexAtts;
	bool m_bVisible;
	bool m_bCompositeBBoxInvalid;
	vertexattsframe *m_pParent;
	std::shared_ptr<std::vector<std::shared_ptr<vertexattsframe>>> m_spChildren;
	mat4<T> m_Trns;
	mat4<T> m_CompositeTrns;
	facemodelbbox<T> m_BBox;
	facemodelbbox<T> m_CompositeBBox;
	std::string m_Name;
	T m_dOpacity;
	std::shared_ptr<std::vector<std::shared_ptr<material<T>>>> m_spMaterials;

	vertexattsframe(const type t,const int nVertexAtts):m_Type(t),m_nVertexAtts(nVertexAtts),m_pParent(nullptr),m_bVisible(true),m_dOpacity(1),m_bCompositeBBoxInvalid(true){m_Name="frame";}
	virtual vertexattsframe* clone(vertexattsframe *pParent)const=0;

	static void get_ex(vertexattsframe<T> *pFrom,const int nTypes,std::vector<vertexattsframe<T>*>& vFrames)
	{
		if(!pFrom) return;
		if(pFrom->gettype()&nTypes) vFrames.push_back(pFrom);
		
		if(!pFrom->getchildren()) return;		
		auto i=pFrom->getchildren()->cbegin(),end=pFrom->getchildren()->cend();
		for(;i!=end;++i) get_ex((*i).get(),nTypes,vFrames);
	}
	bool erase(const int n)
	{
		const int nChildren=getchildren()?static_cast<int>(getchildren()->size()):0;
		if(n<0||n>=nChildren)
			return false;
		if((*m_spChildren)[n]->getparent()!=this)
			return false;
		m_spChildren->erase(m_spChildren->cbegin()+n);
		return true;
	}
};

}
