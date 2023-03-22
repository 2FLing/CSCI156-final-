#pragma once
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#include<string>
using std::string;

class CTcpClient
{
private:
	SOCKET m_sock;
public:
	static bool InitNet() {
		WSADATA wsadata;
		return 0 == WSAStartup(MAKEWORD(2, 2), &wsadata);
	}
	static void ClearNet() {
		WSACleanup();
	}

public:
	CTcpClient() {
		m_sock = INVALID_SOCKET;
	}
	~CTcpClient()
	{
		if (m_sock != INVALID_SOCKET) {
			closesocket(m_sock);
		}
	}
	bool Create() {
		m_sock = socket(AF_INET,SOCK_STREAM,0);
		return m_sock != INVALID_SOCKET;
	}

	bool Connect(int port,int ip) {
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = htonl(ip);
		return -1!=connect(m_sock,(sockaddr*)&addr,sizeof(addr));
	}

	void Close() {
		if (m_sock != INVALID_SOCKET) {
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
		}
	}

	bool IsValid() {
		return m_sock != INVALID_SOCKET;
	}
	int Send(string msg, int len) {
		return send(m_sock, msg.c_str(), len, 0);
	}
	int Recv(char* buf, int size) {
		memset(buf, 0, 255);
		return recv(m_sock, buf, size, 0);
	}

};

