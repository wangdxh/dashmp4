// dashmp4.h : main header file for the DASHMP4 application
//

#if !defined(AFX_DASHMP4_H__6C94CAE6_70C5_47D2_A881_FCE683D447A1__INCLUDED_)
#define AFX_DASHMP4_H__6C94CAE6_70C5_47D2_A881_FCE683D447A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDashmp4App:
// See dashmp4.cpp for the implementation of this class
//

class CDashmp4App : public CWinApp
{
public:
	CDashmp4App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDashmp4App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDashmp4App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DASHMP4_H__6C94CAE6_70C5_47D2_A881_FCE683D447A1__INCLUDED_)
