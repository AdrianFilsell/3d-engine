
#include "pch.h"
#include "viewtool.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "3d_planebasis.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

viewtool::viewtool(CrasterizerView *pV)
{
	m_pView=pV;
	m_InDrag=td_null;
	m_bSeenLButtonDownMove=false;
	m_bSeenLButtonDown=false;
	m_bSeenLButtonDownClick=false;
	m_ptLButtonDown={0,0};
	m_bCaptureInput=true;
	m_nHandleRadius=6;
	m_HandleCol={0.9,0.5,0.25,1};
	m_LineCol={0,0.4,1,1};

	m_nDragMoveThreshold = 3; // lets not wait for more than 3 pixels to start a drag EVEN if the system says we should i.e. my windows 7 is 4 pixels
}

CrasterizerDoc *viewtool::getdoc(void) const
{
	return m_pView?static_cast<CrasterizerDoc*>(m_pView->GetDocument()):nullptr;
}

void viewtool::setactive( const bool b )
{
	if( b )
	{
		return;
	}
	
	if(m_InDrag != td_null)
		cancel();

	stop_ex();
}

void viewtool::stop( void )
{
	m_InDrag = td_null;

	stop_ex();
}

void viewtool::cancel( void )
{
	if( m_InDrag == td_null )
	{
		if(m_pView)
			m_pView->setselection(nullptr);
		return;
	}

	m_InDrag = td_null;
	
	stop_ex();
}

void viewtool::stop_ex(void)
{
	ASSERT( m_InDrag == td_null );

	m_bSeenLButtonDownMove=false;
	m_bSeenLButtonDown=false;
	m_bSeenLButtonDownClick=false;
	m_ptLButtonDown={0,0};

	if(m_bCaptureInput)
		releasecapturedinput();
}

bool viewtool::getmousecaps_lButtondown( void )
{
	SHORT s = ::GetKeyState( VK_LBUTTON );
	return s & 0x8000 ? true : false;
}

void viewtool::capturechanged( void )
{
	if( m_InDrag != td_null )
		cancel();
}

bool viewtool::captureinput( void )
{
	if( !m_pView )
		return false;
	return m_pView->captureinput();
}

bool viewtool::releasecapturedinput( void )
{
	if( !m_pView )
		return false;
	return m_pView->releasecapturedinput();
}

bool viewtool::mousewheel(const bool bUp,const int nIts,const af3d::vec2<int>& rtpt )
{
	return false;
}

void viewtool::lbuttondown(const af3d::vec2<int>& rtpt)
{
	ASSERT( m_InDrag == td_null );
	if( m_InDrag != td_null )
		return;
	if( !m_pView )
		return;

	m_bSeenLButtonDown = true;
	m_bSeenLButtonDownMove = false;
	m_bSeenLButtonDownClick = false;
	m_ptLButtonDown = rtpt;

	// ht
	std::shared_ptr<hittest<>> docht, viewht;
	getviewht( rtpt, viewht );
	if(!viewht || viewht->gettype()==hittest<>::t_null)
		getdocht( rtpt, docht );
	
	{
		const lbuttonwaittype wt = lbuttondownwait(m_pView,m_ptLButtonDown,m_nDragMoveThreshold);
		
		const bool bClickPending = clickpending( wt );
		const bool bDragPending = dragpending( wt );
		
		if( getupdateselection() || bClickPending )
			updateselection(m_ptLButtonDown,wt,docht.get(),viewht.get());

		if( bClickPending )
		{
			m_bSeenLButtonDownClick = true;
			click(rtpt);
		}
		else
		if( bDragPending )
		{
			move(rtpt);
		}
	}
}

void viewtool::lbuttonup(const af3d::vec2<int>& rtpt)
{
	if( m_InDrag != td_null )
	{
		const dragtype td = m_InDrag;
		enddrag(rtpt);	
	}
	else
	if( !m_bSeenLButtonDownMove && !m_bSeenLButtonDownClick )
	{
		// since lbutton down there has been no move
		click(rtpt);
	}
	m_bSeenLButtonDownMove = false;
	m_bSeenLButtonDownClick = false;

	setcursor();
}

void viewtool::lbuttondblclk(const af3d::vec2<int>& rtpt)
{
	dblclick( rtpt );
}

void viewtool::click(const af3d::vec2<int>& rtpt)
{
}

void viewtool::dblclick(const af3d::vec2<int>& rtpt)
{
}

void viewtool::mousemove(const af3d::vec2<int>& rtpt)
{
	move( rtpt );
}

void viewtool::move(const af3d::vec2<int>& rtpt)
{
	m_bSeenLButtonDownMove = true;

	if( m_InDrag == td_null && m_bSeenLButtonDown )
	{
		if(getmousecaps_lButtondown())
		{			
			// lbutton down sel change
			if( getselectiondrag() ? !!m_pView->getselection() : true )
			{
				if( beindragmovedelta( m_ptLButtonDown, rtpt, m_nDragMoveThreshold ) )
					begindrag( m_ptLButtonDown, rtpt );
				else
					return; // dont update the hit test or drag move
			}
		}
	}

	if( m_InDrag != td_null )
	{
		movedrag( rtpt );
		return;
	}
}

void viewtool::getdocht(const af3d::vec2<int>& rtpt,std::shared_ptr<hittest<>>& res) const
{
	res = nullptr;

	if(m_pView && m_pView->getsurface() && m_pView->GetDocument()->getscene())
	{
		af3d::mat4<> m,mProj;
		m_pView->getsurface()->getprojmat(mProj);
		af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m);
		
		hittest<> r;
		m_pView->getsurface()->ht(m_pView->GetDocument()->getscene(),rtpt,m.inverse(),m_pView->getrendertypes(),r);
		if(r.getvertexframe())
		{
			res =std::shared_ptr<hittest<>>(std::make_unique<hittest<>>(r));
			res->setrtpt(rtpt);
		}
		else
			res=nullptr;
	}
}

bool viewtool::setcursor(void)
{
	LPCTSTR lpszNew=NULL;
	if(m_InDrag == dragtype::td_null)
	{
		const af3d::vec2<int> rtpt=m_pView->getclientcursorpos();
		std::shared_ptr<hittest<>> docht, viewht;
		getviewht(rtpt, viewht);
		if(!viewht || viewht->gettype()==hittest<>::t_null)
			getdocht(rtpt, docht);
	
		lpszNew=viewht?viewht->getcursor():(docht?docht->getcursor():NULL);
	}
	if(!lpszNew)
		return false;

	HCURSOR h = LoadCursor(NULL,lpszNew);
	SetCursor(h);
	return true;
}

viewtool::lbuttonwaittype viewtool::lbuttondownwait(CWnd *p,const af3d::vec2<int>& rtpt,const int nDragMoveThreshold)
{
	if(!p)
		return wt_null;
	return msgwait(p->GetSafeHwnd(),rtpt,200,nDragMoveThreshold);
}

void viewtool::updateselection( const af3d::vec2<int>& rtpt, const lbuttonwaittype wt, const hittest<> *pDocht, const hittest<> *pViewht )
{
	if(pViewht && pViewht->gettype()!=hittest<>::t_null)
		return;

	if(pDocht && pDocht->gettype()==hittest<>::t_mesh && pDocht->getvertexframe())
	{
		// update
		if(pDocht->getvertexframe()!=m_pView->getselection())
			m_pView->setselection(pDocht->getvertexframe());
		return;
	}

	if(m_pView->getselection())
	{
		// clear
		m_pView->setselection(nullptr);
	}
}

void viewtool::getselectionbbox(std::vector<af3d::vec3<>>& vWorldSpaceHandles)const
{
	vWorldSpaceHandles.clear();
	vWorldSpaceHandles.reserve(8);
	
	if(!m_pView || !m_pView->getselection())
		return;

	m_pView->getselection()->validatecompositebbox();

	const af3d::facetrnsbbox<> bb(m_pView->getselection()->getbbox(true),m_pView->getselection()->getcompositetrns());
	for(int n=0;n<8;++n)
		vWorldSpaceHandles.push_back((bb)[n]);
}

void viewtool::renderbbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const af3d::facetrnsbbox<>& bbox,const int nHandleTypes)const
{
	std::vector<af3d::vec3<>> vWorldSpaceHandles={bbox[0],bbox[1],bbox[2],bbox[3],bbox[4],bbox[5],bbox[6],bbox[7]};
	renderbbox(pDst,rDevice,pSurface,vWorldSpaceHandles,nHandleTypes);
}

void viewtool::renderbbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const int nHandleTypes)const
{
	std::vector<af3d::vec3<>> vWorldSpaceHandles;
	getselectionbbox(vWorldSpaceHandles);
	if(vWorldSpaceHandles.size()!=8)
		return;

	renderbbox(pDst,rDevice,pSurface,vWorldSpaceHandles,nHandleTypes);
}

void viewtool::renderbbox(afdib::dib *pDst,const af3d::rect& rDevice,const surface *pSurface,const std::vector<af3d::vec3<>>& vWorldSpaceHandles,const int nHandleTypes)const
{
	af3d::mat4<> m,mProj;
	m_pView->getsurface()->getprojmat(mProj);
	af3d::mat4<>::mul(m_pView->getsurface()->getcamera()->gettrns(),mProj,m);

	for(int n=0;n<3;++n)
	{
		const af3d::line_pos_vertex_data<af3d::vec3<>> l0({vWorldSpaceHandles[n],vWorldSpaceHandles[n+1]});
		pSurface->renderline(pDst,l0,m_LineCol,m);
		const af3d::line_pos_vertex_data<af3d::vec3<>> l1({vWorldSpaceHandles[4+n],vWorldSpaceHandles[4+n+1]});
		pSurface->renderline(pDst,l1,m_LineCol,m);
	}
	const af3d::line_pos_vertex_data<af3d::vec3<>> l0({vWorldSpaceHandles[3],vWorldSpaceHandles[0]});
	pSurface->renderline(pDst,l0,m_LineCol,m);
	const af3d::line_pos_vertex_data<af3d::vec3<>> l1({vWorldSpaceHandles[4+3],vWorldSpaceHandles[4+0]});
	pSurface->renderline(pDst,l1,m_LineCol,m);

	for(int n=0;n<4;++n)
	{
		const af3d::line_pos_vertex_data<af3d::vec3<>> l({vWorldSpaceHandles[0+n],vWorldSpaceHandles[4+n]});
		pSurface->renderline(pDst,l,m_LineCol,m);
	}

	if(!m_pView->getselection() || (m_pView->getselection()->gettype()&nHandleTypes))
		for(int n=0;n<8;++n)
			pSurface->rendercircle(pDst,vWorldSpaceHandles[n],m_nHandleRadius,m_HandleCol,m);
}

void viewtool::synclights(const std::vector<af3d::vertexattsframe<>*>& v)const
{
	auto i=v.begin(),end=v.end();
	for(;i!=end;++i)
		if((*i) && (*i)->gettype()&af3d::vertexattsframe<>::t_light_mesh)
			dynamic_cast<af3d::lightmeshcache<>*>(*i)->synclight(*i);
}

bool viewtool::beindragmovedelta( const af3d::vec2<int>& from, const af3d::vec2<int>& to, const int nDragMoveThreshold )
{
	const std::pair<af3d::vec2<int>,bool> minmove = {{nDragMoveThreshold,nDragMoveThreshold},false};
		
	const int nDeltaX = ( from[0] - to[0] ) > 0 ? ( from[0] - to[0] ) : -( from[0] - to[0] );
	const int nDeltaY = ( from[1] - to[1] ) > 0 ? ( from[1] - to[1] ) : -( from[1] - to[1] );
	
	if( minmove.second )
	{
		if( nDeltaX < minmove.first[0] || nDeltaY < minmove.first[1] )
			return false;
		return true;
	}
	else
	{
		if( nDeltaX < minmove.first[0] && nDeltaY < minmove.first[1] )
			return false;
		return true;
	}
}
