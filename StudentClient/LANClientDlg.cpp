#include "pch.h"
#include "framework.h"
#include "LANClient.h"
#include "LANClientDlg.h"
#include "afxdialogex.h"
#include<string>
#include<locale>
using namespace std;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString formatString(CString);

HWND CLANClientDlg::hwnd = NULL;

CLANClientDlg::CLANClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LANCLIENT_DIALOG, pParent)
	, m_name(_T(""))
	, isCon(false)
	, m_et_Msg(_T(""))
	, isSet(false)
	, inRoom(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLANClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IP_ADDR, m_ip);
	DDX_Text(pDX, IDC_ET_NAME, m_name);
	DDX_Control(pDX, IDC_LS_MEMBER, m_Member);
	DDX_Text(pDX, IDC_ET_SHOWMSG, m_et_Msg);
	DDX_Control(pDX, IDC_ET_SHOWMSG, m_showMSg);
	DDX_Control(pDX, IDC_PC_LIST, m_List);
}

unsigned __stdcall CLANClientDlg::RecvMsg(void* param)
{
	CTcpClient* cli = (CTcpClient*)param;
	while (1) {
		char* buf = new char[0xFF]{};
		int len=cli->Recv(buf,0xFF);
		if (len <= 0) {
			::PostMessageW(hwnd, UM_MODIUSER, 0, (LPARAM)buf); 
			break;
		}
		::PostMessageW(hwnd, UM_MODIUSER, 1, (LPARAM)buf);
	}
	return 0;
}

BEGIN_MESSAGE_MAP(CLANClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_CNT, &CLANClientDlg::OnBnClickedBtnCnt)
	ON_BN_CLICKED(IDC_BTN_SEND, &CLANClientDlg::OnBnClickedBtnSend)
	ON_MESSAGE(UM_MODIUSER, &CLANClientDlg::OnUmSelSevMsg)
	ON_BN_CLICKED(IDC_BTN_PSEND, &CLANClientDlg::OnBnClickedBtnPsend)
END_MESSAGE_MAP()

BOOL CLANClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	ShowWindow(SW_NORMAL);
	if (!CTcpClient::InitNet()) {
		AfxMessageBox(L"Failed to initialized web environment");
	}
	if (!m_client.Create()) {
		AfxMessageBox(L"Failed to create client");
	}
	m_ip.SetAddress(127, 0, 0, 1);
	hwnd = m_hWnd;
	SetDlgItemInt(IDC_ET_PORT,3000);
	setlocale(LC_ALL, "");
	return TRUE;
}

void CLANClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); 
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CLANClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLANClientDlg::OnBnClickedBtnCnt()
{
	if (isCon) {
		m_client.Close();
		isCon = false;
		AfxMessageBox(L"Disconnect Succefully");
		SetDlgItemText(IDC_BTN_CNT, _T("Connect"));
		for (int i = 0; i < m_List.GetCount(); i++)
		{
			m_List.DeleteString(i);
		}
		CRect rect;
		m_List.GetClientRect(&rect);
		m_List.SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height() + 1, SWP_NOMOVE);
		m_Member.DeleteAllItems();
		return;
	}
	if (!m_client.IsValid()) {  
		m_client.Create();
	}
	UpdateData();
	if (m_name.IsEmpty()) {
		AfxMessageBox(L"Please enter your name");
		return;
	}
	for (int i = 0; i < m_name.GetLength(); i++)
	{
		char letter = m_name.GetAt(i);
		if (_istspace(letter))
		{
			AfxMessageBox(L"There should not be space in the name");
			return;
		}
	}
	DWORD ip;
	m_ip.GetAddress(ip);
	int port = GetDlgItemInt(IDC_ET_PORT);
	if (!m_client.Connect(port,ip)) {
		AfxMessageBox(L"Failed to connect server");
		return;
	}
	for (int i = 0; i < m_Member.GetItemCount(); i++)
	{
		m_List.InsertString(0, m_Member.GetItemText(i, 0));
	}
	m_Member.InsertItem(0, m_name);
	_beginthreadex(0, 0, RecvMsg, &m_client, 0, 0);
	Sleep(300);
	SetDlgItemText(IDC_BTN_CNT, _T("Disconnect"));
	isCon = true;
	USES_CONVERSION;
	string name = W2A(m_name);
	name = "S:" + name;
	m_client.Send(name.data(), name.size());
}


void CLANClientDlg::OnBnClickedBtnSend()
{
	if (!m_client.IsValid()) {
		return;
	}
	CString msg;
	GetDlgItemText(IDC_ET_MSG, msg);
	if (msg.IsEmpty()) {
		return;
	}
	if (msg.GetLength() > 240)
	{
		AfxMessageBox(_T("The content to be sent cannot exceed 240 characters"));
		return;
	}
	UpdateData();
	USES_CONVERSION;
	string str = W2A(msg);
	int len=m_client.Send(str.data(), str.size());
	if (len == str.size()) {
		msg = formatString(msg);
		msg = _T("["+m_name+"]:") + msg;
		m_et_Msg.Append(msg);
		SetDlgItemText(IDC_ET_MSG, _T(""));
		UpdateData(false);
	}
	
	m_showMSg.LineScroll(m_showMSg.GetLineCount() - 10);
}


afx_msg LRESULT CLANClientDlg::OnUmSelSevMsg(WPARAM wParam, LPARAM lParam)
{
	//operations after member left room
	if (!wParam) {
		char* msg = (char*)lParam;
		delete[] msg;
		UpdateData();
		m_et_Msg.Append(_T("You are disconnected\r\n"));
		UpdateData(false);
		m_Member.DeleteAllItems();
		for (int i = 0; i < m_List.GetCount(); i++)
		{
			m_List.DeleteString(i);
		}
		CRect rect;
		m_List.GetClientRect(&rect);
		m_List.SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height() + 1, SWP_NOMOVE);
		return -1;
	}
	//member enter room
	char* msg = (char*)lParam;
	if (msg[0] == '1' && msg[1] == ':') {
		USES_CONVERSION;
		CString s = A2W(&msg[2]);
		UpdateData();
		m_et_Msg.Append(s + " entered the room" + L"\r\n");
		UpdateData(false);
		int index = s.Find(L':');
		s.GetBuffer()[index] = L'\0';
		m_List.InsertString(0, s);
		m_Member.InsertItem(0, s);
	}
	//member leave room
	else if (msg[0] == '2' && msg[1] == ':') {
		USES_CONVERSION;
		CString s = A2W(&msg[2]);
		UpdateData();
		m_et_Msg.Append(s + L"\r\n");
		int index = s.Find(L':');
		UpdateData(false);
		s.GetBuffer()[index] = L'\0';
		int nSpacePos = 0;
		CString strToken = s.Tokenize(_T(" "), nSpacePos);
		for (int i = 0; i < m_Member.GetItemCount(); i++) {
			if (m_Member.GetItemText(i, 0)==strToken) {
				m_Member.DeleteItem(i);
				m_List.DeleteString(i);
				break;
			}
		}
		UpdateData(false);
	}
	//message transfer
	else if (msg[0] == '3' && msg[1] == ':') {
		USES_CONVERSION;
		CString s = A2W(&msg[2]);
		int index = s.Find(':');
		CString userName = s.Mid(0, index);
		CString msg = s.Mid(index);
		msg = formatString(msg);
		UpdateData();
		m_et_Msg.Append(L"[" + userName + L"]" + msg);
		UpdateData(false);
	}
	//initialized the room
	else if (msg[0] == '4' && msg[1] == ':') {
		USES_CONVERSION;
		CString s = A2W(&msg[2]);
		int index = 0;
		for (int i = 0; i < s.GetLength(); i++) {
			if (s[i] == L':') {
				s.GetBuffer()[i] = L'\0';
				m_Member.InsertItem(0, &s.GetBuffer()[index]);
				m_List.InsertString(0, &s.GetBuffer()[index]);
				index = i + 1;
			}
		}
		
	}
	//private chat
	else if (msg[0] == '5' && msg[1] == ':') {
		USES_CONVERSION;
		CString s = A2W(&msg[2]);
		int index = s.Find(':');
		CString userName = s.Mid(0, index);
		CString msg = s.Mid(index);
		msg = userName + _T(" whispered to you ") + msg;
		msg = formatString(msg);
		m_et_Msg.Append(msg);
		UpdateData(false);
	}
	//initial breakout room
	else if (msg[0] == '6' && msg[1] == ':') {
		USES_CONVERSION;
		CString s = A2W(&msg[2]);
		int posNameList = s.Find(':');
		CString roomNum = s.Mid(0, posNameList);
		CString notification = _T("host has assigned you to room ") + roomNum; //room number
		AfxMessageBox(notification);
		UpdateData();
		//clean screen
		m_et_Msg = _T("");
		CString nameList = s.Mid(posNameList+1);
		//erase people not in the same room 
		while (nameList.GetLength()!=0) {
			CString name = nameList.Mid(0, nameList.Find(':'));
			nameList = nameList.Mid(nameList.Find(':')+1);
			for (int i = 0; i < m_Member.GetItemCount(); i++) {
				if (m_Member.GetItemText(i, 0) == name) {
					m_Member.DeleteItem(i);
					break;
				}
			}
		}
		inRoom = true;
		UpdateData(false);
	}
	//leave breakout room
	else if (msg[0] == '7' && msg[1] == ':') {
		USES_CONVERSION;
		CString s = A2W(&msg[2]);
		int index = s.Find(':');
		CString notification = s.Mid(0, index);
		CString nameList = s.Mid(index + 1);
		AfxMessageBox(notification);
		UpdateData();
		m_et_Msg = _T("");
		m_Member.DeleteAllItems();
		while (nameList.GetLength() != 0) {
			CString name = nameList.Mid(0, nameList.Find(' '));
			nameList = nameList.Mid(nameList.Find(' ') + 1);
			m_Member.InsertItem(0,name);
		}
		inRoom = false;
		UpdateData(false);
	}
	m_showMSg.LineScroll(m_showMSg.GetLineCount() - 10);
	delete[] msg;
	return 0;
}


void CLANClientDlg::OnBnClickedBtnPsend()
{
	if (!m_client.IsValid()) {
		SetDlgItemText(IDC_BTN_PSEND, _T("Internet Error"));
		Sleep(200);
		SetDlgItemText(IDC_BTN_PSEND, _T("send"));
		return;
	}
	CString msg;
	GetDlgItemText(IDC_ET_PMSG, msg);
	if (msg.IsEmpty()) {
		SetDlgItemText(IDC_BTN_SEND, _T("Empty Message"));
		Sleep(200);
		SetDlgItemText(IDC_BTN_SEND, _T("send"));
		return;
	}
	if (msg.GetLength() > 240)
	{
		AfxMessageBox(_T("The content to be sent cannot exceed 240 characters"));
		return;
	}
	CString userName;
	GetDlgItemText(IDC_PC_LIST, userName);
	if (userName.IsEmpty()) {
		return;
	}
	UpdateData();
	USES_CONVERSION;
	string str = W2A(msg);
	userName += " ";
	str = W2A(userName) + str;
	str = "5:" + str;
	int len = m_client.Send(str.data(), str.size());
	if (len == str.size()) {
		msg = _T("You whispered to ") + userName+ ": " + msg;
		msg = formatString(msg);
		m_et_Msg.Append(msg);
		SetDlgItemText(IDC_ET_PMSG, _T(""));
		UpdateData(false);
	}
	m_showMSg.LineScroll(m_showMSg.GetLineCount() - 10);
}
CString formatString(CString str)
{	
	CString newStr = _T("");
	int head = 0;
	while (head < str.GetLength())
	{
		CString sentence = str.Mid(head,55);
		sentence += _T("\r\n");
		newStr += sentence;
		head += 55;
	}
	return newStr;
}