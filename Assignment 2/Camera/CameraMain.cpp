#include <zmq.hpp>
#include <Windows.h>
#include <iostream>

//#include "SMStructs.h"
//#include "SMFcn.h"
#include <SMObject.h>
#include <SMStruct.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <turbojpeg.h>

#define WAIT_TIME 2000

#using <System.dll>

using namespace System;
using namespace System::Diagnostics;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

void display();
void idle();

GLuint tex;

//ZMQ settings
zmq::context_t context(1);
zmq::socket_t subscriber(context, ZMQ_SUB);

// In global scope, declare shared memory
SMObject GalilSMObj(_TEXT("GalilSMObject"), sizeof(SMData));
SMObject PMSMObj(_TEXT("PMSMObj"), sizeof(PM));

int main(int argc, char** argv) {
	// Instantiate shared memory and structs
	// SM Stuff
	GalilSMObj.SMAccess();
	// PM Stuff
	PMSMObj.SMAccess();

	//Define window size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	//GL Window setup
	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - Camera");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glGenTextures(1, &tex);

	//Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:26000");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	glutMainLoop();

	return 1;
}


void display()
{
	//Set camera as gl texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Map Camera to window
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1, -1);
	glTexCoord2f(1, 1); glVertex2f(1, -1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(0, 0); glVertex2f(-1, 1);
	glEnd();
	glutSwapBuffers();
}
void idle()
{
	// Initialise Pointers
	SMData* GalilSMObjPtr = (SMData*)GalilSMObj.pData;
	PM* PMSMObjPtr = (PM*)PMSMObj.pData;

	// Heartbeat Stuff
	if (PMSMObjPtr->Shutdown.Flags.Camera) {
		exit(0);
	}

	// Heartbeat Checks
	if (PMSMObjPtr->Heartbeats.Flags.Camera == 0) {
		// Set Heartbeat Flag
		PMSMObjPtr->Heartbeats.Flags.Camera = 1;
		GalilSMObjPtr->Camera.PMCheck = 0;
	}
	// Check PM Heartbeat
	else {
		//Accumulate WaitAndSeeTime
		GalilSMObjPtr->Camera.PMCheck++;
		Console::Write("PM Check\t");
		Console::WriteLine(GalilSMObjPtr->Camera.PMCheck);
		if (GalilSMObjPtr->Camera.PMCheck > WAIT_TIME) {
			PMSMObjPtr->Shutdown.Flags.Vehicle = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.GPS = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Laser = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.OpenGL = 1;
			System::Threading::Thread::Sleep(100);
			PMSMObjPtr->Shutdown.Flags.Camera = 1;
			System::Threading::Thread::Sleep(100);
		}
	}

	//receive from zmq
	zmq::message_t update;
	if (subscriber.recv(&update, ZMQ_NOBLOCK))
	{
		//Receive camera data
		long unsigned int _jpegSize = update.size();
		std::cout << "received " << _jpegSize << " bytes of data\n";
		unsigned char* _compressedImage = static_cast<unsigned char *>(update.data());
		int jpegSubsamp = 0, width = 0, height = 0;

		//JPEG Decompression
		tjhandle _jpegDecompressor = tjInitDecompress();
		tjDecompressHeader2(_jpegDecompressor, _compressedImage, _jpegSize, &width, &height, &jpegSubsamp);
		unsigned char * buffer = new unsigned char[width*height*3]; //!< will contain the decompressed image
		printf("Dimensions:  %d   %d\n", height, width);
		tjDecompress2(_jpegDecompressor, _compressedImage, _jpegSize, buffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
		tjDestroy(_jpegDecompressor);

		//load texture
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
		delete[] buffer;
	}
	
	display();
}

