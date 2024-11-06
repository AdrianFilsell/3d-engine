#pragma once

#include "viewtool.h"
#include "3d_planebasis.h"

class rotate_viewtool:public viewtool
{
public:
	rotate_viewtool(CrasterizerView *pV);
	virtual ~rotate_viewtool(){}

	virtual type gettype( void ) const {return t_rotate;}

	virtual bool render(void) const;
	virtual void render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const;

	virtual void cancel(void);

	virtual bool getupdateselection(void)const{return true;}
	virtual void selectionchanged( void );
protected:
	RAS_FLTTYPE m_dRotationBBoxSweepDeg;

	std::vector<af3d::vertexattsframe<>*> m_vLights;		// lights mutated

	af3d::mat4<> m_WorldToClipSpaceTrns;					// world space to clip space
	af3d::mat4<> m_ClipToWorldSpaceTrns;					// clip space to world space

	af3d::mat4<> m_BeginTrns;								// initial transform
	af3d::mat4<> m_BeginCompositeTrns;						// initial composite transform
	af3d::facetrnsbbox<> m_BeginWorldBBox;					// initial world space bbox

	af3d::mat4<> m_WorldToParentSpaceTrns;					// world space to parent space
	
	af3d::vec2<int> m_rtptBeginDrag;						// begin drag position
	af3d::vec2<int> m_rtptPrev;								// previous drag position

	bool m_bBound;											// bound plane drag
	bool m_bBoundPlaneColinearHandles;						// bound plane drag handles on same screen space line

	af3d::plane<> m_CameraWorldPlane;						// camera facing world plane
	af3d::vec3<> m_CameraWorldPlaneIntersect;				// camera facing world plane intersect

	af3d::planebasis<> m_BoundWorldPlaneBasis;				// 2d bound world plane
	std::vector<af3d::vec4<>> m_BoundClipPlaneSegments;		// bound clip space plane segments
	af3d::vec3<> m_BoundWorldPlaneNormal;					// bound world plane normal

	af3d::vec3<> m_WorldBBoxCentre;							// bbox centre in world space

	RAS_FLTTYPE m_dBoundWorldPlaneBasisRadians;				// angle in radians on m_BoundWorldPlaneBasis

	virtual void getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht ) const;

	virtual bool getselectiondrag(void)const{return true;}
	virtual void begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove );
	virtual void movedrag( const af3d::vec2<int>& rtpt );
	virtual void enddrag( const af3d::vec2<int>& rtpt );

	void rotate( const af3d::vec2<int>& rtpt );
	void renderrotatebbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const int nHandleTypes)const;

	void getselectionrotatebbox(std::vector<af3d::vec3<>>& vWorld,const RAS_FLTTYPE dRotationBBoxSweepDeg,const af3d::plane<>::axistype at)const;
	void getselectionrotatebbox(std::vector<af3d::vec4<>>& vClip,const RAS_FLTTYPE dRotationBBoxSweepDeg,const af3d::plane<>::axistype at)const;
	void getselectionrotatebboxhandles(std::vector<af3d::vec3<>>& vWorldSpaceHandles)const;

	void bound_rotate( const af3d::vec2<int>& rtpt );
	void unbound_rotate( const af3d::vec2<int>& rtpt );

	std::pair<af3d::plane<>::axistype,bool> getrotplanefromhandle(const int nHandle)const;
	int gethandlefromrotplane(const std::pair<af3d::plane<>::axistype,bool> p)const;
	bool getboundplaneworldintersect(const af3d::vec2<int>& rtpt,af3d::vec3<>& worldintersect)const;
	bool isboundplanecolinearhandles(const std::pair<af3d::plane<>::axistype,bool> p)const;

	void addrotation(const af3d::mat4<>& from,const af3d::rotation3<>& r,af3d::mat4<>& to)const;
};
