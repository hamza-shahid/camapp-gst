// CameraControlsPage.cpp : implementation file
//

#include "pch.h"
#include "camapp-gst.h"
#include "CameraControlsPage.h"
#include "afxdialogex.h"

// CCameraControlsPage dialog

IMPLEMENT_DYNAMIC(CCameraControlsPage, CMFCPropertyPage)

CCameraControlsPage::CCameraControlsPage()
	: CMFCPropertyPage(IDD_PROPPAGE_CAMERA_CONTROLS)
{

}

CCameraControlsPage::~CCameraControlsPage()
{
}

void CCameraControlsPage::DoDataExchange(CDataExchange* pDX)
{
	CMFCPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCameraControlsPage, CMFCPropertyPage)
END_MESSAGE_MAP()


// CCameraControlsPage message handlers
