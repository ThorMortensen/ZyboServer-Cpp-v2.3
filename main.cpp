/* 
 * File:   main.cpp
 * Author: Thor
 *
 * Created on 24. marts 2015, 16:00
 */

#include <stdint.h> //For uint  
#include <sstream>  //String Streams for convertion 
#include <iostream> //Input/output 
#include <vector>   //For string vectors (string arry)
#include <cstdlib>  //For accesses to c++ std namespace 
#include <fstream>  //For files 
#include <time.h>   //For sytem time 
#include <unistd.h> //For nanoSleep 
#include <pthread.h>    //For multithreading 
#include <termios.h> //For uart RS232 terminal 
#include <fcntl.h>  //For som constants in RS232

//For network 
#include <arpa/inet.h>  //For 'inet_ntoa' to get readable ip 
#include <cstring>      //For memset'ing the socket structs 
#include <sys/socket.h> //For sockets :-D
#include <netdb.h>      //For some socket stuff...Dono
#include <errno.h>      //For error msg's

#include "ProgArg_s.h"  //For system arguments 

#define PATH_ARG 1//From OS
#define EXPECTED_NO_OF_ARGS 2  +PATH_ARG//From the user/OS
#define NO_FLAGS 0
#define EXPECTED_CLIENTS 10
#define CMD_AMOUNT 10
#define RX_BUFFER_SIZE 1400
#define SAMPLE_RESOLUTION_s 1000



//Will send IP and thread No back to client if defined 
#define DEBUG




using namespace std;


uint32_t globalSamplerate = 120;
string globalTimeStamp;
int port = 0;
static const char ETX = 3;

//static const string FILE_PATH = "sensorLog.csv";

static const string FILE_PATH = "/home/groupawesome/sensorLog.csv";






//How many sensor's conected to this board
static const uint8_t SENSOR_AMOUNT = 5;
static const string SENSOR_AMOUNT_STR = ("5");

//Can't return character for some strange reson?! Use this instead of std::endl;
static const string ENDL = ("\n");

//The usage for this program
static const string USAGE = ("Usage: -port xxxx");
static const string REMOTE_USAGE = (
        "Remote Usage --> Send a 'TM20' cmd followed by: \n\r"
        "                 'GSA' =  GET_SONSOR_AMOUNT\n\r"
        "                 'GET_S' = READ_SENSOR_NO followed by the sensor No\n\r"
        "                 'KILL_C' = CLOSE_CONNECTION from server side\n\r"
        "                 'ECHO' = MAKE_UPPERCASE followed by a str to send back in uppercase \n\r"
        "                 'SEN_SET' = SET_SAMPLERATE followed by sensor No followed by sample rate No\n\r"
        "                 'STOP_S' = STOP_SENSOR followed by sensor No\n\r"
        "                 'START_S' = START_SENSOR same  \n\r"
        "                 'STATUS' = GET_BOARD_STATUS\n\r"
        "You must send atleast one cmd but you can send as many you want in one go \n\r");

//Strings, each corresponding to a cmd from the client user. See above ^
#define GET_SONSOR_AMOUNT 0
#define READ_SENSOR 1
#define CLOSE_CONNECTION 2
#define MAKE_UPPERCASE 3
#define SET_SAMPLERATE 4
#define START_SENSOR 5
#define STOP_SENSOR 6
#define GET_BOARD_STATUS 7

//For the tokenizer
const char DELIMITER = ' ';
const char DELIMITER_COMMA = ',';

//Argument expected for this program 
//ProgArg_s ipAddresForThisServer(1, "-ip", STRING, 14);
ProgArg_s portNrForThisServer("-port", NUMBER, 1024, 60000);

//For running through the arguments you just made. 
//REMEMBER TO CHANGE THE VECTOR SIZE TO FIT YOUR ARGS :-D 
std::vector<ProgArg_s*> args_v(1);

//Mutex variable to make code sections thread safe (synchronize between threads)
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockForTimestamp = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockForWriteToFile = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockForStatus = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockForSetSensor = PTHREAD_MUTEX_INITIALIZER;
uint16_t threadCount = 0;
bool freeThreadSlots[EXPECTED_CLIENTS];

/**
 * Sensor struct with all the info for the sensors
 */
struct Sensors_s {
    uint32_t smapleRate;
    uint8_t sensorNo;
    string sensorNo_str;
    string sensorName;
    uint64_t sensorValue;
    string timeStamp;
    uint32_t counter;
    bool isActive;
};
Sensors_s sensorList[SENSOR_AMOUNT]; //List of sensors
vector< vector<int> > fileValues; //matrix of int values (sensor data)
vector<string> names; // Holds sensor names.

/**
 * Data needed for the client threads.
 * Mostly used to send debug responses.
 */
struct ClientData_s {
    int socketDescripter; //The socket used to communicate with the client
    uint8_t threadId;
    struct sockaddr_in clientAddr;

};


//______________ From the Internet!!!! ________________
//http://coliru.stacked-crooked.com/a/652f29c0500cf195) 
//http://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c

void tokenize(std::string str, std::vector<string> &token_v) {
    size_t start = str.find_first_not_of(DELIMITER), end = start;

    while (start != std::string::npos) {
        // Find next occurence of delimiter
        end = str.find(DELIMITER, start);
        // Push back the token found into vector
        token_v.push_back(str.substr(start, end - start));
        // Skip all occurences of the delimiter to find new start
        start = str.find_first_not_of(DELIMITER, end);
    }
}


//______________ From the Internet!!!! ________________
//http://coliru.stacked-crooked.com/a/652f29c0500cf195) 
//http://stackoverflow.com/questions/53849/how-do-i-tokenize-a-string-in-c

void tokenizeForCSV(std::string str, std::vector<string> &token_v) {
    size_t start = str.find_first_not_of(DELIMITER_COMMA);
    size_t end = start;

    while (start != std::string::npos) {
        // Find next occurence of delimiter
        end = str.find(DELIMITER_COMMA, start);
        // Push back the token found into vector
        token_v.push_back(str.substr(start, end - start));
        // Skip all occurences of the delimiter to find new start
        start = str.find_first_not_of(DELIMITER_COMMA, end);
    }
}
//========== FROM INTERNET END =================

bool openPort(const string portToOpen) {

    struct termios portConfig;
    port = open(portToOpen.c_str(), O_RDWR | O_NOCTTY);
    if (port == -1) {
        cout << "Port error" << strerror(errno); //Check for errors   
        return false;
    }

    //Check of the port is a port we can use
    if (!isatty(port)) {
        cout << portToOpen << " is not a comport. FClosing it down" << endl;
        close(port);
        return false;
    }
    // Get the current configuration of the serial interface
    if (tcgetattr(port, &portConfig) < 0) {
        cout << portToOpen << "Get config failed" << strerror(errno) << endl; //Check for errors  endl;
        close(port);
        return false;
    }

    portConfig.c_iflag = 0;
    portConfig.c_oflag = 0;
    portConfig.c_lflag = 0;
    portConfig.c_cflag &= ~(CSIZE | PARENB);
    portConfig.c_cflag |= CS8;
    portConfig.c_cc[VMIN] = 1;
    portConfig.c_cc[VTIME] = 5;


    if (cfsetispeed(&portConfig, B115200) < 0) {
        cout << portToOpen << " cfsetispeed failed" << strerror(errno) << endl; //Check for errors  endl;
        close(port);
        return false;
    }

    if (cfsetospeed(&portConfig, B115200) < 0) {
        cout << portToOpen << " cfsetispeed failed" << strerror(errno) << endl; //Check for errors  endl;
        close(port);
        return false;
    }

    //Finally, apply the configuration
    if (tcsetattr(port, TCSAFLUSH, &portConfig) < 0) {
        cout << portToOpen << " tcsetattr failed" << strerror(errno) << endl; //Check for errors  endl;
        close(port);
        return false;
    }
    return true;
}

/**
 * From the internet!
 * Get the system time as a string. 
 * @return time as a string 
 */
void makeTimeStamp() {
    // Get current date/time
    pthread_mutex_lock(&lockForTimestamp);
    time_t now = time(0);
    struct tm tstruct;
    char timebuf[80];
    tstruct = *localtime(&now);
    strftime(timebuf, sizeof (timebuf), "%X. %d-%m-%Y", &tstruct);
    globalTimeStamp.assign(timebuf);
    pthread_mutex_unlock(&lockForTimestamp);
}

void fetchFileData() {

    names.clear();
    fileValues.clear();
    vector<string> numberValStrings;

    string lineInFile;
    ifstream fileToRead;
    fileToRead.open(FILE_PATH.c_str());
    getline(fileToRead, lineInFile);

    tokenizeForCSV(lineInFile, names);

    vector<int> numbers;

    while (fileToRead.tellg() != -1) {
        numbers.clear();
        numberValStrings.clear();
        getline(fileToRead, lineInFile);
        tokenizeForCSV(lineInFile, numberValStrings);
        for (int i = 0; i < numberValStrings.size(); i++) {
            // cout << numberValStrings.at(i) << endl;
            stringstream converter(numberValStrings.at(i));
            int number = 0;
            converter >> number;
            numbers.push_back(number);
        }
        fileValues.push_back(numbers);
    }
}

/**
 * !!!NOT IMPLIMENTED!!! 
 * @TODO Make it. 
 * @Remember Lock mutex for thread safety!
 * @param log
 */
bool writeLogToFile(void) {
    pthread_mutex_lock(&lockForWriteToFile);
    makeTimeStamp();
    ofstream fileToWrite;
    char rcvChar = 0;
    fileToWrite.open(FILE_PATH.c_str(), ios::out | ios::trunc);
    if (!openPort("/dev/ttyPS0")) return false;
    string getSensorCmd = ("loserville\n");

    write(port, getSensorCmd.c_str(), getSensorCmd.length());
    while (rcvChar != ETX) {
        if (read(port, &rcvChar, 1) > 0) {
            fileToWrite.put(rcvChar);
        }
    }
    fileToWrite.close();
    close(port);
    pthread_mutex_unlock(&lockForWriteToFile);
    return true;
}

bool enableDisableSensor(const string onOffCmd) {
    pthread_mutex_lock(&lockForSetSensor);
    if (!openPort("/dev/ttyPS0")) return false;
    write(port, onOffCmd.c_str(), onOffCmd.length());
    close(port);
    pthread_mutex_unlock(&lockForSetSensor);
    return true;
}

const string getRemoteSensorStatus(void) {
    pthread_mutex_lock(&lockForStatus);
    if (!openPort("/dev/ttyPS0")) return NULL;
    string statusCmd = ("status\n");
    string statusStr;
    write(port, statusCmd.c_str(), statusCmd.length());
    char rcvBuffer[50] = {0};
    char rcvChar = 0;
    int i = 0;
    while (rcvChar != 'd') {
        if (read(port, &rcvChar, 1) > 0) {
            rcvBuffer[i++] = rcvChar;
            if (i == 50) { //Safety first
                break;
            }
        }
    }
    //rcvBuffer[i-1] = 0;
    close(port);
    statusStr.assign(rcvBuffer);
    pthread_mutex_unlock(&lockForStatus);
    return statusStr;
}

/**
 * Simulating a sensor read. 
 * Returns a random number.
 * @param uint8_t sensorNo
 * @return int64_t random No
 */
int64_t readSensor(uint8_t sensorNo) {
    return random();
}

/**
 * Set the sample rate for a sensor 
 * @ThreadSafe Yes 
 * @param uint8_t sensorNo
 * @param uint32_t sempleRate
 */
void setSensorSampleRate(uint32_t sampleRate) {
    pthread_mutex_lock(&lock);
    globalSamplerate = sampleRate;
    pthread_mutex_unlock(&lock);
}

int getSensorData(uint8_t sensorNo) {
    int sizeOfInputs = fileValues.size();
    int biggestVal = 0;
    for (int i = 0; i < sizeOfInputs - 1; i++) {
        int currentVal = fileValues.at(i).at(sensorNo);
        if (biggestVal < currentVal)
            biggestVal = currentVal;
    }
    return biggestVal;
}

/**
 * This is to be able to handle more than one client at the time.
 * 
 * 
 * This is the thread a clients gets when she is connected to this server. 
 * It handles the commands described in the "REMOTE_USAGE". 
 * It will keep the connection up until client disconnect or get a cmd to shut 
 * down by the client user. 
 * It will check the inputs from the client user and act accordingly. 
 * @param void *clientSocket struct (info about this client)
 * @return void * (nothing)
 */
#define CLINET_ARG 8

void *clientHandlerThread(void *clientSocket) {

    int theClientArgIs = -1;

    vector<ProgArg_s*> clientArguments(CLINET_ARG);

    ProgArg_s tm20("TM20", NOTHING);
    ProgArg_s gsa("GSA", NOTHING);
    ProgArg_s get_s("GET_S", NUMBER, 1, 6);
    ProgArg_s kill_c("KILL_C", NOTHING);
    ProgArg_s echo("ECHO", NOTHING);
    ProgArg_s sampleRate_set("SEN_SET", NUMBER, 1, 2629743); //Sample rate can be one second to one month 
    ProgArg_s startSensor("START_S", NUMBER, 1, 6);
    ProgArg_s stopSensor("STOP_S", NUMBER, 1, 6);
    ProgArg_s status("STATUS", NOTHING);

    clientArguments[0] = &gsa;
    clientArguments[1] = &get_s;
    clientArguments[2] = &kill_c;
    clientArguments[3] = &echo;
    clientArguments[4] = &sampleRate_set;
    clientArguments[5] = &startSensor;
    clientArguments[6] = &stopSensor;
    clientArguments[7] = &status;

    bool keepAlive = true;
    stringstream log; //Not used 
    stringstream converter; //For converting strings to numbers and vice versa
    vector<string> rxStrings_v; //Hold the cmd's after they have bean tokenized
    string rxString; //For converting the char buf to string so it can be tokenized (easer than going through the buf byte by byte :-D)
    string txMsg; //Msg to send
    char rxBuffer [RX_BUFFER_SIZE]; //For holding the incoming bytes 

    struct ClientData_s *thisClientData; //To get the clientSocket out from the void*

    thisClientData = (struct ClientData_s *) clientSocket; //Get the data from the void*

    while (keepAlive) {
        //Clear all containers 
        theClientArgIs = -1;
        converter.str(std::string());
        converter.clear();
        txMsg.clear();
        rxString.clear();
        rxStrings_v.clear();
        memset(rxBuffer, 0, sizeof (rxBuffer));

        //Wait for incoming bytes from the client user 
        ssize_t bytesRx = recv(thisClientData->socketDescripter, rxBuffer, RX_BUFFER_SIZE, NO_FLAGS);

        // ________ Error checking _________
        //This should go to a log stream and then to a file. (In the future)
        if (bytesRx == 0) {
            cout << "Connection from: " << inet_ntoa(thisClientData->clientAddr.sin_addr) << " is lost. Closing thread and conection" << ENDL;
            keepAlive = false;
            continue;
        }

        if (bytesRx == -1) {
            cout << "Rx error!" << strerror(errno) << ENDL;
            keepAlive = false;
            continue;
        }
        // ________ Error checking END _________

#ifdef DEBUG //Sending info back to the user client
        int threadId = thisClientData->threadId;
        converter << threadId;
        txMsg += "Hello '";
        txMsg += inet_ntoa(thisClientData->clientAddr.sin_addr); //Get readable IP address
        txMsg += "' you have client thread:" + converter.str() + ENDL;
#endif

        rxString.append(rxBuffer); //Convert char buf to string for tokenizer
        tokenize(rxString, rxStrings_v); //Tokenize the incoming data string

        if (!tm20.equals(rxStrings_v.at(0)) || rxStrings_v.size() < 2) {//Check for TM20 cmd
            txMsg += tm20.getArgError();
        } else {
            for (uint8_t i = 0; i < clientArguments.size(); i++) {//Go through all incoming cmds/sub-strings
                if (clientArguments[i]->equals(rxStrings_v.at(1))) {
                    if (rxStrings_v.size() < 3) {
                        theClientArgIs = i;
                        break;
                    } else if (clientArguments[i]->isValid(rxStrings_v.at(2))) {
                        theClientArgIs = i;
                        break;
                    } else {
                        txMsg += clientArguments[i]->getParamError();
                        txMsg += REMOTE_USAGE;
                        break;
                    }

                }
            }
        }

        converter.str(std::string());
        converter.clear();
        switch (theClientArgIs) {
            case GET_SONSOR_AMOUNT:
                for (int i = 0; i < names.size(); i++) {
                    txMsg += names.at(i) + ": ";
                    txMsg += (char) (i + 0x30);
                    txMsg += ENDL;
                }
                break;

            case READ_SENSOR:
                if (!get_s.hasValue) {
                    txMsg += get_s.getParamError();
                    break;
                }
                fetchFileData();
                converter << getSensorData(get_s.getParamValNo());
                txMsg += "Sampled sensor at time: " + globalTimeStamp + ENDL;
                txMsg += "Sensor " + names.at(get_s.getParamValNo()) + " has maximum value: ";
                txMsg += converter.str() + ENDL;
                break;

            case CLOSE_CONNECTION:
                txMsg += "bye bye. Closing down from server side" + ENDL;
                keepAlive = false;
                break;

            case MAKE_UPPERCASE:
                if (rxStrings_v.size() >= 2) {
                    for (uint8_t ch = 0; ch < rxStrings_v[2].length(); ch++) {//Convert string to upper case
                        txMsg += toupper(rxStrings_v[2].at(ch));
                    }
                    txMsg += ENDL;
                } else
                    txMsg += "Missing a string to echo" + ENDL;
                break;

            case SET_SAMPLERATE:
                if (!sampleRate_set.hasValue) {
                    txMsg += sampleRate_set.getParamError();
                    break;
                }
                setSensorSampleRate(sampleRate_set.getParamValNo());
                writeLogToFile();
                txMsg += "Sampled sensors: " + globalTimeStamp + ENDL;
                txMsg += "Sensors sample at a rate of: " + sampleRate_set.getParamVal() + " seconds" + ENDL;
                break;

            case STOP_SENSOR:
                if (!stopSensor.hasValue) {
                    txMsg += stopSensor.getParamError();
                    break;
                }
                enableDisableSensor("off " + stopSensor.getParamVal() + "\n");
                txMsg += "Sensor " + names.at(stopSensor.getParamValNo()) + " is now DISABLED" + ENDL;
                break;

            case START_SENSOR:
                if (!startSensor.hasValue) {
                    txMsg += startSensor.getParamError();
                    break;
                }
                enableDisableSensor("on " + startSensor.getParamVal() + "\n");
                txMsg += "Sensor " + names.at(startSensor.getParamValNo()) + " is now ENABLED " + ENDL;
                break;

            case GET_BOARD_STATUS:
                converter << globalSamplerate;
                txMsg += "Sensors sample at a rate of: " + converter.str();
                txMsg += " seconds" + ENDL;
                txMsg += "Last sampled at: " + globalTimeStamp + ENDL;
                txMsg += "Path to log file is: " + FILE_PATH + ENDL;
                txMsg += "Disabled sensors:" + ENDL;
                txMsg += getRemoteSensorStatus() + ENDL;
                break;

            default:
                txMsg += "No valid arguments. RTFM!:" + ENDL;
                txMsg += REMOTE_USAGE;
                txMsg += "There you go sunshine. :-) " + ENDL;
                break;
        }
        ssize_t bytesSent = send(thisClientData->socketDescripter, txMsg.c_str(), txMsg.length(), NO_FLAGS);
        if (bytesSent != txMsg.length())std::cout << "Send error. Bytes lost";

    }//While alive


    close(thisClientData->socketDescripter); //Close this socket 
    //Lock for updating the thread list
    pthread_mutex_lock(&lock);
    cout << "Closing from: " << inet_ntoa(thisClientData->clientAddr.sin_addr) << ENDL;
    freeThreadSlots[thisClientData->threadId] = true; //This slot is now open 
    threadCount--; //One les thread is now running 
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL); //Close down this thread
}

/**
 * A thread for polling the sensors at the sample-rate interval specified  for ech sensor.
 * Set the resolution for the polling at the "SAMPLE_RESOLUTION_ms" define
 * @param void* for pthread
 * @return void* for pthread 
 */
void *sensorHandler(void *arg) {
    for (;;) {
        // Set the sleep time in nano sec. This needs a funky struct to work, 
        // thats why its a bit strange


        //        nanosleep((struct timespec[]) {
        //            {0, 1000000L * SAMPLE_RESOLUTION_s}
        //        }, NULL);

        //Go trough sensor list to see if its time to sample the specific sensor
        sleep(globalSamplerate);
        writeLogToFile();
    }
}

/**
 * Initialize the sensors
 */
void initSensors() {
    writeLogToFile();
    fetchFileData();
}

/**
 * Main 
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char** argv) {

    system("stop ttyPS0");



    //Collection of client data. Each thread need one for a safe location in mem
    ClientData_s clientData[EXPECTED_CLIENTS] = {0};

    //Threads used in this program
    pthread_t threads[EXPECTED_CLIENTS];
    pthread_t sensorPollingThread;

    int32_t errorCode = 0;
    int32_t socketId = 0;

    //  args_v[0] = &ipAddresForThisServer;
    args_v[0] = &portNrForThisServer; //Argument from user 

    //For holding and filling socket struct's 
    struct addrinfo hostInfo;
    struct addrinfo* hostInfoList;

    //Clean 
    memset(&hostInfo, 0, sizeof (hostInfo));
    memset(&freeThreadSlots, true, sizeof (freeThreadSlots));


    //================== Argument Checks ===================
    //See ProgArg_s files for the args used here 
    if (argc < EXPECTED_NO_OF_ARGS || argc > EXPECTED_NO_OF_ARGS) {
        cout << "Wrong number of arguments" << ENDL;
        cout << USAGE << ENDL;
        return false;
    }

    if (portNrForThisServer.equals(argv[1])) {
        if (!portNrForThisServer.isValid(argv[2])) {
            cout << portNrForThisServer.getParamVal() << ENDL;
            return false;
        }
    } else {
        cout << portNrForThisServer.getArgError() << ENDL;
        cout << USAGE << ENDL;
        return false;
    }
    //=============== Argument Checks finished ==================

    //For Socket initialization see 
    //http://codebase.eu/tutorial/linux-socket-programming-c/ 

    initSensors();


    //Address info : address = Address field unspecifed (both IPv4 & 6)
    hostInfo.ai_addr = AF_UNSPEC;

    // Address info : socket type. (Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.)
    hostInfo.ai_socktype = SOCK_STREAM;

    hostInfo.ai_flags = AI_PASSIVE; //From .h file: "Socket address is intended for `bind'." 

    //getaddrinfo is used to get info for the socket.
    //Null: use local host. Port no is set by user args (5555)
    errorCode = getaddrinfo(NULL, portNrForThisServer.getParamVal().c_str(), &hostInfo, &hostInfoList);
    if (errorCode != 0) {
        cout << "getaddrinfo error" << gai_strerror(errorCode);
        return false;
    }

    //Make a socket and returns a socket descriptor. 
    //All info comes from 'getaddrinfo' --> into the struct 'hostInfoList'
    socketId = socket(hostInfoList->ai_family, hostInfoList->ai_socktype, hostInfoList->ai_protocol);
    if (socketId == -1) {
        cout << "Socket error" << strerror(errno);
        return false;
    }

    //Socket options is set to reuse the add: http://pubs.opengroup.org/onlinepubs/7908799/xns/setsockopt.html
    //This is to make sure the port is not in use by a previous call by this code. 
    int optionValue_yes = 1;
    errorCode = setsockopt(socketId, SOL_SOCKET, SO_REUSEADDR, &optionValue_yes, sizeof optionValue_yes);
    //Bind socket to local port.
    errorCode = bind(socketId, hostInfoList->ai_addr, hostInfoList->ai_addrlen);
    if (errorCode == -1) {
        cout << "Bind error" << strerror(errno);
        return false;
    }

    errorCode = listen(socketId, EXPECTED_CLIENTS);
    if (errorCode == -1) {
        cout << "Listen error" << strerror(errno);
        return false;
    }

    freeaddrinfo(hostInfoList); //Not needed anymore

    //Make thread for polling the sensors
    pthread_create(&sensorPollingThread, NULL, sensorHandler, NULL);

    for (;;) {
        if (threadCount < EXPECTED_CLIENTS) {
            int newIncommingSocket = 0; //New socket ID
            struct sockaddr_in incommingAddr; //New socket address info
            socklen_t addrSize = sizeof (incommingAddr); //The size of the address
            //Wait for a client to connect. PROGRAM BLOCKS HERE UNTIL CLIENTS CONNECT 
            newIncommingSocket = accept(socketId, (struct sockaddr*) &incommingAddr, &addrSize);
            if (newIncommingSocket == -1) std::cout << "Accept error" << strerror(errno); //Check for errors
            else {
                //Show the incoming IP in readable txt
                cout << "Connection accepted. From : " << inet_ntoa(incommingAddr.sin_addr) << ENDL;
                //Find a free slot for this client
                for (uint8_t freeSlot = 0; freeSlot < EXPECTED_CLIENTS; freeSlot++) {
                    if (freeThreadSlots[freeSlot]) {
                        //Free slot found. Lock for thread safe updating of the thread list
                        pthread_mutex_lock(&lock);
                        freeThreadSlots[freeSlot] = false; //This slot is now taken
                        threadCount++;
                        pthread_mutex_unlock(&lock);
                        //Copy info to safe location before parsing to new thread
                        //These structs holds info about the client for the thread to service
                        clientData[freeSlot].threadId = freeSlot; //What slot does this thread have
                        clientData[freeSlot].clientAddr = incommingAddr; //This cleint address
                        clientData[freeSlot].socketDescripter = newIncommingSocket; //This clients socket
                        //Make the new thread and parse the info struct as void pointer
                        pthread_create(&threads[freeSlot], NULL, clientHandlerThread, static_cast<void*> (&clientData[freeSlot]));
                        break;
                    }
                }
            }
        }

    }
    return 0;
}
