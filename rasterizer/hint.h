
#pragma once

#include "3d_camera.h"
#include "viewtool.h"

class CrasterizerView;
class CrasterizerDoc;

class hint : public CObject
{
public:
	enum type {t_null=0,
			   t_drag,t_selection,t_frame_visible,t_frame_name,t_frame_opacity,t_frame_effect,
			   t_frame_append,t_frame_erase,t_frame_reparent,
			   t_view_active,t_view_stop,t_initial_update,
			   t_facebuffer_pos,t_facebuffer_norm,t_facebuffer_tex,t_facebuffer_col,t_facebuffer_bump,
			   t_light_atten,t_light_range,t_spotlight_umbra_penumbra,t_light_col,
			   t_material_add,t_material_del,t_material_range,t_material_col,t_material_enable,t_material_shininess,t_material_image,t_material_quantize_diffuse,t_material_quantize_specular};
	
	hint(){m_pMat=nullptr;m_pDoc=nullptr;m_pView=nullptr;m_Type=t_null;m_DragType=viewtool::td_null;m_pFrame=nullptr;m_pCamera=nullptr;}
	hint(CrasterizerView *pV,CrasterizerDoc *pD,const type t):hint(){m_pDoc=pD;m_pView=pV;m_Type=t;}
	hint(CrasterizerView *pV,CrasterizerDoc *pD,const type t,af3d::vertexattsframe<> *p):hint(){m_pDoc=pD;m_pView=pV;m_Type=t;m_pFrame=p;}
	hint(CrasterizerView *pV,CrasterizerDoc *pD,const viewtool::dragtype t,af3d::vertexattsframe<> *p):hint(){m_pDoc=pD;m_pView=pV;m_Type=t_drag;m_DragType=t;m_pFrame=p;}
	hint(CrasterizerView *pV,CrasterizerDoc *pD,const viewtool::dragtype t,af3d::camera<> *p):hint(){m_pDoc=pD;m_pView=pV;m_Type=t_drag;m_DragType=t;m_pCamera=p;}
	hint(CrasterizerView *pV,CrasterizerDoc *pD,const type t,af3d::vertexattsframe<> *p,af3d::material<> *pM):hint(){m_pDoc=pD;m_pView=pV;m_Type=t;m_pFrame=p;m_pMat=pM;}
	virtual ~hint(){}

	type gettype(void)const{return m_Type;}
	CrasterizerView *getview(void)const{return m_pView;}
	CrasterizerDoc *getdoc(void)const{return m_pDoc;}
	viewtool::dragtype getdragtype(void)const{return m_DragType;}
	af3d::vertexattsframe<> *getframe(void)const{return m_pFrame;}
	af3d::camera<> *getcamera(void)const{return m_pCamera;}
	af3d::material<> *getmaterial(void)const{return m_pMat;}
protected:
	type m_Type;
	CrasterizerView *m_pView;
	CrasterizerDoc *m_pDoc;
	viewtool::dragtype m_DragType;
	af3d::vertexattsframe<> *m_pFrame;
	af3d::camera<> *m_pCamera;
	af3d::material<> *m_pMat;
};
