#pragma once

#include "dib.h"
#include "3d_light.h"
#include "3d_tbuffer.h"

namespace af3d
{

template <typename BT> class fragmentshaderscratch
{
public:
	fragmentshaderscratch(){}
	~fragmentshaderscratch(){}
	__forceinline BT::template t_xform_maxvertexbuffer_clipped *getclipped(void){return &m_Clipped;}
	__forceinline BT::template t_xform_maxvertexbuffer_clipped *getlocal(void){return &m_Local;}
	__forceinline BT::template t_xform_maxvertexbuffer_triangulated *gettriangulated(void){return &m_Triangulated;}
protected:
	BT::template t_xform_maxvertexbuffer_triangulated m_Triangulated;
	BT::template t_xform_maxvertexbuffer_clipped m_Clipped;
	BT::template t_xform_maxvertexbuffer_clipped m_Local;
};

template <typename KNL> class fragmentshader
{
public:
	using t_flt=KNL::template t_vs_types::template t_base_types::template t_flt;

	static void render(const afthread::taskscheduler *pSched,
					   const vertexattsframe<t_flt> *pFrame,
					   const lightcache<t_flt>& lm,const camera<t_flt> *pCamera,const bool bRenderShadows,
					   const crect<t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
					   const vertexshaderscratch<t_flt> *pVSScratch,
					   zbuffer<t_flt> *pZBuffer,gbuffer<t_flt> *pGBuffer)
	{
		if(!pFrame||!pVSScratch||rDstNDCClip.isempty()) return;

		const bool bZBuffer=!(!pZBuffer||(pZBuffer->getwidth()<rDstNDCClip.get(rect::v_br)[0]+dstNDCtobuf[0])||(pZBuffer->getheight()<rDstNDCClip.get(rect::v_br)[1]+dstNDCtobuf[1]));
		if(!bZBuffer)
			return;

		const bool bGBuffer=!(!pGBuffer||(pGBuffer->getwidth()<rDstNDCClip.get(rect::v_br)[0]+dstNDCtobuf[0])||(pGBuffer->getheight()<rDstNDCClip.get(rect::v_br)[1]+dstNDCtobuf[1]));
		if(pGBuffer)
		{
			if(!bGBuffer)
				return;
		}

		const std::vector<face_xform_union<t_flt>>& vTransformed = pVSScratch->getxformed()->get();
		const face_xform_union<t_flt> *pTransformed=&(vTransformed[0]);
		const int nFaces=static_cast<int>(vTransformed.size());
		
		const mesh<KNL::template t_vs_types::template t_base_types::template t_fb> *pMesh=static_cast<const mesh<KNL::template t_vs_types::template t_base_types::template t_fb>*>(pFrame);
		
		const std::vector<std::shared_ptr<material<t_flt>>> *pMaterials=pGBuffer ? pMesh->getmaterials() : nullptr;
		const material<t_flt> *pMaterial=pMaterials && pMaterials->size()?(*pMaterials)[0].get():nullptr;
		const int nMaterials=pMaterials?static_cast<int>(pMaterials->size()):0;
		int nMaterial=0,nFace=0;

		while(nFace<nFaces)
		{
			int nPreMaterialInclusiveTo;
			if(pMaterial)
			{
				nPreMaterialInclusiveTo=pMaterial->getfrom()-1;
				if(nPreMaterialInclusiveTo>=nFaces)
					nPreMaterialInclusiveTo=nFaces-1;
			}
			else
				nPreMaterialInclusiveTo=nFaces-1;
			if(nPreMaterialInclusiveTo-nFace+1)
			{
				renderfaces<0,false>(pSched,pMesh,lm,pCamera,pTransformed,nFace,nPreMaterialInclusiveTo,nullptr,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
				nFace=nPreMaterialInclusiveTo+1;
			}
			else
			if(pMaterial)
			{
				int nMatMaterialInclusiveTo=pMaterial->getinclusiveto();
				if(nMatMaterialInclusiveTo>=nFaces)
					nMatMaterialInclusiveTo=nFaces-1;
				if(nMatMaterialInclusiveTo-nFace+1 && pMaterial->getatts(true))
				{
					if(pMaterial->getcol().getquantize())
						mat_renderfaces<true>(pSched,pMesh,lm,pCamera,pTransformed,nFace,nMatMaterialInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
					else
						mat_renderfaces<false>(pSched,pMesh,lm,pCamera,pTransformed,nFace,nMatMaterialInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
					nFace=nMatMaterialInclusiveTo+1;
				}
				++nMaterial;
				pMaterial=nMaterial<nMaterials?(*pMaterials)[nMaterial].get():nullptr;
			}
		}
	}
	static void render(const afthread::taskscheduler *pSched,
					   const vertexattsframe<t_flt> *pFrame,
					   const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
					   gbuffer<t_flt> *pGBuffer,afdib::dib *pDeviceDib)
	{
		if(rDstNDCClip.isempty()) return;

		const bool bDib=!(!pDeviceDib||(pDeviceDib->getwidth()<rDstNDCClip.get(rect::v_br)[0])||(pDeviceDib->getheight()<rDstNDCClip.get(rect::v_br)[1]));
		if(!bDib)
			return;

		const bool bGBuffer=!(!pGBuffer||(pGBuffer->getwidth()<rDstNDCClip.get(rect::v_br)[0]+dstNDCtobuf[0])||(pGBuffer->getheight()<rDstNDCClip.get(rect::v_br)[1]+dstNDCtobuf[1]));
		if(!bGBuffer)
			return;
			
		const int nFromY=rDstNDCClip.get(rect::v_tl)[1];
		const int nInclusiveToY=rDstNDCClip.get(rect::v_br)[1]-1;

		const mesh<KNL::template t_vs_types::template t_base_types::template t_fb> *pMesh=static_cast<const mesh<KNL::template t_vs_types::template t_base_types::template t_fb>*>(pFrame);

		if(pSched)
			pSched->parallel_for(nFromY,rDstNDCClip.getheight(),pSched->getcores(),pixelop(rDstNDCClip,dstNDCtobuf,pMesh->getopacity(),pGBuffer,pDeviceDib));
		else
			pixelop(rDstNDCClip,dstNDCtobuf,pMesh->getopacity(),pGBuffer,pDeviceDib)(nFromY,nFromY+rDstNDCClip.getheight()-1,nullptr);
	}
protected:
	class pixelop
	{
	public:
		pixelop(const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,const t_flt dOpacity,gbuffer<t_flt> *pGBuffer,afdib::dib *pDeviceDib):
				m_dstNDCtobuf(dstNDCtobuf),m_rDstNDCClip(rDstNDCClip),m_pGBuffer(pGBuffer),m_pDeviceDib(pDeviceDib),m_dOpacity(dOpacity){}
		void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
		{
			const int nFromY=nFrom;
			const int nInclusiveToY=nInclusiveTo;
			
			const int nFromX=m_rDstNDCClip.get(rect::v_tl)[0];
			const int nInclusiveToX=m_rDstNDCClip.get(rect::v_br)[0]-1;
		
			unsigned char *pScanline=m_pDeviceDib->getscanline(nFromY);
			const int nBytesPerScanline=m_pDeviceDib->getbytesperscanline();

			gvertex<t_flt> *pGScanline=m_pGBuffer->getscanline(nFromY+m_dstNDCtobuf[1]);
			for(int nDstY=nFromY;nDstY<=nInclusiveToY;++nDstY,pScanline+=nBytesPerScanline,pGScanline+=m_pGBuffer->getwidth())
			{
				KNL::template t_vs_types::template t_pt *pPixels=reinterpret_cast<KNL::template t_vs_types::template t_pt *>(pScanline);
				for(int nDstX=nFromX;nDstX<=nInclusiveToX;++nDstX)
				{
					const int nBufDstX=nDstX+m_dstNDCtobuf[0];
					if(pGScanline[nBufDstX].isfrag())
					{
						pGScanline[nBufDstX].clear();
						const vec3<t_flt>& colour=pGScanline[nBufDstX].getfrag();
						afdib::pixel<t_flt>::blend<KNL::template t_vs_types::template t_pt>(pPixels[nDstX],colour[0],colour[1],colour[2],m_dOpacity);
					}
				}
			}
		}
	protected:
		const t_flt m_dOpacity;
		const rect& m_rDstNDCClip;
		const vec2<int>& m_dstNDCtobuf;
		gbuffer<t_flt> *m_pGBuffer;
		afdib::dib *m_pDeviceDib;
	};
	template <int MAT,bool QUANTIZE> class fragop
	{
	public:
		fragop(const mesh<KNL::template t_vs_types::template t_base_types::template t_fb> *pMesh,
			   const lightcache<t_flt>& lm,const camera<t_flt> *pCamera,
			   const face_xform_union<t_flt> *pTransformed,
			   const material<t_flt> *pMaterial,
			   const crect<t_flt>& rDstNDC,const rect& rDeviceClip,const vec2<int>& bufOffset,
			   zbuffer<t_flt> *pZBuffer,gbuffer<t_flt> *pGBuffer):
					m_pMesh(pMesh),m_lm(lm),m_pCamera(pCamera),m_pTransformed(pTransformed),
					m_BufOffset(bufOffset),m_pMaterial(pMaterial),m_rDstNDC(rDstNDC),m_rDeviceClip(rDeviceClip),m_pZBuffer(pZBuffer),m_pGBuffer(pGBuffer){}
		void operator()(const int nFrom,const int nInclusiveTo,const afthread::taskinfo *m_pTaskInfo)const
		{
			fragmentshaderscratch<KNL::template t_vs_types::template t_base_types> scratch;
			const vec3<t_flt>& worldcampos=m_pCamera->getorigin();
			const KNL::template t_vs_types::template t_base_types::template t_face *pFaces=&(m_pMesh->getvertexbuffer()->get()[0]);
			for(int nRangeFace=nFrom;nRangeFace<=nInclusiveTo;++nRangeFace)
			{
				const KNL::template t_vs_types::template t_base_types::template t_xform_face& f=m_pTransformed[nRangeFace].get<KNL::template t_vs_types::template t_base_types::template t_xform_face>();
				switch(f.getclippostype())
				{
					case face_pos3_xform<t_flt>::cpt_outside:break;
					case face_pos3_xform<t_flt>::cpt_inside:
						renderface<MAT,QUANTIZE>(pFaces[nRangeFace],f,m_lm,worldcampos,m_pMaterial,m_rDstNDC,m_rDeviceClip,m_BufOffset,m_pZBuffer,m_pGBuffer);
					break;
					case face_pos3_xform<t_flt>::cpt_unknown:
					{
						KNL::template t_vs_types::template t_proj::template t_clipper::clip<projectionclipper<>::et_all>(f,scratch.getclipped(),scratch.getlocal());
						if(scratch.getclipped()->size())
						{
							KNL::template t_vs_types::template t_proj::template t_clipper::fantriangulate(f,scratch.getclipped(),scratch.gettriangulated());
							const KNL::template t_vs_types::template t_base_types::template t_xform_maxvertexbuffer_triangulated *pTriangulated=scratch.gettriangulated();
							const int nFaces=static_cast<int>(pTriangulated->size());
							for(int nFace=0;nFace<nFaces;++nFace)
								renderface<MAT,QUANTIZE>(pFaces[nRangeFace],pTriangulated->get()[nFace],m_lm,worldcampos,m_pMaterial,m_rDstNDC,m_rDeviceClip,m_BufOffset,m_pZBuffer,m_pGBuffer);
							continue;
						}
					}
					break;
				}
			}
		}
	protected:
		const mesh<KNL::template t_vs_types::template t_base_types::template t_fb> *m_pMesh;
		const lightcache<t_flt>& m_lm;
		const camera<t_flt> *m_pCamera;
		const face_xform_union<t_flt> *m_pTransformed;
		const material<t_flt> *m_pMaterial;
		const crect<t_flt>& m_rDstNDC;
		const rect& m_rDeviceClip;
		const vec2<int>& m_BufOffset;
		zbuffer<t_flt> *m_pZBuffer;
		gbuffer<t_flt> *m_pGBuffer;
	};
	template <int MAT,bool QUANTIZE> static void renderfaces(const afthread::taskscheduler *pSched,
															 const mesh<KNL::template t_vs_types::template t_base_types::template t_fb> *pMesh,
															 const lightcache<t_flt>& lm,const camera<t_flt> *pCamera,
															 const face_xform_union<t_flt> *pTransformed,const int nFrom,const int nInclusiveTo,
															 const material<t_flt> *pMaterial,
															 const crect<t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
															 zbuffer<t_flt> *pZBuffer,gbuffer<t_flt> *pGBuffer)
	{
		if(pSched)
			pSched->parallel_for(nFrom,(nInclusiveTo-nFrom+1),pSched->getcores(),fragop<MAT,QUANTIZE>(pMesh,lm,pCamera,pTransformed,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer));
		else
			fragop<MAT,QUANTIZE>(pMesh,lm,pCamera,pTransformed,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer)(nFrom,nFrom+(nInclusiveTo-nFrom+1)-1,nullptr);
	}
	template <bool QUANTIZE>
	static void mat_renderfaces(const afthread::taskscheduler *pSched,
								const mesh<KNL::template t_vs_types::template t_base_types::template t_fb> *pMesh,
								const lightcache<t_flt>& lm,const camera<t_flt> *pCamera,
								const face_xform_union<t_flt> *pTransformed,const int nFrom,const int nInclusiveTo,
								const material<t_flt> *pMaterial,
								const crect<t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
								zbuffer<t_flt> *pZBuffer,gbuffer<t_flt> *pGBuffer)
	{
		switch(pMaterial->getattsize(true))
		{
			case 1:
				switch(pMaterial->getatts(true))
				{
					case face_vertex_att::t_col:
						renderfaces<face_vertex_att::t_col,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					case face_vertex_att::t_tex:
						renderfaces<face_vertex_att::t_tex,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					case face_vertex_att::t_bump:
						renderfaces<face_vertex_att::t_bump,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					case face_vertex_att::t_env_cubic:
						renderfaces<face_vertex_att::t_env_cubic,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					default:break;
				}
				break;
			case 2:
				switch(pMaterial->getatts(true))
				{
					case face_vertex_att::t_bump|face_vertex_att::t_env_cubic:
						renderfaces<face_vertex_att::t_bump|face_vertex_att::t_env_cubic,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					case face_vertex_att::t_tex|face_vertex_att::t_bump:
						renderfaces<face_vertex_att::t_tex|face_vertex_att::t_bump,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					case face_vertex_att::t_tex|face_vertex_att::t_env_cubic:
						renderfaces<face_vertex_att::t_tex|face_vertex_att::t_env_cubic,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;

					case face_vertex_att::t_col|face_vertex_att::t_env_cubic:
						renderfaces<face_vertex_att::t_col|face_vertex_att::t_env_cubic,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					case face_vertex_att::t_col|face_vertex_att::t_bump:
						renderfaces<face_vertex_att::t_col|face_vertex_att::t_bump,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;

					case face_vertex_att::t_col|face_vertex_att::t_tex:
						renderfaces<face_vertex_att::t_col|face_vertex_att::t_tex,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;

					default:break;
				}
				break;
			case 3:
				switch(pMaterial->getatts(true))
				{
					case face_vertex_att::t_col|face_vertex_att::t_bump|face_vertex_att::t_env_cubic:
						renderfaces<face_vertex_att::t_col|face_vertex_att::t_bump|face_vertex_att::t_env_cubic,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;

					case face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_bump:
						renderfaces<face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_bump,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;
					
					case face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_env_cubic:
						renderfaces<face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_env_cubic,QUANTIZE>(pSched,pMesh,lm,pCamera,pTransformed,nFrom,nInclusiveTo,pMaterial,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
						return;

					default:break;
				}
				break;
			
			default:break;
		}
	}
	__forceinline static t_flt edgefn(const vec3<t_flt>& v0,const vec3<t_flt>& v1,const t_flt dP_x,const t_flt dP_y){return edgefn(v0[0],v0[1],(v1[1]-v0[1]),(v1[0]-v0[0]),dP_x,dP_y);}
	__forceinline static t_flt edgefn(const t_flt v0_x,const t_flt v0_y,const t_flt v1_y_minus_v0_y,const t_flt v1_x_minus_v0_x,const t_flt dP_x,const t_flt dP_y)
	{
		// This essentially calculates the signed area of the parallelogram formed by the vectors v0v1 and v0p
		// This function is mathematically equivalent to the magnitude of the cross products between the vector (v1-v0) and (p-v0)
		return (dP_x-v0_x)*(v1_y_minus_v0_y)-(dP_y-v0_y)*(v1_x_minus_v0_x);
	}
	__forceinline static bool edgetest(const t_flt p_x,const t_flt p_y,
									   const t_flt v0_x,const t_flt v0_y,const t_flt v1_x,const t_flt v1_y,const t_flt v2_x,const t_flt v2_y,
									   const t_flt v0_y_minus_v2_y,const t_flt v1_y_minus_v0_y,const t_flt v2_y_minus_v1_y,
									   const t_flt v0_x_minus_v2_x,const t_flt v1_x_minus_v0_x,const t_flt v2_x_minus_v1_x,
									   vec3<t_flt>& edges_01_12_20)
	{
		edges_01_12_20[0]=edgefn(v0_x,v0_y,v1_y_minus_v0_y,v1_x_minus_v0_x,p_x,p_y); // edge01
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			if(edges_01_12_20[0]>0)
				return false;
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			if(edges_01_12_20[0]<0)
				return false;
		#endif
				
		edges_01_12_20[1]=edgefn(v1_x,v1_y,v2_y_minus_v1_y,v2_x_minus_v1_x,p_x,p_y); // edge12
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			if(edges_01_12_20[1]>0)
				return false;
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			if(edges_01_12_20[1]<0)
				return false;
		#endif

		edges_01_12_20[2]=edgefn(v2_x,v2_y,v0_y_minus_v2_y,v0_x_minus_v2_x,p_x,p_y); // edge20
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			if(edges_01_12_20[2]>0)
				return false;
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			if(edges_01_12_20[2]<0)
				return false;
		#endif

		return true;
	}
	template <int MAT,bool QUANTIZE> __forceinline static void renderface(const KNL::template t_vs_types::template t_base_types::template t_face& src,const KNL::template t_vs_types::template t_base_types::template t_xform_face& xformed,
																		  const lightcache<t_flt>& lm,const vec3<t_flt>& worldcampos,
																		  const material<t_flt> *pMaterial,const crect<t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
																		  zbuffer<t_flt> *pZBuffer,gbuffer<t_flt> *pGBuffer)
	{
		const t_flt epsilon = std::numeric_limits<t_flt>::epsilon();

		#if (RAS_PARADIGM==RAS_DX_PARADIGM) || (RAS_PARADIGM==RAS_OGL_PARADIGM)
		#else
			return;
		#endif

		// clipspace -> NDC (de-homogenise) -> device
		#ifdef _DEBUG
			vec4<t_flt> ndc[3];
			const face_pos_vertex_data<vec4<t_flt>>& ndcsrc=xformed.getclippos();
			for(int n=0;n<3;++n)
				ndc[n]=ndcsrc.getpos()[n].getdehomogenise();
		#endif
		
		vec3<t_flt> dstNDCspace[3];
		const face_pos_vertex_data<vec4<t_flt>>& clipspacepos = xformed.getclippos();
		if(KNL::getreverse())
		{
			clipspacepos.getpos()[0].getdevice(rDstNDC,dstNDCspace[2]);
			clipspacepos.getpos()[1].getdevice(rDstNDC,dstNDCspace[1]);
			clipspacepos.getpos()[2].getdevice(rDstNDC,dstNDCspace[0]);
		}
		else
		{
			clipspacepos.getpos()[0].getdevice(rDstNDC,dstNDCspace[0]);
			clipspacepos.getpos()[1].getdevice(rDstNDC,dstNDCspace[1]);
			clipspacepos.getpos()[2].getdevice(rDstNDC,dstNDCspace[2]);
		}

		rect rDiscrete;
		crect<t_flt>::postodiscrete(dstNDCspace,3,rDiscrete);
		rect::intersect(rDstNDCClip,rDiscrete,rDiscrete);
		if(rDiscrete.isempty())
			return;

		const t_flt v0_x=dstNDCspace[0][0];
		const t_flt v0_y=dstNDCspace[0][1];
		const t_flt v1_x=dstNDCspace[1][0];
		const t_flt v1_y=dstNDCspace[1][1];
		const t_flt v2_x=dstNDCspace[2][0];
		const t_flt v2_y=dstNDCspace[2][1];

		const t_flt dTotalArea=(v1_x-v0_x)*(v2_y-v0_y)-(v1_y-v0_y)*(v2_x-v0_x);
		#if (RAS_PARADIGM==RAS_DX_PARADIGM)
			bool bDegenerateFace=(dTotalArea<=epsilon);			// positive values imply CW which we use when in DX paradigm
		#elif (RAS_PARADIGM==RAS_OGL_PARADIGM)
			bool bDegenerateFace=(dTotalArea>=epsilon);			// negative values imply CCW which we use when in OGL paradigm
		#endif
		if(bDegenerateFace)
		{
			// point or line
			return;
		}

		const t_flt v1_y_minus_v0_y=dstNDCspace[1][1]-dstNDCspace[0][1];
		const t_flt v1_x_minus_v0_x=dstNDCspace[1][0]-dstNDCspace[0][0];
		const t_flt v2_y_minus_v1_y=dstNDCspace[2][1]-dstNDCspace[1][1];
		const t_flt v2_x_minus_v1_x=dstNDCspace[2][0]-dstNDCspace[1][0];
		const t_flt v0_y_minus_v2_y=dstNDCspace[0][1]-dstNDCspace[2][1];
		const t_flt v0_x_minus_v2_x=dstNDCspace[0][0]-dstNDCspace[2][0];
		
		const KNL::fragment<MAT,QUANTIZE> knlfrag;
		KNL::template t_vs_types::template t_base_types::template t_xform_face::template vertex frag;
		const int nGWidth=pGBuffer?pGBuffer->getwidth():0;
		gvertex<t_flt> *pScanlineG=pGBuffer?pGBuffer->getscanline(rDiscrete.get(rect::v_tl)[1]+dstNDCtobuf[1]):nullptr;
		const int nZBWidth=pZBuffer->getwidth();
		zvertex<t_flt> *pScanlineZB=pZBuffer->getscanline(rDiscrete.get(rect::v_tl)[1]+dstNDCtobuf[1]);
		const t_flt dstclipspaceWrecip[3]={KNL::getreverse() ? 1.0/clipspacepos.getpos()[2][3] : 1.0/clipspacepos.getpos()[0][3],
										   1.0/clipspacepos.getpos()[1][3],
										   KNL::getreverse() ? 1.0/clipspacepos.getpos()[0][3] : 1.0/clipspacepos.getpos()[2][3]};
		for(int nDstY=rDiscrete.get(rect::v_tl)[1];nDstY<rDiscrete.get(rect::v_br)[1];++nDstY,pScanlineG+=nGWidth,pScanlineZB+=nZBWidth)
		{
			for(int nDstX=rDiscrete.get(rect::v_tl)[0];nDstX<rDiscrete.get(rect::v_br)[0];++nDstX)
			{
				// leverage the properties of the cross product to perform inside-outside tests quickly
				const t_flt p_x=af::tocontinuous<t_flt>(nDstX);
				const t_flt p_y=af::tocontinuous<t_flt>(nDstY);

				// edge test
				vec3<t_flt> edges_01_12_20;
				if(!edgetest(p_x,p_y,
							 v0_x,v0_y,v1_x,v1_y,v2_x,v2_y,
							 v0_y_minus_v2_y,v1_y_minus_v0_y,v2_y_minus_v1_y,v0_x_minus_v2_x,v1_x_minus_v0_x,v2_x_minus_v1_x,
							 edges_01_12_20))
					continue;

				// barycentric
				vec3<t_flt> alpha_beta_gamma;
				#if (RAS_PARADIGM==RAS_DX_PARADIGM)
					const t_flt dDivisor = -dTotalArea;
				#else
					const t_flt dDivisor = dTotalArea;
				#endif
				alpha_beta_gamma[0]=edges_01_12_20[1]/dDivisor;
				alpha_beta_gamma[1]=edges_01_12_20[2]/dDivisor;
				alpha_beta_gamma[2]=edges_01_12_20[0]/dDivisor;
				#ifdef _DEBUG
					const t_flt dBarySum=alpha_beta_gamma[0]+alpha_beta_gamma[1]+alpha_beta_gamma[2];
				#endif

				// buffer offset
				const int nBuffDstX=nDstX+dstNDCtobuf[0];

				// z buffer
				const t_flt dAlphaWa=alpha_beta_gamma[0]*dstclipspaceWrecip[0];
				const t_flt dBetaWb=alpha_beta_gamma[1]*dstclipspaceWrecip[1];
				const t_flt dGammaWc=alpha_beta_gamma[2]*dstclipspaceWrecip[2];
				const t_flt dRecipWp=1.0/(dstclipspaceWrecip[0]*alpha_beta_gamma[0]+dstclipspaceWrecip[1]*alpha_beta_gamma[1]+dstclipspaceWrecip[2]*alpha_beta_gamma[2]);
				const t_flt dZ=(dstNDCspace[0][2]*dAlphaWa + dstNDCspace[1][2]*dBetaWb + dstNDCspace[2][2]*dGammaWc)*dRecipWp;
				if(dZ >= pScanlineZB[nBuffDstX].get())
					continue;									// dont need to acquire for this quick check	
				pScanlineZB[nBuffDstX].acquire();
				if(dZ < pScanlineZB[nBuffDstX].get())			// actual check
				{
					if(KNL::getwritezbuffer())
						pScanlineZB[nBuffDstX].set(dZ);
				}
				else
				{
					pScanlineZB[nBuffDstX].release();
					continue;
				}
				
				if(!pGBuffer)
				{
					pScanlineZB[nBuffDstX].release();
					continue;
				}
				
				// interpolate atts
				frag.barylerp<KNL::getreverse()>(xformed,dAlphaWa,dBetaWb,dGammaWc,dRecipWp);

				// render fragment
				knlfrag.render<KNL::template t_vs_types::template t_base_types::template t_face>(lm,worldcampos,pScanlineG[nBuffDstX].getfrag(),src,xformed,frag,dZ,pMaterial);
				pScanlineZB[nBuffDstX].release();
				
				// visible fragment
				pScanlineG[nBuffDstX].setfrag();
			}
		}
	}
};

template <typename KNL> class silhouette_fragmentshader : public fragmentshader<KNL>
{
public:
	static void render(const afthread::taskscheduler *pSched,
					   const vertexattsframe<t_flt> *pFrame,
					   const camera<t_flt> *pCamera,
					   const crect<t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
					   const vertexshaderscratch<t_flt> *pVSScratch,
					   zbuffer<t_flt> *pZBuffer,gbuffer<t_flt> *pGBuffer)
	{
		if(!pFrame||!pVSScratch||rDstNDCClip.isempty()) return;

		const bool bZBuffer=!(!pZBuffer||(pZBuffer->getwidth()<rDstNDCClip.get(rect::v_br)[0]+dstNDCtobuf[0])||(pZBuffer->getheight()<rDstNDCClip.get(rect::v_br)[1]+dstNDCtobuf[1]));
		if(!bZBuffer)
			return;

		const bool bGBuffer=!(!pGBuffer||(pGBuffer->getwidth()<rDstNDCClip.get(rect::v_br)[0]+dstNDCtobuf[0])||(pGBuffer->getheight()<rDstNDCClip.get(rect::v_br)[1]+dstNDCtobuf[1]));
		if(pGBuffer)
		{
			if(!bGBuffer)
				return;
		}

		const std::vector<face_xform_union<t_flt>>& vTransformed = pVSScratch->getxformed()->get();
		const face_xform_union<t_flt> *pTransformed=&(vTransformed[0]);
		const int nFaces=static_cast<int>(vTransformed.size());
		
		const mesh<KNL::template t_vs_types::template t_base_types::template t_fb> *pMesh=static_cast<const mesh<KNL::template t_vs_types::template t_base_types::template t_fb>*>(pFrame);
		
		materialcol<t_flt> col({0,0,0},{0,0,0},{0,0,0},0);
		material<t_flt> mat(0,nFaces);
		mat.setcol(col);
		mat.enable(face_vertex_att::t_col,true);

		mat_renderfaces<false>(pSched,pMesh,{nullptr},pCamera,pTransformed,0,nFaces,&mat,rDstNDC,rDstNDCClip,dstNDCtobuf,pZBuffer,pGBuffer);
	}
};

enum knlflagtype{ft_lit=0x1,ft_reverse=0x2,ft_writezbuffer=0x4};

template <typename VST,int FLAGS> class fragmentshaderknl
{
public:
	using t_vs_types=VST;
	using t_flt=VST::template t_base_types::template t_flt;

	__forceinline static constexpr bool getlit(void){return FLAGS&ft_lit?true:false;}
	__forceinline static constexpr bool getreverse(void){return FLAGS&ft_reverse?true:false;}
	__forceinline static constexpr bool getwritezbuffer(void){return FLAGS&ft_writezbuffer?true:false;}

	template <int MAT,bool QUANTIZE> class fragment
	{
	public:
		template <typename F> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
														vec3<t_flt>& dst,const F& src,const F::template t_xform& xformed,const F::template t_xform::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const;
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3<t_flt>& src,const face_pos3_xform<t_flt>& xformed,const face_pos3_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=0;
			render<nVertex>(lm,campos,dst,
									{},
									{},
									dNDCspaceZ,
									{},
									{},
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_col<t_flt>& src,const face_pos3_col_xform<t_flt>& xformed,const face_pos3_col_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_col;
			render<nVertex>(lm,campos,dst,
									{},
									{},
									dNDCspaceZ,
									src.getcol().getsingular().get()==vertex_data_singular::t_true ?
												src.getcol().getcol()[0] :									// would not have bary value as all same ( so any will do )
												frag.col.col.t,
									{},
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_tex<t_flt>& src,const face_pos3_tex_xform<t_flt>& xformed,const face_pos3_tex_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_tex;
			render<nVertex>(lm,campos,dst,
									{},
									{},
									dNDCspaceZ,
									{},
									frag.tex.tex.t,
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm<t_flt>& src,const face_pos3_norm_xform<t_flt>& xformed,const face_pos3_norm_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									{},
									{},
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm_tex<t_flt>& src,const face_pos3_norm_tex_xform<t_flt>& xformed,const face_pos3_norm_tex_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm|face_vertex_att::t_tex;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									{},
									frag.tex.tex.t,
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm_bump<t_flt>& src,const face_pos3_norm_bump_xform<t_flt>& xformed,const face_pos3_norm_bump_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm|face_vertex_att::t_bump;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									{},
									{},
									frag.bump.bump.t,
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm_col<t_flt>& src,const face_pos3_norm_col_xform<t_flt>& xformed,const face_pos3_norm_col_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm|face_vertex_att::t_col;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									src.getcol().getsingular().get()==vertex_data_singular::t_true ?
												src.getcol().getcol()[0] :									// would not have bary value as all same ( so any will do )
												frag.col.col.t,
									{},
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm_col_tex<t_flt>& src,const face_pos3_norm_col_tex_xform<t_flt>& xformed,const face_pos3_norm_col_tex_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm|face_vertex_att::t_col|face_vertex_att::t_tex;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									src.getcol().getsingular().get()==vertex_data_singular::t_true ?
												src.getcol().getcol()[0] :									// would not have bary value as all same ( so any will do )
												frag.col.col.t,
									frag.tex.tex.t,
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_col_tex<t_flt>& src,const face_pos3_col_tex_xform<t_flt>& xformed,const face_pos3_col_tex_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_col|face_vertex_att::t_tex;
			render<nVertex>(lm,campos,dst,
									{},
									{},
									dNDCspaceZ,
									src.getcol().getsingular().get()==vertex_data_singular::t_true ?
												src.getcol().getcol()[0] :									// would not have bary value as all same ( so any will do )
												frag.col.col.t,
									frag.tex.tex.t,
									{},
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm_tex_bump<t_flt>& src,const face_pos3_norm_tex_bump_xform<t_flt>& xformed,const face_pos3_norm_tex_bump_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm|face_vertex_att::t_bump|face_vertex_att::t_tex;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									{},
									frag.tex.tex.t,
									frag.bump.bump.t,
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm_col_bump<t_flt>& src,const face_pos3_norm_col_bump_xform<t_flt>& xformed,const face_pos3_norm_col_bump_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm|face_vertex_att::t_bump|face_vertex_att::t_col;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									src.getcol().getsingular().get()==vertex_data_singular::t_true ?
												src.getcol().getcol()[0] :									// would not have bary value as all same ( so any will do )
												frag.col.col.t,
									{},
									frag.bump.bump.t,
									pMaterial);
		}
		template <> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
											  vec3<t_flt>& dst,const face_pos3_norm_col_tex_bump<t_flt>& src,const face_pos3_norm_col_tex_bump_xform<t_flt>& xformed,const face_pos3_norm_col_tex_bump_xform<t_flt>::template vertex& frag,const t_flt dNDCspaceZ,const material<t_flt> *pMaterial)const
		{
			constexpr int nVertex=face_vertex_att::t_norm|face_vertex_att::t_bump|face_vertex_att::t_col|face_vertex_att::t_tex;
			render<nVertex>(lm,campos,dst,
									frag.worldpos.pos.t,
									src.getnorm().getsingular().get()==vertex_data_singular::t_true ?
												xformed.getworldnorm().getnorm()[0] :						// would not have bary value as all same ( so any will do )
												frag.worldnorm.norm.t,
									dNDCspaceZ,
									src.getcol().getsingular().get()==vertex_data_singular::t_true ?
												src.getcol().getcol()[0] :									// would not have bary value as all same ( so any will do )
												frag.col.col.t,
									frag.tex.tex.t,
									frag.bump.bump.t,
									pMaterial);
		}
	protected:
		template <int VERTEX> __forceinline void render(const lightcache<t_flt>& lm,const vec3<t_flt>& campos,
														vec3<t_flt>& dst,
														const vec3<t_flt>& worldpos,const vec3<t_flt>& worldnorm,const t_flt dNDCspaceZ,
														const vec3<t_flt>& col,
														const vec2<t_flt>& texuv,
														const vec2<t_flt>& bumpuv,
														// ...
														const material<t_flt> *pMaterial)const
		{
			vec3<t_flt> tex;
			constexpr bool bVertexTex=((MAT & face_vertex_att::t_tex) && (VERTEX & face_vertex_att::t_tex));
			if(bVertexTex)
				pMaterial->gettex()->getcolour<t_flt,VST::template t_pt>(texuv,tex);

			constexpr bool bVertexNorm=(VERTEX & face_vertex_att::t_norm)?true:false;

			vec3<t_flt> bumpworldnorm;
			constexpr bool bVertexBump =((MAT & face_vertex_att::t_bump) && (VERTEX & face_vertex_att::t_bump) && bVertexNorm);
			if(bVertexBump)
				pMaterial->getbump()->perturb(bumpuv,worldnorm,bumpworldnorm);

			vec3<t_flt> cubictex;
			constexpr bool bVertexCubicTex=((MAT & face_vertex_att::t_env_cubic) && bVertexNorm);
			if(bVertexCubicTex)
				pMaterial->getcubicenv()->getcolour<t_flt,VST::template t_pt>(campos,worldpos,bVertexBump?bumpworldnorm:worldnorm,cubictex);

			constexpr bool bVertexCol=(VERTEX & face_vertex_att::t_col)?true:false;

			const quantize_static_3<t_flt> quantizeNull;
			const quantize_static_3<t_flt>& diffuseQuantize=(QUANTIZE && bVertexNorm)?pMaterial->getcol().getdiffusequantize():quantizeNull;
			const quantize_static_3<t_flt>& specularQuantize=(QUANTIZE && bVertexNorm)?pMaterial->getcol().getspecularquantize():quantizeNull;
			
			vec3<t_flt> tmp;
			switch(MAT)
			{
				case 0:
				{
					if(bVertexCol)
					{
						dst[0]=col[0];
						dst[1]=col[1];
						dst[2]=col[2];
					}
					else
					{
						dst[0]=0;
						dst[1]=0;
						dst[2]=0;
					}
				}
				break;
				
				case face_vertex_att::t_col:
				case face_vertex_att::t_col|face_vertex_att::t_bump:
				{
					if(bVertexCol)
						lerp(pMaterial->getcol().getdiffuse(),col,dst);
					else
					{
						dst[0]=pMaterial->getcol().getdiffuse()[0];
						dst[1]=pMaterial->getcol().getdiffuse()[1];
						dst[2]=pMaterial->getcol().getdiffuse()[2];
					}
				}
				break;
				case face_vertex_att::t_col|face_vertex_att::t_tex:
				case face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_bump:
				{
					if(bVertexCol && bVertexTex)
						lerp(pMaterial->getcol().getdiffuse(),tex,col,dst);
					else
					if(bVertexCol)
						lerp(pMaterial->getcol().getdiffuse(),col,dst);
					else
					if(bVertexTex)
						lerp(pMaterial->getcol().getdiffuse(),tex,dst);
					else
					{
						dst[0]=pMaterial->getcol().getdiffuse()[0];
						dst[1]=pMaterial->getcol().getdiffuse()[1];
						dst[2]=pMaterial->getcol().getdiffuse()[2];
					}
				}
				break;
				case face_vertex_att::t_col|face_vertex_att::t_env_cubic:
				case face_vertex_att::t_col|face_vertex_att::t_env_cubic|face_vertex_att::t_bump:
				{
					if(bVertexCol && bVertexCubicTex)
						lerp(pMaterial->getcol().getdiffuse(),cubictex,col,dst);
					else
					if(bVertexCol)
						lerp(pMaterial->getcol().getdiffuse(),col,dst);
					else
					if(bVertexCubicTex)
						lerp(pMaterial->getcol().getdiffuse(),cubictex,dst);
					else
					{
						dst[0]=pMaterial->getcol().getdiffuse()[0];
						dst[1]=pMaterial->getcol().getdiffuse()[1];
						dst[2]=pMaterial->getcol().getdiffuse()[2];
					}
				}
				break;
				case face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_env_cubic:
				case face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_env_cubic|face_vertex_att::t_bump:
				{
					if(bVertexCol && bVertexTex && bVertexCubicTex)
						lerp(pMaterial->getcol().getdiffuse(),cubictex,tex,col,dst);
					else
					if(bVertexCol && bVertexTex)
						lerp(pMaterial->getcol().getdiffuse(),tex,col,dst);
					else
					if(bVertexCol && bVertexCubicTex)
						lerp(pMaterial->getcol().getdiffuse(),cubictex,col,dst);
					else
					if(bVertexTex && bVertexCubicTex)
						lerp(pMaterial->getcol().getdiffuse(),tex,cubictex,dst);
					else
					if(bVertexCol)
						lerp(pMaterial->getcol().getdiffuse(),col,dst);
					else
					if(bVertexTex)
						lerp(pMaterial->getcol().getdiffuse(),tex,dst);
					else
					if(bVertexCubicTex)
						lerp(pMaterial->getcol().getdiffuse(),cubictex,dst);
					else
					{
						dst[0]=pMaterial->getcol().getdiffuse()[0];
						dst[1]=pMaterial->getcol().getdiffuse()[1];
						dst[2]=pMaterial->getcol().getdiffuse()[2];
					}
				}
				break;


				// -------------------------------------------------------------- //


				case face_vertex_att::t_tex:
				case face_vertex_att::t_tex|face_vertex_att::t_bump:
				{
					if(bVertexCol && bVertexTex)
						lerp(tex,col,dst);
					else
					if(bVertexCol)
					{
						dst[0]=col[0];
						dst[1]=col[1];
						dst[2]=col[2];
					}
					else
					if(bVertexTex)
					{
						dst[0]=tex[0];
						dst[1]=tex[1];
						dst[2]=tex[2];
					}
					else
						return;
				}
				break;
				case face_vertex_att::t_tex|face_vertex_att::t_env_cubic:
				case face_vertex_att::t_tex|face_vertex_att::t_env_cubic|face_vertex_att::t_bump:
				{
					if(bVertexCol && bVertexTex && bVertexCubicTex)
						lerp(cubictex,tex,col,dst);
					else
					if(bVertexCol && bVertexTex)
						lerp(tex,col,dst);
					else
					if(bVertexCol && bVertexCubicTex)
						lerp(cubictex,col,dst);
					else
					if(bVertexTex && bVertexCubicTex)
						lerp(tex,cubictex,dst);
					else
					if(bVertexCol)
					{
						dst[0]=col[0];
						dst[1]=col[1];
						dst[2]=col[2];
					}
					else
					if(bVertexTex)
					{
						dst[0]=tex[0];
						dst[1]=tex[1];
						dst[2]=tex[2];
					}
					else
					if(bVertexCubicTex)
					{
						dst[0]=cubictex[0];
						dst[1]=cubictex[1];
						dst[2]=cubictex[2];
					}
					else
						return;
				}
				break;


				// -------------------------------------------------------------- //


				case face_vertex_att::t_env_cubic:
				case face_vertex_att::t_env_cubic|face_vertex_att::t_bump:
				{
					if(bVertexCol && bVertexCubicTex)
						lerp(cubictex,col,dst);
					else
					if(bVertexCol)
					{
						dst[0]=col[0];
						dst[1]=col[1];
						dst[2]=col[2];
					}
					else
					if(bVertexCubicTex)
					{
						dst[0]=cubictex[0];
						dst[1]=cubictex[1];
						dst[2]=cubictex[2];
					}
					else
						return;
				}
				break;


				// -------------------------------------------------------------- //


				case face_vertex_att::t_bump:
				{
					if(bVertexCol)
					{
						dst[0]=col[0];
						dst[1]=col[1];
						dst[2]=col[2];
					}
				}
				break;
			}

			if(bVertexNorm && getlit())
				lm.modulate<QUANTIZE>(campos,
									  worldpos,
									  bVertexBump?bumpworldnorm:worldnorm,
									  dNDCspaceZ,
									  dst,
									  (MAT & face_vertex_att::t_col) ? pMaterial->getcol().getambient() : materialcol<t_flt>::getdefambient(),
									  (MAT & face_vertex_att::t_col) ? pMaterial->getcol().getspecular() : materialcol<t_flt>::getdefspecular(),
									  (MAT & face_vertex_att::t_col) ? pMaterial->getcol().getshininess() : materialcol<t_flt>::getdefshininess(),
									  diffuseQuantize,specularQuantize,
									  dst);
		}
		template <typename LERP> __forceinline void lerp(const vec3<t_flt>& a,const vec3<t_flt>& b,const vec3<t_flt>& c,const vec3<t_flt>& d,LERP& o)const
		{
			const t_flt dbl = af::getrecip_4<t_flt>();
			o[0]=(a[0]+b[0]+c[0]+d[0])*dbl;
			o[1]=(a[1]+b[1]+c[1]+d[1])*dbl;
			o[2]=(a[2]+b[2]+c[2]+d[2])*dbl;
		}
		template <typename LERP> __forceinline void lerp(const vec3<t_flt>& a,const vec3<t_flt>& b,const vec3<t_flt>& c,LERP& o)const
		{
			const t_flt dbl = af::getrecip_3<t_flt>();
			o[0]=(a[0]+b[0]+c[0])*dbl;
			o[1]=(a[1]+b[1]+c[1])*dbl;
			o[2]=(a[2]+b[2]+c[2])*dbl;
		}
		template <typename LERP> __forceinline void lerp(const vec3<t_flt>& a,const vec3<t_flt>& b,LERP& o)const
		{
			const t_flt dbl = af::getrecip_2<t_flt>();
			o[0]=(a[0]+b[0])*dbl;
			o[1]=(a[1]+b[1])*dbl;
			o[2]=(a[2]+b[2])*dbl;
		}
	};
};

}
