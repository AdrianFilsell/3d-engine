#pragma once

#include "3d_mesh.h"
#include "dib.h"

namespace af3d
{

template <typename BT,typename P,typename PT> class vertexshader_types
{
public:
	using t_base_types=BT;							// face base types

	using t_proj=P;									// projection

	using t_pt=PT;									// pixel
};

template <typename T=RAS_FLTTYPE> class vertexshaderscratch
{
public:
	vertexshaderscratch(){m_spXFormed=std::shared_ptr<facebuffer<face_xform_union<T>>>(new facebuffer<face_xform_union<T>>);}
	~vertexshaderscratch(){}
	__forceinline const facebuffer<face_xform_union<T>> *getxformed(void)const{return m_spXFormed.get();}
	
	__forceinline mat4<T>& getmat4modeltoclipspace(void){return m_Mat4ModelToClipSpace;}
	__forceinline mat4<T>& getmat4scratch(void){return m_Mat4Scratch;}
	__forceinline mat4<T>& getmat4transposedinversemodeltoworld(void){return m_TransposedInverseModelToWorld;}
	__forceinline facebuffer<face_xform_union<T>> *getmutablexformed(void){return m_spXFormed.get();}
protected:
	std::shared_ptr<facebuffer<face_xform_union<T>>> m_spXFormed;
	mat4<T> m_Mat4ModelToClipSpace;
	mat4<T> m_Mat4Scratch;
	mat4<T> m_TransposedInverseModelToWorld;
};

template <typename VST> class vertexshader
{
public:
	using t_vs_types=VST;
	using t_base_types=VST::template t_base_types;
	
	static void render(const afthread::taskscheduler *pSched,
					   const vertexattsframe<t_base_types::template t_flt> *pFrame,
					   const camera<t_base_types::template t_flt> *pCamera,const t_vs_types::template t_proj *pProjection,
					   vertexshaderscratch<t_base_types::template t_flt> *pScratch)
	{
		if(!pFrame||!pCamera||!pProjection||!pScratch)return;

		const mesh<t_base_types::template t_fb> *pMesh=static_cast<const mesh<t_base_types::template t_fb>*>(pFrame);

		const t_base_types::template t_fb *pVertexBuffer=pMesh->getvertexbuffer();

		mat4<t_base_types::template t_flt>::mul(pMesh->getcompositetrns(),pCamera->gettrns(),pScratch->getmat4scratch());
		mat4<t_base_types::template t_flt>::mul(pScratch->getmat4scratch(),pProjection->gettrns(),pScratch->getmat4modeltoclipspace());
		
		if(t_base_types::template t_xform_face::xform_normal())
			pScratch->getmat4transposedinversemodeltoworld()=(pMesh->getcompositetrns().inverse()).transpose(); // needed for normal transform

		t_base_types::template t_fb::xform<projectionclipper<>::et_all,t_vs_types>(pSched,pMesh->getcompositetrns(),pScratch->getmat4transposedinversemodeltoworld(),pScratch->getmat4modeltoclipspace(),pVertexBuffer,pScratch->getmutablexformed());
	}
};

}
