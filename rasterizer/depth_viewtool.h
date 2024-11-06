#pragma once

#include "viewtool.h"
#include "3d_planebasis.h"

class depth_viewtool:public viewtool
{
public:
	depth_viewtool(CrasterizerView *pV):viewtool(pV){}
	virtual ~depth_viewtool(){}

	virtual type gettype( void ) const {return t_depth;}

	virtual bool render(void) const;
	virtual void render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const;

	virtual void cancel(void);

	virtual bool getupdateselection(void)const{return true;}
	virtual void selectionchanged( void );
protected:
	std::vector<af3d::vertexattsframe<>*> m_vLights;		// lights mutated

	af3d::mat4<> m_WorldToClipSpaceTrns;					// world space to clip space
	af3d::mat4<> m_ClipToWorldSpaceTrns;					// clip space to world space

	af3d::mat4<> m_BeginTrns;								// initial transform
	af3d::mat4<> m_BeginCompositeTrns;						// initial composite transform

	af3d::mat4<> m_WorldToParentSpaceTrns;					// world space to parent space

	af3d::plane<> m_CameraWorldPlane;						// camera facing world plane
	af3d::vec3<> m_CameraWorldPlaneIntersect;				// camera facing world plane intersect

	af3d::planebasis<> m_CameraWorldPlaneBasis;				// 2d camera facing world plane
	af3d::planebasis<> m_DepthWorldPlaneBasis;				// translate ( depth ) 2d world plane
	
	virtual void getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht ) const;

	virtual bool getselectiondrag(void)const{return true;}
	virtual void begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove );
	virtual void movedrag( const af3d::vec2<int>& rtpt );
	virtual void enddrag( const af3d::vec2<int>& rtpt );

	void translate( const af3d::vec2<int>& rtpt );
};
