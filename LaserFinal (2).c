#include "gpiolib_addr.h"
#include "gpiolib_reg.h"
#include "gpiolib_reg.c"

#include <stdint.h>
#include <stdio.h>		//for the printf() function
#include <fcntl.h>
#include <linux/watchdog.h> 	//needed for the watchdog specific constants
#include <unistd.h> 		//needed for sleep
#include <sys/ioctl.h> 		//needed for the ioctl function
#include <stdlib.h> 		//for atoi
#include <time.h> 		//for time_t and the time() function
#include <sys/time.h>           //for gettimeofday()
#include <errno.h>
#include <string.h>

#define PRINT_MSG(file, time, programName, str, int) \
	do{ \
			fprintf(file, "%s : %s : %s : %d", time, programName, str, int); \
			fflush(file); \
	}while(0)
	
#define PRINT_MSG_2(file, time, programName, str) \
do{ \
		fprintf(file, "%s : %s : %s", time, programName, str); \
		fflush(file); \
}while(0)

//HARDWARE DEPENDENT CODE BELOW

//this function will find the time
void getTime(char* buffer)
{
	//Create a timeval struct named tv
  	struct timeval tv;

	//Create a time_t variable named curtime
  	time_t curtime;

	//Get the current time and store it in the tv struct
  	gettimeofday(&tv, NULL); 

	//Set curtime to be equal to the number of seconds in tv
  	curtime=tv.tv_sec;

	//This will set buffer to be equal to a string that in
	//equivalent to the current date, in a month, day, year and
	//the current time in 24 hour notation.
  	strftime(buffer,40,"%m-%d-%Y  %T.",localtime(&curtime));

}

GPIO_Handle initializeGPIO(FILE* logFile, char time[], char programName[])
{
	GPIO_Handle gpio;
	gpio = gpiolib_init_gpio();
	if(gpio == NULL)
	{
		//perror("Could not initialize GPIO");
		getTime(time);
		PRINT_MSG_2(logFile, time, programName, "Could not initialize GPIO");
	}
	return gpio;	
}


/*This function will change the appropriate pins value in the select register so that the pin can function as an output
void setToOutput(GPIO_Handle gpio, int pinNumber, FILE* logFile, char time[], char programName[])
{

	//Check that the gpio is functional
	if(gpio == NULL)
	{
		getTime(time);
		PRINT_MSG_2(logFile, time, programName, "The GPIO has not been intitialized properly \n");
		//printf("The GPIO has not been intitialized properly \n"); //use definition?
		return;
	}

	//Check that we are trying to set a valid pin number
	if(pinNumber < 2 || pinNumber > 27)
	{
		getTime(time);
		PRINT_MSG_2(logFile, time, programName, "Not a valid pinNumber \n");
		//printf("Not a valid pinNumber \n");
		return;
	}

	//This will create a variable that has the appropriate select register number. 
	int registerNum = pinNumber / 10;

	//This will create a variable that is the appropriate amount that the 1 will need to be shifted by to set the pin to be an output
	int bitShift = (pinNumber % 10) * 3;

	uint32_t sel_reg_pin_number = gpiolib_read_reg(gpio, GPFSEL(registerNum));
	sel_reg_pin_number |= 1  << bitShift;
	gpiolib_write_reg(gpio, GPFSEL(registerNum), sel_reg_pin_number);
	
	//Log that pin has been set to an output
	PRINT_MSG(logFile, time, programName, "Pin %d has been set to output\n\n", pinNumber);
	
}*/

//This function should accept the diode number (1 or 2) and output
//a 0 if the laser beam is not reaching the diode, a 1 if the laser
//beam is reaching the diode or -1 if an error occurs.

#define LASER1_PIN_NUM 4 //This will replace LASER1_PIN_NUM with 4 when compiled
#define LASER2_PIN_NUM 18//This will replace LASER2_PIN_NUM with 18 when compiled

int laserDiodeStatus(GPIO_Handle gpio, int diodeNumber)
{
	if(gpio == NULL)
	{
		return -1;
	}

	if(diodeNumber == 1)
	{
		uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));

		if(level_reg & (1 << LASER1_PIN_NUM))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	else if(diodeNumber == 2)
	{
		uint32_t level_reg = gpiolib_read_reg(gpio, GPLEV(0));

		if(level_reg & (1 << LASER2_PIN_NUM))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	
	else
	{
		return -1;
	}
	
}

//This function will make an output pin output 3.3V. 
void outputOn(GPIO_Handle gpio, int pinNumber)
{
	gpiolib_write_reg(gpio, GPSET(0), 1 << pinNumber);
}

//This function will make an output pin turn off. 
void outputOff(GPIO_Handle gpio, int pinNumber)
{
	gpiolib_write_reg(gpio, GPCLR(0), 1 << pinNumber);
}

//END OF HARDWARE DEPENDENT CODE

//////////////////////////////////////////////

//State machine for reading the config file

int compare(char buffer[])
{
	int i = 0;
	char wd[18] = {"WATCHDOG_TIMEOUT "};
	char lf[9] = {"LOGFILE "};
	char sf[11] = {"STATSFILE "};

	int size = 0;


	//printf("%s\n", buffer);

	/*while(buffer[i] != 0)
	{
		printf("%s\n", buffer[i]);
		i++;
	}

	i = 0;*/
	while(buffer[i] != '=')
	{
		//printf("%s\n", buffer);
		size++;
		i++;
	}

	//size = size+1;

	int j = 0;
	if (buffer[j] == 'W')
	{
		for (int i = 0; i< size ; i++)
	{
		if (buffer[i] != wd[i])	
		{
			fprintf(stderr,"1.1\n");
			return 0;
		}

		//continue;
	}
		return 1;
	}	
	else if(buffer[j] == 'L')
	{
		for (int i = 0; i< size; i++)
	{
		if (buffer[i] != lf[i])
		{
			fprintf(stderr,"1.2\n");
			return 0;
		}
		continue;
	}
	return 2;
	}

	else if (buffer[j] == 'S')
	{
	for (int i = 0; i< size; i++)
		{
		if (buffer[i] != sf[i])
		{
			fprintf(stderr,"1.3\n");
			return 0;
		}
		}
		return 3;
	}	
	else
	{
		fprintf(stderr,"1.4\n");
		return 0;
	}
}


void readConfig(FILE* configFile, int* timeout, char* logFileName, char* StatsFileName) //char time1[], char programName[])
{
	enum STATE {START, HASH, NOTHASH, WATCHDOG, LOGFILE, STATSFILE, DONE, ERROR, INPUTLOG, INPUTSTAT, INPUTWD};
	int i = 0; 
	char buffer[255];
	*timeout = 10;
	int j = 0;
	int k = 0;
	int ans = 0;

	enum STATE s = START;

	char *end_of_file = fgets(buffer, 255, configFile);

	while(end_of_file != NULL)
{
	//*end_of_file = fgets(buffer, 255, configFile);
	
	switch (s)
	{
		case START:
			if (buffer[i] == '#')
			{
				end_of_file = fgets(buffer, 255, configFile);
				s = HASH;
			}
		
			else
			{
				s = NOTHASH;
			}
			break;

		case HASH:
			if (buffer[i] == '#')
			{
				//fgets((buffer, 255, configFile)); //how to call the next line?
				//s = HASH;
				end_of_file = fgets(buffer, 255, configFile);
				s = HASH;
			}

			else if (buffer[i] = '\n')
			{
				printf("new line");
				end_of_file = fgets(buffer, 255, configFile);
				s = HASH;
			}
			
			else
			{
				s = NOTHASH;
			}

			break;

		case NOTHASH:



			printf("%s\n", buffer);

			ans = compare(buffer); //just buffer?

			if (ans == 1)
			{
				i++;
				s = WATCHDOG;
			}

			else if(ans == 2)
			{
				i++;
				s = LOGFILE;
			}

			else if (ans == 3)
			{
				i++;
				s= STATSFILE;
			}

			else if (ans == 0)
			{
				fprintf(stderr,"1\n");
				s = ERROR;
			}

			break;

		case WATCHDOG:
			if (buffer[i] == '=') 
			{
				i++;
				s = INPUTWD;
			}

			else if (buffer[i] == 0) //incase its the last item and = is still not find  
			{
				*timeout = 10; //default timeout
				s = DONE;
			}

			else
			{
				i++;    //keep looping till you find the = or NULL
				s = WATCHDOG;
			}
			
			break;

		case LOGFILE:
			if (buffer[i] == '=')
			{
				i++;
				s = INPUTLOG;
			}

			else if (buffer[i] == 0) //in case reaches NULL and = still not found
			{
				char log[100] = {"/home/pi/LaserFinal.log"};
				int i = 0;
				while (log[i] != '\0')
				{
					logFileName[i] = log[i];
					i++;
				}

				logFileName[i] = '\0';
				s = DONE;
			}

			else 
			{
				i++;             //keep looping till  you find the = or NULL
				s = LOGFILE;
			}
			break;

		case STATSFILE:
			if (buffer[i] == '=')
			{
				i++;
				s = INPUTSTAT;
			}

			else if (buffer[i] == 0)   //in case reaches NULL and = still not found
			{
				char stats [100]= {"/home/pi/LaserFinal.stats"};
				int j = 0;
				while(stats[j] != 0)
				{
					StatsFileName[j] = stats[j];
					j++;
				}

				StatsFileName[j] = '\0';

				 s= DONE;
			}

			else 
			{
				i++;
				s = STATSFILE;     //keep looping till  you find the = or NULL
			}
			break;

		case INPUTLOG:
			if (buffer[i] != ' ' && buffer[i] != '=') //add all data into the list
			{
				logFileName[j] = buffer[i];
				i++;
				j++;

				s = INPUTLOG;
			}

			else if (buffer[i] == ' ') //ignore whitespace
			{
				i++;
				s = INPUTLOG;
			}

			else if (buffer[i] == 0) //if it reaches end of line, then its done
			{
				logFileName[j] = 0; //add null terminator at end
				s = DONE;
			}

			else 
			{
				fprintf(stderr,"2\n");
				s = ERROR;
			}
			
			break;

		case INPUTSTAT:
			if (buffer[i] != ' ' && buffer [i] != '=')  //insert data into location pointer
			{
				StatsFileName[k] = buffer[i];
				i++;
				k++;
				s = INPUTSTAT;
			}
			else if (buffer[i] == ' ') //ignore whitespace
			{ 
				i++;
				s = INPUTSTAT;
			}

			else if(buffer[i] == 0) //once it reaches end, hopefully all data is inserted in
			{
				StatsFileName[k] = 0; //add null terminator at end
				end_of_file = fgets(buffer, 255, configFile);
				s = HASH;
			}

			else 
			{
				fprintf(stderr,"3\n");
				s = ERROR;
			}
			
			break;

		case INPUTWD:
			if (buffer[i] == ' ')
			{
				i++;
				s = INPUTWD;
			}

			else if(buffer[i] >= '0' && buffer[i] <= '9')
			{
				*timeout = (*timeout *10) + (buffer[i] - '0');
				i++;
				s = INPUTWD;
			}

			else if (buffer[i] == '\0')
			{
				end_of_file = fgets(buffer, 255, configFile);
				s = HASH;
			}

			else 
			{
				fprintf(stderr,"4\n");
				s = ERROR;
			}
			
			break;

		case DONE:
		end_of_file = fgets(buffer, 255, configFile);
		break;
		//get new line

		case ERROR:
		//getTime(time1);
		//FILE* logfile;
		
		//logfile = fopen(logFileName,"a");
		//break; 
		fprintf(stderr, "There's an error in readconf");
		s = DONE;
		break;
		//PRINT_MSG_2(logFile, time1, programName, "An error within the state machine has occurred.");
//we can't do this because we access logfile after 
		
		//do the printmessage one 

		default:
		break;
	}

}
}



/////////////////////////////////////////////////

//This function will output the number of times each laser was broken
//and it will output how many objects have moved into and out of the room.

//laser1Count will be how many times laser 1 is broken (the left laser).
//laser2Count will be how many times laser 2 is broken (the right laser).
//numberIn will be the number  of objects that moved into the room.
//numberOut will be the number of objects that moved out of the room.
void outputMessage(int laser1Count, int laser2Count, int numberIn, int numberOut, char time[], FILE* statsFile, char programName[])
{
	PRINT_MSG(statsFile, time, programName, "Laser 1 was broken %d times \n", laser1Count);
	PRINT_MSG(statsFile, time, programName, "Laser 2 was broken %d times \n", laser2Count);
	PRINT_MSG(statsFile, time, programName, "%d objects entered the room \n", numberIn);
	PRINT_MSG(statsFile, time, programName, "%d objects exited the room \n", numberOut);
	PRINT_MSG_2(statsFile, time, programName, "The program is still running\n");
	/*printf("Laser 1 was broken %d times \n", laser1Count);
	printf("Laser 2 was broken %d times \n", laser2Count);
	printf("%d objects entered the room \n", numberIn);
	printf("%d objects exited the room \n", numberOut);*/
}

//This function accepts an errorCode. You can define what the corresponding error code will be for each type of error that may occur.
void errorMessage(int errorCode)
{
	fprintf(stderr, "An error occured; the error code was %d \n", errorCode);
}

enum State { START1, BOTH_NOT_BROKEN, LASER1_BROKEN_FIRST, LASER2_BROKEN_FIRST, ONLY_LASER1_BROKEN, ONLY_LASER2_BROKEN, BOTH_BROKEN, LASER1_NOT_BROKEN, LASER2_NOT_BROKEN };//LASER1_NOT_BROKEN, LASER2_NOT_BROKEN LASER1_BROKEN, LASER2_BROKEN,

int laser(int pin_state_1, int pin_state_2, GPIO_Handle gpio, int *laser1Count, int *laser2Count, int *numberIn, int *numberOut, FILE* logFile, char time1[], char programName[], int watchdog, FILE* statsFile)
{	
	pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
	pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
	
	*laser1Count = 0;
	*laser2Count = 0;
	
	*numberIn = 0;
	*numberOut = 0;
	
	int left = 0;
	int right = 0;
	
	enum State s = START1;
	
	time_t startTime = time(NULL);
	//time_t startTime = getTime(time);
	time_t timeLimit = 60;
	
	//In the while condition, we check the current time minus the start time and
	//see if it is less than the number of seconds that was given from the 
	//command line
	//while((time(NULL) - startTime) < timeLimit)
	while(1)
	{ 
		//char input = getNextinput(data, i); 
		// Process transitions
		//printf("%d", gpiolib_read_reg(gpio, GPLEV(0)));
		
		if((time(NULL) - startTime) >= timeLimit)
		{
			outputMessage(*laser1Count, *laser2Count, *numberIn, *numberOut, time1, statsFile, programName);
			startTime = time(NULL);
		}
			
		switch(s)
		{ 
			case START1:
				printf("Start\n");
				printf("pin_state_1: %d\n", pin_state_1);
				printf("pin_state_2: %d\n", pin_state_2);
				if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					s = BOTH_NOT_BROKEN;	
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 0)
				{
					s = LASER1_NOT_BROKEN;
				}
				
				else if(pin_state_2 == 1 && pin_state_1 == 0)
				{
					s = LASER2_NOT_BROKEN;
				}
				
				else
				{
					s = BOTH_BROKEN;
				}
				
				break;

			case BOTH_NOT_BROKEN:
				//printf("Both not broken\n");
				if(pin_state_1 == 0 && pin_state_2 == 1)
				{
					++*laser1Count;
					++left;
					printf("laser1Count : %d\n", *laser1Count);
					s = LASER1_BROKEN_FIRST;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 0)
				{
					++*laser2Count;
					++right;
					printf("laser2Count : %d\n", *laser2Count);
					s = LASER2_BROKEN_FIRST;
				}
				
				else if(pin_state_1 == 0 && pin_state_2 == 0)
				{
					s = BOTH_BROKEN;
				}
				
				else
				{
					//printf("something\n");
					//return -1;
					s = BOTH_NOT_BROKEN;
				}
				
				break;
				
			case LASER1_BROKEN_FIRST:
				//printf("Laser1 broken first\n");
				if(pin_state_1 == 0 && pin_state_2 == 0)
				{
					++*laser2Count;
					++right;
					printf("laser2Count : %d\n", *laser2Count);
					s = BOTH_BROKEN;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					s = BOTH_NOT_BROKEN;
				}
				
				else
				{
					s = LASER1_BROKEN_FIRST;
				}
	
				break;
				
			case LASER2_BROKEN_FIRST:
				//printf("Laser2 broken first\n");
				if(pin_state_1 == 0 && pin_state_2 == 0)
				{
					++*laser1Count;
					++left;
					printf("laser1Count : %d\n", *laser1Count);
					s = BOTH_BROKEN;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					s = BOTH_NOT_BROKEN;
				}
				
				else
				{
					s = LASER2_BROKEN_FIRST;
				}
	
				break;
				
			case ONLY_LASER2_BROKEN:
				//printf("Only laser2 broken\n");
				if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					++*numberIn;
					printf("numberIn : %d\n", *numberIn);
					s = START1;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 0)
				{
					s = ONLY_LASER2_BROKEN;
				}
				
				else if(pin_state_1 == 0 && pin_state_2 == 0)
				{
					++*laser1Count; //edited
					printf("laser1Count : %d\n", *laser1Count);
					s = BOTH_BROKEN;
				}
				
				else
				{
					s = BOTH_NOT_BROKEN;
				}
				
				break;
				
			case ONLY_LASER1_BROKEN:
				//printf("Only laser1 broken\n");
				if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					++*numberOut;
					printf("numberOut : %d\n", *numberOut);
					s = START1;
				}
				
				else if(pin_state_1 == 0 && pin_state_2 == 1)
				{
					s = ONLY_LASER1_BROKEN;
				}
				
				else if(pin_state_1 == 0 && pin_state_2 == 0)
				{
					++*laser2Count; //edited
					printf("laser2Count : %d\n", *laser2Count);
					s = BOTH_BROKEN;
				}
				
				else
				{
					s = BOTH_NOT_BROKEN;
				}
				
				break;
				
			case LASER1_NOT_BROKEN:
				//printf("lASER1 not broken\n");
				if(pin_state_1 == 0 && pin_state_2 == 0)
				{
					++*laser1Count;
					printf("laser1Count : %d\n", *laser1Count);
					s = BOTH_BROKEN;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					s = BOTH_NOT_BROKEN;
					
				}
				
				else
				{
					s = LASER1_NOT_BROKEN;
				}
	
				break;
			
			case LASER2_NOT_BROKEN:
				//printf("laser2 not broken\n");
				if(pin_state_1 == 0 && pin_state_2 == 0)
				{
					++*laser2Count;
					printf("laser2Count : %d\n", *laser2Count);
					s = BOTH_BROKEN;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					s = BOTH_NOT_BROKEN;
					
				}
				
				else if(pin_state_1 == 0 && pin_state_2 == 1)
				{
					s = ONLY_LASER1_BROKEN;
					
				}
				
				else
				{
					s = LASER2_NOT_BROKEN;
				}
	
				break;
				
			case BOTH_BROKEN:
				//printf("Both broken\n");
				if(left == right && pin_state_1 == 0 && pin_state_2 == 1)
				{
					s = ONLY_LASER1_BROKEN;
				}
				
				else if(left == right && pin_state_1 == 1 && pin_state_2 == 0)
				{
					s = ONLY_LASER2_BROKEN;
				}
				
				else if(pin_state_1 == 0 && pin_state_2 == 1)
				{
					s = LASER2_NOT_BROKEN;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 0)
				{
					s = LASER1_NOT_BROKEN;
				}
				
				else if(pin_state_1 == 1 && pin_state_2 == 1)
				{
					s = BOTH_NOT_BROKEN;
				}
				
				else 
				{
					s = BOTH_BROKEN;
				}
	
				break;

			//case DONE:
				//break;
				
			default:
				return -1;
				break;
			//}
		}
		
		// Process actions on entering the state
		switch (s) 
		{ //while(i < size - 1){
			case START1:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
			
			case BOTH_NOT_BROKEN:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
			
			case BOTH_BROKEN:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
				
			case LASER1_BROKEN_FIRST:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
				
			case LASER2_BROKEN_FIRST:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
				
			case LASER1_NOT_BROKEN:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
				
			case LASER2_NOT_BROKEN:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
				
			case ONLY_LASER1_BROKEN:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time, programName, "The Watchdog was kicked\n");
				break;
				
			case ONLY_LASER2_BROKEN:
				pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
				pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
				ioctl(watchdog, WDIOC_KEEPALIVE, 0);
				getTime(time1);
				//Log that the Watchdog was kicked
				PRINT_MSG_2(logFile, time1, programName, "The Watchdog was kicked\n");
				break;
			  
			default:
				return -1;
				break;
		} 
	} 
}


int main(const int argc, const char* const argv[])
{
	//We want to accept a command line argument that will be the number
	//of seconds that the program will run for, so we need to ensure that
	//the user actually gives us a time to run the program for
	/*if(argc < 2)
	{
		printf("Error, no time given: exiting\n");
		return -1;
	}*/
	
	//Create a string that contains the program name
	const char* argName = argv[0];

	//These variables will be used to count how long the name of the program is
	int i = 0;
	int namelength = 0;

	while(argName[i] != 0)
	{
		namelength++;
		i++;
	} 

	char programName[namelength];

	i = 0;

	//Copy the name of the program without the ./ at the start
	//of argv[0]
	while(argName[i + 2] != 0)
	{
		programName[i] = argName[i + 2];
		i++;
	} 	
	
	/////////////////////////////////////////////////
	
	//Declare the variables that will be passed to the readConfig function
	int timeout;
	char logFileName[50];
	char statsFileName[50];
	
	//Create a file pointer named configFile
	FILE* configFile;
	//Set configFile to point to the Lab4Sample.cfg file. It is set to read the file.
	configFile = fopen("/home/pi/LaserFinal.cfg", "r");

	//Output a warning message if the file cannot be opened
	if(!configFile)
	{
		timeout = 15;
		char log[100] = {"/home/pi/LaserFinal.log"};
		int i = 0;
		while(log[i] != 0)
		{
			logFileName[i] = log[i];
			i++;
		}

		logFileName[i] = '\0';
		char stats [100]= {"/home/pi/LaserFinal.stats"};

		int j = 0;
		while(stats[j] != 0)
		{
			statsFileName[j] = stats[j];
			j++;
		}

		statsFileName[j] = '\0';

		perror("The config file could not be opened");
		//return -1;
	}
	
	else
	{
	
	fprintf(stderr,"at config read\n");	//Call the readConfig function to read from the config file
	readConfig(configFile, &timeout, logFileName, statsFileName);// time, programName);
	//Close the configFile now that we have finished reading from it
	fclose(configFile);

	}
	

	//Create a new file pointer to point to the log file
	FILE* logFile;
	//Set it to point to the file from the config file and make it append to
	//the file when it writes to it.
	logFile = fopen(logFileName, "a");
	//fprintf(stderr, " Could not open %s (errno %d)\n ", logFileName, errno); 

	//Check that the file opens properly.
	if(!logFile)
	{
		char log[100] = {"/home/pi/LaserFinal.log"};
		int i = 0;
		while(log[i] != 0)
		{
			logFileName[i] = log[i];
			i++;
		}

		logFileName[i] = '\0';

		logFile = fopen(logFileName, "a");
		if (!logFile)
		  fprintf(stderr, "Could not open \"%s\" (errno %d: %s)\n ", logFileName, errno, strerror(errno)); 
		else
			fprintf(stderr,"The log file has been opened\n");

		//PRINT_MSG_2(logFile, time, programName, "The log file could not be opened");
		//printf("The %s log file could not be opened\n\n", logFileName);
		//return -1;
	}
	
	else
	{
		fprintf(stderr,"The log file has been!! opened\n");
		//PRINT_MSG_2(logFile, time, programName, "The log file has been opened\n");
	}
	
	fprintf(stderr,"I've reached here\n");
	
	//Create a new file pointer to point to the stats file
	FILE* statsFile;
	//Set it to point to the file from the config file and make it append to
	//the file when it writes to it.
	
	
	statsFile = fopen(statsFileName, "a");

	//Check that the file opens properly.
	if(!statsFile)
	{
		char stats [100]= {"/home/pi/LaserFinal.stats"};

		int j = 0;
		while(stats[j] != 0)
		{
			statsFileName[j] = stats[j];
			j++;
		}

		statsFileName[j] = '\0';
		PRINT_MSG_2(logFile, time, programName, "The stats file could not be opened\n\n");
		
		statsFile = fopen(statsFileName, "a");
		if (!statsFile)
		  fprintf(stderr, "Could not open \"%s\" (errno %d: %s)\n ", statsFileName, errno, strerror(errno)); 
		else
			fprintf(stderr,"The stats file has been opened\n");
		//return -1;
	}
	
	else
	{
		fprintf(stderr,"The stats file has been!! opened\n");
		PRINT_MSG_2(logFile, time, programName, "The stats file has been opened\n");
	}

	//Create a char array that will be used to hold the time values
	char time[30];
	getTime(time);
	

	
	///////////////////////////////////////////////////////////
	
	//Initialize the GPIO pins
	GPIO_Handle gpio = initializeGPIO(logFile, time, programName);
	//Log that the GPIO pins have been initialized
	PRINT_MSG_2(logFile, time, programName, "The GPIO pins have been initialized\n");
	
	//Get the current time
	getTime(time);
	

	/*Need to set two pins as output pins - pin 17 and pin 18 	
	Set pin 17 as an output pin
	setToOutput(gpio, 17);
	Set pin 18 as an output pin
	setToOutput(gpio, 18);	
	*/
	
	//Create a variable that represents the initial start time
	//time_t is a special variable type used by the time library for calendar times
	//the time() function will return the current calendar time. You will need to put
	//NULL as the argument for the time() function
	//time_t startTime = time(NULL);

	//This variable will represent the amount of time the program should run for
	//int timeLimit = atoi(argv[1]);


	int pin_state_1 = laserDiodeStatus(gpio, 1);	//left diode
	//getTime(time);
	int pin_state_2 = laserDiodeStatus(gpio, 2);	//right diode
	//getTime(time);
	
	/////////////////////////////////////////
	
	//This variable will be used to access the /dev/watchdog file, similar to how
	//the GPIO_Handle works
	int watchdog;

	//We use the open function here to open the /dev/watchdog file. If it does
	//not open, then we output an error message. We do not use fopen() because we
	//do not want to create a file if it doesn't exist
	if ((watchdog = open("/dev/watchdog", O_RDWR | O_NOCTTY)) < 0) 
	{
		printf("Error: Couldn't open watchdog device! %d\n", watchdog);
		return -1;
	} 
	
	//Get the current time
	getTime(time);
	//Log that the watchdog file has been opened
	PRINT_MSG_2(logFile, time, programName, "The Watchdog file has been opened\n\n");
	
	//This line uses the ioctl function to set the time limit of the watchdog
	//timer to 15 seconds. The time limit can not be set higher that 15 seconds
	//so please make a note of that when creating your own programs.
	//If we try to set it to any value greater than 15, then it will reject that
	//value and continue to use the previously set time limit
	ioctl(watchdog, WDIOC_SETTIMEOUT, &timeout);
	
	//Get the current time
	getTime(time);
	//Log that the Watchdog time limit has been set
	PRINT_MSG_2(logFile, time, programName, "The Watchdog time limit has been set\n\n");

	//The value of timeout will be changed to whatever the current time limit of the
	//watchdog timer is
	ioctl(watchdog, WDIOC_GETTIMEOUT, &timeout);

	//This print statement will confirm to us if the time limit has been properly
	//changed. The \n will create a newline character similar to what endl does.
	//printf("The watchdog timeout is %d seconds.\n\n", timeout);
	PRINT_MSG(logFile, time, programName, "The watchdog timeout is %d seconds.\n\n", timeout);
	
	///////////////////////////////////////////
	
	int laser1Count = 0;
	int laser2Count = 0;
	int numberIn = 0;
	int numberOut = 0;
	//This function should initialize the GPIO pins

	
	laser(pin_state_1, pin_state_2, gpio, &laser1Count, &laser2Count, &numberIn, &numberOut, logFile, time, programName, watchdog, statsFile);
	outputMessage(laser1Count, laser2Count, numberIn, numberOut, time, statsFile, programName);
	
	PRINT_MSG_2(logFile, time, programName, "The program shouldn't have reached here\n\n");
	
	/*printf("Laser 1 was broken %d times \n", laser1Count);
	printf("Laser 2 was broken %d times \n", laser2Count);
	printf("%d objects entered the room \n", numberIn);
	printf("%d objects exitted the room \n", numberOut);*/

	//Turn off the LED before the program ends
	//outputOff(gpio, 17);
	
	//Writing a V to the watchdog file will disable to watchdog and prevent it from
	//resetting the system
	write(watchdog, "V", 1);
	getTime(time);
	//Log that the Watchdog was disabled
	PRINT_MSG_2(logFile, time, programName, "The Watchdog was disabled\n\n");

	//Close the watchdog file so that it is not accidentally tampered with
	close(watchdog);
	getTime(time);
	//Log that the Watchdog was closed
	PRINT_MSG_2(logFile, time, programName, "The Watchdog was closed\n\n");

	//Free the gpio pins
	gpiolib_free_gpio(gpio);
	getTime(time);
	//Log that the GPIO pins were freed
	PRINT_MSG_2(logFile, time, programName, "The GPIO pins have been freed\n\n");

	//Return to end the program
	return 0;	
}


