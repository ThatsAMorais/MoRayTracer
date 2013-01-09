//Author:  Alex Morais
//Filename:  Main.cpp
//Assignment:  Homework 2 - Exercise 2 - MUTEX enabled
//
//Description:  Main.cpp executes the producer and consumer threads.  It is 
//		in this file that the FIFO is declared globally.
//
//Note to the Grader:  I have coded a few added features that were not in
//		Dr. Dandass's specification that allows the consumers to terminate.  
//		I asked Dr. Dandass if this was okay, and he said that my solution
//		would work and its fine that I implemented it.  
//
//			How it works is using a global array of integers(aiProdJoinRet) and 
//		an isEmpty() FIFO function.  Fundamentally, there's an index in the
//		array corresponding to each producer.  When a producer is initialized, 
//		its index in this array is set to 99 leaving the unused indices set
//		to 0.  The FIFO isEmpty function is self-explanatory.  The consumers
//		will loop, checking for an item to consume, only while there is at 
//		least one producer and the buffer is not empty.  It knows which 
//		producers are still executing because the array index of the producers
//		is set to 0 when pthread_join returns(without errors).  Further 
//		comments are included in the code.

#include <iostream> 			// for cout
#include <stdio.h>   		// for perror
#include <stdlib.h>			// for rand(), srand()
#include <time.h>				// for use with srand()
#include <cstdlib>	  		// for atoi
#include <pthread.h>			// for pthread_create, pthread_t, etc...
#include <fcntl.h>   	 	// for open
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "FIFO.h"

using namespace std;

///////////
//GLOBALS//
///////////

//FIFO buffer
FIFO g_FIFO;

//Number of ints each producer will produce
//	- 'n' in the function t*1000000 + n-1
int g_iIntsPerProducer = 0;

//MUTEX attribute
pthread_mutexattr_t g_tMutex_attr;

//MUTEX
pthread_mutex_t g_tMutex;

//Array of return values from pthread_join(atProducers[x])
//This is used to let the consumers know whether there are producers
//	still running.
int aiProdJoinRet[10] = {0};

//Pre-declaration of Producer/Consumer functions
void *Producer(void* seqNum);
void *Consumer(void* seqNum);

int main(int argc, char *argv[]){
	
	int iNumOfProducers = 0,
		iNumOfConsumers = 0,
		iCreateReturn;
	
	char *tName;
		
	pthread_t atProducers[10],
		atConsumers[10];

	//error check the arguments; must be 3
	if(argc != 4){
		cout << "Invalid Arguments: ./hw2_ex1 \"Prod\" \"Con\" \"Ints per Prod\" " << endl;
		exit(-1);
	}
	
	cout << "Correct # of arguments found!" << endl << endl;
		
	//get the number of producers from the arguments
	if((iNumOfProducers = atoi(argv[1])) == 0){
		cout << "Must have at least one producer.\n"
				<< "With no producers the consumers will just loop with no result.\n" 
				<< endl;
		exit(-1);				
	}
	
	if(iNumOfProducers > 10){
		cout << "This is more Producers than can be handled" << endl;
		exit(-1);
	}
	
	cout << "Number of Producers : " << iNumOfProducers << endl;
		
	//get the number of consumers from the arguments
	if((iNumOfConsumers = atoi(argv[2])) == 0){
		cout << "Must have at least one consumer.\n"
				<< "With no consumers the buffer will at some point become full with nothing to empty it.\n" 
				<< endl;
		exit(-1);				
	}
	
	if(iNumOfConsumers > 10){
		cout << "This is more Consumers than can be handled" << endl;
		exit(-1);
	}
	
	cout << "Number of Consumers : " << iNumOfConsumers << endl << endl;
	
	//get the number of integers each producer should produce
	if((g_iIntsPerProducer = atoi(argv[3])) < 10){
		cout << "numIntegers < 10; producer must add at least 10 integers" << endl;
		exit(-1);
	}
	
	//Init mutex defaults
	pthread_mutexattr_init(&g_tMutex_attr);
	
	//Change attr type - enable error checking - prevent deadlock
	pthread_mutexattr_settype(&g_tMutex_attr, PTHREAD_MUTEX_ERRORCHECK);
	
	//Init the mutex
	pthread_mutex_init(&g_tMutex, &g_tMutex_attr);
	
	//Init the random number generator's starting point using the curr time
	srand(time(NULL));
	
	cout << "Starting the Producers...." << endl;
	//Start the Producers
	for(int p = 0; p < iNumOfProducers; p++){
		//Create a new thread
		//	-at pthread_t, p
		//	-as a Producer
		//	-with a random, unique, sequence number argument
		iCreateReturn = pthread_create(&atProducers[p], NULL, Producer, (void *)(rand()));
				
		//error check the return//
		if(iCreateReturn == EAGAIN){
			cout << "The system lacks the resources to create another thread" << endl;
			exit(-1);
		}
			
		if(iCreateReturn == EFAULT){
			cout << "Invalid thread or attr pointer" << endl;
			exit(-1);
		}
		
		if(iCreateReturn == EINVAL){
			cout << "attr is not an initialized thread attr object" << endl;
			exit(-1);
		}
		//end - error check the return//
		
		//indicate in the array that a producer exists at this index
		aiProdJoinRet[p] = 99;
	}
	
	cout << "Starting the Consumers...." << endl;	
	//Start the Consumers
	for(int c = 0; c < iNumOfConsumers; c++){
		//Create a new thread
		//	-at pthread_t, p
		//	-as a Consumer
		//	-with a random, unique, sequence number argument
		iCreateReturn = pthread_create(&atConsumers[c], NULL, Consumer, (void *)(rand()));
		
		//error check the return.
		if(iCreateReturn == EAGAIN){
			cout << "The system lacks the resources to create another thread" << endl;
			exit(-1);
		}
			
		if(iCreateReturn == EFAULT){
			cout << "Invalid thread or attr pointer" << endl;
			exit(-1);
		}
		
		if(iCreateReturn == EINVAL){
			cout << "attr is not an initialized thread attr object" << endl;
			exit(-1);
		}
	}
	
	//g_bReady = true;
	
	//Join all threads to wait for them to finish before exiting
	for(int joinP = 0; joinP < iNumOfProducers; joinP++)
		//Store join return value so the consumers are aware of all running producers
		aiProdJoinRet[joinP] = pthread_join(atProducers[joinP], NULL);
		
	for(int joinC = 0; joinC < iNumOfConsumers; joinC++)
		pthread_join(atConsumers[joinC], NULL);		
	
	//Destroy the mutex object and attribute
	pthread_mutex_destroy(&g_tMutex);
	pthread_mutexattr_destroy(&g_tMutex_attr);
		
	cout << endl << "Finished..." << endl << endl;
	
	return 1;
}

//Producer():
//Description:	The Producer puts integers into the global FIFO.  It will poll if the FIFO
//					becomes full before it is able to write its next integer.  It will terminate
//					after writing all of its integers.
void *Producer(void* seqNum){
	//Get the Thread's sequence number and Calc the starting int
	int iSequenceNumber = (int)seqNum,
		iStart = iSequenceNumber * 1000000;
	
	
	//acquire the mutex object -- print the name of this thread
	pthread_mutex_lock(&g_tMutex);
	
	cout << endl << "\tProducer Thread : " << iSequenceNumber << endl << endl;
	
	//release the mutex object
	pthread_mutex_unlock(&g_tMutex);

	//Produce//
	
	//Add g_iIntsPerProducer many integers to the FIFO
	for(int k = iStart ; k < (iStart + g_iIntsPerProducer); k++){
		
		//acquire the mutex object -- produce an integer into the buffer
		pthread_mutex_lock(&g_tMutex);
		
		//Do this output beforehand so that it shows up at an appropriate time
		cout << endl << "Producer_" << iSequenceNumber << "\tproduced\t" << k;
		
		//while the buffere isn't full, add current integer, k
		while(!g_FIFO.put(k));
		
		//release the mutex object
		pthread_mutex_unlock(&g_tMutex);

		//Force another thread to execute
		usleep(1);
	}
	
	//Terminate
}

//Consumer():
//Description:	The Consumer removes integers from the FIFO then prints them.  If 
//					the FIFO is empty when the consumer attempts a read/remove it will
//					print nothing and return to the loop.
void *Consumer(void* seqNum){
	//Get the Thread's sequence number
	int iSequenceNumber = (int)seqNum,
		iCurrInteger;
	
	//acquire the mutex object -- print the name of this thread
	pthread_mutex_lock(&g_tMutex);	
		
	cout << endl << "\tConsumer Thread : " << iSequenceNumber << endl << endl;

	//release the mutex object
	pthread_mutex_unlock(&g_tMutex);
	
	//Consume//
	//While some producers are still alive and the buffer is not empty
	while((aiProdJoinRet[0] != 0 || aiProdJoinRet[1] != 0 || aiProdJoinRet[2] != 0
			|| aiProdJoinRet[3] != 0 || aiProdJoinRet[4] != 0 || aiProdJoinRet[5] != 0
			|| aiProdJoinRet[6] != 0 || aiProdJoinRet[7] != 0 || aiProdJoinRet[8] != 0
			|| aiProdJoinRet[9] != 0) && !g_FIFO.isEmpty()){
		
		//acquire the mutex object -- consume a buffer item
		pthread_mutex_lock(&g_tMutex);
		
		while(!( g_FIFO.get(iCurrInteger)));
		
		cout << endl << "Consumer_" << iSequenceNumber << "\tconsumed\t" << iCurrInteger;
		
		//release the mutex object
		pthread_mutex_unlock(&g_tMutex);

		//Force another thread to execute
		usleep(1);
	}
	
	//acquire the mutex object -- print the name of this thread
	pthread_mutex_lock(&g_tMutex);
	
	cout << endl << endl << "\tConsumer : " << iSequenceNumber << "terminating..."<< endl;

	//release the mutex object
	pthread_mutex_unlock(&g_tMutex);
	
	//Terminate
}
