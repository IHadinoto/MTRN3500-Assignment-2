//Compile with C++ CLR (choose C++ CLR empty project)
#using <System.dll>
#include <conio.h> //_kbhit()

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int main()
{
	// Ethernet port number. For the Galil PLC this value does not matter
	int PortNumber = 26000;
	// Alternating 1 and 0's in the while loop
	int Toggle = 1;
	// Pointer to managed heap for an object of type TcPClient
	TcpClient^ Client;

	//Pointers tounsigned char arrays on the managed heap (that is ay ^ instead of *)
	array<unsigned char>^ SendData1; 
	array<unsigned char>^ SendData2;

	//Strings on managed heap initialized to PLC digital output commands
	String^ LightsON = gcnew String("OP 85, 85;");
	String^ LightsOFF = gcnew String("OP 170, 170;");

	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient("169.254.187.207", PortNumber);
	// Set client parameters
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// Creat send data objects on managed heap
	SendData1 = gcnew array<unsigned char>(16);
	SendData2 = gcnew array<unsigned char>(16);

	// Fill senddata unsigned char array with unsigned char values of strings LightsOn/OFF
	SendData1 = System::Text::Encoding::ASCII->GetBytes(LightsON);
	SendData2 = System::Text::Encoding::ASCII->GetBytes(LightsOFF);

	// Get a network stream object so we can use it to write and read
	NetworkStream^ Stream = Client->GetStream();

	while (!_kbhit())
	{
		Toggle = 1 - Toggle;
		if (Toggle)
		// Note we alwars write and read arrays of unsigned chars
		{
			Stream->Write(SendData1, 0, SendData1->Length);
		}
		else
		{
			Stream->Write(SendData2, 0, SendData2->Length);
		}
		// Wait for one second
		System::Threading::Thread::Sleep(1000);//ms
	}

	// close the stram and client
	Stream->Close();
	Client->Close();

	Console::ReadKey();


	return 0;
}