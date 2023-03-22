#pragma once
#include "afxdialogex.h"


// RoomNum dialog

class RoomNum : public CDialogEx
{
	DECLARE_DYNAMIC(RoomNum)

public:
	RoomNum(CWnd* pParent = nullptr);   // standard constructor
	virtual ~RoomNum();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INPUT_NUM_ROOM };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString num_room;
};
