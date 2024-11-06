#pragma once

#include "viewtool.h"
#include "3d_planebasis.h"

class camerarotate_viewtool:public viewtool
{
public:
	camerarotate_viewtool(CrasterizerView *pV);
	virtual ~camerarotate_viewtool(){}

	virtual type gettype( void ) const {return t_camerarotate;}

	virtual bool render(void) const;
	virtual void render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const;

	virtual void cancel(void);

	virtual bool getupdateselection(void)const{return false;}
	virtual void selectionchanged( void );
protected:
	af3d::mat4<> m_WorldToClipSpaceTrns;					// world space to clip space
	af3d::mat4<> m_ClipToWorldSpaceTrns;					// clip space to world space

	af3d::vec3<> m_BeginUp;									// initial transform
	af3d::vec3<> m_BeginDir;								// initial transform
	af3d::mat4<> m_AggregateUnBoundTrns;					// aggregate unbound transform

	af3d::vec2<int> m_rtptBeginDrag;						// begin drag position
	af3d::vec2<int> m_rtptPrev;								// previous drag position

	af3d::plane<> m_CameraWorldPlane;						// camera facing world plane
	af3d::vec3<> m_CameraWorldPlaneIntersect;				// camera facing world plane intersect

	virtual void getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht ) const;

	virtual bool getselectiondrag(void)const{return false;}
	virtual void begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove );
	virtual void movedrag( const af3d::vec2<int>& rtpt );
	virtual void enddrag( const af3d::vec2<int>& rtpt );

	void rotate( const af3d::vec2<int>& rtpt );
	void unbound_rotate( const af3d::vec2<int>& rtpt );

	void addrotation(const af3d::mat4<>& from,const af3d::rotation3<>& r,af3d::mat4<>& to)const;
};
