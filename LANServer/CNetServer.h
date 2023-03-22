#pragma once
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include<string>
using std::string;
class CTcpOfClient
{
private:
	string m_ip;
	SOCKET m_sock;
public:
	CTcpOfClient() {
		m_sock = INVALID_SOCKET;
	}
	~CTcpOfClient()
	{
		if (m_sock != INVALID_SOCKET) {
			closesocket(m_sock);
		}
	}
	void SetSocket(string ip, SOCKET sock) {
		m_sock = sock;
		m_ip = ip;
	}
	string GetIp() {
		return m_ip;
	}
	int Send(string msg, int len) {
		return send(m_sock, msg.c_str(), len, 0);
	}
	int Recv(char* buf, int size) {
		memset(buf, 0, 255);
		return recv(m_sock, buf, size, 0);
	}

};


class CTcpListen {
private:
	SOCKET m_sock;
public:
	CTcpListen() {
		m_sock = INVALID_SOCKET;
	}
	~CTcpListen()
	{
		if (m_sock != INVALID_SOCKET) {
			closesocket(m_sock);
		}
	}

public:
	bool Create() {
		m_sock = socket(AF_INET, SOCK_STREAM, 0);
		return m_sock != INVALID_SOCKET;
	}
	bool Bind(int port, string ip = "0.0.0.0") {
		SOCKADDR_IN addrSev;
		addrSev.sin_family = AF_INET;
		addrSev.sin_port = htons(port);
		addrSev.sin_addr.S_un.S_addr = inet_addr(ip.data());
		return -1 != bind(m_sock, (sockaddr*)&addrSev, sizeof(addrSev));
	}

	bool Listen() {
		return -1 != listen(m_sock, 5);
	}
	bool Accept(CTcpOfClient& cli) {
		SOCKADDR_IN addrCli;
		int len = sizeof(addrCli);
		SOCKET sockCli = accept(m_sock, (sockaddr*)&addrCli, &len);
		string ip = inet_ntoa(addrCli.sin_addr); 
		cli.SetSocket(ip, sockCli); 
		return sockCli != INVALID_SOCKET;
	}
	static bool InitNet() {
		WSADATA wsadata;
		return 0 == WSAStartup(MAKEWORD(2, 2), &wsadata);
	}
	static void ClearNet() {
		WSACleanup();
	}

};
