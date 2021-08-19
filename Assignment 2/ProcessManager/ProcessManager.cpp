//Compile in a C++ CLR empty project
#using <System.dll>

#include <conio.h>//_kbhit()
#include <SMObject.h>
#include <SMStruct.h>

#define LASER_WAIT_TIME 70000
#define GPS_WAIT_TIME 50000
#define OPENGL_WAIT_TIME 50000
#define CAMERA_WAIT_TIME 50000
#define VEHICLE_WAIT_TIME 70000

using namespace System;
using namespace System::Diagnostics;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

int LaserHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckLaser);
int GPSHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckGPS);
int OpenGLHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckOpenGL);
int CameraHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckCamera);
int VehicleHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckVehicle);

int main() {
	// SM Stuff
	SMObject GalilSMObj(_TEXT("GalilSMObject"), sizeof(SMData));
	GalilSMObj.SMCreate();
	GalilSMObj.SMAccess();
	SMData* GalilSMObjPtr = (SMData*)GalilSMObj.pData;

	// Heartbeats
	// PM Stuff
	SMObject PMSMObj(_TEXT("PMSMObj"), sizeof(PM));
	PMSMObj.SMCreate();
	PMSMObj.SMAccess();
	PM* PMSMObjPtr = (PM*)PMSMObj.pData;

	PMSMObjPtr->Heartbeats.Status = 0x00;
	PMSMObjPtr->Heartbeats.Flags.PM = 0x01;
	PMSMObjPtr->Shutdown.Status = 0x00;
	GalilSMObjPtr->OpenGL.PMCheck = 0;
	GalilSMObjPtr->Camera.PMCheck = 0;

	// Timers for each system
	int PMCheckLaser = 0;
	int PMCheckGPS = 0;
	int PMCheckOpenGL = 0;
	int PMCheckVehicle = 0;
	int PMCheckCamera = 0;

	// Execute Projects
	// Array of strings with process names
	array<String^>^ ModuleList = gcnew array<String^>{"Laser.exe", "GPS.exe", "OpenGL.exe", "Camera\\Camera.exe", "Vehicle.exe"};
	// Array of integers with critical flags so you can use it later to shutdown or restart
	// Array size automatically adjusts to your ModuleList size
	array<int>^ Critical = gcnew array<int>(ModuleList->Length) { 1, 0 };
	// Creates a list of handles to processes
	array<Process^>^ ProcessList = gcnew array<Process^>(ModuleList->Length);
	//Run processes
	for (int i = 0; i < ModuleList->Length; i++) {
		ProcessList[i] = gcnew Process();
		ProcessList[i]->StartInfo->FileName = ModuleList[i];
		ProcessList[i]->Start();
	}

	// Heartbeat Checks
	while (!(PMSMObjPtr->Shutdown.Flags.PM)) {
		// Shutdown all processes on Keyboard hit
		if (_kbhit()) {
			break;
		}

		PMCheckLaser = LaserHeartbeat(PMSMObjPtr, ModuleList, ProcessList, PMCheckLaser);
		PMCheckGPS = GPSHeartbeat(PMSMObjPtr,  ModuleList, ProcessList, PMCheckGPS);
		PMCheckOpenGL = OpenGLHeartbeat(PMSMObjPtr,  ModuleList, ProcessList, PMCheckOpenGL);
		PMCheckCamera = CameraHeartbeat(PMSMObjPtr,  ModuleList, ProcessList, PMCheckCamera);
		PMCheckVehicle = VehicleHeartbeat(PMSMObjPtr,  ModuleList, ProcessList, PMCheckVehicle);
	}

	// Shutdown
	// To shutdown all processes
	//PMSMObjPtr->Shutdown.Status = 0xFF;

	// Sequential shutdown
	PMSMObjPtr->Shutdown.Flags.Vehicle = 1;
	System::Threading::Thread::Sleep(100);
	PMSMObjPtr->Shutdown.Flags.Laser = 1;
	System::Threading::Thread::Sleep(100);
	PMSMObjPtr->Shutdown.Flags.GPS = 1;
	System::Threading::Thread::Sleep(100);
	PMSMObjPtr->Shutdown.Flags.OpenGL = 1;
	System::Threading::Thread::Sleep(100);
	PMSMObjPtr->Shutdown.Flags.Camera = 1;
	System::Threading::Thread::Sleep(100);

	return 0;
}


int LaserHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckLaser) {
	// Laser Heartbeat
	if (PMSMObjPtr->Heartbeats.Flags.Laser == 1) {
		PMSMObjPtr->Heartbeats.Flags.Laser = 0;
		PMCheckLaser = 0;
		Console::WriteLine("Laser Heartbeating\n");

	}
	else {
		//Accumulate WaitAndSeeTime
		PMCheckLaser++;
		if (PMCheckLaser > LASER_WAIT_TIME) {
			// Critical Process, if hanging, request Shutdown all
			for (int i = 0; i < ModuleList->Length; i++) {
				if (!(ProcessList[i]->HasExited)) {
					ProcessList[i]->Kill();
				}
			}
			PMSMObjPtr->Shutdown.Flags.PM = 1;
		}
	}

	//Console::Write("PMCheckLaser: ");
	//Console::WriteLine(PMCheckLaser);

	return PMCheckLaser;
}

int GPSHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckGPS) {
	// GPS Heartbeat
	if (PMSMObjPtr->Heartbeats.Flags.GPS == 1) {
		PMSMObjPtr->Heartbeats.Flags.GPS = 0;
		PMCheckGPS = 0;
		Console::WriteLine("GPS Heartbeating\n");
	}
	else {
		//Accumulate WaitAndSeeTime
		PMCheckGPS++;
		// Non-critical device
		if (PMCheckGPS > GPS_WAIT_TIME) {
			if (!(ProcessList[1]->HasExited)) {
				ProcessList[1]->Kill();
			}
			// If GPS not open, open
			if (ProcessList[1]->HasExited) {
				ProcessList[1]->Start();
				PMCheckGPS = 0;
			}
		}
	}

	//Console::Write("PMCheckGPS: ");
	//Console::WriteLine(PMCheckGPS);

	return PMCheckGPS;
}

int OpenGLHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckOpenGL) {
	// OpenGL Heartbeat
	if (PMSMObjPtr->Heartbeats.Flags.OpenGL == 1) {
		PMSMObjPtr->Heartbeats.Flags.OpenGL = 0;
		PMCheckOpenGL = 0;
		Console::WriteLine("OpenGL Heartbeating\n");
	}
	else {
		//Accumulate WaitAndSeeTime
		PMCheckOpenGL++;
		if (PMCheckOpenGL > OPENGL_WAIT_TIME) {
			// Critical Process, if hanging, request Shutdown all
			for (int i = 0; i < ModuleList->Length; i++) {
				if (!(ProcessList[i]->HasExited)) {
					ProcessList[i]->Kill();
				}
			}
			PMSMObjPtr->Shutdown.Flags.PM = 1;
		}
	}

	//Console::Write("PMCheckOpenGL: ");
	//Console::WriteLine(PMCheckOpenGL);

	return PMCheckOpenGL;
}

int CameraHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckCamera) {
	// Camera Heartbeat
	if (PMSMObjPtr->Heartbeats.Flags.Camera == 1) {
		PMSMObjPtr->Heartbeats.Flags.Camera = 0;
		PMCheckCamera = 0;
		Console::WriteLine("Camera Heartbeating\n");
	}
	else {
		//Accumulate WaitAndSeeTime
		PMCheckCamera++;
		if (PMCheckCamera > CAMERA_WAIT_TIME) {
			// Critical Process, if hanging, request Shutdown all
			for (int i = 0; i < ModuleList->Length; i++) {
				if (!(ProcessList[i]->HasExited)) {
					ProcessList[i]->Kill();
				}
			}
			PMSMObjPtr->Shutdown.Flags.PM = 1;
		}
	}

	//Console::Write("PMCheckCamera: ");
	//Console::WriteLine(PMCheckCamera);

	return PMCheckCamera;
}

int VehicleHeartbeat(PM* PMSMObjPtr, array<String^>^ ModuleList, array<Process^>^ ProcessList, int PMCheckVehicle) {
	// Vehicle Heartbeat
	if (PMSMObjPtr->Heartbeats.Flags.Vehicle == 1) {
		PMSMObjPtr->Heartbeats.Flags.Vehicle = 0;
		PMCheckVehicle = 0;
		Console::WriteLine("Vehicle Heartbeating\n");
	}
	else {
		//Accumulate WaitAndSeeTime
		PMCheckVehicle++;
		if (PMCheckVehicle > VEHICLE_WAIT_TIME) {
			// Critical Process, if hanging, request Shutdown all
			for (int i = 0; i < ModuleList->Length; i++) {
				if (!(ProcessList[i]->HasExited)) {
					ProcessList[i]->Kill();
				}
			}
			PMSMObjPtr->Shutdown.Flags.PM = 1;
		}
	}

	//Console::Write("PMCheckVehicle: ");
	//Console::WriteLine(PMCheckVehicle);

	return PMCheckVehicle;
}