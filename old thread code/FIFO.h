//Author:  Alex Morais
//Filename:  FIFO.h
//
//Description:  FIFO class declaration.


#include <unistd.h>

#define FIFO_SIZE 512

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
		
		//Check the buffer's empty status
		bool isEmpty();
};
