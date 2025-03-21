
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "GstPlayer.h"

#define WM_ON_CHILD_LBUTTONUP (WM_USER + 31)

// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:
	void SetGstPlayer(CGstPlayer* pGstPlayer);
	void Resize(int iWidth, int iHeight);

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()

protected:
	CGstPlayer* m_pGstPlayer;
};

