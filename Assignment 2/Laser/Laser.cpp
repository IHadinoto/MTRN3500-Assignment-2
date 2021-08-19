//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <SMObject.h>
#include <SMStruct.h>

#define WAIT_TIME 100

using namespace System;
using namespace System::Diagnostics;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

void Authenticate(NetworkStream^ Stream);
int HeartbeatChecks(PM* PMSMObjPtr, int PMCheck);
array<String^>^ GetData(NetworkStream^ Stream);
void ProcessSharedMemory(double Resolution, int NumRanges, array<String^>^ StringArray, SMData* GalilSMObjPtr);
TcpClient^ ClientSetUp(int PortNumber);

int main() {
	// SM Stuff
	SMObject GalilSMObj(_TEXT("GalilSMObject"), sizeof(SMData));
	GalilSMObj.SMAccess();
	SMData* GalilSMObjPtr = (SMData*)GalilSMObj.pData;

	//PM Stuff
	SMObject PMSMObj(_TEXT("PMSMObj"), sizeof(PM));
	PMSMObj.SMAccess();
	PM* PMSMObjPtr = (PM*)PMSMObj.pData;

	
	// LMS151 port number must be 23000
	int PortNumber = 23000;
	// Pointer to TcpClient type object on managed heap 
	TcpClient^ Client = ClientSetUp(PortNumber);
	// Get the network stream object associated with client so we can use it to read and write
	NetworkStream^ Stream = Client->GetStream();
	// Authenticate
	Authenticate(Stream);

	// Check PM Heartbeat
	int PMCheck = 0;

	// Loop
	while (!(PMSMObjPtr->Shutdown.Flags.Laser)) {
		// Heartbeat Checks
		PMCheck = HeartbeatChecks(PMSMObjPtr, PMCheck);

		// Extract Laser data from receivers
		array<String^>^ StringArray = GetData(Stream);

		// Processing Data and pushing to Shared Memory
		// Getting StartAngle, Resolution, NumRangesand Range
		double StartAngle = (System::Convert::ToInt32(StringArray[23], 16)) / 10000.0 / 180 * 2 * 3.1415926535;
		double Resolution = System::Convert::ToInt32(StringArray[24], 16) * 0.5 / 10000.0;
		int NumRanges = System::Convert::ToInt32(StringArray[25], 16);

		// Checking validity of data
		int i = 0;
		while (i < 5) {
			String^ Checker = gcnew String("LMDscandata");
			/*Console::Write("Checker: ");
			Console::WriteLine(Checker);
			Console::Write("StringArray: ");
			Console::WriteLine(StringArray[i]);*/
			if (Checker == StringArray[i]) {
				break;
			}
			i++;
		}

		/*Console::Write("i: ");
		Console::WriteLine(i);

		Console::Write("Size of Array: ");
		Console::WriteLine(StringArray->Length);

		Console::Write("NumRanges: ");
		Console::WriteLine(NumRanges);*/

		// Process data
		if (StringArray->Length > NumRanges && i < 5) {
			ProcessSharedMemory(Resolution, NumRanges, StringArray, GalilSMObjPtr);
		}

		// Print the received string on the screen
		for (int i = 0; i < NumRanges; i++) {
			Console::Write("X:");
			Console::Write("{0,12:F5}\t", GalilSMObjPtr->Laser.XCoordinate[i]);
			Console::Write("Y:\t");
			Console::WriteLine("{0,12:F5}", GalilSMObjPtr->Laser.YCoordinate[i]);
		}
	}

	Stream->Close();
	Client->Close();

	return 0;
}

void Authenticate(NetworkStream^ Stream){
	// String command to send zID
	String^ zID = gcnew String("5161724\n");

	// String and Array to store received data for display
	String^ ResponseData = nullptr;
	array<unsigned char>^ ReadData = gcnew array<unsigned char>(2500);

	// Convert string command to an array of unsigned char
	array<unsigned char>^ SendData = System::Text::Encoding::ASCII->GetBytes(zID);

	// Write command asking for data
	Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 100 ms
	System::Threading::Thread::Sleep(100);
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


int HeartbeatChecks(PM* PMSMObjPtr, int PMCheck){
	// Heartbeat Checks
	if (PMSMObjPtr->Heartbeats.Flags.Laser == 0) {
		// Set Heartbeat Flag
		PMSMObjPtr->Heartbeats.Flags.Laser = 1;
		PMCheck = 0;
	}
	else {
		//Accumulate WaitAndSeeTime
		PMCheck++;
		Console::Write("PMCheck\t");
		Console::WriteLine(PMCheck);
		if (PMCheck > WAIT_TIME) {
			PMSMObjPtr->Shutdown.Flags.Vehicle = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.GPS = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.OpenGL = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Camera = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Laser = 1;
			System::Threading::Thread::Sleep(100);
		}
	}
	return PMCheck;
}

array<String^>^ GetData(NetworkStream^ Stream){
	// Arrays of unsigned chars to send and receive data
	array<unsigned char>^ SendData = gcnew array<unsigned char>(16);

	// Get Data sRN LMDscandata
	String^ ScanData = gcnew String("sRN LMDscandata");
	SendData = System::Text::Encoding::ASCII->GetBytes(ScanData);

	// Write command asking for data
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);

	// String and Array to store received data for display
	String^ ResponseData = nullptr;
	array<unsigned char>^ ReadData = gcnew array<unsigned char>(2500);

	// Wait for the server to prepare the data
	while (!(Stream->DataAvailable)) {
		System::Threading::Thread::Sleep(100);
		Console::Write("Data Available");
		Console::WriteLine(Stream->DataAvailable);
	}

	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);



	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);

	// We split ResponseData into individual sub strings separated by spaces.
	array<wchar_t>^ Space = { ' ' };

	return ResponseData->Split(Space);
}

void ProcessSharedMemory(double Resolution, int NumRanges, array<String^>^ StringArray, SMData* GalilSMObjPtr) {
	// Getting X and Y Positions
	array<double>^ Range = gcnew array<double>(NumRanges);
	GalilSMObjPtr->Laser.NumberRange = NumRanges;
	for (int i = 0; i < NumRanges; i++) {
		Range[i] = System::Convert::ToInt32(StringArray[26 + i], 16);
		GalilSMObjPtr->Laser.XCoordinate[i] = Range[i] * sin(i * Resolution / 180 * 2 * 3.1415926535);
		GalilSMObjPtr->Laser.YCoordinate[i] = -Range[i] * cos(i * Resolution / 180 * 2 * 3.1415926535);
	}

	return;
}