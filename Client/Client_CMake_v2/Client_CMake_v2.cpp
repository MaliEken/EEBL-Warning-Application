#include <iostream>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;


void WinStartUp() {
	WSADATA data;
	// Winsocket version 0x0202
	WORD version = MAKEWORD(2, 2);

	// Start WinSock
	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		cout << "Can't start Winsock! " << wsOk;
		return;
	}

}
//Send UDP message for Emergency Break Event(EBE)
void UDPSendEBE(string message) {
	// WinSocket setup
	
	WinStartUp();
	// Server Parameters and Connection
	sockaddr_in server;
	server.sin_family = AF_INET; // AF_INET = IPv4 addresses
	server.sin_port = htons(54000); // Little to big endian conversion
	inet_pton(AF_INET, "192.168.1.255", &server.sin_addr); // Convert from string to byte array

	// Socket creation
	SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

	cout << " Parameters are broadcasted to other vehicles!!"<<endl;
	// Write out to the socket
	int sendOk = sendto(out, message.c_str(), sizeof(message) + 1, 0, (sockaddr*)&server, sizeof(server));
	if (sendOk == SOCKET_ERROR)
	{
		cout << "That didn't work! " << WSAGetLastError() << endl;
	}

	// Close the socket
	closesocket(out);
	// Close down Winsock
	WSACleanup();
}

void main(int argc, char* argv[]) 
{	
	uint32_t ego_speed;
	uint32_t ego_deceleration;
	uint32_t ego_pedalpos;
	uint32_t ego_globalpos_x;
	uint32_t ego_globalpos_y;
	string message;
	
	//The Algorithm runs for sending emergency break event
	while (true) {
		cout << " Enter the inputs to check if emergency break event happens." << endl;
		cout << " Speed - Range: 0-90 (km/h) = "; cin >> ego_speed; cout << endl;
		cout << " Deceleration - Range: 0-10 (m/s^2) = "; cin >> ego_deceleration; cout << endl;
		cout << " Break Pedal Position - Range: 0-100 (%) = "; cin >> ego_pedalpos; cout << endl;
		cout << " Global Position - Range: 0-10000 (m): " << endl;
		cout << " X coordinate = "; cin >> ego_globalpos_x;
		cout << " Y coordinate = "; cin >> ego_globalpos_y; 
		cout << endl;
		

		//Not crucial for the application but to ensure not exceeding specifications
		ego_speed        = min(ego_speed, 90);
		ego_deceleration = min(ego_deceleration, 10);
		ego_pedalpos     = min(ego_pedalpos, 100);
		ego_globalpos_x  = min(ego_globalpos_x, 10000);
		ego_globalpos_y  = min(ego_globalpos_y, 10000);

		if ((ego_pedalpos > 70 || ego_deceleration >= 7)) {
			cout << " Emergency Break Event!! " << endl;

			uint32_t* q = (uint32_t*)message.c_str();
			*q = ego_speed;        q++;
			*q = ego_deceleration; q++;
			*q = ego_pedalpos;     q++;
			*q = ego_globalpos_x;  q++;
			*q = ego_globalpos_y;

			UDPSendEBE(message);
		}

		else {
			cout << " No Emergency Break Happening!! Continuing Normal Operations." << endl;
		}
		
	}
}