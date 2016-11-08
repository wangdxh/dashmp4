// dashmp4Dlg.h : header file
//

#if !defined(AFX_DASHMP4DLG_H__53701EEB_8592_4E12_B621_20C4C58A4F51__INCLUDED_)
#define AFX_DASHMP4DLG_H__53701EEB_8592_4E12_B621_20C4C58A4F51__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CDashmp4Dlg dialog

class CDashmp4Dlg : public CDialog
{
// Construction
public:
	CDashmp4Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CDashmp4Dlg)
	enum { IDD = IDD_DASHMP4_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDashmp4Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CDashmp4Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DASHMP4DLG_H__53701EEB_8592_4E12_B621_20C4C58A4F51__INCLUDED_)
