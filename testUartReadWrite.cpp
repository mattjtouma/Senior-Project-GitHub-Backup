#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<string.h>
#include<cstring>
#include<termios.h>
#include <unistd.h>
#include <iostream>         // for the input/output
#include <stdlib.h>         // for the getenv call
#include <sys/sysinfo.h>    // for the system uptime call
#include <cgicc/Cgicc.h>    // the cgicc headers
#include <cgicc/CgiDefs.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>
#include <algorithm>
#include <cctype>
#include <iterator>

#define LED_GPIO "/sys/class/gpio/gpio60/"
using namespace std;
using namespace cgicc;

int openUART(void){
	int file;
	if ((file = open("/dev/ttyO4", O_RDWR | O_NOCTTY | O_NDELAY))<0){
		perror("UART: Failed to open the file.\n");
		return -1;
	}
	else fcntl(file, F_SETFL, 0); 
	
	struct termios options;               //The termios structure is vital
	tcgetattr(file, &options);            //Sets the parameters associated with file
	// Set up the communications options:
	//   9600 baud, 8-bit, enable receiver, no modem control lines
	options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
	options.c_iflag = IGNPAR | ICRNL;    //ignore partity errors, CR -> newline
	tcflush(file, TCIFLUSH);             //discard file information not transmitted
	tcsetattr(file, TCSANOW, &options);  //changes occur immmediately
	
	return file;
}

int uartWrite(const char *transmit){
	int count;
	int file = openUART();

	if ((count = write(file, transmit, strlen(transmit)+1))<0){        //send the string
		perror("Failed to write to the output\n");
		close(file);
		return -1;
	}
	else{
		close(file);
		return 0;
	}
}

string writeAndRead(const char *transmit){
   int count;
   int file = openUART();	

   if ((count = write(file, transmit, strlen(transmit)+1))<0){        //send the string
      perror("Failed to write to the output\n");
      return "";
   }

   usleep(100000);                  //give the Arduino a chance to respond

   char buffer[100];      //declare a buffer for receiving data
   count = read(file, buffer, sizeof(buffer));
   if (count<0){
      perror("Failed to read from the input\n");
      return "";
   }
   if (count==0) printf("There was no data available to read!\n");
   else {
      printf("The following was read in [%d]: %s\n",count,buffer);
   }
   close(file);
   return buffer;
}

bool isNumber(const string& str){
	for (char const &c : str) {
        	if (isdigit(c) == 0) return false;
	return true;
	}
}

void writeGPIO(string filename, string value){
   fstream fs;
   string path(LED_GPIO);
   fs.open((path + filename).c_str(), fstream::out);
   fs << value;
   fs.close();
}

// Function to remove non-digit characters from a string

inline bool not_digit(char ch) {
	    return '0' <= ch && ch <= '9';
}

string remove_non_digits(const string& input) {
	string result;
        copy_if(input.begin(), input.end(),
        back_inserter(result), not_digit);
    return result;
}

int main(){
//   Cgicc form;                      // the CGI form object
  // string func;                      // the Set LED command
   //string freq;
   //string mult;
   // Aquire the current settings on the AD9833
   // Reuest that all settings be sent
   string myString = writeAndRead("getOS");
   
   // Remove this line when done testing?
   //myString.erase(remove_if(myString.begin(), myString.end(), ::isspace), myString.end());
   //string result = remove_non_digits(myString);

   cout << "read: " << myString << endl;
   //cout << "read: " << result << endl;
}
