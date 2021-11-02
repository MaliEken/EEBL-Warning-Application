#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
using namespace std;

void WinSockSetup() {
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
void main()
{
	// WinSocket setup
	WinSockSetup();

	// Create a udp socket
	SOCKET in = socket(AF_INET, SOCK_DGRAM, 0);

	// Create a server hint structure for the server
	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint.sin_family = AF_INET; // Address format is IPv4
	serverHint.sin_port = htons(54000); // Convert from little to big endian

	// Try and bind the socket to the IP and port
	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		cout << "Can't bind socket! " << WSAGetLastError() << endl;
		return;
	}


	sockaddr_in client; // Use to hold the client information (port / ip address)
	int clientLength = sizeof(client); // The size of the client information

	char buf[1024];

	uint32_t ego_speed;
	uint32_t ego_deceleration;
	uint32_t ego_pedalpos;
	uint32_t ego_globalpos_x;
	uint32_t ego_globalpos_y;

	// Enter a loop
	while (true)
	{
		ZeroMemory(&client, clientLength); // Clear the client structure
		ZeroMemory(buf, 1024); // Clear the receive buffer

		cout << " Enter the inputs to check if emergency break event happens." << endl;
		cout << " Speed - Range: 0-90 (km/h) = "; cin >> ego_speed; cout << endl;
		cout << " Deceleration - Range: 0-10 (m/s^2) = "; cin >> ego_deceleration; cout << endl;
		cout << " Break Pedal Position - Range: 0-100 (%) = "; cin >> ego_pedalpos; cout << endl;
		cout << " Global Position - Range: 0-10000 (m): " << endl;
		cout << " X coordinate = "; cin >> ego_globalpos_x;
		cout << " Y coordinate = "; cin >> ego_globalpos_y;
		cout << " Listening for incoming messages!!";


		// Wait for message
		int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
		if (bytesIn == SOCKET_ERROR)
		{
			cout << "Error receiving from client " << WSAGetLastError() << endl;
			continue;
		}

		// Display message and client info
		char clientIp[256]; // Create enough space to convert the address byte array
		ZeroMemory(clientIp, 256); // to string of characters
		// Convert from byte array to chars
		inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);
		
		cout << " Message recv from :  " << clientIp << endl;
		
		uint32_t* q = (uint32_t*)buf;
		// The parameters of the vehicle that sends the emergency break event message 
		uint32_t lead_speed        = *q; cout << " Sender Speed (km/h) = " <<*q << endl; q++;
		uint32_t lead_deceleration = *q; cout << " Sender Deceleration (m/s^2) = "<< * q << endl; q++;
		uint32_t lead_pedalpos     = *q; cout << " Sender Break Pedal Position (%) = " <<* q << endl; q++;
		cout << " Sender Global Position (m): " << endl;
		uint32_t lead_globalpos_x  = *q; cout <<" X coordinate = "<< * q << endl; q++;
		uint32_t lead_globalpos_y  = *q; cout <<" Y coordinate = "<< * q << endl;
		
		// Assume the vehicles travel along x axis of the map
		
		if ((lead_globalpos_y - ego_globalpos_y) <= 2) { // Check if the vehicles are on same lane
			if (lead_globalpos_x > ego_globalpos_x) {    // Check if the the vehicle sent eebl event is in front
				if (ego_speed >= 20 && (lead_globalpos_x - ego_globalpos_x) < 300) { // Check if event is too far away
					cout << "EEBL Event Warning !! " << endl;
				}
				else {
					cout << " Emergency Break Event is too far away" << endl;
				}
			}
			else {
				cout << " Emergency Break Event is left behind" << endl;
			}
		}
		else {
			cout << " Emergency Break Event is not on the Lane " << endl;
		}
	}

	// Close socket
	closesocket(in);

	// Shutdown winsock
	WSACleanup();
}
