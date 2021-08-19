//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <SMObject.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

struct SMData {
	double SMFromGalil;
};

int main() {
	// SM Stuff
	SMObject GalilSMObj(_TEXT("GalilSMObject"), sizeof(SMData));
	GalilSMObj.SMAccess();
	SMData* GalilSMObjPtr = (SMData*)GalilSMObj.pData;

	while (!_kbhit()) {
		Console::WriteLine("{0,12:F3}", GalilSMObjPtr->SMFromGalil);
	}

	return 0;
}