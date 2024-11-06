#pragma once

#include "viewtool.h"
#include "3d_planebasis.h"

class translate_scale_viewtool :public viewtool
{
public:
	translate_scale_viewtool(CrasterizerView *pV):viewtool(pV){}
	virtual ~translate_scale_viewtool(){}

	virtual type gettype( void ) const {return t_translate_scale;}

	virtual bool render(void) const;
	virtual void render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const;

	virtual void cancel(void);

	virtual bool getupdateselection(void)const{return true;}
	virtual void selectionchanged( void );
protected:	
	bool m_bScale;

	std::vector<af3d::vertexattsframe<>*> m_vLights;			// lights mutated

	af3d::mat4<> m_WorldToClipSpaceTrns;						// world space to clip space
	af3d::mat4<> m_ClipToWorldSpaceTrns;						// clip space to world space

	af3d::mat4<> m_BeginTrns;									// initial transform
	af3d::mat4<> m_BeginCompositeTrns;							// initial composite transform
	af3d::facetrnsbbox<> m_BeginWorldBBox;						// initial world space bbox

	af3d::mat4<> m_WorldToParentSpaceTrns;						// world space to parent space

	af3d::mat4<> m_WorldSpaceAxisAlignTrns;						// world space to axis aligned world space
	af3d::mat4<> m_InvWorldSpaceAxisAlignTrns;					// axis aligned world space to world space
	af3d::mat4<> m_ModelToAxisAlignedWorldSpaceTrns;			// modelspace to axis aligned world space
	
	af3d::plane<> m_CameraWorldPlane;							// camera facing world plane
	af3d::vec3<> m_CameraWorldPlaneIntersect;					// camera facing world plane intersect

	af3d::plane<> m_ScaleWorldPlane;							// world scale plane
	af3d::vec3<> m_ScaleWorldPlaneIntersect;					// world scale plane begin drag intersect
	af3d::facetrnsbbox<>::planetype m_WorldBBoxScalePrimary;	// world bbox scale primary plane
	af3d::planebasis<> m_ScaleWorldPlaneBasis;					// 2d world scale plane

	std::vector<af3d::facetrnsbbox<>::vertextype> m_ScaleU;		// scale in U
	std::vector<af3d::facetrnsbbox<>::vertextype> m_ScaleV;		// scale in V
	std::vector<af3d::facetrnsbbox<>::vertextype> m_ScaleUV;	// scale in U and V

	int m_nHandle;												// scale handle

	virtual void getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht ) const;

	virtual bool getselectiondrag(void)const{return true;}
	virtual void begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove );
	virtual void movedrag( const af3d::vec2<int>& rtpt );
	virtual void enddrag( const af3d::vec2<int>& rtpt );

	void scale( const af3d::vec2<int>& rtpt );
	void translate( const af3d::vec2<int>& rtpt );

	void addscale(const af3d::mat4<>& from,const af3d::scale3<>& r,af3d::mat4<>& to)const;
	void addtranslate(const af3d::facetrnsbbox<>& worldto,af3d::mat4<>& trns)const;
	af3d::vec3<> getscale(const af3d::facetrnsbbox<>& from,const af3d::facetrnsbbox<>& to)const;
	af3d::facetrnsbbox<> getscalebbox(const af3d::vec3<>& worldpos)const;
	void getscalebbox_tr(const bool bFront);
	void getscalebbox_br(const bool bFront);
	void getscalebbox_tl(const bool bFront);
	void getscalebbox_bl(const bool bFront);
	void getscaleplane(void);
};
