#include <iomanip>
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
#include <algorithm>
#include <cctype>
#include <iterator>
#define LED_GPIO "/sys/class/gpio/gpio60/"
using namespace std;
using namespace cgicc;

// The maximum frequency that can be programmed to the AD9833
#define MAX_FREQ 12500000

// Function to initialize UART on the BeagleBone
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

// This function sends a string over UART
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

// This function writes a string to UART and listens for a response
string writeAndRead(const char *transmit){
   int count;
   int file = openUART();	

   if ((count = write(file, transmit, strlen(transmit)+1))<0){        //send the string
//      perror("Failed to write to the output\n");
      return "";
   }

//   usleep(100000);                  //give the Arduino a chance to respond

   char buffer[100];      //declare a buffer for receiving data
   count = read(file, buffer, sizeof(buffer));
   if (count<0){
//      perror("Failed to read from the input\n");
      return "";
   }
//   if (count==0) printf("There was no data available to read!\n");
//   else {
//      printf("The following was read in [%d]: %s\n",count,buffer);
//   }
   close(file);
   return buffer;
}

bool isNumber(const string& str){
	for (char const &c : str) {
        	if (isdigit(c) == 0) return false;
	return true;
	}
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


void writeGPIO(string filename, string value){
   fstream fs;
   string path(LED_GPIO);
   fs.open((path + filename).c_str(), fstream::out);
   fs << value;
   fs.close();
}

// Function to get current wavetype
string get_current_wavetype(void){

    // Get the current waveform type
    string current_wave_in = writeAndRead("getwaveform");
    usleep(1000000); // This delay is necessary to give the Arduino time

    // Remove non-numerical characters
    string current_wave = remove_non_digits(current_wave_in);

    string current_wavetype;

    // Evaluate the wavetype
    if (current_wave == "8192"){
            current_wavetype = "Sine";
    }
    else if (current_wave == "8232"){
            current_wavetype = "Square";
    }
    else if (current_wave == "8194"){
            current_wavetype = "Triangle";
    }
    else {
            current_wavetype = "";
    }

    return current_wavetype;
}



int main(){

   Cgicc form;                      // the CGI form object
   HTMLAttribute value;
   string func;
   string freq_with_multiplier = "";
   string freq_without_multiplier = "";
   string mult;
   string en;
   string current_freq = "";
   string current_wavetype;
   string outputStatus;
   int mult_flag = 0;
   float multiplier;
   

   // generate the form but use states that are set in the submitted form
   // Functions come from the cgicc library
   cout << HTTPHTMLHeader() << endl;       // Generate the HTML form
   cout << html() << head() << title("Smart-Control Digital Waveform Synthesizer") << head() << endl;
   cout << body() << h1("Smart-Control Digital Waveform Synthesizer Control Page") << endl;
   cout << h1("Enter commands below to execute them on the device.") << endl;
   cout << "<div style=\"color:blue;\">Please enter a frequency value, and select a waveform type.</div>";
   // Start a form
   cout << "<form action=\"/cgi-bin/controlAD9833.cgi\" method=\"POST\">\n";

  

   // Waveform Set form
   form_iterator it = form.getElement("func");  // the radio command
   //func = it->getValue(); // use submitted value
   if (it == form.getElements().end() || it->getValue()==""){
	// Get the current wavetype
      	func = get_current_wavetype();  // if it is invalid use the current wavetype
   }
   else {
	func = it->getValue(); // otherwise use submitted value
   }


   // Frequency Set form
   form_iterator freq_it = form.getElement("freq");
   if (freq_it == form.getElements().end() || freq_it->getValue()==""){
   	   // Get the current frequency running on the board
	   current_freq = writeAndRead("getfrequency");
	   usleep(1000000); // This delay is necessary to give the Arduino time

           freq_with_multiplier = current_freq; // if it is invalid don't change the frequency
	   mult_flag = 1;
   }
   else{
	   freq_without_multiplier = freq_it->getValue(); // otherwise use submitted value
	   mult_flag = 0;
   }

   // Frequency multiplier
   form_iterator mult_it = form.getElement("mult");
  // mult = mult_it->getValue(); // use submitted value
   
   //if (mult_it == form.getElements().end() || mult_it->getValue()==""){
    if (mult_it == form.getElements().end()){
   	   mult = "Hz"; // if it is invalid multiply by 1
   }
   else{
   	   mult = mult_it->getValue();
   }
   

   switch(mult_flag){
	   case 0 : // if frequency was entered use the multiplier
	
	   	// Obtain the correct multiplier
		if (mult == "mHz") multiplier = 0.001;
	        else if (mult == "Hz") multiplier = 1;
	        else if (mult == "kHz") multiplier = 1000;
	        else multiplier = 1000000;

		// Modify freq_without_multiplier
		//freq_without_multiplier = to_string(stof(freq_with_multiplier)/multiplier);

		// Define freq_with_multiplier
		freq_with_multiplier = to_string(stof(freq_without_multiplier)*multiplier);
		
		break;

	   case 1 : // if no frequency was entered, get the multiplier
		
		// freq_without_multiplier isn't defined yet, so define it
		if (stof(freq_with_multiplier) < 1){
			freq_without_multiplier = to_string(stof(freq_with_multiplier)*1000);
			mult = "mHz";
    		}
		else if (stof(freq_with_multiplier) >= 1 & stof(freq_with_multiplier) < 1000){
			freq_without_multiplier = freq_with_multiplier;
			mult = "Hz";
    		}
		else if (stof(freq_with_multiplier) >= 1000 & stof(freq_with_multiplier) < 1000000){
			freq_without_multiplier = to_string(stof(freq_with_multiplier)/1000);
			mult = "kHz";
		}
    		else{
			freq_without_multiplier = to_string(stof(freq_with_multiplier)/1000000);
			mult = "MHz";
    		}

		break;
   }// switch
   
   // At this point, both freq_with_multiplier and freq_without_multiplier are defined.



   // Process the information entered in the HTML forms
   // Send the waveform type    
   // Send the frequency
   
   // Update the current waveform
   //current_wavetype = func;

   // Update the current frequency
   //current_freq = freq;

   // Get the output status
   outputStatus = writeAndRead("getOS");
   usleep(1000000);

   // Remove whitespace from outputStatus
   //outputStatus.erase(remove_if(outputStatus.begin(), outputStatus.end(), ::isspace), outputStatus.end());
   // Remove any non-digits
   //outputStatus = remove_non_digits(outputStatus);
   

   // if outputStatus starts with 0, output is off
   // if outputStatus starts with 1, output is on
   // Here, I am just re-assigning "ON" or "OFF" to outputStatus.
   if (outputStatus.rfind("1", 0) == 0) { // pos=0 limits the search to the prefix
	   // outputStatus starts with 1
	   outputStatus = "ON";
   }
   else{
	  // outputStatus starts with 0
	  outputStatus = "OFF";
   }


   // Validate the frequency
   // display the freq variable (for debugging purposes)
   //cout << h2(freq) << endl;


   // If nothing is entered but the form is submitted,
   // don't change the frequency
  // if (freq == ""){
	   // No need to change the frequency
	   // Re-write the current frequency just in case the waveform type changed
	   // Get the current frequency running on the board
	   //string current_freq2 = writeAndRead("getfrequency");
           //usleep(1000000); // This delay is necessary to give the Arduino time
//	   uartWrite((func + "," + current_freq).c_str());
   //}
   if ((isNumber(freq_without_multiplier)) && (stof(freq_without_multiplier)) >= 0){
	// Only update the freq/wavetype if something has been entered.
	if (func != ""){
		if(stof(freq_with_multiplier) > 12500000){
			freq_with_multiplier = "12500000";
			freq_without_multiplier = "12.5"; // freq must be a string type
			mult = "MHz";
		}// if
		// Update the frequency and/or wave type running on the device
  		uartWrite((func + "," + freq_with_multiplier).c_str());	 
	}// if
	else{
		// If func is empty, get the current wavetype
		func = get_current_wavetype();
	}// else
   }// if
   // If the frequency is invalid, display a message to the user
   else{
	   cout << "<div style=\"color:red;\"> Please enter a positive numerical frequency value. </div>";
	   //current_freq = writeAndRead("getfrequency");
	   //usleep(1000000); // This delay is necessary to give the Arduino time
   }




  


   // Enable and Disable buttons call their own scripts
   cout << "<p>";
   cout << "<input type=\"submit\" value=\"Enable Output\" formaction=\"/cgi-bin/enable.cgi\" />";
   cout << "<input type=\"submit\" value=\"Disable Output\" formaction=\"/cgi-bin/disable.cgi\" />";
   cout << "</p>";

   // Select the waveform
   //if (func == "Sine"){
	   cout << "<div>Set Waveform: <input type=\"radio\" name=\"func\" value=\"Sine\""
		<< ( func=="Sine" ? "checked":"") << "/checked> Sine ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Triangle\""
		<< ( func=="Triangle" ? "checked":"") << "/> Triangle ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Square\""
		<< ( func=="Square" ? "checked":"") << "/> Square ";
   //}
   /*
   else if (func == "Triangle"){
	   cout << "<div>Set Waveform: <input type=\"radio\" name=\"func\" value=\"Sine\""
		<< "/" << ( func=="Sine" ? "checked":"") << "> Sine ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Triangle\""
		<< ( func=="Triangle" ? "checked":"") << "/checked> Triangle ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Square\""
		<< ( func=="Square" ? "checked":"") << "/> Square ";
   }
   else if (func == "Square"){
	   cout << "<div>Set Waveform: <input type=\"radio\" name=\"func\" value=\"Sine\""
		<< ( func=="Sine" ? "checked":"") << "/> Sine ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Triangle\""
		<< ( func=="Triangle" ? "checked":"") << "/> Triangle ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Square\""
		<< ( func=="Square" ? "checked":"") << "/checked> Square ";
   }
   else{
	   cout << "<div>Set Waveform: <input type=\"radio\" name=\"func\" value=\"Sine\""
		<< ( func=="Sine" ? "checked":"") << "/checked> Sine ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Triangle\""
		<< ( func=="Triangle" ? "checked":"") << "/> Triangle ";
	   cout << "<input type=\"radio\" name=\"func\" value=\"Square\""
		<< ( func=="Square" ? "checked":"") << "/> Square ";
   }
   */

   cout << "<input type=\"submit\" value=\"Set Output\" />";
   
   // Select Hz as default multiplier
   cout << "<p>Frequency: <input type=\"text\" name=\"freq\"/>";
   cout << " <input type=\"radio\" name=\"mult\" value=\"mHz\""
	<< (mult=="mHz" ? "checked":"") << "/> mHz ";
   cout << "<input type=\"radio\" name=\"mult\" value=\"Hz\""
	<< (mult=="Hz" ? "checked":"") << "/checked> Hz ";
   cout << "<input type=\"radio\" name=\"mult\" value=\"kHz\""
	<< (mult=="kHz" ? "checked":"") << "/> kHz ";
   cout << "<input type=\"radio\" name=\"mult\" value=\"MHz\""
	<< (mult=="MHz" ? "checked":"") << "/> MHz</p> ";
   cout << "</div>";
 
   	      	

   cout << "<b><h2><u> Current Settings </u>" << endl;

   cout << "<p> Frequency: " << freq_without_multiplier << " " << mult << "</p>" << endl;
   cout << "<p> Wave Type: " << func << "</p>" << endl;
  // cout << "<p> Wave Type: " << func << "</p>" << endl;
   cout << "<p> Output: " << outputStatus << "</p>" << endl;
   cout << "</b></h2>" << endl;
   cout << "</form>" << endl; // end of form

   // Refresh button
   // Start a new form
   cout << "<form action=\"/cgi-bin/controlAD9833.cgi\" method=\"POST\">\n";
   cout << "<input type=\"submit\" value=\"Refresh\" /> </p>";
   cout << "</form>" << endl; // end of form
   // End the HTML
   cout << body() << html();
   
   return 0;
}
