
#pragma once

#ifndef __AFXWIN_H__
#endif

#include "resource.h"	

class CLANClientApp : public CWinApp
{
public:
	CLANClientApp();
public:
	virtual BOOL InitInstance();
	DECLARE_MESSAGE_MAP()
};

extern CLANClientApp theApp;
