# ASSIGNMENT 2

```
MTRN3500 Computing Applications in Mechatronic Systems - 2 020
A Systems Integration Practical with an Unmanned Ground Vehicle
```
# 1 Aims

- To implement object-oriented programs using C++.
- To demonstrate interprocess communication and process management.
- To interface with live streams of data and to use them to assist in thetele-operation of a UGV.

Note: The word “module” is used to describe a piece of self-contained software. The word “process” is
used to describe a module that is running.

# 2 Background

Often, in writing software for Mechatronic Systems, we need to integratesoftware modules that inter-
act with sensors and actuators and software modules that carry out computational tasks, under the
supervision of a process management software module. Rather than building a monolithic applica-
tion, a multi-process approach can be used to separate functional tasks and to simplify the software
development, upgrade and maintenance. Such an approach has the added benefit of limiting the
impact of software errors that may occur during operation, via process isolation. As such, we need to
use interprocess communication to allow for these specialised processes to communicate with each
other.

# 3 What is Required of You

In this assignment, you are expected to write independent modules thatinteract with each other
through shared memory. At a minimum, your development should include the following software
modules:

- a Process Management Moduleto set up shared memory and to startup, monitor and shutdown
    the operation of all the modules listed below.
- a Laser Moduleto interface to a data stream originating from an LMS151 laser rangefinder.
- a GNSS Moduleto interface to a data stream originating from a Novatel SMART-VI GPS receiver.
- a Vehicle Control Moduleto control the unmanned ground vehicle.
- a Camera Moduleto visualize data from an on-board camera
- a Display Moduleto graphically display the x-y data of the laser range finder and GNSS data in
    a virtual world.


You are then required to (i). demonstrate GPS data display, (ii). demonstrate steering and propulsion
control of the UGV and (iii). demonstrate the graphical display of laser data, camera data and GPS
data. See from Section 4 onwards for the details. Start by completing Part I in Section 4 and then
follow the other parts.

# 4 Part 1 - Process Management Module

The purpose of the process management module is to set up shared memory,start up all processes
and monitor the operational health of all the processes and finally to shutdown all the processes.
Develop the process management module to:

1. set up shared memory. the shared memory module must have the capability to:

```
(a) provide read/write access to laser data.
(b) provide read/write access to GPS data.
(c) provide read/write access to Camera data.
(d) provide read/write access to vehicle control data.
(e) provide read/write access to process management data.
```
2. start all other processes one by one in the most logical sequence suitable for tele-operation of
    the UGV.
3. monitor the heartbeats of all the other processes.
4. carry out a complete shutdown in the event the process management process detects the failure
    of a critically important process.
5. attempt to restart a failed non-critical process.
6. carry out a routine shutdown of all the processes in response to a keypress event of your
    choosing
7. indicate to all the other modules that it is alive and operational.If process management fails all
    other processes should terminate.

Process Management is the most important module and therefore will form acritical process.

# 5 Part 2 - Laser Module

The UGV mentioned earlier will have its own server running onboard. The purpose of the server is
to allow you to interact with the UGV. Therefore, in this part your first task is to establish a Wi-Fi
connection with this server over the wireless network. Use 192.168.1.200 as the IP address and
23000 as the port number to connect to and receive laser data. The serverwill authenticate you as a
permitted user, subsequent to which, you will be allowed to connect to the laser rangefinder to enable
you to collect laser range data. The most recent laser range data you collected must then be stored
as an array of X, Y coordinates in the shared memory for other modules to use.

Incorporate mechanisms in the laser module to enable the process management module to detect the
heartbeat of the laser process and for the laser process to respond to shutdown commands form the
process management process. This is a critical process for tele-operation.


# 6 Part 3 - GNSS Module

First, establish a Wi-Fi connection with the server over the wireless network. Use 192.168.1.200 as
the IP address and 24000 as the port number. The server does not need any authentication. As
soon as you connect to the GNSS receiver it will allow you to get global position data records as
a continuous stream of binary data. Process the binary data to obtain theX, Y position in UTM
coordinates and height in meters, and make them available in the shared memory for other modules
to use. In addition, this module should print on the screen, Northing, Easting and Height in meters
+ the CRC value.

Incorporate mechanisms in the GNSS module to enable the process management module to detect
the heartbeat of the GNSS process and for the GNSS process to respond to shutdown commands form
the process management process. This is a non-critical process for tele-operation.

# 7 Part 4 - Camera Module

Establish a Wi-Fi connection with the server over the wireless network. Use 192.168.1.200 as the IP
address and 26000 as the port number. The server does not need any authentication. As soon as
you connect you will be allowed to access the camera data. The most recent camera image frame you
collected must be displayed on the screen. The data need not be placed on shared memory.

Incorporate mechanisms in the camera module to enable the process management module to detect
the heartbeat of the camera module and for the camera process to respond toshutdown commands
form the process management process. This is a critical process.

# 8 Part 5 - Vehicle Control Module

The purpose of the vehicle control module is to control the UGV. To controlthe UGV, you must first
establish a Wi-Fi connection with the onboard server of the UGV. Use 192.168.1.200 as the IP address
and 25000 as the port number to establish this connection. You must sendthe UGV the following

## ASCII string to make it move. Replace the field < steer > by a numerical value between± 40 ◦as the

## required steering angle and, < speed > by a numerical value between±1m/s. as the required speed.

Replace the _<_ flag _>_ field by 0 and 1 alternatingly to indicate you are actively controlling the vehicle.
the spaces and the # are required.

## # <steer> <speed> <flag> #

Incorporate mechanisms in the vehicle control module to enable the process management process to
detect the heartbeat of the vehicle control process and for the vehicle control process to respond to
shutdown commands form the process management process.

# 9 Part 6 - Display Module

The supplied code (See Week 10 Assessment Tasks Section in Moodle) can directly be compiled,
linked and executed. When executed, it will show a virtual world centred at the origin of the vehicle
coordinate frame which is also called the body fixed coordinate frame. The vehicle coordinate frame


is such that the positive _x_ -axis is pointing in the direction of the front of the vehicle, the _z_ axis is
vertically up and the _y_ axis is such that it forms a right-handed coordinate frame.

Modify the supplied code, provide access to the shared memory, and plot the laser data and GNSS
data in the virtual world. The laser rangefinder’s scanning plane is 30 cm above the ground level.
You will then be able to select the pursuit position by pressing the key ‘P’ and tele-operate the vehicle
using the keyboard while visualizing the laser data in real time.

### SEE NEXT PAGE


# 10 Assessment

**Important note:** None of the marks will be valid if plagiarism is detected in the submitted code or if
you did not submit your entire source code by the deadline.

1. You are required to write all the software to ensure the operation of theabove-mentioned pro-
    cesses in your own time. You will use your own computers or one ofthe six Uni computers to
    develop and demonstrate your software.
2. Complete **progress check** in week 8. The first progress check is to demonstrate the layout of
    all the skeletal modules (the modules may not be fully fleshed out but the laser module must be
    complete) and the operation of the interprocess communication throughheartbeat management.
    The demonstration required is,

```
(a) The Laser process printing laser range data on the screen. (3 marks)
(b) The automatic shutdown of all other modules in response to a routine shutdown of the
process management process. A routine shutdown of the process management process is
```
## “pressing a key” to exit itswhileloop. (3 marks)

```
You must get this marked during your allocated lab time in week 8. Late penalties per
School policy applies.
```
3. Complete **assessment** in week 10. This is an assessment and therefore there areno repeat
    opportunitiesfor these tasks. You get only one opportunity to demonstrate.

```
(a) The operation of the GNSS process plotting GNSS data on the graphicaldisplay and printing
Northing, Easting, Height and CRC on the GNSS process console. (3 marks)
(b) Plotting laser data on the graphical display and printing x-y range coordinates on the laser
process console. (3 marks)
(c) Demonstrating steering and propulsion of the vehicle in response to keyboard commands.
(4 marks)
(d) Demonstrating shutdown of all processes in response to a keyboardcommand. (2 marks)
(e) Answering the two questions your demonstrator will ask about your software. (2 marks)
```
```
You must get this marked during your allocated lab time in week 10. Late penalties per
School policy applies.
```
4. **DO NOT FORGET** to submit all your software in a zip file named z<1234567>.zip with <1234567> re-
    placed by your student number.Submit your file by 11.59 pm of 20 November 2020. Failing to
    do this will apply late submission penalties to your full assignment marks.

**NOTE:** The penalty for late assessment is as per the School policy. It applies if you did not complete
the above items at the specified times.

Associate Professor Jay Katupitiya
Lecturer-in-charge
July 2020


