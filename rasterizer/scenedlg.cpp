
#include "pch.h"
#include "scenedlg.h"
#include "hint.h"
#include "rasterizerView.h"
#include "rasterizerDoc.h"
#include "rasterizer.h"
#include "splitterwnd.h"

scenedlg::scenedlg(CWnd* pParent /*=nullptr*/)
	: propertiesdlg(IDD, pParent)
{
	m_pView=nullptr;
	m_pDoc=nullptr;
	m_bInOnUpdate=false;
	m_nSliderComboGap=0;
	m_nEffectCombo=0;
}

scenedlg::~scenedlg()
{
}

void scenedlg::DoDataExchange(CDataExchange* pDX)
{
	propertiesdlg::DoDataExchange(pDX);
	DDX_Control(pDX,IDC_SCENE_TREE,m_Tree);
	DDX_Control(pDX,IDC_OPACITY_SLIDER,m_OpacitySlider);
	DDX_CBIndex(pDX,IDC_EFFECT_COMBO,m_nEffectCombo);
}


BEGIN_MESSAGE_MAP(scenedlg,propertiesdlg)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
    ON_WM_HSCROLL()

	ON_NOTIFY(TVN_SELCHANGING,IDC_SCENE_TREE,&scenedlg::OnTreeSelChanging)
    ON_NOTIFY(TVN_SELCHANGED,IDC_SCENE_TREE,&scenedlg::OnTreeSelChanged)
	ON_NOTIFY(TVN_ITEMCHANGED,IDC_SCENE_TREE,&scenedlg::OnTreeItemChanged)
	ON_NOTIFY(TVN_ENDLABELEDIT,IDC_SCENE_TREE,&scenedlg::OnTreeItemEndLabelEdit)
	ON_MESSAGE(WM_SCENETREE_REPARENT,&scenedlg::OnTreeItemReparent)

	ON_CBN_SELCHANGE(IDC_EFFECT_COMBO,OnEffectComboSelChanged)
END_MESSAGE_MAP()


// scenedlg message handlers

BOOL scenedlg::OnInitDialog()
{
	propertiesdlg::OnInitDialog();

	createtitle(IDC_TITLE,IDC_SCENE_TITLE,_T("Scene"),false);

	m_OpacitySlider.SetRange(0,100);
	
	CRect rc[2];
	GetDlgItem(IDC_EFFECT_COMBO)->GetWindowRect(rc[0]);
	m_OpacitySlider.GetWindowRect(rc[1]);
	m_nSliderComboGap=rc[0].top-rc[1].bottom;

	m_bInitialised=true;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL scenedlg::OnEraseBkgnd(CDC *pDC)
{
	std::vector<CWnd*> vErase;
	vErase.push_back(&m_Title);
	vErase.push_back(&m_Tree);
	vErase.push_back(&m_OpacitySlider);
	vErase.push_back(GetDlgItem(IDC_OPACITY_STATIC));
	vErase.push_back(GetDlgItem(IDC_EFFECT_STATIC));
	vErase.push_back(GetDlgItem(IDC_EFFECT_COMBO));

	splitterwnd::excludecliprect(pDC,vErase);

	return propertiesdlg::OnEraseBkgnd(pDC);
}

void scenedlg::OnSize(UINT nType,int cx,int cy)
{
	propertiesdlg::OnSize(nType,cx,cy);

	CRect rcClient;
	GetClientRect(rcClient);

	std::vector<std::pair<CWnd*,CRect>> vRepos;
	
	CRect rcTitle;
	if(m_Title.GetSafeHwnd())
	{
		m_Title.GetWindowRect(rcTitle);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcTitle,2);
		rcTitle.right=rcClient.right-(rcTitle.left-rcClient.left);
		vRepos.push_back({&m_Title,rcTitle});
	}

	CRect rcCombo;
	if(GetDlgItem(IDC_EFFECT_COMBO))
	{
		CRect rcStatic;

		GetDlgItem(IDC_EFFECT_COMBO)->GetWindowRect(rcCombo);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcCombo,2);

		GetDlgItem(IDC_EFFECT_STATIC)->GetWindowRect(rcStatic);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcStatic,2);

		const int nDY=rcClient.bottom-rcCombo.bottom;
		rcCombo.OffsetRect(0,nDY);
		rcStatic.OffsetRect(0,nDY);
		if(rcCombo.top<rcTitle.bottom)
		{
			const int nDY=rcTitle.bottom-rcCombo.top;
			rcCombo.OffsetRect(0,nDY);
			rcStatic.OffsetRect(0,nDY);
		}
		
		rcStatic.OffsetRect(2-rcStatic.left,0);
		vRepos.push_back({GetDlgItem(IDC_EFFECT_STATIC),rcStatic});

		rcCombo.OffsetRect(rcStatic.right+2-rcCombo.left,0);
		rcCombo.right=rcClient.right;
		vRepos.push_back({GetDlgItem(IDC_EFFECT_COMBO),rcCombo});
	}

	CRect rcSlider;
	if(m_OpacitySlider.GetSafeHwnd())
	{
		CRect rcStatic;

		m_OpacitySlider.GetWindowRect(rcSlider);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcSlider,2);

		GetDlgItem(IDC_OPACITY_STATIC)->GetWindowRect(rcStatic);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rcStatic,2);

		const int nDY=(rcCombo.top - m_nSliderComboGap)-rcSlider.bottom;
		rcSlider.OffsetRect(0,nDY);
		rcStatic.OffsetRect(0,nDY);
		if(rcSlider.top<rcTitle.bottom)
		{
			const int nDY=rcTitle.bottom-rcSlider.top;
			rcSlider.OffsetRect(0,nDY);
			rcStatic.OffsetRect(0,nDY);
		}
		
		rcStatic.OffsetRect(2-rcStatic.left,0);
		vRepos.push_back({GetDlgItem(IDC_OPACITY_STATIC),rcStatic});

		rcSlider.OffsetRect(rcStatic.right+2-rcSlider.left,0);
		rcSlider.right=rcClient.right;
		vRepos.push_back({&m_OpacitySlider,rcSlider});
	}

	if(m_Tree.GetSafeHwnd())
	{
		CRect rc;

		m_Tree.GetWindowRect(rc);
		::MapWindowPoints(NULL,m_hWnd,(LPPOINT)&rc,2);
		
		int nTarget = rcSlider.top-5;
		rc.bottom=rc.top<=nTarget?nTarget:rc.top;
		
		nTarget = rcClient.right-2;
		rc.right=rc.left<=nTarget?nTarget:rc.left;
		vRepos.push_back({&m_Tree,rc});
	}

	splitterwnd::repos(vRepos);
}

void scenedlg::OnTreeSelChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
    // Get information about the selection change
    NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pNMHDR;

    // Get the current mouse position
    CPoint pt;
    GetCursorPos(&pt);
    m_Tree.ScreenToClient(&pt);

    // Perform a hit test to check where the click occurred
    UINT uFlags;
    HTREEITEM hItem = m_Tree.HitTest(pt, &uFlags);

    // If the click was on the checkbox, prevent the selection change
	if (hItem != NULL && (uFlags & TVHT_ONITEMSTATEICON))
    {
        *pResult = TRUE;  // Cancel the selection change
        return;
    }

    // Allow normal selection change for other clicks
    *pResult = FALSE;
}

void scenedlg::OnTreeSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	if(m_bInOnUpdate)
		return;

	NM_TREEVIEW *pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	if(!m_pView || !pNMTreeView || !pNMTreeView->itemNew.hItem)
		return;
	af3d::vertexattsframe<> *p=reinterpret_cast<af3d::vertexattsframe<>*>(m_Tree.GetItemData(pNMTreeView->itemNew.hItem));
	m_pView->setselection(p);
}

void scenedlg::OnTreeItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVITEMCHANGE *pItemChange = reinterpret_cast<NMTVITEMCHANGE*>(pNMHDR);
    
	if (pItemChange->uStateNew & TVIS_STATEIMAGEMASK)
	{
		const bool bOldSel = pItemChange->uStateOld & TVIS_SELECTED;
		const bool bNewSel = pItemChange->uStateNew & TVIS_SELECTED;

		const int nChecked = (pItemChange->uStateNew & TVIS_STATEIMAGEMASK) >> 12;
		HTREEITEM hItem = pItemChange->hItem;
		const CString cs=m_Tree.GetItemText(hItem);
		af3d::vertexattsframe<> *p=reinterpret_cast<af3d::vertexattsframe<>*>(m_Tree.GetItemData(hItem));
		if(!m_bInOnUpdate)
			if(nChecked == 2)
			{
				// checked
				if(m_pDoc && p)
					m_pDoc->setvisible(p,true);
			}
			else
			if(nChecked == 1)
			{
				// unchecked
				if(m_pView && p && m_pView->getselection()==p)
					m_pView->setselection(nullptr);
				if(m_pDoc && p)
					m_pDoc->setvisible(p,false);
			}
	}

	*pResult = 0;
}

LRESULT scenedlg::OnTreeItemReparent(WPARAM wParam,LPARAM lParam)
{
	const scenetree::reparent *pR=reinterpret_cast<const scenetree::reparent*>(lParam);
	if(pR)
	{
		af3d::vertexattsframe<> *pFrom=pR && pR->hFrom ? reinterpret_cast<af3d::vertexattsframe<>*>(m_Tree.GetItemData(pR->hFrom)):nullptr;
		af3d::vertexattsframe<> *pTo=pR && pR->hTo ? reinterpret_cast<af3d::vertexattsframe<>*>(m_Tree.GetItemData(pR->hTo)) : nullptr;
		switch(pR->t)
		{
			case scenetree::reparent::t_null:break;
			case scenetree::reparent::t_first_child:m_pDoc->reparent(pFrom,pTo,nullptr,false);break;
			case scenetree::reparent::t_sibling_above:m_pDoc->reparent(pFrom,pTo->getparent(),pTo,true);break;
			case scenetree::reparent::t_sibling_below:m_pDoc->reparent(pFrom,pTo->getparent(),pTo,false);break;
		}
	}

	return 0;
}

void scenedlg::OnTreeItemEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMTVDISPINFO* pTVDispInfo = reinterpret_cast<NMTVDISPINFO*>(pNMHDR);

	if(pTVDispInfo->item.pszText != nullptr)  // If pszText is not NULL, the user committed the edit
	{
		HTREEITEM hItem = pTVDispInfo->item.hItem;
		af3d::vertexattsframe<> *p=reinterpret_cast<af3d::vertexattsframe<>*>(m_Tree.GetItemData(hItem));
		const std::string s(CStringA(pTVDispInfo->item.pszText));
		if(m_pDoc && p && !(p->gettype()&af3d::vertexattsframe<>::t_scene) && s.size())
		{
			m_pDoc->setname(p,std::string(CStringA(pTVDispInfo->item.pszText)));
			*pResult = true;  // accept the new text
			return;
		}
	}
	*pResult = false;  // canceled
}

void scenedlg::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar *pScrollBar )
{
	propertiesdlg::OnHScroll( nSBCode, nPos, pScrollBar );

	if(pScrollBar->GetSafeHwnd()==m_OpacitySlider.GetSafeHwnd())
	{
		switch (nSBCode)
		{
			case TB_THUMBTRACK:
			case TB_ENDTRACK:
			case TB_THUMBPOSITION:

			case TB_LINEUP:
			case TB_LINEDOWN:
			case TB_PAGEUP:
			case TB_PAGEDOWN:
			{
				const RAS_FLTTYPE d=m_OpacitySlider.GetPos()/100.0;
				if(m_pView && m_pView->getselection())
				{
					af3d::vertexattsframe<> *p=m_pView->getselection();
					m_pDoc->setopacity(p,d);
				}
			}
			break;
		}
	}
}

void scenedlg::OnEffectComboSelChanged(void)
{
	oneffectcomboselchanged();
}

BOOL scenedlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_DELETE)
		{
			if(GetFocus() == &m_Tree)
			{
				HTREEITEM hItem = m_Tree.GetSelectedItem();
				if(hItem != NULL)
				{
					af3d::vertexattsframe<> *p=reinterpret_cast<af3d::vertexattsframe<>*>(m_Tree.GetItemData(hItem));
					if(p && m_pDoc)
						m_pDoc->erase(p);
				}
				return TRUE;
			}
		}
		else
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			if(GetFocus() == m_Tree.GetEditControl())
			{
				const bool bCancel=pMsg->wParam == VK_ESCAPE;
				m_Tree.SendMessage(TVM_ENDEDITLABELNOW,static_cast<WPARAM>(bCancel),0);
				return TRUE;
			}
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

void scenedlg::populate(af3d::vertexattsframe<> *p)
{
	if(!p)
		return;
	if(p && p->getchildren())
	{
		auto i=p->getchildren()->cbegin(),end=p->getchildren()->cend();
		for(;i!=end;++i)
			populate((*i).get());
	}
	auto i = m_Map.find(p);
	if(i!=m_Map.cend())
		return;
	appendframe(p);
}

void scenedlg::clear(void)
{
	m_pView=nullptr;
	m_pDoc=nullptr;
	m_Map.clear();
	m_Tree.DeleteAllItems();
}

void scenedlg::expandframe(af3d::vertexattsframe<> *p)
{
	HTREEITEM hItem=nullptr;
	if(p)
	{
		auto i = m_Map.find(p);
		if(i!=m_Map.cend())
			hItem=(*i).second;
	}
	if(hItem)
		m_Tree.Expand(hItem,TVE_EXPAND);
}

void scenedlg::setframecheck(af3d::vertexattsframe<> *p)
{
	auto i=m_Map.find(p);
	if(i==m_Map.cend())
		return;
	if(m_Tree.GetSelectedItem()!=(*i).second)
		return;
	if(!!m_Tree.GetCheck((*i).second)==p->getvisible())
		return;
	m_Tree.SetCheck((*i).second,p->getvisible());
}

void scenedlg::setframename(af3d::vertexattsframe<> *p)
{
	auto i=m_Map.find(p);
	if(i==m_Map.cend())
		return;
	if(m_Tree.GetSelectedItem()!=(*i).second)
		return;
	const CStringA csA(m_Tree.GetItemText((*i).second));
	if(std::string(csA)==p->getname())
		return;
	m_Tree.SetItemText((*i).second,CString(csA));
}

void scenedlg::setframeopacity(af3d::vertexattsframe<> *p)
{
	if(!p)
	{
		const int nPos=0;
		if(m_OpacitySlider.GetPos()!=nPos)
			m_OpacitySlider.SetPos(nPos);
		GetDlgItem(IDC_OPACITY_STATIC)->EnableWindow(false);
		m_OpacitySlider.EnableWindow(false);
		return;
	}
	GetDlgItem(IDC_OPACITY_STATIC)->EnableWindow(true);
	m_OpacitySlider.EnableWindow(true);
	const int nPos=af::posround<RAS_FLTTYPE,int>(p->getopacity()*100.0);
	if(m_OpacitySlider.GetPos()!=nPos)
		m_OpacitySlider.SetPos(nPos);
}

void scenedlg::setframeeffect(af3d::vertexattsframe<> *p)
{
	if(!p)
	{
		GetDlgItem(IDC_EFFECT_STATIC)->EnableWindow(false);
		GetDlgItem(IDC_EFFECT_COMBO)->EnableWindow(false);
		if(m_nEffectCombo!=0)
		{
			m_nEffectCombo=0;
			UpdateData(false);
		}
		return;
	}
	GetDlgItem(IDC_EFFECT_STATIC)->EnableWindow(true);
	GetDlgItem(IDC_EFFECT_COMBO)->EnableWindow(true);
	const af3d::vertexattsframe<>::effecttype t=p->geteffect();
	int n=0;
	switch(t)
	{
		case af3d::vertexattsframe<>::et_null:break;
		case af3d::vertexattsframe<>::et_silhouette:n=1;break;
	}
	if(n==m_nEffectCombo)
		return;
	m_nEffectCombo=n;
	UpdateData(false);
}

void scenedlg::selectframe(af3d::vertexattsframe<> *p)
{
	HTREEITEM hItem=nullptr;
	if(p)
	{
		auto i = m_Map.find(p);
		if(i!=m_Map.cend())
			hItem=(*i).second;
	}
	if(m_Tree.GetSelectedItem()!=hItem)
	{
		m_Tree.Select(hItem,TVGN_CARET);
		std::function<bool(HTREEITEM h)> ensureexpanded=[&](HTREEITEM h) -> bool
		{
			if(!h)return false;
			bool bRetVal=ensureexpanded(m_Tree.GetParentItem(h));
			
			const UINT uiState = m_Tree.GetItemState(hItem, TVIS_EXPANDED);		
			const bool bExpanded = (uiState & TVIS_EXPANDED) != 0;
			if(!bExpanded) { m_Tree.Expand(hItem,TVE_EXPAND); bRetVal=true; }
			return bRetVal;
		};
		const bool bExpand=false;
		if(bExpand && ensureexpanded(m_Tree.GetParentItem(hItem)))
			m_Tree.EnsureVisible(hItem);
	}
}

HTREEITEM scenedlg::insertframe(af3d::vertexattsframe<> *p)
{
	int n;
	if(!p || !p->getparent() || !p->getindex(n))
		return nullptr;
	
	af3d::vertexattsframe<> *pParent=p->getparent();
	auto i = m_Map.find(pParent);
	if(i==m_Map.cend())
		return nullptr;
	
	m_Map.insert({p,m_Tree.insert((*i).second,p,n)});

	if(p->getchildren())
	{
		auto i=p->getchildren()->cbegin(),end=p->getchildren()->cend();
		for(;i!=end;++i)
			appendframe((*i).get());
	}

	return (*m_Map.find(p)).second;
}

HTREEITEM scenedlg::appendframe(af3d::vertexattsframe<> *p)
{
	if(!p)
		return nullptr;

	HTREEITEM hParent=nullptr;
	af3d::vertexattsframe<> *pParent=p->getparent();
	auto i = m_Map.find(pParent);
	if(pParent)
	{
		auto i = m_Map.find(pParent);
		hParent=(i==m_Map.cend()) ? appendframe(pParent) : (*i).second;
	}
	else
		hParent=TVI_ROOT;

	m_Map.insert({p,m_Tree.append(hParent,p)});
	
	return (*m_Map.find(p)).second;
}

void scenedlg::eraseframe(af3d::vertexattsframe<> *p)
{
	auto i = m_Map.find(p);
	if(i==m_Map.cend())
		return;
	m_Tree.DeleteItem((*i).second);
	m_Map.erase(i);

	std::vector<af3d::vertexattsframe<>*> vFrames;
	af3d::vertexattsframe<>::get(p,af3d::vertexattsframe<>::t_all,vFrames);
	vFrames.erase(vFrames.cbegin());
	
	auto j=vFrames.cbegin(),end=vFrames.cend();
	for(;j!=end;++j)
	{
		i = m_Map.find(p);
		if(i==m_Map.cend())
			continue;
		m_Map.erase(i);
	}
}

void scenedlg::oneffectcomboselchanged(void)
{
	UpdateData();

	af3d::vertexattsframe<>::effecttype t=af3d::vertexattsframe<>::et_null;
	switch(m_nEffectCombo)
	{
		case 0:break;
		case 1:t=af3d::vertexattsframe<>::et_silhouette;break;
	}
	
	const bool bSel=m_pView && m_pView->getselection();
	if(!bSel)
		return;

	if(m_pDoc)
		m_pDoc->seteffect(m_pView->getselection(),t);
}

int scenedlg::getmin(void)const
{
	if(!GetSafeHwnd())
		return -1; // unbound
	CRect rc;
	GetDlgItem(IDC_TITLE)->GetWindowRect(rc);
	ScreenToClient(rc);
	return rc.bottom + m_nMinMaxGap;
}

int scenedlg::getmax(void)const
{
	return -1; // unbound
}

void scenedlg::onupdate(hint *p)
{
	propertiesdlg::onupdate(p);

	m_bInOnUpdate=true;
	if(p)
		switch(p->gettype())
		{
			case hint::t_initial_update:
			case hint::t_view_active:
			{
				if(theApp.getinitialised() && (p->getdoc()!=m_pDoc || p->getview()!=m_pView))
				{
					clear();
					m_pView=p->getview();
					m_pDoc=m_pView?static_cast<CrasterizerDoc*>(m_pView->GetDocument()):nullptr;
					populate(m_pDoc?m_pDoc->getscene():nullptr);
					expandframe(m_pDoc?m_pDoc->getscene():nullptr);
					selectframe(m_pView?m_pView->getselection():nullptr);
					setframeopacity(m_pView?m_pView->getselection():nullptr);
					setframeeffect(m_pView?m_pView->getselection():nullptr);
					m_Tree.Invalidate();
				}
			}
			break;
			case hint::t_view_stop:
			{
				if(p->getdoc()==m_pDoc)
					clear();
			}
			break;
			case hint::t_selection:if(m_pView){selectframe(p->getframe());setframeopacity(p->getframe());setframeeffect(p->getframe());}break;
			case hint::t_frame_append:if(m_pView)appendframe(p->getframe());m_Tree.Invalidate();break;
			case hint::t_frame_erase:if(m_pView)eraseframe(p->getframe());m_Tree.Invalidate();break;
			case hint::t_frame_visible:
			{
				if(p->getdoc()==m_pDoc)
					setframecheck(p->getframe());
			}
			break;
			case hint::t_frame_name:
			{
				if(p->getdoc()==m_pDoc)
					setframename(p->getframe());
			}
			break;
			case hint::t_frame_opacity:
			{
				if(p->getdoc()==m_pDoc)
					setframeopacity(p->getframe());
			}
			break;
			case hint::t_frame_effect:
			{
				if(p->getdoc()==m_pDoc)
					setframeeffect(p->getframe());
			}
			break;
			case hint::t_frame_reparent:
			{
				auto i=m_Map.find(p->getframe());
				if(i!=m_Map.cend())
				{
					const bool bSelect=(*i).second == m_Tree.GetSelectedItem();
					std::vector<af3d::vertexattsframe<>*> vExpanded;
					std::function<void(HTREEITEM h)> expanded=[&](HTREEITEM h) -> void
					{
						if(m_Tree.GetItemState(h, TVIS_EXPANDED)&TVIS_EXPANDED)
						{
							vExpanded.push_back(reinterpret_cast<af3d::vertexattsframe<>*>(m_Tree.GetItemData(h)));
							HTREEITEM hC=m_Tree.GetChildItem(h);
							while(hC) { expanded(hC); hC=m_Tree.GetNextSiblingItem(hC); }
						}
					};
					expanded((*i).second);

					eraseframe(p->getframe());
					insertframe(p->getframe());

					auto i=vExpanded.cbegin(),end=vExpanded.cend();
					for(;i!=end;++i)
					{
						auto j=m_Map.find(*i);
						if(j!=m_Map.cend())
							m_Tree.Expand((*j).second,TVE_EXPAND);
					}
					if(bSelect)
						m_Tree.SelectItem((*m_Map.find(p->getframe())).second);
					m_Tree.EnsureVisible((*m_Map.find(p->getframe())).second);
					m_Tree.Invalidate();
				}
			}
			break;
		}
	m_bInOnUpdate=false;
}
