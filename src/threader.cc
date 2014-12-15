//Author:  Alex Morais
//Filename:  hw2_ex1.cpp
//

//and m
//

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
#include <unistd.h>

#define FIFO_SIZE 512

using namespace std;


class FIFO{
	protected:
		int m_iHead,
			m_iTail,
			m_aiElements[FIFO_SIZE];
	
	public:
		FIFO();
		~FIFO();
		
		//Producer function
		bool put(int i);
		
		//Consumer function
		bool get(int &i);		
};

FIFO::FIFO(){
	m_iHead = 0;
	m_iTail = 0;
}

FIFO::~FIFO(){
}

bool FIFO::put(int i){
	int iTemp = (m_iTail+1) % FIFO_SIZE;
	
	//The FIFO is full
	if(iTemp == m_iHead)
		return false;
	
	//Add i to the FIFO
	m_aiElements[m_iTail] = i;
	
	//Force another thread to execute
	usleep(1);
	
	//Move the tail position down
	m_iTail = iTemp;
	
	//An integer has been successfully added to the FIFO
	return true;
}

bool FIFO::get(int &i){
	
	//The FIFO is empty
	if(m_iHead == m_iTail)
		return false;
	
	//Get the next element from the FIFO	
	i = m_aiElements[m_iHead];
	
	//Force another thread to execute
	usleep(1);
	
	//Move the head position
	m_iHead = (m_iHead + 1) % FIFO_SIZE;
	
	return true;	
}

//gloabal FIFO
//FIFO g_FIFO;

//Producer:
//Description:	The Producer puts integers into the global FIFO.  It will poll if the FIFO
//				becomes full before it is able to write its next integer.  It will terminate
//				after writing all of its integers.
void *Producer(void* seqNum){
	//Get the Thread's sequence number and Calc the starting int
	int iSequenceNumber = (int)seqNum,
		iStart = iSequenceNumber * 1000000;
	
	cout << "\tProducer Thread, " << iSequenceNumber << " created." << endl;
			
	//Produce//
	
	//Add g_iIntsPerProducer many integers to the FIFO
	for(int k = iStart ; k < (iStart + g_iIntsPerProducer); k++){
		//Do this output beforehand so that it shows up at an appropriate time
		cout << "Producer_" << iSequenceNumber << " produced " << k << endl;
		
		//while the buffere isn't full, add current integer, k
		while(!g_FIFO.put(k));
	}
	
	//Terminate
}

//Consumer:
//Description:	The Consumer removes integers from the FIFO then prints them.  If 
//					the FIFO is empty when the consumer attempts a read/remove it will
//					print nothing and return to the loop.
void *Consumer(void* seqNum){
	//Get the Thread's sequence number
	int iSequenceNumber = (int)seqNum,
		iCurrInteger;
	
	cout << "\tConsumer Thread, " << iSequenceNumber << " created." << endl;
	
	//Consume//
	while(1){
		while( !( g_FIFO.get(iCurrInteger) ) );
		
		cout << "Consumer_" << iSequenceNumber << " consumed " << iCurrInteger << endl;
	}
}

/*//testing function
void *print_message_function( void *ptr ){
     char *message;
     message = (char *) ptr;
     printf("%s \n", message);
}*/

int main(int argc, char *argv[]){

	char *tName;

	//error check the arguments; must be 3
	if(argc != 4){
		cout << "Invalid Arguments: ./hw2_ex1 \"Prod\" \"Con\" \"Ints per Prod\" " << endl;
		exit(-1);
	}

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
	}
	
	//Join all threads to wait for them to finish before exiting
	for(int joinP = 0; joinP < iNumOfProducers; joinP++)
		pthread_join(atProducers[joinP], NULL);
		
	for(int joinC = 0; joinC < iNumOfConsumers; joinC++)
		pthread_join(atConsumers[joinC], NULL);		
	
	return 1;
}
