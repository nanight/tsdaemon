#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "ftd2xx.h"
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>

#include <CoreServices/CoreServices.h>
//#include <ApplicationServices/ApplicationServices.h>
#include <signal.h>
#include <launch.h>
#include <asl.h>

#define BUF_SIZE 100
#define MAX_DEVICES	4
#define MAX_RETRIES 10
char * 	pcBufRead = NULL;

int createNexaString(const char *pHouseStr, const char *pChannelStr, 
					 const char *pOn_offStr, char * pTxStr, int waveman);


// This is a callback to handle SIGTERM and SIGINT signals
void sigtermHandler(int sig)
{
	CFRunLoopRef rl = CFRunLoopGetCurrent();
	if (rl == NULL)
		exit(1); // something when wrong. Better just exit
	else
		CFRunLoopStop(rl);
}

int sendSomething(char *strMessage, FT_HANDLE *fthHandle) {
	int intReturn = -1;
//	FT_HANDLE fthHandle = 0;
	FT_STATUS ftStatus = FT_OK;
	
	DWORD dwNumberOfDevices = 0;
	
	FT_SetVIDPID(0x1781, 0x0C30);
	
	ftStatus = FT_CreateDeviceInfoList(&dwNumberOfDevices);
	if (ftStatus == FT_OK) { 
		for (int i = 0; i < (int)dwNumberOfDevices; i++) {  
			
			FT_PROGRAM_DATA pData;
			char ManufacturerBuf[32]; 
			char ManufacturerIdBuf[16]; 
			char DescriptionBuf[64]; 
			char SerialNumberBuf[16]; 
			
			pData.Signature1 = 0x00000000; 
			pData.Signature2 = 0xffffffff; 
			pData.Version = 0x00000002;      // EEPROM structure with FT232R extensions 
			pData.Manufacturer = ManufacturerBuf; 
			pData.ManufacturerId = ManufacturerIdBuf; 
			pData.Description = DescriptionBuf; 
			pData.SerialNumber = SerialNumberBuf; 
			
			ftStatus = FT_Open(i, fthHandle);
			ftStatus = FT_EE_Read(*fthHandle, &pData);
			if(ftStatus == FT_OK){
				if(pData.VendorId == 6017 && pData.ProductId == 3120){
					intReturn = i;
					if(FT_SetBaudRate(*fthHandle, 4800) != FT_OK) {
						printf("Error setting device baud rate\n");
						return 0;
					}

					ULONG bytesWritten;
					if(ftStatus = FT_Write(*fthHandle, strMessage, strlen(strMessage)+1, &bytesWritten) != FT_OK) {
						printf("Error writing to device\n");
						return 0;
					}
					
					//					printf("Device %d, %s, Serial Number - %s \n", i, pData.Description, pData.SerialNumber);
					//ftStatus = FT_Close(fthHandle);
					break;
				}
			}
			ftStatus = FT_Close(*fthHandle);
		}
	}
	return 1;
}

int getDongleIndex(){
	int intReturn = -1;
	FT_HANDLE fthHandle = 0;
	FT_STATUS ftStatus = FT_OK;
	
	DWORD dwNumberOfDevices = 0;
	
	FT_SetVIDPID(0x1781, 0x0C30);
	
	ftStatus = FT_CreateDeviceInfoList(&dwNumberOfDevices);
	if (ftStatus == FT_OK) { 
		for (int i = 0; i < (int)dwNumberOfDevices; i++) {  
			
			FT_PROGRAM_DATA pData;
			char ManufacturerBuf[32]; 
			char ManufacturerIdBuf[16]; 
			char DescriptionBuf[64]; 
			char SerialNumberBuf[16]; 
			
			pData.Signature1 = 0x00000000; 
			pData.Signature2 = 0xffffffff; 
			pData.Version = 0x00000002;      // EEPROM structure with FT232R extensions 
			pData.Manufacturer = ManufacturerBuf; 
			pData.ManufacturerId = ManufacturerIdBuf; 
			pData.Description = DescriptionBuf; 
			pData.SerialNumber = SerialNumberBuf; 
			
			ftStatus = FT_Open(i, &fthHandle);
			ftStatus = FT_EE_Read(fthHandle, &pData);
			if(ftStatus == FT_OK){
				if(pData.VendorId == 6017 && pData.ProductId == 3120){
					intReturn = i;
//					printf("Device %d, %s, Serial Number - %s \n", i, pData.Description, pData.SerialNumber);
					ftStatus = FT_Close(fthHandle);
					break;
				}
			}
			ftStatus = FT_Close(fthHandle);
		}
	}
	return intReturn;
}

int send(char *strMessage, FT_HANDLE *fthHandle){
	
	FT_STATUS ftStatus = FT_OK;
	
	int intDongleIndex = getDongleIndex();
	if (intDongleIndex < 0) {
		return -1;
	}
	
	ftStatus = FT_Open(intDongleIndex, fthHandle);
	// Set baudRate
	if(FT_SetBaudRate(*fthHandle, 4800) != FT_OK) {
		printf("Error setting device baud rate\n");
		return -1;
	}
	
	ULONG bytesWritten;
	
//	char *tempMessage = (char *)malloc(sizeof(char) * (strMessage.size()+1));
//	strcpy(tempMessage, strMessage.c_str());
	
//	printf("test \n");
//	printf("%i \n", strlen(strMessage));
//	printf("%s \n", strMessage);
	
	if(ftStatus = FT_Write(*fthHandle, strMessage, strlen(strMessage)+1, &bytesWritten) != FT_OK) {
		printf("Error writing to device\n");
		return -1;
	}
	//	free(tempMessage);
	
	return 1;
}


/*
 int CheckConfig(DWORD * iVID, DWORD * iPID, const char * args[]) {
	 char cVID[5];
	 char cPID[5];
	 
	 strcpy(cVID, args[1]);
	 strcpy(cPID, args[2]);
	 
	 sscanf(cVID, "%X\n", iVID);
	 sscanf(cPID, "%X\n", iPID);
	 
	 return 1;
 }
 

FT_STATUS my_FT_ListDevices(PVOID pArg1, PVOID pArg2, DWORD Flags, const char * args[]) {
	FT_STATUS	ftStatus;
	DWORD iVID, iPID;
	DWORD iOldVID, iOldPID;
	
	CheckConfig(&iVID, &iPID, args);		// get our VID and PID from config file or other source
	FT_GetVIDPID(&iOldVID, &iOldPID);	// get original VID and PID	
	FT_SetVIDPID(iVID, iPID);							// use our VID and PID
	ftStatus = FT_ListDevices(pArg1, pArg2, Flags);		// Call FTDI function
	FT_SetVIDPID(iOldVID, iOldPID);						// restore original VID and PID
	
	return ftStatus;
}

FT_STATUS my_FT_Open(FT_HANDLE *pHandle, const char * args[]) {
	FT_STATUS	ftStatus;
	DWORD iVID, iPID;
	DWORD iOldVID, iOldPID;
	
	CheckConfig(&iVID, &iPID, args);		// get our VID and PID from config file or other source
	FT_GetVIDPID(&iOldVID, &iOldPID);	// get original VID and PID
	FT_SetVIDPID(iVID, iPID);							// use our VID and PID

	ftStatus = FT_OpenEx("Homeautomation USB-Dongle", FT_OPEN_BY_DESCRIPTION, pHandle);

//	ftStatus = FT_OpenEx(deviceNumber, pHandle);
	FT_SetVIDPID(iOldVID, iOldPID);						// restore original VID and PID
	
	return ftStatus;	
}
*/
// Init Semaphore 
int initsem(key_t key, int nsems) {
    int i;
    union semun arg;
    struct semid_ds buf;
    struct sembuf sb;
    int semid;
	
    semid = semget(key, nsems, IPC_CREAT | IPC_EXCL | 0666);
	
    if (semid >= 0) { /* we got it first */
        sb.sem_op = 1; sb.sem_flg = 0;
        arg.val = 1;
		
//        printf("press return\n"); getchar();
		
        for(sb.sem_num = 0; sb.sem_num < nsems; sb.sem_num++) { 
            /* do a semop() to "free" the semaphores. */
            /* this sets the sem_otime field, as needed below. */
            if (semop(semid, &sb, 1) == -1) {
                int e = errno;
                semctl(semid, 0, IPC_RMID); /* clean up */
                errno = e;
                return -1; /* error, check errno */
            }
        }
		
    } else if (errno == EEXIST) { /* someone else got it first */
        int ready = 0;
		
        semid = semget(key, nsems, 0); /* get the id */
        if (semid < 0) {
//			printf("semget errno: %i\n", errno);
			return semid;
		}/* error, check errno */
		
        /* wait for other process to initialize the semaphore: */
        arg.buf = &buf;
        for(i = 0; i < MAX_RETRIES && !ready; i++) {
            semctl(semid, nsems-1, IPC_STAT, arg);
            if (arg.buf->sem_otime != 0) {
                ready = 1;
            } else {
                sleep(1);
            }
        }
        if (!ready) {
            errno = ETIME;
            return -1;
        }
    } else {
//		printf("semget errno: %i\n", errno);
        return semid; /* error, check errno */
    }
	
    return semid;
}

int main (int argc, const char * argv[]) {
	
	// Commandline use
	if( argc < 3 ){
		printf("Usage: \n");
		printf("tsdaemon -d <receiver type> <house> <unit> <level>\n");
		printf("Where \n"); 
		printf("<receiver type> = nexa or waveman, \n");
		printf("<house> = A - P, \n");
		printf("<unit> = 1 - 16 and \n");
		printf("<command> = 0 (off) or 1 (on) and \n\n");
		printf("<level> = 0 - 100, 0 = of, 1 and 100 = full on, 2-99 = dimming level, i.e. subsequent on - commands. \n");
		printf("Example: tsdaemon -d nexa A 1 1\n\n");
		exit(1);
	}
	
	// Ulterius use
	// tsdaemon <VID> <PID> <Command> <command> <level>
	// Example: tsdaemon 1718 0c30 S$kk$$k$k$k$k$k$k$k$k$k$k$k$k$k$k$k$k$kk$$kk$$k$k$}+ 50
	
	
	/*
	 * Create a semaphore
	 *
	 */   
	
	key_t key;
    int semid;
    struct sembuf sb;
	union semun arg;
    
    if ((key = ftok(argv[0], 10)) == -1) {
        perror("ftok");
        exit(1);
    }
	
//	printf("key: %i\n", key);
	
    /* grab the semaphore set created by initsem: */
    if ((semid = initsem(key, 1)) == -1) {
        perror("initsem");
        exit(1);
    }
	
  /*  printf("Press return to lock: ");
    getchar();
    printf("Waiting to lock semid: %i...\n", semid);
*/	
	sb.sem_num = 0;
    sb.sem_op = -1;  /* set to allocate resource */
    sb.sem_flg = SEM_UNDO;	
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
	
 //   printf("Semid: %i Locked\n", semid);

/*    printf("Press return to unlock: ");
    getchar();
*/	
	
	/*
	 * Create a new ASL log
	 *
	 */   
	
	aslclient asl = NULL;
	aslmsg log_msg = NULL;
//	int retval = EXIT_SUCCESS;
	launch_data_t checkin_request;
	
	/*
	 * Create a new ASL log
	 *
	 */   
	asl = asl_open("tsdaemon", "Daemon", ASL_OPT_STDERR);
	log_msg = asl_new(ASL_TYPE_MSG);
	asl_set(log_msg, ASL_KEY_SENDER, "tsdaemon");
	
	/*
	 * Register ourselves with launchd.
	 * 
	 */
	if ((checkin_request = launch_data_new_string(LAUNCH_KEY_CHECKIN)) == NULL) { 
		asl_log(asl, log_msg, ASL_LEVEL_ERR, "launch_data_new_string(\"" LAUNCH_KEY_CHECKIN "\") Unable to create string.");	
		asl_close(asl);
		return EXIT_FAILURE;
	}
	
	// as recommended for launchd daemons, we handle SEGTERM signals.
	// sigtermHandler just tells the runloop to stop. 
	signal(SIGTERM, &sigtermHandler);
	
//	printf("initializing tsdaemon... \n");
	if(argc < 3){
		printf("Usage: tsdaemon VID PID command\n");
		return 1;
	}
	
	char * 	pcBufLD[MAX_DEVICES + 1];
	char 	cBufLD[MAX_DEVICES][64];
//	DWORD 	dwBytesWritten;//, dwBytesRead;
//	DWORD	dwRxSize = 0;
//	LPDWORD lpdwAmountInRxQueue, lpdwAmountInTxQueue, lpdwEventStatus;

	FT_STATUS	ftStatus;
	FT_HANDLE	ftHandle0;
	int	i;//, iNumDevs;
	char txStr[BUF_SIZE];
	
	for(i = 0; i < MAX_DEVICES; i++) {
		pcBufLD[i] = cBufLD[i];
	}
	pcBufLD[MAX_DEVICES] = NULL;
	
	// Look for devices
/*	if(my_FT_ListDevices(pcBufLD, &iNumDevs, FT_LIST_ALL | FT_OPEN_BY_DESCRIPTION, argv) != FT_OK) {
		printf("Error: FT_ListDevices(%d)\n", ftStatus);
		return 1;
	}*/
	
	/*	// List devices
	 for(i = 0; ( (i <MAX_DEVICES) && (i < iNumDevs) ); i++) {
	 printf("\nDevice %d Serial Number - %s\n", i, cBufLD[i]);
	 }*/
	
	// If called from command line
	int numLoop = 1;
	char command;
	if(strcmp(argv[1], "-d") == 0) {
		if (!argv[5]){
			numLoop = 1;
			command = '1';
		}
		else if (strcmp(argv[5], "1") == 0 || strcmp(argv[5], "100") == 0){
			numLoop = 1;
			command = '1';
		}
		else if (strcmp(argv[5], "0") == 0) {
			numLoop = 1;
			command = '0';
		} else if ((int)*argv[5] > 1 && (int)*argv[5] < 100 ){
			numLoop = 2;
			command = '1';
		}		
		
		// Check Nexa/Waveman
		if (strcmp(argv[2], "nexa") == 0){
			createNexaString(argv[3], argv[4], &command, txStr, 0);
			argv[1] = "1781";
			argv[2] = "0c30";	
		}
		else if(strcmp(argv[2], "waveman") == 0){
			createNexaString(argv[3], argv[4], &command, txStr, 1);
			argv[1] = "1781";
			argv[2] = "0c30";
		}
	}
	else {
		if (!argv[4]) 
			numLoop = 1;
		else if (strcmp(argv[4], "100") == 0)
			numLoop = 1;
		else if (strcmp(argv[4], "0") == 0)
			numLoop = 1;
		else if ((int)*argv[4] > 0 && (int)*argv[4] < 100 )
			numLoop = 2;
		
		strcpy(txStr, argv[3]);
	}
	
/*	// Open device
	if(my_FT_Open(&ftHandle0, argv) != FT_OK) {
		printf("FT_Open(0) failed\n");
		// Try again
		return 1;
	}
	
	// Set baudRate
	if(FT_SetBaudRate(ftHandle0, 4800) != FT_OK) {
		printf("Error writing to device\n");
		return 1;
	}
	*/
	
	float length;
	for (int i = 0; i < numLoop; i++){
		
/*		// Send the command to the device
		if(FT_Write(ftHandle0, txStr, strlen(txStr)+1, &dwBytesWritten) != FT_OK) {
			printf("Error writing to device\n");
			return 1;
		}*/
		
		//send(txStr, &ftHandle0);
		sendSomething(txStr, &ftHandle0);
		
		length = (float)command*0.8f/100.0f;
		
		sleep(length);
		//	printf("dwBytesWritten: %d \n",dwBytesWritten );
	}
	
	if (strcmp(txStr, "V+") == 0) {
		
		// Read the Tellstick answer
		char RxBuffer[256];
		DWORD TxBytes, RxBytes, BytesReceived;
		DWORD EventDWord;
		
		printf(", ");
		sleep(1);
		FT_GetStatus(ftHandle0, &RxBytes, &TxBytes, &EventDWord); 
		if (RxBytes > 0) { 
			ftStatus = FT_Read(ftHandle0, RxBuffer, RxBytes, &BytesReceived); 
			if (ftStatus == FT_OK) { 
				printf("%s", RxBuffer);
			} 
			else { 
				printf("Error reading from the Tellstick \n");
			} 
		} 
		
		// Close the handler	
//		ftStatus = FT_Close(ftHandle0);
	}
	
	if(ftStatus = FT_Close(ftHandle0) != FT_OK) {
		printf("Error closing the device\n");
		return -1;
	}
	
	
	// Find out if this is the last process using the semaphores set
	int nsemLeft = semctl(semid, 0, GETNCNT, arg);
//	printf("Locked: nsemLeft: %i\n", nsemLeft);	
	
	sb.sem_op = 1; /* free resource */
  //  printf("Semid: %i Unlocking\n", semid);
    if (semop(semid, &sb, 1) == -1) {
        perror("semop");
        exit(1);
    }
	
	if (nsemLeft < 1) {
		
		/*
		 * Try to remove semaphore
		 *
		 */   
		
		// grab the semaphore set
		if ((semid = semget(key, 1, 0)) == -1) {
			return 0;
		}
		
		// remove the semaphore set: 
		if (semctl(semid, 0, IPC_RMID, arg) == -1) {
			perror("semctl");
			exit(1);
		}
	}	
	return 0;
}


int createNexaString(const char *pHouseStr, const char *pChannelStr, 
					 const char *pOn_offStr, char * pTxStr, int waveman) {
	* pTxStr = '\0'; /* Make sure tx string is empty */ 
	int houseCode;
	int channelCode;
	int on_offCode;
	int txCode = 0;
	const int unknownCode =  0x6;
	int bit;
	int bitmask = 0x0001;
	
	houseCode = (int)((*pHouseStr) - 65);	/* House 'A'..'P' */
	channelCode = atoi(pChannelStr) - 1;	/* Channel 1..16 */
	on_offCode =  atoi(pOn_offStr);	        /* ON/OFF 0..1 */
	
#ifdef RFCMD_DEBUG
	printf("House: %d, channel: %d, on_off: %d\n", houseCode, channelCode, on_offCode);
#endif
	
	/* check converted parameters for validity */
	if((houseCode < 0) || (houseCode > 15) ||      // House 'A'..'P'
	   (channelCode < 0) || (channelCode > 15) ||
	   (on_offCode < 0) || (on_offCode > 1))
	{
		
	}
	else
	{
		/* b0..b11 txCode where 'X' will be represented by 1 for simplicity. 
		 b0 will be sent first */ 
		txCode = houseCode;
		txCode |= (channelCode <<  4);
		if (waveman && on_offCode == 0) {
		} else {
			txCode |= (unknownCode <<  8);
			txCode |= (on_offCode  << 11); 
		}
		
		/* convert to send cmd string */
		strcat(pTxStr,"S");	
		for(bit=0;bit<12;bit++)
		{
			if((bitmask & txCode) == 0)
			{
				/* bit timing might need further refinement */
				//strcat(pTxStr," ` `"); /* 320 us high, 960 us low,  320 us high, 960 us low */
				strcat(pTxStr,"$k$k");  /* 360 us high, 1070 us low,  360 us high, 1070 us low */
			}
			else /* add 'X' (floating bit) */
			{
				//strcat(pTxStr," `` "); /* 320 us high, 960 us low, 960 us high,  320 us low */
				strcat(pTxStr,"$kk$");  /* 360 us high, 1070 us low, 1070 us high,  360 us low */
			}
			bitmask = bitmask<<1;
		}
		/* add stop/sync bit and command termination char '+'*/
		//strcat(pTxStr," }+");
		strcat(pTxStr,"$}+");
	}
	
//	printf("txCode: %04X\n", txCode);
	
	return strlen(pTxStr);
}
