//Author Peter Adamson
//This program expects the following syntax when being run: ./a.out n file.txt  ARG < sample
//where n is the number of frames, file.txt is the memory file to be read, and sample is the list of logical addresses, and ARG is either F, L, or O.  If ARG is F, the program uses first in first out.  If ARG is L, the program uses LRU.  If ARG is O, the program uses optimal.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

//define the node structure for the queue
typedef struct qNode Node;
struct qNode
{
	int pageNumber;
	Node *next;
	Node *prev;	
};

typedef struct qOptNode optNode;
struct qOptNode
{
	int pageNumber;
	int logAddr;
	char val;
	int offset;
	optNode *next1;
	optNode *prev1;	
};

//global variable declarations
int pageTable[256];
int indicatorTable[256];
int pageFaults;
int freeFrame;
int full;
int numOfFrames;
int alg;
Node *head;
Node *tail;
optNode *optHead;
optNode *optTail;

//function declarations
int findFreeFrameFIFO();
int findFreeFrameLRU();
int findFreeFrameOPT();
void initialize();
bool isEmpty();
bool isEmptyOpt();
void enqueue(int pageNum);
void enqueueOpt(int pageNum,int offset,char val,long int logAddr);
Node *dequeue();
Node* delete(int pageNum);

int main(int argc, char **argv)
{

	initialize();
	
	//variable set up
	pageFaults = 0;
	freeFrame = 0;
	full = 0;
	numOfFrames = atoi(argv[1]);
	int frames[numOfFrames * 256];
	long int logAddr;
	int i;
	char *character;
	character = (char*)malloc(sizeof(char*));

	//check which algorithm to use
	if(strcmp(argv[3],"F") == 0)
	{
		alg = 0;
	}
	else if(strcmp(argv[3],"L") == 0)
	{
		alg = 1;
	}
	else if(strcmp(argv[3],"O") == 0)
	{
		alg = 2;
	}

	//open the file
	FILE *file = fopen(argv[2], "rb");

	//populate page table and indicator table
	for(i = 0; i < 256; i++)
	{
		pageTable[i] = 0;
		indicatorTable[i] = 0;
	}

	//loop as long as there are still logical addresses to be read and the format is correct
	while(!feof(stdin) && (1 == scanf("%ld",&logAddr)))
	{
		fseek(file, logAddr, SEEK_SET);	//set the file stream to the appropriate location
		int val = fgetc(file);		//get the character at the file stream location
		val = (char) val;		//cast the character to char

		//get page number and offset of logical address
		int pageNum = logAddr / 256;	
		int offset = logAddr % 256;

		if(alg == 0)	//we are using FIFO
		{
			if(indicatorTable[pageNum] == 1)	//we have a valid page table entry
			{
				int physAddr = pageTable[pageNum] * 256 + offset;
				printf("%d->%d->%c\n",logAddr,physAddr,frames[physAddr]);
			}
			else					//the page is not loaded
			{	
				pageFaults = pageFaults + 1;
				int frameNum;
				frameNum = findFreeFrameFIFO();
				indicatorTable[pageNum] = 1;
				pageTable[pageNum] = frameNum;
				int physAddr = frameNum * 256 + offset;
				int ch[255];

				//load the page
				for(i = 0; i < 256; i++)
				{
					fseek(file,pageNum*256 + i, SEEK_SET);
					int v = fgetc(file);
					v = (char) v;
					ch[i] = v;
				}

				//put the page into physical memory
				int count = 0;
				for(i = frameNum * 256; i < frameNum * 256 + 256; i++)
				{
					frames[i] = ch[count];
					count = count + 1;
				}
				printf("%d->%d->%c\n",logAddr,physAddr,frames[frameNum * 256 + offset]);
			}
		}
		else if(alg == 1)	//we are using LRU
		{
			if(indicatorTable[pageNum] == 1)	//we have a valid page table entry
			{
				int physAddr = pageTable[pageNum] * 256 + offset;
				Node *valid = delete(pageNum);
				enqueue(valid->pageNumber);
				printf("%d->%d->%c\n",logAddr,physAddr,frames[physAddr]);
			}
			else					//the page is not loaded
			{	
				pageFaults = pageFaults + 1;
				int frameNum;
				frameNum = findFreeFrameLRU(pageNum);
				indicatorTable[pageNum] = 1;
				pageTable[pageNum] = frameNum;
				int physAddr = frameNum * 256 + offset;
				int ch[255];

				//load the page
				for(i = 0; i < 256; i++)
				{
					fseek(file,pageNum*256 + i, SEEK_SET);
					int v = fgetc(file);
					v = (char) v;
					ch[i] = v;
				}

				//put the page into physical memory
				int count = 0;
				for(i = frameNum * 256; i < frameNum * 256 + 256; i++)
				{
					frames[i] = ch[count];
					count = count + 1;
				}
				printf("%d->%d->%c\n",logAddr,physAddr,frames[frameNum * 256 + offset]);
			}
		}
		else if(alg == 2)	//we are using optimal
		{
			//populate the queue
			enqueueOpt(pageNum,offset,val,logAddr);
		}	
	}
	if(alg == 2)	//we are using optimal
	{
		optNode *counter = optHead;
		while(counter != NULL)
		{
			char val = counter->val;
			//get page number and offset of logical address
			int pageNum = counter->pageNumber;	
			int offset = counter->offset;
			logAddr = counter->logAddr;
			if(indicatorTable[pageNum] == 1)	//we have a valid page table entry
			{
				int physAddr = pageTable[pageNum] * 256 + offset;
				printf("%d->%d->%c\n",logAddr,physAddr,frames[physAddr]);
			}
			else					//the page is not loaded
			{	
				pageFaults = pageFaults + 1;
				int frameNum;
				frameNum = findFreeFrameOPT(pageNum);
				indicatorTable[pageNum] = 1;
				pageTable[pageNum] = frameNum;
				int physAddr = frameNum * 256 + offset;
				int ch[255];

				//load the page
				for(i = 0; i < 256; i++)
				{
					fseek(file,pageNum*256 + i, SEEK_SET);
					int v = fgetc(file);
					v = (char) v;
					ch[i] = v;
				}

				//put the page into physical memory
				int count = 0;
				for(i = frameNum * 256; i < frameNum * 256 + 256; i++)
				{
					frames[i] = ch[count];
					count = count + 1;
				}
				printf("%d->%d->%c\n",logAddr,physAddr,frames[frameNum * 256 + offset]);
			}
			counter = counter->next1;
		}
	}

	//close the file
	fclose(file);

	printf("total page faults: %d\n",pageFaults);
}

//will find the first free frame using FIFO
int findFreeFrameFIFO()
{
	int found;
	if(full == 0)	//still free frames
	{
		found = freeFrame;
		freeFrame = freeFrame + 1;
		if(freeFrame >= numOfFrames)	//loop freeframes
		{
			full = 1;
			freeFrame = 0;
		}
		return found;
	}
	else		//no free frames
	{
		int j;
		int index;
		for(j = 0; j < 256; j++)
		{
			if(pageTable[j] == freeFrame)	//we have found the entry to remove
			{
				pageTable[j] = 0;
				indicatorTable[j] = 0;
				break;
			}
		}
		found = freeFrame;
		freeFrame = freeFrame + 1;
		if(freeFrame >= numOfFrames)	//loop freeframes
		{
			freeFrame = 0;
		}
		return found;
	}
}

//will find the first free frame using LRU
int findFreeFrameLRU(int pageNum)
{
	int found;
	if(full == 0)	//still free frames
	{
		Node *temp = NULL;
		temp = (Node*)malloc(sizeof(Node));
		temp->pageNumber = pageNum;
		enqueue(temp->pageNumber);
		found = freeFrame;
		freeFrame = freeFrame + 1;
		if(freeFrame >= numOfFrames)	//loop freeframes
		{
			full = 1;
			freeFrame = 0;
		}
		return found;
	}
	else	//no free frames
	{
		Node *last = dequeue();
		Node *add = NULL;
		add = (Node*)malloc(sizeof(Node));
		add->pageNumber = pageNum;
		enqueue(add->pageNumber);
		found = pageTable[last->pageNumber];
		pageTable[last->pageNumber] = 0;
		indicatorTable[last->pageNumber] = 0;
		return found;
	}
}

//will find the first free frame using OPT
int findFreeFrameOPT(int pageNum)
{
	int found;
	if(full == 0)	//still free frames
	{
		Node *temp = NULL;
		temp = (Node*)malloc(sizeof(Node));
		temp->pageNumber = pageNum;
		enqueue(temp->pageNumber);
		found = freeFrame;
		freeFrame = freeFrame + 1;
		if(freeFrame >= numOfFrames)	//loop freeframes
		{
			full = 1;
			freeFrame = 0;
		}
		return found;
	}
	else	//no free frames
	{
		Node *curr1 = head;
		optNode *curr2 = optHead;
		Node *toRemove = NULL;
		toRemove = (Node*)malloc(sizeof(Node));
		int maxDistance = 0;
		int checkDistance = 0;
		while(curr1 != NULL)
		{
			checkDistance = 0;
			while(curr2 != NULL)
			{
				if(curr1->pageNumber == curr2->pageNumber)
				{
					break;
				}
				else
				{
					checkDistance = checkDistance + 1;
					curr2 = curr2->next1;
				}
			}
			if(checkDistance > maxDistance)
			{
				maxDistance = checkDistance;
				toRemove = curr1;
			}
			curr1 = curr1->next;
		}
		Node *add = NULL;
		add = (Node*)malloc(sizeof(Node));
		add->pageNumber = pageNum;
		delete(toRemove->pageNumber);
		enqueue(add->pageNumber);
		found = pageTable[toRemove->pageNumber];
		pageTable[toRemove->pageNumber] = 0;
		indicatorTable[toRemove->pageNumber] = 0;
		return found;
	}
}

//sets the head and tail pointers to null and indicates to the user that the pointers are ready to use
void initialize()
{
	head = tail = NULL;
	optHead = optTail = NULL;
}

//if list is empty, returns true
bool isEmpty() {
   return head == NULL;
}

//if list is empty, returns true (OPT only)
bool isEmptyOpt() {
   return optHead == NULL;
}

//loads a job into the back of the list
void enqueue(int pageNum)
{
	Node *temp = (Node*) malloc(sizeof(Node));
	temp->pageNumber = pageNum;
	
	if(isEmpty()) 	//empty list
	{
		tail = temp;
		head = temp;
	}
	else 		//not empty list
	{
		tail->next = temp;     
		temp->prev = tail;
	}

	tail = temp;
}

//loads a job into the back of the list (OPT only)
void enqueueOpt(int pageNum,int offset,char val,long int logAddr)
{
	optNode *temp = (optNode*) malloc(sizeof(optNode));
	temp->pageNumber = pageNum;
	temp->offset = offset;
	temp->val = val;
	temp->logAddr = logAddr;
	
	if(isEmptyOpt()) 	//empty list
	{
		optTail = temp;
		optHead = temp;
	}
	else 		//not empty list
	{
		optTail->next1 = temp;     
		temp->prev1 = optTail;
	}

	optTail = temp;
}

//removes a job from the front of the list
Node *dequeue()
{
	Node *temp = head;
	
	if(head == NULL)	//list is empty
	{
		return NULL;
	}
	
	if(head->next == NULL)	//only one element in list
	{
		tail = NULL;
	} 
	else 			//more than one element in list
	{
		head->next->prev = NULL;
	}

	head = head->next;
	return temp;
}

//removes a specific page from the list
Node *delete(int pageNum) 
{

	Node* curr = head;
	Node* previ = NULL;
	
	if(head == NULL) 	//list is empty
	{
		return NULL;
	}

	//iterate through list until pagenumber
	while(curr->pageNumber != pageNum) 
	{
		if(curr->next == NULL)	//last node
		{
			return NULL;
		} 
		else 				//not last node
		{
			previ = curr;
			curr = curr->next;             
		}
	}

	if(curr == head)	//first node
	{
		head = head->next;
   	} 
	else			//not first node
	{
		curr->prev->next = curr->next;
	}    

	if(curr == tail)	//last node
	{
		tail = curr->prev;
	} 
	else 			//not last node
	{
		curr->next->prev = curr->prev;
	}
	
	return curr;
}
