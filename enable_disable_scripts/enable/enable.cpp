#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <cstring>
#include <termios.h>
#include <unistd.h>
#include <iostream>         // for the input/output
#include <stdlib.h>         // for the getenv call
#include <sys/sysinfo.h>    // for the system uptime call
#include <cgicc/Cgicc.h>    // the cgicc headers
#include <cgicc/CgiDefs.h>
#include <cgicc/HTMLAttribute.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>
#define LED_GPIO "/sys/class/gpio/gpio60/"
using namespace std;
using namespace cgicc;

int openUART(void){
	int file;
	if ((file = open("/dev/ttyO4", O_RDWR | O_NOCTTY | O_NDELAY))<0){
//		perror("UART: Failed to open the file.\n");
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
//		perror("Failed to write to the output\n");
		close(file);
		return -1;
	}
	else{
		close(file);
		return 0;
	}
}

int main(){

	// Enable the function generator
	uartWrite("enable");
	// Redirect back to the control page
	cout << HTTPHTMLHeader() << endl;
	//cout << html() << head() << "<meta http-equiv=\"refresh\" content=\"0; URL=192.168.7.2:8080/cgi-bin/controlAD9833.cgi\" />" << head() << html();
	cout << html() << body() << h1("Output is enabled.") << endl;
	cout << h3("Click the back button on your browser to return to the previous page.") << endl;
	cout << h3("Note: you may need to refresh the control page for the output status displayed there to be up-to-date.") << endl;
	cout << body() << html();
	return 0;
}
