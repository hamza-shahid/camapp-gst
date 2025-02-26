
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include "framework.h"
#include "camapp-gst.h"
#include "ChildView.h"

#include <gst/video/videooverlay.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
	: m_pGstPlayer(NULL)
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	//ON_WM_LBUTTONUP()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	
	// Do not call CWnd::OnPaint() for painting messages
}

void CChildView::SetGstPlayer(CGstPlayer* pGstPlayer)
{
	m_pGstPlayer = pGstPlayer;
}

void CChildView::Resize(int iWidth, int iHeight)
{
	CRect desiredClientRect(0, 0, iWidth, iHeight);

	// Get window styles and extended styles
	DWORD dwStyle = GetStyle();
	DWORD dwExStyle = GetExStyle();

	// Adjust the window size to account for borders, title bar, etc.
	::AdjustWindowRectEx(&desiredClientRect, dwStyle, FALSE, dwExStyle);  // TRUE if you have a menu

	// Resize the frame window to include these non-client areas
	SetWindowPos(nullptr, 0, 0, desiredClientRect.Width(), desiredClientRect.Height(), SWP_NOMOVE | SWP_NOZORDER);
}

void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	
	if (m_pGstPlayer)
		m_pGstPlayer->Redraw();
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	CWnd* pMainFrame = GetParent();
	if (pMainFrame)
	{
		pMainFrame->PostMessage(WM_ON_CHILD_LBUTTONUP, (WPARAM)nFlags, MAKELPARAM(point.x, point.y));
	}
}
