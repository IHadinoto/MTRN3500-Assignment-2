#using <System.dll>
#include <conio.h>//_kbhit()

using namespace System;
using namespace System::Diagnostics;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int main(void) {
    double pi = 3.1415;
    double nat = 2.71828;

    Console::WriteLine("X\t" + "Y");
    Console::Write("{0:F3}\t", pi);
    Console::WriteLine("{0:F3}", nat);


	Console::ReadKey();

	return 0;
}