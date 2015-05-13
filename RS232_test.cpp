///* 
// * File:   RS232_test.cpp
// * Author: Thor
// *
// * Created on May 5, 2015, 1:44 PM
// */
//
//#include <cstdlib>
//#include <termios.h> //For uart RS232 terminal 
//#include <fcntl.h>  //For som constants in RS232
//#include <strstream>
//#include <sstream>
//#include <iostream> 
//#include <errno.h>
//#include <string.h>
//#include <fstream>  //For files 
//
//#define UART_BUFFER_SIZE 100
//
//using namespace std;
//
//
//int port = 0;
//
//void closePort(int portToClose) {
//    close(portToClose);
//}
//
//bool openPort(const string portToOpen) {
//
//    struct termios portConfig;
//    port = open(portToOpen.c_str(), O_RDWR | O_NOCTTY);
//    if (port == -1) {
//        cout << "Port error" << strerror(errno); //Check for errors   
//        return false;
//
//    }
//
//    //Check of the port is a port we can use
//    if (!isatty(port)) {
//        cout << portToOpen << " is not a comport. FClosing it down" << endl;
//        closePort(port);
//        return false;
//
//    }
//    // Get the current configuration of the serial interface
//    if (tcgetattr(port, &portConfig) < 0) {
//        cout << portToOpen << "Get config failed" << strerror(errno) << endl; //Check for errors  endl;
//        closePort(port);
//        return false;
//
//    }
//
//    portConfig.c_iflag = 0;
//    portConfig.c_oflag = 0;
//    portConfig.c_lflag = 0;
//    portConfig.c_cflag &= ~(CSIZE | PARENB);
//    portConfig.c_cflag |= CS8;
//    portConfig.c_cc[VMIN] = 1;
//    portConfig.c_cc[VTIME] = 5;
//
//
//    if (cfsetispeed(&portConfig, B115200) < 0) {
//        cout << portToOpen << " cfsetispeed failed" << strerror(errno) << endl; //Check for errors  endl;
//        closePort(port);
//        return false;
//
//    }
//
//    if (cfsetospeed(&portConfig, B115200) < 0) {
//        cout << portToOpen << " cfsetispeed failed" << strerror(errno) << endl; //Check for errors  endl;
//        closePort(port);
//        return false;
//
//    }
//
//    //Finally, apply the configuration
//    if (tcsetattr(port, TCSAFLUSH, &portConfig) < 0) {
//        cout << portToOpen << " tcsetattr failed" << strerror(errno) << endl; //Check for errors  endl;
//        closePort(port);
//        return false;
//    }
//    return true;
//}
//
//
//static const string ENDL = ("\n\r");
//static const char ETX = 3;
//static const char STX = 2;
//
///*
// * 
// */
//
//int main(int argc, char** argv) {
//
//    system("stop ttyPS0");
//
//    ofstream fileToWrite;
//    fileToWrite.open("testlog.csv", ios::out | ios::trunc);
//
//    if (!openPort("/dev/ttyPS0")){
//        return false ;
//        cout << "Fejl i åbne port" << ENDL;
//    }
//   string dataToSend2 = ("off 3\n");
//   
//    
//    write(port, dataToSend2.c_str(), dataToSend2.length());
//
//    for (long  i = 0; i < 100000000; i++) {
//        int elem = 0;
//
//    }
//
//   string  dataToSend = ("status\n");
//
//
//    write(port, dataToSend.c_str(), dataToSend.length());
//
//    stringstream conv;
//    int sized = 0;
//    int bytesRecived = 0;
//    char rcvChar = 0;
//    tcflush(port, TCIOFLUSH);
//
//    while (rcvChar != 'e') {
//        if (bytesRecived = read(port, &rcvChar, 1) > 0) {
//            fileToWrite.put(rcvChar);
//          //  fileToWrite << rcvChar;
//        }
//    }
//    close(port);
//    fileToWrite.close();
//
//
//
//
//    return 0;
//}
//
//
//
//
////