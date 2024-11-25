#pragma once

#include "3d_scene.h"
#include "3d_camera.h"
#include "3d_projection.h"
#include "3d_vertexshader.h"
#include "3d_fragmentshader.h"
#include "3d_tbuffer.h"

namespace af3d
{

class rendercontext
{
public:
	rendercontext(){}
	~rendercontext(){}

	template <typename P,typename PT> static void renderscene(const afthread::taskscheduler *pSched,
															  const int nTypes,
															  const scene<P::template t_flt> *pScene,
															  const lightcache<P::template t_flt>& lm_lights,const lightcache<P::template t_flt>& lm,
															  const camera<P::template t_flt> *pCamera,const P *pProjection,
															  const crect<P::template t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
															  vertexshaderscratch<P::template t_flt> *pVSScratch,
															  zbuffer<P::template t_flt> *pZBuffer,gbuffer<P::template t_flt> *pGBuffer,
															  afdib::dib *pDeviceDib)
	{
		if(pScene)
			renderframes<P,PT>(pSched,nTypes,pScene->getchildren(),lm_lights,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
	}
	template <typename P,typename PT> __forceinline static void renderframes(const afthread::taskscheduler *pSched,
																			 const int nTypes,
																			 const std::vector<std::shared_ptr<vertexattsframe<P::template t_flt>>> *pFrames,
																			 const lightcache<P::template t_flt>& lm_lights,const lightcache<P::template t_flt>& lm,
																			 const camera<P::template t_flt> *pCamera,const P *pProjection,
																			 const crect<P::template t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
																			 vertexshaderscratch<P::template t_flt> *pVSScratch,
																			 zbuffer<P::template t_flt> *pZBuffer,gbuffer<P::template t_flt> *pGBuffer,afdib::dib *pDeviceDib)
	{
		if(pFrames)
		{
			auto i=pFrames->cbegin(),end=pFrames->cend();
			for(;i!=end;++i)
				if((*i)->getvisible())
				{
					const lightcache<P::template t_flt> *pLM=((*i)->gettype()==vertexattsframe<P::template t_flt>::t_light_mesh)?
															 &lm_lights:&lm;
					
					if((*i)->gettype()&nTypes)
						render<P,PT>(pSched,nTypes,(*i).get(),*pLM,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
					renderframes<P,PT>(pSched,nTypes,(*i)->getchildren(),lm_lights,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				}
		}
	}
protected:
	template <typename P,typename PT> __forceinline static void render(const afthread::taskscheduler *pSched,
																	   const int nTypes,
																	   const vertexattsframe<P::template t_flt> *pFrame,
																	   const lightcache<P::template t_flt>& lm,const camera<P::template t_flt> *pCamera,const P *pProjection,
																	   const crect<P::template t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
																	   vertexshaderscratch<P::template t_flt> *pVSScratch,
																	   zbuffer<P::template t_flt> *pZBuffer,gbuffer<P::template t_flt> *pGBuffer,afdib::dib *pDeviceDib)
	{
		if(pFrame)
			switch(pFrame->getvertexatts())
			{
				case (face_vertex_att::t_pos):
					render<P,PT,face_pos3<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;

				case (face_vertex_att::t_pos|face_vertex_att::t_norm):
					render<P,PT,face_pos3_norm<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_norm|face_vertex_att::t_bump):
					render<P,PT,face_pos3_norm_bump<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_norm|face_vertex_att::t_tex):
					render<P,PT,face_pos3_norm_tex<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_norm|face_vertex_att::t_tex|face_vertex_att::t_bump):
					render<P,PT,face_pos3_norm_tex_bump<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_norm|face_vertex_att::t_col|face_vertex_att::t_bump):
					render<P,PT,face_pos3_norm_col_bump<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_norm|face_vertex_att::t_col):
					render<P,PT,face_pos3_norm_col<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_norm|face_vertex_att::t_col|face_vertex_att::t_tex):
					render<P,PT,face_pos3_norm_col_tex<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_norm|face_vertex_att::t_col|face_vertex_att::t_tex|face_vertex_att::t_bump):
					render<P,PT,face_pos3_norm_col_tex_bump<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;

				case (face_vertex_att::t_pos|face_vertex_att::t_col):
					render<P,PT,face_pos3_col<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				case (face_vertex_att::t_pos|face_vertex_att::t_col|face_vertex_att::t_tex):
					render<P,PT,face_pos3_col_tex<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;

				case (face_vertex_att::t_pos|face_vertex_att::t_tex):
					render<P,PT,face_pos3_tex<P::template t_flt>>(pSched,nTypes,pFrame,lm,pCamera,pProjection,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer,pDeviceDib);
				break;
				
				default:break;
			}
	}
	template <typename P,typename PT,typename F> __forceinline static void render(const afthread::taskscheduler *pSched,
																				  const int nTypes,
																				  const vertexattsframe<P::template t_flt> *pFrame,
																				  const lightcache<P::template t_flt>& lm,const camera<P::template t_flt> *pCamera,const P *pProjection,
																				  const crect<P::template t_flt>& rDstNDC,const rect& rDstNDCClip,const vec2<int>& dstNDCtobuf,
																				  vertexshaderscratch<P::template t_flt> *pVSScratch,
																				  zbuffer<P::template t_flt> *pZBuffer,gbuffer<P::template t_flt> *pGBuffer,afdib::dib *pDeviceDib)
	{
		using t_base_types=face_types<F>;
		using t_vs_types=vertexshader_types<t_base_types,P,PT>;

		// effect
		switch(pFrame->geteffect())
		{
			case vertexattsframe<P::template t_flt>::et_silhouette:
			{
				const P::template t_flt dWorldScale=1.075; // fixed value could be in the effect if we didnt just store an enum

				using t_vs=silhouette_vertexshader<t_vs_types>;
				t_vs::render(pSched,pFrame,dWorldScale,pCamera,pProjection,pVSScratch);

				using t_fs=silhouette_fragmentshader<fragmentshaderknl<t_vs_types,ft_reverse>>;
				t_fs::render(pSched,pFrame,pCamera,rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer);
			}
			break;
		}

		using t_vs=vertexshader<t_vs_types>;
		t_vs::render(pSched,pFrame,pCamera,pProjection,pVSScratch);

		using t_fs=fragmentshader<fragmentshaderknl<t_vs_types,ft_lit|ft_writezbuffer>>;
		t_fs::render(pSched,pFrame,lm,pCamera,(nTypes & vertexattsframe<P::template t_flt>::t_shadow),rDstNDC,rDstNDCClip,dstNDCtobuf,pVSScratch,pZBuffer,pGBuffer);
		t_fs::render(pSched,pFrame,rDstNDCClip,dstNDCtobuf,pGBuffer,pDeviceDib);
	}
};

}
