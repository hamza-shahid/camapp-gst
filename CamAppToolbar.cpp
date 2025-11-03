#include "pch.h"
#include "CamAppToolbar.h"
#include "resource.h"

IMPLEMENT_DYNAMIC(CCamAppToolbar, CMFCToolBar)

BEGIN_MESSAGE_MAP(CCamAppToolbar, CMFCToolBar)
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

CCamAppToolbar::CCamAppToolbar()
    : m_bMouseCapture(FALSE)
{

}

BOOL CCamAppToolbar::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP)
    {
        CPoint point;
        GetCursorPos(&point);
        ScreenToClient(&point);

        int nHit = HitTest(point);

        switch (pMsg->message)
        {
        case WM_LBUTTONDOWN:
            if (nHit >= 0)
            {
                CMFCToolBarButton* pBtn = GetButton(nHit);
                if (pBtn && 
                    !(pBtn->m_nStyle & TBBS_DISABLED) &&
                    (pBtn->m_nID == ID_OPTIONS_PAN_LEFT ||
                     pBtn->m_nID == ID_OPTIONS_PAN_RIGHT ||
                     pBtn->m_nID == ID_OPTIONS_TILT_UP ||
                     pBtn->m_nID == ID_OPTIONS_TILT_DOWN ||
                     pBtn->m_nID == ID_OPTIONS_ZOOMIN ||
                     pBtn->m_nID == ID_OPTIONS_ZOOMOUT) )
                {
                    m_bMouseCapture = TRUE;
                    GetOwner()->SendMessage(WM_USER_TOOLBAR_BUTTONDOWN, pBtn->m_nID);
                }
            }
            break;

        case WM_LBUTTONUP:
            if (m_bMouseCapture)
            {
                m_bMouseCapture = FALSE;
                GetOwner()->SendMessage(WM_USER_TOOLBAR_BUTTONUP, 0);
            }
            break;
        }
    }

    return CMFCToolBar::PreTranslateMessage(pMsg);
}
