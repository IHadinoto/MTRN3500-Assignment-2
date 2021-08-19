#pragma once


struct LaserSM {
	double XCoordinate[1024];
	double YCoordinate[1024];
	int NumberRange;
};

#pragma pack(push, 4)
struct GPSSM {
	unsigned int Header;
	unsigned char Discards1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char Discards2[40];
	unsigned int Checksum;
};
#pragma pack(pop)

struct GPSPlot {
	double Northing[2048];
	double Easting[2048];
	int NumberPoints;
};

struct VehicleSM {
	double Steer;
	double Speed;
};

struct OpenGLSM {
	int PMCheck;
};

struct CameraSM {
	int PMCheck;
};

struct SMData {
	LaserSM Laser;
	GPSSM GPS;
	GPSPlot GPSP;
	VehicleSM Vehicle;
	OpenGLSM OpenGL;
	CameraSM Camera;
};

struct ModuleFlags {
	unsigned char
		PM : 1,
		GPS : 1,
		Laser : 1,
		Xbox : 1,
		Vehicle : 1,
		OpenGL: 1,
		Camera: 1,
		Unused : 1;
};

//For collective handling
//of individual bits
union ExecFlags {
	unsigned char Status;
	ModuleFlags Flags;
};

struct PM {
	ExecFlags Heartbeats;
	ExecFlags Shutdown;
};