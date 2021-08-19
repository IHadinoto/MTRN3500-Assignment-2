//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <SMObject.h>
#include <SMStruct.h>

#define WAIT_TIME 200

using namespace System;
using namespace System::Diagnostics;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

void Authenticate(NetworkStream^ Stream);
int HeartbeatChecks(PM* PMSMObjPtr, int PMCheck);
void SendCommand(NetworkStream^ Stream, SMData* GalilSMObjPtr, int Flag);
void ProcessSharedMemory(double Resolution, int NumRanges, array<String^>^ StringArray, SMData* GalilSMObjPtr);
TcpClient^ ClientSetUp(int PortNumber);

int main(void) {
	// SM Stuff
	SMObject GalilSMObj(_TEXT("GalilSMObject"), sizeof(SMData));
	GalilSMObj.SMAccess();
	SMData* GalilSMObjPtr = (SMData*)GalilSMObj.pData;

	//PM Stuff
	SMObject PMSMObj(_TEXT("PMSMObj"), sizeof(PM));
	PMSMObj.SMAccess();
	PM* PMSMObjPtr = (PM*)PMSMObj.pData;


	// Vehicle port number must be 25000
	int PortNumber = 25000;
	// Pointer to TcpClient type object on managed heap 
	TcpClient^ Client = ClientSetUp(PortNumber);
	// Get the network stream object associated with client so we can use it to read and write
	NetworkStream^ Stream = Client->GetStream();
	// Authenticate
	Authenticate(Stream);

	// Check PM Heartbeat
	int PMCheck = 0;

	// Initialise Steer, Speed and Flag
	int Flag = 0;

	// Loop
	while (!(PMSMObjPtr->Shutdown.Flags.Vehicle)) {
		// Heartbeat Checks
		PMCheck = HeartbeatChecks(PMSMObjPtr, PMCheck);

		Flag = !Flag;
		// Send Vehicle Command data
		SendCommand(Stream, GalilSMObjPtr,Flag);

		System::Threading::Thread::Sleep(100);
	}

	Stream->Close();
	Client->Close();

	return 0;
}

void Authenticate(NetworkStream^ Stream) {
	// String command to send zID
	String^ zID = gcnew String("5161724\n");

	// String and Array to store received data for display
	String^ ResponseData = nullptr;
	array<unsigned char>^ ReadData = gcnew array<unsigned char>(2500);

	// Convert string command to an array of unsigned char
	array<unsigned char>^ SendData = System::Text::Encoding::ASCII->GetBytes(zID);

	// Write command asking for data
	Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	// Print the received string on the screen
	Console::WriteLine(ResponseData);

	return;
}

TcpClient^ ClientSetUp(int PortNumber) {
	Console::WriteLine("Connecting to Client");
	// Creat TcpClient object and connect to it
	TcpClient^ Client;
	Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	return Client;
}

int HeartbeatChecks(PM* PMSMObjPtr, int PMCheck) {
	// Heartbeat Checks
	if (PMSMObjPtr->Heartbeats.Flags.Vehicle == 0) {
		// Set Heartbeat Flag
		PMSMObjPtr->Heartbeats.Flags.Vehicle = 1;
		PMCheck = 0;
	}
	else {
		//Accumulate WaitAndSeeTime
		PMCheck++;
		//Console::Write("PMCheck\t");
		//Console::WriteLine(PMCheck);
		if (PMCheck > WAIT_TIME) {
			PMSMObjPtr->Shutdown.Flags.GPS = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Laser = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.OpenGL = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Camera = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Vehicle = 1;
			System::Threading::Thread::Sleep(100);
		}
	}
	return PMCheck;
}


void SendCommand(NetworkStream^ Stream, SMData* GalilSMObjPtr, int Flag) {
	// Arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData = gcnew array<unsigned char>(16);
	double Speed = GalilSMObjPtr->Vehicle.Speed;
	double Steer = GalilSMObjPtr->Vehicle.Steer;
	// Send Control Data
	String^ Message = gcnew String("# ");
	Message = Message + Steer.ToString("F3") + " " + Speed.ToString("F3") + " " + Flag.ToString("D") + " #";
	Console::Write("Message received: ");
	Console::WriteLine(Message);
	SendData = System::Text::Encoding::ASCII->GetBytes(Message);
	Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);

	return;
}