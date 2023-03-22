#define  _WINSOCK_DEPRECATED_NO_WARNINGS
#include<iostream>
#include<map>
#include"CNetServer.h"
#include"Person.h"
#include<string>
using namespace std;
map<CTcpOfClient*, Person> m_clients; 
DWORD  WINAPI RecvMSG(LPVOID param); 
int main() {
	if (!CTcpListen::InitNet()) {
		cout << "Failed to intialized the web environment";
		return -1;
	}
	CTcpListen sockServer; 
	if (!sockServer.Create()) {
		cout << "Failed to create server";
		return -1;
	}
	if (!sockServer.Bind(3000)) {
		cout << "Failed to bind port";
		return -1;
	}
	if (!sockServer.Listen()) {
		cout << "Failed to listen on port";
		return -1;
	}
	cout << "Server is running" << endl;

	while (true) {
		CTcpOfClient *cli=new CTcpOfClient; 
		if (!sockServer.Accept(*cli)) { 
			cout <<cli->GetIp() << "" << endl;
			continue;
		}
		char username[20]{};
		int len=cli->Recv(username,20);
		if (len <= 0) {
			cout << cli->GetIp() << "Failed to establish connection" << endl;
			delete cli;
			continue;
		}
		//convert username to string
		string name = username;
		int index = name.find(':');
		string userRole = name.substr(0,index);
		string userName = name.substr(index+1);
		for (auto client : m_clients) {
			if (client.second.name == userName)
			{
				bool exist = true;
				int num = 1;
				string originalName = userName;
				while (exist)
				{
					userName = originalName + to_string(num);
					for (auto client : m_clients)
					{
						if (client.second.name == userName)
						{
							exist = true;
							num += 1;
							break;
						}
						exist = false;
					}
				}
			}
		}
		Person user;
		if (userRole == "I")
		{
			user.role = "instructor";
		}
		else
		{
			user.role = "student";
		}
		user.name = userName;
		m_clients.insert(pair<CTcpOfClient*, Person>(cli, user));
		//create thread to receive the client information from the new member
		CreateThread(0, 0, RecvMSG, cli, 0, 0);
	}
	CTcpListen::ClearNet();

}

DWORD  WINAPI RecvMSG(LPVOID param)
{
	CTcpOfClient* cli = (CTcpOfClient*)param; 
	for (auto& i : m_clients) {
		if (i.first == cli) continue; 
		if (i.second.room == 0) {
			string tm = "1:";
			string name = m_clients[cli].name;
			tm += name;
			i.first->Send(tm.data(), tm.size());
		}
	}
	string tn = "4:"; 
	for (auto& i : m_clients) { 
		if (i.first == cli) continue;
		string name = i.second.name;
		tn += (name+":");
	}

	cli->Send(tn.data(), tn.size()); 
	char msg[0xFF]{}; 
	while (1) {
		int len = cli->Recv(msg, 0xFF);
		if (len > 0) {
			if (msg[0] == '5' && msg[1] == ':'){
				string s = &msg[2];
				int index = s.find(' ');
				string receiver = s.substr(0,index);
				string msg = s.substr(index);
				Person sender = m_clients[cli];
				for (auto& i : m_clients) {
					if (i.second.name == receiver){
						string tm;
						if (sender.room!=0 && i.second.role == "instructor") 
							tm = "5:" + sender.name + " from room "+to_string(sender.room) + ':';
						else 
							tm = "5:" + sender.name + ':';
						tm += msg;
						i.first->Send(tm.data(), tm.size());
					}
				}
			}
			// notify students and arrange students to break out room
			else if (msg[0] == '6' && msg[1] == ':') {
				string numRoom = &msg[2];
				int numStudents = 0;
				for (auto& i : m_clients) {
					if (i.second.role == "student") {
						numStudents++;
					}
				}
				// how many students in a room
				int numInARoom = numStudents / stoi(numRoom);
				numInARoom = numInARoom == 0 ? 1 : numInARoom;
				int roomNum = 1;
				int count = 0;
				for (auto& i : m_clients) {
					if (i.second.role != "student")continue;
					i.second.room = roomNum;
					count++;
					// if the room is full and not the last room
					if (count == numInARoom && roomNum != stoi(numRoom)) {
						count = 0;
						roomNum++;
					}
				}
				roomNum = roomNum > numStudents ? numStudents : roomNum;
				string peopleNotInSameRoom="";
				for (auto& i : m_clients) {
					if (i.second.role != "student") {
						string num = to_string(roomNum);
						num = "6:" + num;
						i.first->Send(num.data(), num.size());
						continue;
					}
					for (auto& client : m_clients) {
						if (client.second.name ==i.second.name || client.second.role != "student")continue;
						string name = client.second.name;
						if (client.second.room!=i.second.room) {
							peopleNotInSameRoom += name + ":";
						}
					}
					string tm = "6:" + to_string(i.second.room)+":"+peopleNotInSameRoom;
					i.first->Send(tm.data(), tm.size());
					peopleNotInSameRoom = "";
				}

			}
			//close breakout room
			else if (msg[0] == '7' && msg[1] == ':') {
				string s = &msg[2],nameList = "";
				for (auto& i : m_clients) {
					if (i.second.role == "instructor") {
						string tm = "7:";
						i.first->Send(tm.data(), tm.size());
					}
					if (i.second.room != 0) {
						i.second.room = 0;
					}
					nameList += i.second.name + " ";
				}
				string tm = "7:" + s + ":" + nameList;
				for (auto& i : m_clients) {
					i.first->Send(tm.data(), tm.size());
				}
			}
			//instructor send message to the room
			else if (msg[0] == '8' && msg[1] == ':') {
				Person sender = m_clients[cli];
				string s = &msg[2];
				int index = s.find(' ');
				int roomNum = stoi(s.substr(0, index));
				string message = s.substr(index);
				string tm = "3:" + sender.name + ":" + message;
				for (auto& i : m_clients) {
					if (i.second.room == roomNum) {
						i.first->Send(tm.data(), tm.size());
					}
				}
			}
			else
			{
				string tm = "3:" + m_clients[cli].name + ':';
				tm += msg;
				for (auto& i : m_clients) {
					if (i.first == cli) continue;
					if (m_clients[cli].room == 0) {		
						i.first->Send(tm.data(), tm.size());
					}
					else if (i.second.room == m_clients[cli].room) {
						i.first->Send(tm.data(), tm.size());
						}
					
				}
			}
			continue;
		}
		for (auto& i : m_clients) {
			if (i.first == cli) continue;
			if (m_clients[cli].room != 0) {
				if (i.second.room == 0 || i.second.room == m_clients[cli].room) {
					string exitMsg = "2:";
					exitMsg += (m_clients[cli].name + " left the room");
					i.first->Send(exitMsg.data(), exitMsg.size());
				}
			}
			else {
				if (i.second.room == 0) {
					string exitMsg = "2:";
					exitMsg += (m_clients[cli].name + " left the room");
					i.first->Send(exitMsg.data(), exitMsg.size());
				}
			}
		}
		m_clients.erase(cli);
		delete cli; 
		break; 
	}

	return 0;
}
