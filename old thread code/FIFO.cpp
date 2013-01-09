//Author:  Alex Morais
//Filename:  FIFO.cpp
//
//Description:  FIFO class definition					

#include <unistd.h>

using namespace std;

#include "FIFO.h"

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
	
	//Move the tail position down
	m_iTail = iTemp;
	
	//An integer has been successfully added to the FIFO
	return true;
}

bool FIFO::get(int &i){
	
	//The FIFO is empty
	if(isEmpty())
		return false;
	
	//Get the next element from the FIFO	
	i = m_aiElements[m_iHead];

	//Move the head position
	m_iHead = (m_iHead + 1) % FIFO_SIZE;
	
	return true;	
}

bool FIFO::isEmpty(){
	
	//Is the FIFO empty?
	if(m_iHead == m_iTail)
		return true;				//YES
	else
		return false;				//NO
}
