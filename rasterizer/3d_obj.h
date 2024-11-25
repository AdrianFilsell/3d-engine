#pragma once

#include "3d_face_pos_aux.h"
#include "3d_material.h"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace af3d
{

template <typename T=RAS_FLTTYPE> class objcfg
{
public:
	objcfg(const objcfg& o){*this=o;}
	objcfg(const int nVertexAtts,const std::pair<bool,vec3<T>>& c, const face_norm_vertex_data<T>::template type n, const face_tex_vertex_data<T>::template type tex, const face_tex_vertex_data<T>::template type bump,const material<T>& mat,const T dOpacity, const vertexattsframe<T>::template effecttype e):m_nVertexAtts(nVertexAtts),m_dOpacity(dOpacity),m_Mat(mat),m_FaceColour(c),m_FaceNormal(n),m_FaceTexUV(tex),m_FaceBumpUV(bump),m_Effect(e){}

	T getopacity(void)const{return m_dOpacity;}
	vertexattsframe<T>::template effecttype geteffect(void)const{return m_Effect;}
	const material<T>& getmat(void)const{return m_Mat;}
	const std::pair<bool,vec3<T>>& getfacecolour(void)const{return m_FaceColour;}
	face_norm_vertex_data<T>::template type getfacenormal(void)const{return m_FaceNormal;}
	face_tex_vertex_data<T>::template type getfacetexuv(void)const{return m_FaceTexUV;}
	face_tex_vertex_data<T>::template type getfacebumpuv(void)const{return m_FaceBumpUV;}
	int getvertexatts(void)const{return m_nVertexAtts;}

	objcfg& operator =(const objcfg& o)
	{
		m_dOpacity=o.m_dOpacity;
		m_Mat=o.m_Mat;
		m_FaceColour=o.m_FaceColour;
		m_FaceNormal=o.m_FaceNormal;
		m_FaceTexUV=o.m_FaceTexUV;
		m_FaceBumpUV=o.m_FaceBumpUV;
		m_nVertexAtts=o.m_nVertexAtts;
		m_Effect=o.m_Effect;
		return *this;
	}
protected:
	std::pair<bool,vec3<T>> m_FaceColour;
	face_norm_vertex_data<T>::template type m_FaceNormal;
	face_tex_vertex_data<T>::template type m_FaceTexUV;
	face_tex_vertex_data<T>::template type m_FaceBumpUV;
	material<T> m_Mat;
	T m_dOpacity;
	vertexattsframe<T>::template effecttype m_Effect;
	int m_nVertexAtts;
};

template <typename T=RAS_FLTTYPE> class	obj
{
public:
	enum primitivetype {pt_unit_camera,pt_unit_spot_light,pt_unit_directional_light,pt_unit_point_light};

	obj():m_bRightHanded(true){}
	~obj(){}

	const facemodelbbox<T>& getbbox(void)const{return m_BBox;}
	template<typename FB> void push_back(FB *pDst,const objcfg<T>& cfg)const
	{
		if(!pDst||!m_spVertexData)
			return;

		#if(RAS_PARADIGM==RAS_DX_PARADIGM)
			const bool bCCWVertices=false;
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			const bool bCCWVertices=true;
		#endif
		const bool bObjCCWVertices=m_bRightHanded;

		for(size_t nFace=0;nFace<m_spFaceVertices->size();++nFace)
		{
			const vec3<int>& vFaceVertices=(*m_spFaceVertices)[nFace];
			const vec3<T>& a=(*m_spVertexData)[vFaceVertices[0]];
			const vec3<T>& b=(*m_spVertexData)[vFaceVertices[1]];
			const vec3<T>& c=(*m_spVertexData)[vFaceVertices[2]];
			
			const vec3<T> *pNormA=nullptr,*pNormB=nullptr,*pNormC=nullptr;
			if(m_spNormalData)
			{
				const vec3<int>& vFaceNormals=(*m_spFaceNormals)[nFace];
				pNormA=&(*m_spNormalData)[vFaceNormals[0]];
				pNormB=&(*m_spNormalData)[vFaceNormals[1]];
				pNormC=&(*m_spNormalData)[vFaceNormals[2]];
			}

			const vec2<T> *pTexCoordA=nullptr,*pTexCoordB=nullptr,*pTexCoordC=nullptr;
			if(m_spTexCoordData)
			{
				const vec3<int>& vFaceTexCoords=(*m_spFaceTexCoords)[nFace];
				pTexCoordA=&(*m_spTexCoordData)[vFaceTexCoords[0]];
				pTexCoordB=&(*m_spTexCoordData)[vFaceTexCoords[1]];
				pTexCoordC=&(*m_spTexCoordData)[vFaceTexCoords[2]];
			}

			using t_face=FB::template t_face;
			pDst->push_back(createface<t_face>(a,pNormA,pTexCoordA,b,pNormB,pTexCoordB,c,pNormC,pTexCoordC,cfg));
		}
	}
	bool read(const std::wstring& filename)
	{
		std::ifstream file(filename);
		if(!file.is_open())
			return false;
	
		m_bRightHanded=true;
		
		int nMaxVertexIndex=-1;
		int nMaxNormalIndex=-1;
		int nMaxTexIndex=-1;

		std::string line;
		std::shared_ptr<std::vector<vec3<T>>> spVertexData(new std::vector<vec3<T>>);
		std::shared_ptr<std::vector<vec3<T>>> spNormalData(new std::vector<vec3<T>>);
		std::shared_ptr<std::vector<vec2<T>>> spTexCoordData(new std::vector<vec2<T>>);
		std::shared_ptr<std::vector<vec3<int>>> spFaceVertices(new std::vector<vec3<int>>);
		std::shared_ptr<std::vector<vec3<int>>> spFaceTexCoords(new std::vector<vec3<int>>);
		std::shared_ptr<std::vector<vec3<int>>> spFaceNormals(new std::vector<vec3<int>>);

		while(std::getline(file,line))
		{
			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;
			if(prefix == "v")
			{
				vec3<T> v;
				iss >> v[0] >> v[1] >> v[2];
				spVertexData->push_back(v);
			}
			else
			if(prefix == "vn")
			{
				vec3<T> v;
				iss >> v[0] >> v[1] >> v[2];
				spNormalData->push_back(v);
			}
			else
			if(prefix == "vt")
			{
				vec2<T> v;
				iss >> v[0] >> v[1];
				spTexCoordData->push_back(v);
			}
			else
			if(prefix == "f")
			{
				int nV=0;
				std::string vertex;
				vec3<int> f{-1,-1,-1},fn{-1,-1,-1},ft{-1,-1,-1};
				while (iss >> vertex && nV<3)
				{
					std::string index;
					std::istringstream vertexStream(vertex);

					std::getline(vertexStream, index, '/');
					f[nV] = std::stoi(index) - 1;

					if(f[nV]>nMaxVertexIndex)
						nMaxVertexIndex=f[nV];

					if(vertexStream.peek() == '/')
					{
						// no tex but there is a normal
						vertexStream.get(); // consume the '/'
					}
					else
					{
						// tex
						std::getline(vertexStream, index, '/');
						if (!index.empty())
						{
							ft[nV] = std::stoi(index) - 1;

							if(ft[nV]>nMaxTexIndex)
								nMaxTexIndex=ft[nV];
						}
					}
					
					if(vertexStream.peek() == '/')
					{
						// erroneous, what could be after the normal???
						vertexStream.get(); // consume the '/'
					}
					else
					{
						// normal
						std::getline(vertexStream, index, '/');
						if (!index.empty())
						{
							fn[nV] = std::stoi(index) - 1;

							if(fn[nV]>nMaxNormalIndex)
								nMaxNormalIndex=fn[nV];
						}
					}

					++nV;
				}
				
				if(f[0]>-1 && f[1]>-1 && f[2]>-1)
					spFaceVertices->push_back(f);
				if(fn[0]>-1 && fn[1]>-1 && fn[2]>-1)
					spFaceNormals->push_back(fn);
				if(ft[0]>-1 && ft[1]>-1 && ft[2]>-1)
					spFaceTexCoords->push_back(ft);
			}
		}

		const bool bVertexData=spVertexData && spVertexData->size() && spFaceVertices && spFaceVertices->size();
		const bool bNormalData=spNormalData && spNormalData->size() && spFaceNormals && spFaceNormals->size();
		const bool bTexCoordData=spTexCoordData && spTexCoordData->size() && spFaceTexCoords && spFaceTexCoords->size();

		if(!bVertexData)
		{
			spVertexData=nullptr;
			spFaceVertices=nullptr;
		}
		if(!bNormalData)
		{
			spNormalData=nullptr;
			spFaceNormals=nullptr;
		}
		if(!bTexCoordData)
		{
			spTexCoordData=nullptr;
			spFaceTexCoords=nullptr;
		}

		if(!spFaceVertices ||
		   (spFaceNormals && spFaceNormals->size()!=spFaceVertices->size()) ||
		   (spFaceTexCoords && spFaceTexCoords->size()!=spFaceVertices->size()))
			return false;

		if((spFaceNormals && spNormalData && nMaxNormalIndex>=static_cast<int>(spNormalData->size())) ||
		   (spFaceVertices && spVertexData && nMaxVertexIndex>=static_cast<int>(spVertexData->size())) ||
		   (spFaceTexCoords && spTexCoordData && nMaxTexIndex>=static_cast<int>(spTexCoordData->size())))
			return false;
		
		m_spVertexData=spVertexData;
		m_spNormalData=spNormalData;
		m_spTexCoordData=spTexCoordData;
		m_spFaceVertices=spFaceVertices;
		m_spFaceNormals=spFaceNormals;
		m_spFaceTexCoords=spFaceTexCoords;
		
		vec3<T> tl,br;
		{
			tl=(*spVertexData)[(*spFaceVertices)[0][0]];
			br=tl;
			auto i = spFaceVertices->cbegin(),end=spFaceVertices->cend();
			for(;i!=end;++i)
			{
				for(int nV=0;nV<3;++nV)
					for(int nC=0;nC<3;++nC)
						if((*spVertexData)[(*i)[nV]][nC]<tl[nC])
							tl[nC]=(*spVertexData)[(*i)[nV]][nC];
						else
						if((*spVertexData)[(*i)[nV]][nC]>br[nC])
							br[nC]=(*spVertexData)[(*i)[nV]][nC];
			}
		}
		m_BBox=facemodelbbox<T>(tl,br);

		process();

		return true;
	}
	bool create(const primitivetype t)
	{
		switch(t)
		{
			case pt_unit_camera:if(!createunitcamera())return false;break;
			case pt_unit_spot_light:if(!createunitspotlight())return false;break;
			case pt_unit_point_light:if(!createunitpointlight())return false;break;
			case pt_unit_directional_light:if(!createunitdirectionallight())return false;break;
			default:return false;
		}

		process();

		return true;
	}
protected:
	bool m_bRightHanded;
	facemodelbbox<T> m_BBox;
	std::shared_ptr<std::vector<vec3<T>>> m_spVertexData;
	std::shared_ptr<std::vector<vec3<T>>> m_spNormalData;
	std::shared_ptr<std::vector<vec2<T>>> m_spTexCoordData;
	std::shared_ptr<std::vector<vec3<int>>> m_spFaceVertices;
	std::shared_ptr<std::vector<vec3<int>>> m_spFaceNormals;
	std::shared_ptr<std::vector<vec3<int>>> m_spFaceTexCoords;
	template <typename F> F createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const;
	template <> face_pos3<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		return {p};
	}
	template <> face_pos3_tex<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_tex_vertex_data<T>tex=gettexture(a,pATexCoord,b,pBTexCoord,c,pCTexCoord,cfg);
		return {p,tex};
	}
	template <> face_pos3_col<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_col_vertex_data<T>clr=getcolour(cfg);	
		return {p,clr};
	}
	template <> face_pos3_norm<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal); 
		return {p,n};
	}
	template <> face_pos3_norm_col<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal); 
		const face_col_vertex_data<T>clr=getcolour(cfg);	
		return {p,n,clr};
	}
	template <> face_pos3_norm_tex<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal); 
		const face_tex_vertex_data<T>tex=gettexture(a,pATexCoord,b,pBTexCoord,c,pCTexCoord,cfg);
		return {p,n,tex};
	}
	template <> face_pos3_col_tex<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_col_vertex_data<T>clr=getcolour(cfg);	
		const face_tex_vertex_data<T>tex=gettexture(a,pATexCoord,b,pBTexCoord,c,pCTexCoord,cfg);
		return {p,clr,tex};
	}
	template <> face_pos3_norm_col_tex<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_col_vertex_data<T>clr=getcolour(cfg);	
		const face_tex_vertex_data<T>tex=gettexture(a,pATexCoord,b,pBTexCoord,c,pCTexCoord,cfg);
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal);
		return {p,n,clr,tex};
	}
	template <> face_pos3_norm_bump<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_bump_vertex_data<T>bump=getbump(a,b,c,cfg);
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal);
		return {p,n,bump};
	}
	template <> face_pos3_norm_tex_bump<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_bump_vertex_data<T>bump=getbump(a,b,c,cfg);
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal);
		const face_tex_vertex_data<T>tex=gettexture(a,pATexCoord,b,pBTexCoord,c,pCTexCoord,cfg);
		return {p,n,tex,bump};
	}
	template <> face_pos3_norm_col_bump<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_bump_vertex_data<T>bump=getbump(a,b,c,cfg);
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal);
		const face_col_vertex_data<T>clr=getcolour(cfg);	
		return {p,n,clr,bump};
	}
	template <> face_pos3_norm_col_tex_bump<T> createface(const vec3<T>& a,const vec3<T> *pANormal,const vec2<T> *pATexCoord,const vec3<T>& b,const vec3<T> *pBNormal,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec3<T> *pCNormal,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		const face_pos_vertex_data<vec3<T>>p({a,b,c});
		const face_bump_vertex_data<T>bump=getbump(a,b,c,cfg);
		const face_norm_vertex_data<T>n=getnormal(cfg,p,pANormal,pBNormal,pCNormal);
		const face_col_vertex_data<T>clr=getcolour(cfg);	
		const face_tex_vertex_data<T>tex=gettexture(a,pATexCoord,b,pBTexCoord,c,pCTexCoord,cfg);
		return {p,n,clr,tex,bump};
	}
	
	face_tex_vertex_data<T> gettexture(const vec3<T>& a,const vec2<T> *pATexCoord,const vec3<T>& b,const vec2<T> *pBTexCoord,const vec3<T>& c,const vec2<T> *pCTexCoord,const objcfg<T>& cfg)const
	{
		switch(cfg.getfacetexuv())
		{
			case face_tex_vertex_data<T>::t_null:
			{
				if(pATexCoord)
				{
					const face_tex_vertex_data<T>tex(vertex_data<vec2<T>>(*pATexCoord,*pBTexCoord,*pCTexCoord));
					return tex;
				}
			}
			break;
			case face_tex_vertex_data<T>::t_flat:
			{
				vec2<T> tex[3];
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_flat>(m_BBox,a,tex[0]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_flat>(m_BBox,b,tex[1]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_flat>(m_BBox,c,tex[2]);
				return face_tex_vertex_data<T>(vertex_data<vec2<T>>(tex[0],tex[1],tex[2]));
			}
			break;
			case face_tex_vertex_data<T>::t_cylindrical:
			{
				vec2<T> tex[3];
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_cylindrical>(m_BBox,a,tex[0]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_cylindrical>(m_BBox,b,tex[1]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_cylindrical>(m_BBox,c,tex[2]);
				return face_tex_vertex_data<T>(vertex_data<vec2<T>>(tex[0],tex[1],tex[2]));
			}
			break;
			case face_tex_vertex_data<T>::t_spherical:
			{
				vec2<T> tex[3];
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_spherical>(m_BBox,a,tex[0]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_spherical>(m_BBox,b,tex[1]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_spherical>(m_BBox,c,tex[2]);
				return face_tex_vertex_data<T>(vertex_data<vec2<T>>(tex[0],tex[1],tex[2]));
			}
			break;
		}
		return face_tex_vertex_data<T>(vertex_data<vec2<T>>({0,0},{0,0},{0,0}));
	}
	face_bump_vertex_data<T> getbump(const vec3<T>& a,const vec3<T>& b,const vec3<T>& c,const objcfg<T>& cfg)const
	{
		switch(cfg.getfacebumpuv())
		{
			case face_tex_vertex_data<T>::t_null:break;
			case face_tex_vertex_data<T>::t_flat:
			{
				vec2<T> tex[3];
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_flat>(m_BBox,a,tex[0]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_flat>(m_BBox,b,tex[1]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_flat>(m_BBox,c,tex[2]);
				return face_bump_vertex_data<T>(vertex_data<vec2<T>>(tex[0],tex[1],tex[2]));
			}
			break;
			case face_tex_vertex_data<T>::t_cylindrical:
			{
				vec2<T> tex[3];
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_cylindrical>(m_BBox,a,tex[0]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_cylindrical>(m_BBox,b,tex[1]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_cylindrical>(m_BBox,c,tex[2]);
				return face_bump_vertex_data<T>(vertex_data<vec2<T>>(tex[0],tex[1],tex[2]));
			}
			break;
			case face_tex_vertex_data<T>::t_spherical:
			{
				vec2<T> tex[3];
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_spherical>(m_BBox,a,tex[0]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_spherical>(m_BBox,b,tex[1]);
				face_tex_vertex_data<T>::getuv<face_tex_vertex_data<T>::t_spherical>(m_BBox,c,tex[2]);
				return face_bump_vertex_data<T>(vertex_data<vec2<T>>(tex[0],tex[1],tex[2]));
			}
			break;
		}
		return face_bump_vertex_data<T>(vertex_data<vec2<T>>({0,0},{0,0},{0,0}));
	}
	face_col_vertex_data<T> getcolour(const objcfg<T>& cfg)const
	{
		face_col_vertex_data<T> c;
		if(cfg.getfacecolour().first)
			c=vertex_data<vec3<T>>(cfg.getfacecolour().second,cfg.getfacecolour().second,cfg.getfacecolour().second);
		else
			c=vertex_data<vec3<T>>(materialcol<T>::getdefdiffuse(),materialcol<T>::getdefdiffuse(),materialcol<T>::getdefdiffuse());
		return c;
	}
	face_norm_vertex_data<T> getnormal(const objcfg<T>& cfg,const face_pos_vertex_data<vec3<T>>& p,const vec3<T> *pANormal,const vec3<T> *pBNormal,const vec3<T> *pCNormal)const
	{
		const bool bFaceNormals=cfg.getfacenormal()!=face_norm_vertex_data<T>::t_null;
		bool bGeometric=cfg.getfacenormal()==face_norm_vertex_data<T>::t_geometric;
		if(pANormal)
		{
			if(!bFaceNormals)
			{
				face_norm_vertex_data<T> n=face_norm_vertex_data<T>({*pANormal,*pBNormal,*pCNormal});
				return n;
			}
		}
		else
			bGeometric=true;

		const vec3<T> nml=bGeometric ?
							face_norm_vertex_data<T>::geometricfacenormal(p.getpos()):
							face_norm_vertex_data<T>::averagefacenormal({*pANormal,*pBNormal,*pCNormal});
		face_norm_vertex_data<T> n=face_norm_vertex_data<T>({nml,nml,nml});
		return n;
	}
	bool createunitspotlight(void)
	{
		return createunitdirectionallight(); // lets use directional model
	}
	bool createunitpointlight(void)
	{
		std::vector<vec3<T>> vFrontFront,vFrontBack;
		std::vector<vec3<T>> vBackFront,vBackBack;
		std::vector<vec3<T>> vLeftFront,vLeftBack;
		std::vector<vec3<T>> vRightFront,vRightBack;
		std::vector<vec3<T>> vAboveFront,vAboveBack;
		std::vector<vec3<T>> vBelowFront,vBelowBack;
		
		const T dDim=0.4,dDepth=1;

		vFrontFront={{0,0,(dDepth+dDim*0.5)*-1.0},{0,0,(dDepth+dDim*0.5)*-1.0},{0,0,(dDepth+dDim*0.5)*-1.0},{0,0,(dDepth+dDim*0.5)*-1.0}};
		vFrontBack={{dDim*0.5,dDim*0.5,-1.0*dDim*0.5},{-dDim*0.5,dDim*0.5,-1.0*dDim*0.5},
					{-dDim*0.5,-dDim*0.5,-1.0*dDim*0.5},{dDim*0.5,-dDim*0.5,-1.0*dDim*0.5}};

		vBackFront={vFrontBack[1],vFrontBack[0],vFrontBack[3],vFrontBack[2]};
		vBackFront[0][2]=-vBackFront[0][2];
		vBackFront[1][2]=-vBackFront[1][2];
		vBackFront[2][2]=-vBackFront[2][2];
		vBackFront[3][2]=-vBackFront[3][2];
		vBackBack={{0,0,(dDepth+dDim*0.5)},{0,0,(dDepth+dDim*0.5)},{0,0,(dDepth+dDim*0.5)},{0,0,(dDepth+dDim*0.5)}};

		vRightFront={{dDim*0.5,dDim*0.5,0.5*dDim},{dDim*0.5,dDim*0.5,0.5*dDim*-1.0},{dDim*0.5,-dDim*0.5,0.5*dDim*-1.0},{dDim*0.5,-dDim*0.5,0.5*dDim}};
		vRightBack={{dDim*0.5+dDepth,0.0,0.0},{dDim*0.5+dDepth,0.0,0.0},{dDim*0.5+dDepth,0.0,0.0},{dDim*0.5+dDepth,0.0,0.0}};
		
		vLeftFront={vRightFront[1],vRightFront[0],vRightFront[3],vRightFront[2]};
		vLeftFront[0][0]=-vLeftFront[0][0];
		vLeftFront[1][0]=-vLeftFront[1][0];
		vLeftFront[2][0]=-vLeftFront[2][0];
		vLeftFront[3][0]=-vLeftFront[3][0];
		vLeftBack={{(-dDepth-dDim*0.5),0,0},{(-dDepth-dDim*0.5),0,0},{(-dDepth-dDim*0.5),0,0},{(-dDepth-dDim*0.5),0,0}};
		
		vAboveFront={{-dDim*0.5,dDim*0.5,0.5*dDim*-1.0},{dDim*0.5,dDim*0.5,0.5*dDim*-1.0},
					 {dDim*0.5,dDim*0.5,0.5*dDim},{-dDim*0.5,dDim*0.5,0.5*dDim}};
		vAboveBack={{0.0,dDim*0.5+dDepth,0.0},{0.0,dDim*0.5+dDepth,0.0},{0.0,dDim*0.5+dDepth,0.0},{0.0,dDim*0.5+dDepth,0.0}};
		
		vBelowFront={vAboveFront[1],vAboveFront[0],vAboveFront[3],vAboveFront[2]};
		vBelowFront[0][1]=-vLeftFront[0][1];
		vBelowFront[1][1]=-vLeftFront[1][1];
		vBelowFront[2][1]=-vLeftFront[2][1];
		vBelowFront[3][1]=-vLeftFront[3][1];
		vBelowBack={{0.0,(-dDepth-dDim*0.5),0.0},{0.0,(-dDepth-dDim*0.5),0.0},{0.0,(-dDepth-dDim*0.5),0.0},{0.0,(-dDepth-dDim*0.5),0.0}};
		
		std::shared_ptr<std::vector<vec3<T>>> spVertexData(new std::vector<vec3<T>>);
		std::shared_ptr<std::vector<vec3<int>>> spFaceVertices(new std::vector<vec3<int>>);

		m_bRightHanded=false;
		m_spVertexData=spVertexData;
		m_spFaceVertices=spFaceVertices;
		
		push_back_cuboid(vFrontFront,vFrontBack,spVertexData,spFaceVertices);
		push_back_cuboid(vBackFront,vBackBack,spVertexData,spFaceVertices);
		push_back_cuboid(vLeftFront,vLeftBack,spVertexData,spFaceVertices);
		push_back_cuboid(vRightFront,vRightBack,spVertexData,spFaceVertices);
		push_back_cuboid(vAboveFront,vAboveBack,spVertexData,spFaceVertices);
		push_back_cuboid(vBelowFront,vBelowBack,spVertexData,spFaceVertices);
		
		m_BBox=facemodelbbox<T>(*spVertexData);
		if(m_BBox.getwidth()<=0.0 || m_BBox.getheight()<=0.0 || m_BBox.getdepth()<=0.0)
			return false;
		{
			const vec3<T> centre=m_BBox.getorigin();
			const T dMax=m_BBox.getwidth()>m_BBox.getheight()?
							(m_BBox.getwidth()>m_BBox.getdepth()?m_BBox.getwidth():m_BBox.getdepth()):
							(m_BBox.getheight()>m_BBox.getdepth()?m_BBox.getheight():m_BBox.getdepth());
			const vec3<T> scale(1.0/dMax,1.0/dMax,1.0/dMax);
			auto i=spVertexData->begin(),end=spVertexData->end();
			for(;i!=end;++i) { (*i)+=-centre; (*i)*=scale; }
		}
		m_BBox=facemodelbbox<T>(*spVertexData);
		
		return true;
	}
	bool createunitdirectionallight(void)
	{
		std::vector<vec3<T>> vBodyFront,vBodyBack;
		std::vector<vec3<T>> vHeadFront,vHeadBack;

		const T dBodyWidth=0.25,dBodyHeight=0.25,dBodyDepth=1;
		const vec3<T> bodyCentre(dBodyWidth*0.5,-dBodyHeight*0.5,dBodyDepth*0.5);
		vBodyFront={{0.0,0.0,0.0},
					{dBodyWidth,0.0,0.0},
					{dBodyWidth,-dBodyHeight,0.0},
					{0.0,-dBodyHeight,0.0}};

		vBodyBack={vBodyFront[1],vBodyFront[0],vBodyFront[3],vBodyFront[2]};
		auto i=vBodyBack.begin(),end=vBodyBack.end();
		for(;i!=end;++i)
			(*i)[2]+=dBodyDepth;

		const T dHeadWidth=0.5,dHeadHeight=0.5,dHeadDepth=0.5;
		const vec3<T> backCentre(dBodyWidth*0.5,-dBodyHeight*0.5,vBodyBack[0][2]);
		const T dHeadZ=backCentre[2]-0.05;
		vHeadFront={{backCentre[0]-(dHeadWidth*0.5),backCentre[1]+(dHeadHeight*0.5),dHeadZ},
					{backCentre[0]+(dHeadWidth*0.5),backCentre[1]+(dHeadHeight*0.5),dHeadZ},
					{backCentre[0]+(dHeadWidth*0.5),backCentre[1]-(dHeadHeight*0.5),dHeadZ},
					{backCentre[0]-(dHeadWidth*0.5),backCentre[1]-(dHeadHeight*0.5),dHeadZ}};

		vHeadBack={{backCentre[0],backCentre[1],dHeadZ+(dHeadDepth)},{backCentre[0],backCentre[1],dHeadZ+(dHeadDepth)},
				   {backCentre[0],backCentre[1],dHeadZ+(dHeadDepth)},{backCentre[0],backCentre[1],dHeadZ+(dHeadDepth)}};
		
		std::shared_ptr<std::vector<vec3<T>>> spVertexData(new std::vector<vec3<T>>);
		std::shared_ptr<std::vector<vec3<int>>> spFaceVertices(new std::vector<vec3<int>>);

		m_bRightHanded=false;
		m_spVertexData=spVertexData;
		m_spFaceVertices=spFaceVertices;
		
		push_back_cuboid(vBodyFront,vBodyBack,spVertexData,spFaceVertices);
		push_back_cuboid(vHeadFront,vHeadBack,spVertexData,spFaceVertices);
		
		m_BBox=facemodelbbox<T>(*spVertexData);
		if(m_BBox.getwidth()<=0.0 || m_BBox.getheight()<=0.0 || m_BBox.getdepth()<=0.0)
			return false;
		{
			const vec3<T> centre=m_BBox.getorigin();
			const T dMax=m_BBox.getwidth()>m_BBox.getheight()?
							(m_BBox.getwidth()>m_BBox.getdepth()?m_BBox.getwidth():m_BBox.getdepth()):
							(m_BBox.getheight()>m_BBox.getdepth()?m_BBox.getheight():m_BBox.getdepth());
			const vec3<T> scale(1.0/dMax,1.0/dMax,1.0/dMax);
			auto i=spVertexData->begin(),end=spVertexData->end();
			for(;i!=end;++i) { (*i)+=-centre; (*i)*=scale; }
		}
		m_BBox=facemodelbbox<T>(*spVertexData);
		
		return true;
	}
	bool createunitcamera(void)
	{
		std::vector<vec3<T>> vBodyFront,vBodyBack;
		std::vector<vec3<T>> vHandleFront,vHandleBack;
		std::vector<vec3<T>> vLensFront,vLensBack;

		const T dBodyWidth=0.5,dBodyHeight=0.55,dBodyDepth=1;
		const vec3<T> bodyCentre(dBodyWidth*0.5,-dBodyHeight*0.5,dBodyDepth*0.5);
		vBodyFront={{0.0,0.0,0.0},
					{dBodyWidth,0.0,0.0},
					{dBodyWidth,-dBodyHeight,0.0},
					{0.0,-dBodyHeight,0.0}};
		
		vBodyBack={vBodyFront[1],vBodyFront[0],vBodyFront[3],vBodyFront[2]};
		auto i=vBodyBack.begin(),end=vBodyBack.end();
		for(;i!=end;++i)
			(*i)[2]+=dBodyDepth;

		const T dHandleWidth=dBodyWidth*0.4,dHandleDepth=dBodyDepth*(1/3.0),dHandleHeight=dBodyHeight*1.75;
		vHandleFront={{bodyCentre[0]-(dHandleWidth*0.5),bodyCentre[1],bodyCentre[2]-(dHandleDepth*0.5)},
					  {bodyCentre[0]+(dHandleWidth*0.5),bodyCentre[1],bodyCentre[2]-(dHandleDepth*0.5)},
					  {bodyCentre[0]+(dHandleWidth*0.5),bodyCentre[1]-dHandleHeight,vBodyFront[0][2]-(dHandleDepth*0.5)},
					  {bodyCentre[0]-(dHandleWidth*0.5),bodyCentre[1]-dHandleHeight,vBodyFront[0][2]-(dHandleDepth*0.5)}};
		
		vHandleBack={vHandleFront[1],vHandleFront[0],vHandleFront[3],vHandleFront[2]};
		i=vHandleBack.begin(),end=vHandleBack.end();
		for(;i!=end;++i)
			(*i)[2]+=dHandleDepth;

		vLensFront={bodyCentre,bodyCentre,bodyCentre,bodyCentre};

		const vec3<T> lensOffset(-dBodyWidth*0.25,-dBodyWidth*0.25,dBodyWidth*0.75);
		vLensBack={vBodyBack[0]+lensOffset,
				   vBodyBack[1]+vec3<T>(-lensOffset[0],lensOffset[1],lensOffset[2]),
				   vBodyBack[2]+vec3<T>(-lensOffset[0],-lensOffset[1],lensOffset[2]),
				   vBodyBack[3]+vec3<T>(lensOffset[0],-lensOffset[1],lensOffset[2])};

		std::shared_ptr<std::vector<vec3<T>>> spVertexData(new std::vector<vec3<T>>);
		std::shared_ptr<std::vector<vec3<int>>> spFaceVertices(new std::vector<vec3<int>>);

		m_bRightHanded=false;
		m_spVertexData=spVertexData;
		m_spFaceVertices=spFaceVertices;
		
		push_back_cuboid(vBodyFront,vBodyBack,spVertexData,spFaceVertices);
		push_back_cuboid(vHandleFront,vHandleBack,spVertexData,spFaceVertices);
		push_back_cuboid(vLensFront,vLensBack,spVertexData,spFaceVertices);
		
		m_BBox=facemodelbbox<T>(*spVertexData);
		if(m_BBox.getwidth()<=0.0 || m_BBox.getheight()<=0.0 || m_BBox.getdepth()<=0.0)
			return false;
		{
			const vec3<T> centre=m_BBox.getorigin();
			const T dMax=m_BBox.getwidth()>m_BBox.getheight()?
							(m_BBox.getwidth()>m_BBox.getdepth()?m_BBox.getwidth():m_BBox.getdepth()):
							(m_BBox.getheight()>m_BBox.getdepth()?m_BBox.getheight():m_BBox.getdepth());
			const vec3<T> scale(1.0/dMax,1.0/dMax,1.0/dMax);
			auto i=spVertexData->begin(),end=spVertexData->end();
			for(;i!=end;++i) { (*i)+=-centre; (*i)*=scale; }
		}
		m_BBox=facemodelbbox<T>(*spVertexData);
		
		return true;
	}
	void push_back_cuboid(const std::vector<vec3<T>>& vFront,const std::vector<vec3<T>>& vBack,
						  std::shared_ptr<std::vector<vec3<T>>> spVertexData,
						  std::shared_ptr<std::vector<vec3<int>>> spFaceVertices)const
	{
		if(vFront.size() != vBack.size() || vBack.size()<4)
			return;
		
		const bool bObjCCWVertices=m_bRightHanded;

		const int nFrontVertices=static_cast<int>(spVertexData->size());
		spVertexData->push_back(vFront[0]);
		spVertexData->push_back(vFront[1]);
		spVertexData->push_back(vFront[2]);
		spVertexData->push_back(vFront[3]);
		const int nBackVertices=static_cast<int>(spVertexData->size());
		spVertexData->push_back(vBack[0]);
		spVertexData->push_back(vBack[1]);
		spVertexData->push_back(vBack[2]);
		spVertexData->push_back(vBack[3]);
		
		// front
		spFaceVertices->push_back({nFrontVertices+0,nFrontVertices+1,nFrontVertices+2});
		spFaceVertices->push_back({nFrontVertices+2,nFrontVertices+3,nFrontVertices+0});

		// back
		spFaceVertices->push_back({nBackVertices+0,nBackVertices+1,nBackVertices+2});
		spFaceVertices->push_back({nBackVertices+2,nBackVertices+3,nBackVertices+0});
		
		// above
		spFaceVertices->push_back({nFrontVertices+0,nBackVertices+1,nFrontVertices+1});
		spFaceVertices->push_back({nFrontVertices+1,nBackVertices+1,nBackVertices+0});

		// below
		spFaceVertices->push_back({nFrontVertices+3,nFrontVertices+2,nBackVertices+2});
		spFaceVertices->push_back({nFrontVertices+2,nBackVertices+3,nBackVertices+2});

		// right
		spFaceVertices->push_back({nBackVertices+3,nFrontVertices+2,nFrontVertices+1});
		spFaceVertices->push_back({nFrontVertices+1,nBackVertices+0,nBackVertices+3});

		// left
		spFaceVertices->push_back({nFrontVertices+0,nFrontVertices+3,nBackVertices+2});
		spFaceVertices->push_back({nBackVertices+2,nBackVertices+1,nFrontVertices+0});
	}
	void process(void)
	{
		#if(RAS_PARADIGM==RAS_DX_PARADIGM)
			const bool bRightHanded=false;
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			const bool bRightHanded=true;
		#endif
	
		// what the model author considered the front of the model we consider the back so flip
		const bool bFlipZ=(bRightHanded!=m_bRightHanded);
		if(bFlipZ)
		{
			const T dDepth = m_BBox.getbackmax()[2] - m_BBox.getfrontmin()[2];
			const T dMid = m_BBox.getfrontmin()[2]+(dDepth*0.5);
			auto i=m_spVertexData->begin(),end=m_spVertexData->end();
			for(;i!=end;++i)
				(*i)[2]=dMid-((*i)[2]-dMid);
			const vec3<T> fmin=m_BBox.getfrontmin();
			const vec3<T> bmax=m_BBox.getbackmax();
		}

		// what the model author considered front facing normal we consider back facing so flip
		const bool bFlipN=(bRightHanded!=m_bRightHanded);
		if(bFlipN)
			if(m_spNormalData)
			{
				auto i=m_spNormalData->begin(),end=m_spNormalData->end();
				for(;i!=end;++i)
					(*i)[2]*=-1.0;
			}

		// what the model author considered the correct CW/CCW vertex order we consider the opposite so flip
		const bool bFlipOrder=(bRightHanded!=m_bRightHanded);
		if(bFlipOrder)
		{
			if(m_spFaceVertices)
			{
				auto i=m_spFaceVertices->begin(),end=m_spFaceVertices->end();
				for(;i!=end;++i)
					std::swap((*i)[2],(*i)[0]);
			}
			if(m_spFaceNormals)
			{
				auto i=m_spFaceNormals->begin(),end=m_spFaceNormals->end();
				for(;i!=end;++i)
					std::swap((*i)[2],(*i)[0]);
			}
			if(m_spFaceTexCoords)
			{
				auto i=m_spFaceTexCoords->begin(),end=m_spFaceTexCoords->end();
				for(;i!=end;++i)
					std::swap((*i)[2],(*i)[0]);
			}
		}
	}
};

}
