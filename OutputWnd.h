// OutputPane.h
#pragma once
#include <afxdockablepane.h> // For CDockablePane
#include <string>

class COutputPane : public CDockablePane
{
public:
    COutputPane();
    virtual ~COutputPane();

    void AppendOutput(const std::string& barcode, const std::string& format);

protected:
    CListCtrl   m_wndOutput;
    CString     m_barcodeText;
    int         m_nbarcodeItemIdx;

    DECLARE_MESSAGE_MAP()
    afx_msg BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnContextMenuClear();
    afx_msg void OnContextMenuDelete();
    afx_msg void OnContextMenuCopy();
};
