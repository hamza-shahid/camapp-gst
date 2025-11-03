#pragma once
#include <afxtoolbar.h>


#define WM_USER_TOOLBAR_BUTTONDOWN  (WM_USER + 40)
#define WM_USER_TOOLBAR_BUTTONUP    (WM_USER + 41)


class CCamAppToolbar : public CMFCToolBar
{
    DECLARE_DYNAMIC(CCamAppToolbar)

public:
    CCamAppToolbar();

protected:
    BOOL m_bMouseCapture;

protected:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    DECLARE_MESSAGE_MAP()
};

