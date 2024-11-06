#pragma once

#include "3d_mesh.h"
#include "surface.h"
#include "hittest.h"
#include <memory>

class CrasterizerView;
class CrasterizerDoc;

class viewtool
{
public:
	enum type { t_translate_scale,t_rotate,t_depth,
				t_cameratranslate,t_camerarotate,t_cameradepth };
	enum dragtype { td_null,
					td_frame_transform,
					td_camera_transform };
	enum lbuttonwaittype { wt_lbuttonup,wt_dragmove,wt_null };

	viewtool(CrasterizerView *pV);

	CrasterizerView *getview(void) const { return m_pView; }
	CrasterizerDoc *getdoc(void) const;

	virtual type gettype(void) const = 0;
	dragtype getindrag(void) const {return m_InDrag;}

	void setactive(const bool b);

	bool setcursor(void);

	bool mousewheel(const bool bUp,const int nIts,const af3d::vec2<int>& rtpt);
	void mousemove(const af3d::vec2<int>& rtpt);
	void lbuttondown(const af3d::vec2<int>& rtpt);
	void lbuttonup(const af3d::vec2<int>& rtpt);
	void lbuttondblclk(const af3d::vec2<int>& rtpt);

	void capturechanged( void );
	virtual bool getupdateselection(void)const=0;
	virtual void selectionchanged( void )=0;

	virtual void start(void){}
	virtual void stop(void);
	virtual void cancel(void);

	virtual bool render(void) const=0;
	virtual void render(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface)const=0;

	static lbuttonwaittype lbuttondownwait(CWnd *p,const af3d::vec2<int>& rtpt,const int nDragMoveThreshold);
	static bool clickpending( const lbuttonwaittype wt ) { return wt == wt_lbuttonup; }
	static bool dragpending( const lbuttonwaittype wt ) { return wt == wt_dragmove; }
	static bool getmousecaps_lButtondown( void );
	static bool beindragmovedelta( const af3d::vec2<int>& from, const af3d::vec2<int>& to, const int nDragMoveThreshold );
protected:
	virtual ~viewtool(){}
	
	// Data
	CrasterizerView *m_pView;
	dragtype m_InDrag;
	bool m_bCaptureInput;

	int m_nHandleRadius;
	af3d::vec4<> m_HandleCol;
	af3d::vec4<> m_LineCol;

	bool m_bSeenLButtonDownMove;
	bool m_bSeenLButtonDown;
	bool m_bSeenLButtonDownClick;
	af3d::vec2<int> m_ptLButtonDown;
	
	int m_nDragMoveThreshold;// lets not wait for more than m_nDragMoveThreshold pixels to start a drag EVEN if the system says we should i.e. my windows 7 is 4 pixels
									
	void getdocht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht ) const;
	virtual void getviewht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& ht ) const=0;

	void updateselection( const af3d::vec2<int>& rtpt, const lbuttonwaittype wt, const hittest<> *pDocht, const hittest<> *pViewht );

	virtual bool captureinput( void );
	virtual bool releasecapturedinput( void );

	void move(const af3d::vec2<int>& rtpt);

	void click(const af3d::vec2<int>& rtpt);
	void dblclick(const af3d::vec2<int>& rtpt);

	virtual bool getselectiondrag(void)const=0;
	virtual void begindrag( const af3d::vec2<int>& rtptStart, const af3d::vec2<int>& rtptMove )=0;
	virtual void movedrag( const af3d::vec2<int>& rtpt )=0;
	virtual void enddrag( const af3d::vec2<int>& rtpt )=0;
	void stop_ex(void);

	static lbuttonwaittype msgwait( const HWND hWnd, const af3d::vec2<int>& ptClientFrom, const int nTimeOut, const int nDragMoveThreshold )
	{
		const std::vector<int> vMsgs = { WM_LBUTTONUP, WM_MOUSEMOVE };

		MSG m;
		auto d=GetTickCount64();
		while( true )
		{
			if( GetTickCount64()-d >= nTimeOut )
				return wt_null;
			for( const auto &i : vMsgs )
			{
				// During this call, the system delivers pending, nonqueued messages, that is, messages sent to windows owned by the calling thread using the
				// SendMessage, SendMessageCallback, SendMessageTimeout, or SendNotifyMessage function. 
				if( !::PeekMessage( &m, hWnd, i, i, PM_NOYIELD|PM_NOREMOVE ) )
					continue;
				if( m.message == vMsgs[0] )
					return wt_lbuttonup;
				if( m.message == vMsgs[1] )
				{
					const int xPos = GET_X_LPARAM(m.lParam);
					const int yPos = GET_Y_LPARAM(m.lParam);
					if( !beindragmovedelta( ptClientFrom, af3d::vec2<int>( xPos, yPos ), nDragMoveThreshold ) )
						continue;
					return wt_dragmove;
				}
			}
		}
	}

	void synclights(const std::vector<af3d::vertexattsframe<>*>& v)const;

	void getselectionbbox(std::vector<af3d::vec3<>>& vWorldSpaceHandles)const;
	void renderbbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const int nHandleTypes)const;
	void renderbbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const af3d::facetrnsbbox<>& bbox,const int nHandleTypes)const;
	void renderbbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const std::vector<af3d::vec3<>>& vWorldSpaceHandles,const int nHandleTypes)const;
};
