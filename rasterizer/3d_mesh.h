#pragma once

#include "3d_light.h"
#include "3d_camera.h"
#include "hittest.h"

namespace af3d
{

template <typename FB> class mesh : public vertexattsframe<FB::template t_flt>
{
public:
	using t_flt=FB::template t_flt;
	using t_face=FB::template t_face;
	
	mesh(const vertexattsframe<FB::template t_flt>::type t=vertexattsframe<FB::template t_flt>::t_mesh):vertexattsframe(t,t_face::getatts()){}
	virtual ~mesh(){}
	
	__forceinline FB *getvertexbuffer(void)const{return m_spVertexBuffer.get();}
	void setvertexbuffer(std::shared_ptr<FB> sp){m_spVertexBuffer=sp;}

	__forceinline bool ht(const plane<t_flt>& worldcameraplane,const vec3<t_flt>& worldpos,const vec3<t_flt>& worlddir,hittest<t_flt>& res)const{return faceht(worldcameraplane,worldpos,worlddir,res);}
			
	mesh& operator =(const mesh& o)
	{
		vertexattsframe::operator=(o);
		m_spVertexBuffer=o.m_spVertexBuffer;
		return *this;
	}
protected:
	std::shared_ptr<FB> m_spVertexBuffer;
	virtual vertexattsframe* clone(vertexattsframe *p)const override
	{
		mesh *pF(new mesh);
		(*pF)=*this;
		pF->m_pParent=p;
		return pF;
	}
	__forceinline bool faceht(const plane<t_flt>& worldcameraplane,const vec3<t_flt>& worldpos,const vec3<t_flt>& worlddir,hittest<t_flt>& res)const
	{
		if(!m_spVertexBuffer)
			return false;

		const mat4<t_flt>& modelToWorld=getcompositetrns();
		const mat4<t_flt> worldtomodel=modelToWorld.inverse();

		vec3<t_flt> modelpos,modeldir,modelorigin;
		worldtomodel.mul(worldpos,modelpos);
		worldtomodel.mul(worlddir,modeldir);
		worldtomodel.mul({0,0,0},modelorigin);
		modeldir=(modeldir-modelorigin).normalize();

		bool bRetVal=false;
		vec2<t_flt> baryUV;
		vec3<t_flt> modelfaceintersect;
		auto i = m_spVertexBuffer->get().cbegin(),end=m_spVertexBuffer->get().cend();
		for(;i!=end;++i)
			if(faceintersect(modelpos,modeldir,(*i),baryUV,modelfaceintersect))
			{
				// check distance
				vec3<t_flt> world,inter;
				modelToWorld.mul(modelfaceintersect,world);
				if(!worldcameraplane.getintersect(world,world-worldcameraplane.getnormal(),inter))
					continue;
				const t_flt dDist=(inter-world).getlength();
				if(!res.getvertexframe() || res.getworlddist()>dDist)
				{
					bRetVal=true;
					res.setworlddist(dDist);
					res.setface(static_cast<int>(std::distance(m_spVertexBuffer->get().cbegin(),i)));
					res.settype(hittest<>::t_mesh);
					res.setbary(baryUV[0],baryUV[1]);
					res.setmodelspacefacepos(modelfaceintersect);
				}
			}
		return bRetVal;
	}
	__forceinline bool faceintersect(const vec3<t_flt>& modelpos,const vec3<t_flt>& modeldir,const face_pos3<t_flt>& face,vec2<t_flt>& baryUV,vec3<t_flt>& modelfaceintersect)const
	{
		// Möller–Trumbore

		// edges
		const vec3<t_flt> edge1 = face.getpos().getpos()[1] - face.getpos().getpos()[0];
		const vec3<t_flt> edge2 = face.getpos().getpos()[2] - face.getpos().getpos()[0];

		// cross product
		const vec3<t_flt> pvec = modeldir.cross(edge2);

		// determinant, if close to 0, the ray lies in the plane of the triangle
		const t_flt det = edge1.dot(pvec);
		constexpr bool bCullBackface = true;
		if(bCullBackface && det < 0)
			return false;
		if(fabs(det) < t_flt(1e-8))
			return false;
		const t_flt invDet = 1.0 / det;

		// vector from p0 to the ray origin
		const vec3<t_flt> tvec = modelpos - face.getpos().getpos()[0];

		// u parameter and test bounds
		const t_flt dU = tvec.dot(pvec) * invDet;
		if(dU < 0.0 || dU > 1.0)
			return false;
		
		// v parameter and test bounds
		const vec3<t_flt> qvec = tvec.cross(edge1);
	    const t_flt dV = modeldir.dot(qvec) * invDet;
		if(dV < 0.0 || dU + dV > 1.0)
			return false;

		// distance along the ray to the intersection point
		const t_flt dDist = edge2.dot(qvec) * invDet;
		baryUV[0]=dU;
		baryUV[1]=dV;
		modelfaceintersect=(face.getpos().getpos()[0]*(1-dU-dV)) + (face.getpos().getpos()[1]*dU) + (face.getpos().getpos()[2]*dV);
		return true;
	}
};

template <typename T=RAS_FLTTYPE> class lightmeshcache
{
public:
	lightmeshcache(){}
	virtual ~lightmeshcache(){}

	__forceinline light<T> *getlight(void)const{return m_spLight.get();}
	__forceinline void setlight(std::shared_ptr<light<T>> sp){m_spLight=sp;}
			
	void syncframe(vertexattsframe<T> *p){synctrns(p,true);}
	void synclight(vertexattsframe<T> *p){synctrns(p,false);}

	lightmeshcache& operator =(const lightmeshcache& o)
	{
		m_spLight=o.m_spLight;
		return *this;
	}
protected:
	std::shared_ptr<light<T>> m_spLight;
	void synctrns(vertexattsframe<T> *p,const bool bFrame)
	{
		p->validatecompositebbox();
		const af3d::facetrnsbbox<> worldbbox(p->getbbox(true),p->getcompositetrns());

		vec3<T> from_to[4][2]; // X,Y,Z,O(rigin)

		from_to[1][bFrame?0:1]=worldbbox.getup(af3d::facetrnsbbox<>::p_front,true);
		from_to[2][bFrame?0:1]=worldbbox.getdir(af3d::facetrnsbbox<>::p_front,true,true);
		from_to[0][bFrame?0:1]=from_to[1][bFrame?0:1].cross(from_to[2][bFrame?0:1]);
		from_to[3][bFrame?0:1]=worldbbox.getcentre();

		from_to[1][bFrame?1:0]=getlight()->getworldup();;
		from_to[2][bFrame?1:0]=getlight()->getworlddir();
		from_to[0][bFrame?1:0]=from_to[1][bFrame?1:0].cross(from_to[2][bFrame?1:0]);
		from_to[3][bFrame?1:0]=getlight()->getworldpos();
		
		mat4<T> modeltoworld;
		if(bFrame)
			modeltoworld=p->getcompositetrns();
		else
			vertexattsframe<T>::getaatrns({1,0,0},{0,1,0},{0,0,getfwd<T>()},{0,0,0},from_to[0][0],from_to[1][0],from_to[2][0],from_to[3][0],modeltoworld);

		mat4<T> aa;
		vertexattsframe<T>::getaatrns(from_to[0][0],from_to[1][0],from_to[2][0],from_to[3][0],
									  from_to[0][1],from_to[1][1],from_to[2][1],from_to[3][1],aa);
		modeltoworld.mul(aa);

		if(bFrame)
		{
			mat4<T> trns;
			modeltoworld.mul(p->getparent()->getcompositetrns().inverse(),trns);
			p->settrns(trns);
			p->setcomposite();
			p->invalidatecompositebbox();
		}
		else
		{
			vec3<T> o,y,z;
			modeltoworld.mul({0,0,0},o);
			modeltoworld.mul({0,1,0},y);
			modeltoworld.mul({0,0,getfwd<T>()},z);
			getlight()->setworldpos(o);
			getlight()->setworldup((y-o).normalize());
			getlight()->setworlddir((z-o).normalize());
		}
		#ifdef _DEBUG
		{
			vec3<T> o[2],x[2],y[2],z[2];
			o[0]=getlight()->getworldpos();
			y[0]=getlight()->getworldup();
			z[0]=getlight()->getworlddir();
			#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
				x[0]=z[0].cross(y[0]);
			#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
				x[0]=y[0].cross(z[0]);
			#endif

			p->validatecompositebbox();

			vec3<T> tmp;
			p->getcompositetrns().mul({0,0,0},tmp);
			p->getcompositetrns().mul(p->getbbox(true).getfrontmin(),o[1]);

			vec3<T> tmp1;
			p->getcompositetrns().mul({1,0,0},tmp1);
			x[1]=(tmp1-tmp).normalize();

			p->getcompositetrns().mul({0,1,0},tmp1);
			y[1]=(tmp1-tmp).normalize();

			#if (RAS_PARADIGM==RAS_OGL_PARADIGM)
				p->getcompositetrns().mul({0,0,-1},tmp1);
			#elif (RAS_PARADIGM==RAS_DX_PARADIGM)
				p->getcompositetrns().mul({0,0,1},tmp1);
			#endif
			z[1]=(tmp1-tmp).normalize();

			ASSERT(fabs(x[0][0]-x[1][0])<1e-3);
			ASSERT(fabs(x[0][1]-x[1][1])<1e-3);
			ASSERT(fabs(x[0][2]-x[1][2])<1e-3);

			ASSERT(fabs(y[0][0]-y[1][0])<1e-3);
			ASSERT(fabs(y[0][1]-y[1][1])<1e-3);
			ASSERT(fabs(y[0][2]-y[1][2])<1e-3);

			ASSERT(fabs(z[0][0]-z[1][0])<1e-3);
			ASSERT(fabs(z[0][1]-z[1][1])<1e-3);
			ASSERT(fabs(z[0][2]-z[1][2])<1e-3);
		}
		#endif
	}
};

template <typename FB> class lightmesh : public mesh<FB>,public lightmeshcache<FB::template t_flt>
{
public:
	lightmesh():mesh<FB>(mesh<FB>::t_light_mesh){}
	virtual ~lightmesh(){}
	
	void sync(void){syncframe(this);}

	lightmesh& operator =(const lightmesh& o)
	{
		mesh::operator=(o);
		lightmeshcache::operator=(o);
		return *this;
	}
protected:
	virtual vertexattsframe* clone(vertexattsframe *p)const override
	{
		lightmesh *pF(new lightmesh());
		(*pF)=*this;
		pF->m_pParent=p;
		return pF;
	}
};

}
