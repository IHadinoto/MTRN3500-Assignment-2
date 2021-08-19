//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <SMObject.h>
#include <SMStruct.h>

#define WAIT_TIME 15
#define CRC32_POLYNOMIAL 0xEDB88320L

using namespace System;
using namespace System::Diagnostics;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

TcpClient^ ClientSetUp(int PortNumber);
int HeartbeatChecks(PM* PMSMObjPtr, int PMCheck);
array<unsigned char>^ GetData(NetworkStream^ Stream);
int ProcessSharedMemory(array<unsigned char>^ ReadData, SMData* GalilSMObjPtr, int NumberPoints);
unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

int main() {
	Console::WriteLine("GPS Started");

	// SM Stuff
	SMObject GalilSMObj(_TEXT("GalilSMObject"), sizeof(SMData));
	GalilSMObj.SMAccess();
	SMData* GalilSMObjPtr = (SMData*)GalilSMObj.pData;

	//PM Stuff
	SMObject PMSMObj(_TEXT("PMSMObj"), sizeof(PM));
	PMSMObj.SMAccess();
	PM* PMSMObjPtr = (PM*)PMSMObj.pData;

	// GPS port number must be 24000
	int PortNumber = 24000;

	// Pointer to TcpClent type object on managed heap
	TcpClient^ Client = ClientSetUp(PortNumber);

	// Get the network stream object associated with client so we 
	// can use it to read and write
	NetworkStream^ Stream = Client->GetStream();

	// Check PM Heartbeat
	int PMCheck = 0;
	int NumberPoints = 0;

	//Loop
	while (!(PMSMObjPtr->Shutdown.Flags.GPS)) {
		// Heartbeat Checks
		PMCheck = HeartbeatChecks(PMSMObjPtr, PMCheck);
		
		// Obtain Data from GPS
		array<unsigned char>^ ReadData = GetData(Stream);
		// Process data and send to SM
		NumberPoints = ProcessSharedMemory(ReadData, GalilSMObjPtr, NumberPoints);
		GalilSMObjPtr->GPSP.NumberPoints = NumberPoints;
		Console::Write("Number Points:");
		Console::WriteLine(GalilSMObjPtr->GPSP.NumberPoints);

		/*for (int i = 0; i < GalilSMObjPtr->GPSP.NumberPoints; i++) {
			Console::Write("Northing Array:");
			Console::WriteLine(GalilSMObjPtr->GPSP.Northing[i]);
			Console::Write("Easting Array:");
			Console::WriteLine(GalilSMObjPtr->GPSP.Easting[i]);
		}*/
	}

	Stream->Close();
	Client->Close();


	return 0;
}

TcpClient^ ClientSetUp(int PortNumber) {
	Console::WriteLine("Connecting to Client");
	// Creat TcpClient object and connect to it
	TcpClient^ Client = gcnew TcpClient("192.168.1.200", PortNumber);
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->ReceiveBufferSize = 300;

	return Client;
}

int HeartbeatChecks(PM* PMSMObjPtr, int PMCheck) {
	// Heartbeat Checks
	if (PMSMObjPtr->Heartbeats.Flags.GPS == 0) {
		// Set Heartbeat Flag
		PMSMObjPtr->Heartbeats.Flags.GPS = 1;
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
			PMSMObjPtr->Shutdown.Flags.OpenGL = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Camera = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Laser = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.GPS = 1;
			System::Threading::Thread::Sleep(100);
		}
	}
	return PMCheck;
}

array<unsigned char>^ GetData(NetworkStream^ Stream) {
	// String and Array to store received data for display
	array<unsigned char>^ ReadData = gcnew array<unsigned char>(300);

	// Wait for the server to prepare the data
	while (!(Stream->DataAvailable)) {
		System::Threading::Thread::Sleep(100);
		//Console::Write("Data Available");
		//Console::WriteLine(Stream->DataAvailable);
	}

	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);

	/*// Print the received string on the screen
	for (int i = 0; i < ReadData->Length; i++) {
		Console::Write("{0:X}", ReadData[i]);
	}*/


	return ReadData;
}

int ProcessSharedMemory(array<unsigned char>^ ReadData, SMData* GalilSMObjPtr, int NumberPoints) {
	// Trapping the Header
	unsigned int Header = 0;
	unsigned char Data;
	int i = 0;
	int Start; //Start of data
	unsigned char CRCCheck[108];
	do {
		Data = ReadData[i++];
		Header = ((Header << 8) | Data);
	} while (Header != 0xaa44121c && i<256);
	Start = i - 4;

	unsigned char* BytePtr = (unsigned char*)&GalilSMObjPtr->GPS;
	for (int i = Start; i < Start + sizeof(GPSSM); i++) {
		if (i < Start + sizeof(GPSSM) - 4) {
			CRCCheck[i] = ReadData[i];
		}
		*(BytePtr++) = ReadData[i];
	}

	/*Console::Write("Read Data:");
	for (int i = Start; i < Start + sizeof(GPSSM); i++) {
		Console::Write("{0:X}\t", ReadData[i]);
	}
	Console::WriteLine(" ");

	Console::Write("Cropped Data:");
	for (int i = Start; i < Start + sizeof(GPSSM); i++) {
		Console::Write("{0:X}\t", CRCCheck[i]);
	}
	Console::WriteLine(" ");*/

	unsigned long CRC = CalculateBlockCRC32(108, CRCCheck);

	Console::Write("Calculated CRC:");
	Console::Write("{0:X}\t", CRC);
	Console::Write("Appended CRC:");
	Console::WriteLine("{0:X}", GalilSMObjPtr->GPS.Checksum);

	if (CRC == GalilSMObjPtr->GPS.Checksum) {
		Console::Write("Header\t");
		Console::WriteLine("{0:X}", GalilSMObjPtr->GPS.Header);
		Console::Write("Northing");
		Console::WriteLine("{0,12:F3}", GalilSMObjPtr->GPS.Northing);
		Console::Write("Easting\t");
		Console::WriteLine("{0,12:F3}", GalilSMObjPtr->GPS.Easting);
		Console::Write("Height\t");
		Console::WriteLine("{0,12:F3}", GalilSMObjPtr->GPS.Height);

		GalilSMObjPtr->GPSP.Northing[NumberPoints] = GalilSMObjPtr->GPS.Northing;
		GalilSMObjPtr->GPSP.Easting[NumberPoints] = GalilSMObjPtr->GPS.Easting;

		NumberPoints++;
	}

	

	return NumberPoints;
}

unsigned long CRC32Value(int i) {
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */unsigned char* ucBuffer/* Data block */)  {
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}

//Note: Data Block ucBuffer should contain all data bytes of the full data record except the checksum bytes.