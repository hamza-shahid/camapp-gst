#pragma once


// CCameraControlsPage dialog

class CCameraControlsPage : public CMFCPropertyPage
{
	DECLARE_DYNAMIC(CCameraControlsPage)

public:
	CCameraControlsPage();   // standard constructor
	virtual ~CCameraControlsPage();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROPPAGE_CAMERA_CONTROLS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
