/* Simple send message example for communicating with the
*  UART that is connected to a desktop PC.  */

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include<iostream>
using namespace std;

int main(int argc, char *argv[]){
   int file, count;
   if(argc!=2){
       cout << "Please pass a message string to send, exiting!\n" << endl;
       return -2;
   }
   if ((file = open("/dev/ttyO4", O_RDWR | O_NOCTTY | O_NDELAY))<0){
      perror("UART: Failed to open the device.\n");
      return -1;
   }
   struct termios options;
   tcgetattr(file, &options);
   options.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
   options.c_iflag = IGNPAR | ICRNL;
   tcflush(file, TCIFLUSH);
   tcsetattr(file, TCSANOW, &options);

   // send the string plus the null character
   if ((count = write(file, argv[1], strlen(argv[1])+1))<0){
      perror("UART: Failed to write to the output.\n");
      return -1;
   }
   close(file);
   cout << "Finished sending the message, exiting.\n" << endl;
   return 0;
}
