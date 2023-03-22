#pragma once
#define UM_MODIUSER WM_USER+1 

#include"CTcpClient.h"
class CLANClientDlg : public CDialogEx
{
public:
	CLANClientDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LANCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	
private:
	
	static unsigned __stdcall RecvMsg(void *param);
	static HWND hwnd;
private: 
	CTcpClient m_client;
	bool isCon;
	bool isSet;
	bool roomSet;
protected:
	HICON m_hIcon;
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnCnt();
	// IP
	CIPAddressCtrl m_ip;
private:
	CString m_name;
	CListCtrl m_Member;
	// private chat list
	CComboBox m_List;
public:
	afx_msg void OnBnClickedBtnSend();
	CString m_et_Msg;
protected:
	afx_msg LRESULT OnUmSelSevMsg(WPARAM wParam, LPARAM lParam);
private:
	CEdit m_showMSg;
public:

	afx_msg void OnBnClickedBtnPsend();
	afx_msg void OnBnClickedBoutBtn();
private:
	CString num_room;
	CComboBox room_list;
public:
	afx_msg void OnBnClickedBtnRsend();
};
