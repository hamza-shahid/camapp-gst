// OutputPane.cpp
#include "pch.h"
#include "OutputWnd.h"
#include "resource.h"
#include "Utils.h"


BEGIN_MESSAGE_MAP(COutputPane, CDockablePane)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_POPUP_CLEAR, &COutputPane::OnContextMenuClear)
    ON_COMMAND(ID_POPUP_COPY, &COutputPane::OnContextMenuCopy)
    ON_COMMAND(ID_POPUP_DELETE, &COutputPane::OnContextMenuDelete)
END_MESSAGE_MAP()

COutputPane::COutputPane() {}

COutputPane::~COutputPane() {}

BOOL COutputPane::PreCreateWindow(CREATESTRUCT& cs)
{
    // Add WS_EX_CLIENTEDGE to the extended style
    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    return CDockablePane::PreCreateWindow(cs);
}

int COutputPane::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDockablePane::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create the rich edit control
    if (!m_wndOutput.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        CRect(0, 0, 0, 0), this, IDC_LIST_BARCODE))
    {
        TRACE("Failed to create output window\n");
        return -1;
    }

    // Set extended styles (optional but recommended)
    m_wndOutput.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    int ret = m_wndOutput.InsertColumn(0, _T("Time"), LVCFMT_LEFT);
    ret = m_wndOutput.InsertColumn(1, _T("Barcode"), LVCFMT_LEFT);
    ret = m_wndOutput.InsertColumn(2, _T("Format"), LVCFMT_LEFT);

    return 0;
}

void COutputPane::OnSize(UINT nType, int cx, int cy)
{
    CDockablePane::OnSize(nType, cx, cy);

    if (m_wndOutput.GetSafeHwnd())
    {
        m_wndOutput.MoveWindow(0, 0, cx, cy);

        CRect rect;
        m_wndOutput.GetClientRect(&rect);

        int totalWidth = rect.Width();
        int columnCount = m_wndOutput.GetHeaderCtrl()->GetItemCount();

        // Example proportions: 30%, 50%, 20%
        int columnWidths[] = { totalWidth * 30 / 100, totalWidth * 30 / 100, totalWidth * 30 / 100 };

        for (int col = 0; col < columnCount; ++col)
        {
            m_wndOutput.SetColumnWidth(col, columnWidths[col]);
        }
    }
}

void COutputPane::OnContextMenu(CWnd* pWnd, CPoint point)
{
    CWnd* pListCtrl = GetDlgItem(IDC_LIST_BARCODE);
    if (pWnd == pListCtrl) // Ensure the context menu is for the list control
    {
        CPoint clientPoint = point;

        // Convert screen coordinates to client coordinates
        m_wndOutput.ScreenToClient(&clientPoint);

        // Determine which item was clicked
        LVHITTESTINFO hitTestInfo = { 0 };
        hitTestInfo.pt = clientPoint;
        m_nbarcodeItemIdx = m_wndOutput.HitTest(&hitTestInfo);

        if (m_nbarcodeItemIdx != -1) // Check if an item was clicked
        {
            m_barcodeText = m_wndOutput.GetItemText(m_nbarcodeItemIdx, 1); // Example: Get text from column 0

            // Load and display the context menu
            CMenu menu;
            if (menu.LoadMenu(IDR_BARCODE_OUTPUT_POPUP)) // Replace with your menu resource ID
            {
                CMenu* pSubMenu = menu.GetSubMenu(0);
                if (pSubMenu)
                {
                    pSubMenu->TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this);
                }
            }
        }
    }
}

void COutputPane::AppendOutput(const std::string& barcode, const std::string& format)
{
    int nRowIdx = m_wndOutput.InsertItem(0, CUtils::GetFormattedTime().c_str());
    m_wndOutput.SetItemText(nRowIdx, 1, barcode.c_str());
    m_wndOutput.SetItemText(nRowIdx, 2, format.c_str());
    
    for (int i = 0; i < m_wndOutput.GetItemCount(); ++i)
        m_wndOutput.SetItemState(i, 0, LVIS_SELECTED | LVIS_FOCUSED);


    // Highlight the added row
    m_wndOutput.SetItemState(nRowIdx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

    // Ensure the row is visible
    m_wndOutput.EnsureVisible(nRowIdx, FALSE);
}

void COutputPane::OnContextMenuClear()
{
    m_wndOutput.DeleteAllItems();
}

void COutputPane::OnContextMenuDelete()
{
    m_wndOutput.DeleteItem(m_nbarcodeItemIdx);
}

void COutputPane::OnContextMenuCopy()
{
    if (!OpenClipboard())
        return;

    EmptyClipboard();

    size_t textLength = (m_barcodeText.GetLength() + 1) * sizeof(TCHAR);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, textLength);
    if (hGlobal == nullptr)
    {
        CloseClipboard();
        return;
    }

    // Lock the global memory and copy the text
    void* pGlobal = GlobalLock(hGlobal);
    memcpy(pGlobal, m_barcodeText.GetBuffer(), textLength);
    GlobalUnlock(hGlobal);

    // Place the handle on the clipboard
#ifdef _UNICODE
    SetClipboardData(CF_UNICODETEXT, hGlobal);
#else
    SetClipboardData(CF_TEXT, hGlobal);
#endif

    CloseClipboard();
}
