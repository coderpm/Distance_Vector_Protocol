/**
 * @prathamm_assignment3
 * @author  Pratham Malik <prathamm@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <iostream>
#include <stdio.h>
#include <fstream>				//Handing File Operation
#include <string.h>				//Handling String Operations
#include <stdlib.h> 			//For atoi
#include <stdint.h>				//For int of variable sizes

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/global.h"
#include "../include/logger.h"

using namespace std;

#define STDIN 0
#define INF 65535
#define maxVertices 5

//Global variables

char FILEPATH[100];
int updateinterval;		//Stores the value of routing update interval given from command line

struct owninfo
{
	uint16_t numserver;				//Global Variable to store the number of servers
	uint16_t numneighbor;			//Global variable to store the number of neighbors
	char myip[17];				//Global Variable to store server IP Address in string format
	uint16_t myport;			//Global Variable to store server port Address in 16 bits
	uint16_t myid;				//Global Variable to store server id in 16 bits
	sockaddr_in myserveraddr;		//Global Variable to store ip address in sockaddr form and 32 bits
};
struct owninfo myserver;
//Structure for Storing all information from topology file about network
struct allinfo
{
	uint16_t serverid;
	char serverip[17];
	uint16_t serverport;
	uint16_t servercost;
	sockaddr_in serveripaddr;
};
struct allinfo networkinfo[6];		//Declared for all information about the network
struct allinfo neighbor[5];

//[PA3] Update Packet Start
struct serverinfosend
{
	sockaddr_in ipaddr;		//For server ip address
	uint16_t port;		//For server port
	uint16_t x;					//Always have the value as 0
	uint16_t id;			//Stores the value for serverid
	uint16_t cost;				//Stores the value for cost
};
struct updatepkt
{
	uint16_t numfield;
	uint16_t serverport;
	sockaddr_in serverip;
	serverinfosend sendinfo[5];
};
struct updatepkt updatepacket;		//Stores the details of sending packet
//[PA3] Update Packet End

struct updatepkt recvpacket;		//Stores the details of received packet
struct updatepkt sortedpacket;		//Used to store the sorted receive packet

struct updatecounter
{
	int serverid;
	int updatecount;
	int firstrecv;
};
struct updatecounter counter[5];

//Structure for command
struct commandentered
{
	char entercmd[100];
	char cmd[30];
	char argument1[30];
	char argument2[30];
	char argument3[30];
};
struct commandentered command;

char commanderror[100];

struct graph
{
	int source;
	int destination;
	int weight;
	};
struct graph edge[30];
int edgenumber=0;			//Stores the total number of edges in the graph

int  samplecostmatrix[6][6];

int  clientfd;		//File Descriptor for socket ---Sending
int  serverfd;		//File Descriptor for socket ---Receiving

//[PA3] Routing Table Start
struct routing_table
{
	uint16_t serverid;
	uint16_t reachcost;
	int nexthop_id;
};
//[PA3] Routing Table End
struct routing_table routingtable[5];

int distancevectorpackets=0;
int deletedlist[5];

//Functions
void checkcommandlineargs(int argc, char **argv);
void retrievefile();
void startserver();
void makepacket();
void sendpacket();
int commandcheck();
void handlerecvdata(int ids);
void applybellmanford();
void BellmanFord(int graph[][maxVertices],int cost[][maxVertices],int size[],int source,int vertices);
int getserverid();
void makeedge();			//makes edges present in the whole network from cost matrix
void sortpacket(int ida);


/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
        cse4589_init_log();

	/* Clear LOGFILE and DUMPFILE */
        fclose(fopen(LOGFILE, "w"));
        fclose(fopen(DUMPFILE, "wb"));

	/*Start Here*/

    	//Reset Sample Cost Matrix
    	for(int hi=0;hi<6;hi++)
    	{
    		for(int sj=0;sj<6;sj++)
    		{
    			samplecostmatrix[hi][sj]=INF;

    		}
  //  		cout<<"\n";
    	}
    	//Decipher the command line arguments
    	checkcommandlineargs(argc,argv);

    	//Call function to Retrieve value from topology File
    	retrievefile();
    	//Call the server program
    	startserver();
    	//Ending the program
    	printf("\n\n");
    	return 0;

}
//Function  to check command line arguments
void checkcommandlineargs(int arg, char **argv)
{
	if(arg==5)
	{
		strcpy(FILEPATH,argv[2]);
		updateinterval=atoi(argv[4]);
	}
	else
	{
		cout<<"Wrong Command Input to invoke the program\n Please Try again....."<<endl;
	}


}

/**
 * Reference: http://www.cplusplus.com/doc/tutorial/files/
 * http://www.learncpp.com/cpp-tutorial/136-basic-file-io/
 */

//Function to Retrieve contents of the topology file
void retrievefile()
{
	int i,j;		//Counter for iterating file contents
	int filemax;	//Stores the maximum index of the file elements
	char line[40][25];
	ifstream myfile (FILEPATH);
	i=0;
	j=0;
	memset(&line,0,sizeof(line));
	//Retrieving all values from file
	if (myfile.is_open())
	 {
//		 printf("\nFile is open");
		 while(myfile)
		 {
			 //Saving File contents in Array
			 myfile >> line[i];
			// printf("\nLine is %s",line[i]);
			 filemax=i;
			 i++;
		 }
//		 printf("\nPrinting File");
		 for(int j=0;j<i-1;j++)
		 {
			 printf("\nContent at index %d is %s",j,line[j]);
		 }
	 }// End of If checking whether file is present
	 else
	 {
		 printf("\nfILE NOT FOUND....Re run the program with correct file path\n");
		 exit(0);
	 }


		int selfid=atoi(line[17]);
		int selfindex=0;
		int chrcheck;
		for(int ind=2;ind<16;ind=ind+3)
		{
			chrcheck=atoi(line[ind]);
			if(selfid==chrcheck)
			{
				selfindex=ind;
				break;
			}
		}

	 //Assign  Global variables for Own server from topology file
	 myserver.numserver = atoi(line[0]);		//Number of servers
	 myserver.numneighbor = atoi(line[1]);		//Number of neighbors
	 myserver.myid = atoi(line[2]);				myserver.myid = selfid;				//My Server Id
	 strcpy(myserver.myip,line[3]);				strcpy(myserver.myip,line[selfindex+1]);				 				//My server ip in character format
	 myserver.myport = atoi(line[4]);			myserver.myport = atoi(line[selfindex+2]);			//My Server Port
	 inet_pton(AF_INET,myserver.myip,&myserver.myserveraddr.sin_addr);		//My server IP in sockaddr_in format

//	 cout <<"\nNum server " <<myserver.numserver;

	 //Saving my network info
	 networkinfo[0].serverid=myserver.myid;
	 strcpy(networkinfo[0].serverip,myserver.myip);
	 networkinfo[0].serverport=myserver.myport;
	 networkinfo[0].servercost=0;
	 inet_pton(AF_INET,networkinfo[0].serverip,&networkinfo[0].serveripaddr.sin_addr);

//	 cout <<"\nNum server " <<myserver.numserver;

	 int countern=1;
	 //for(i=5;i<15;i++)
	 for(i=2;i<15;i++)
	 {
		 if(i==selfindex)
		 {
			 i=i+2;
			 continue;
		 }

		 //Saving server info
		 networkinfo[countern].serverid= atoi(line[i]);
		 strcpy(networkinfo[countern].serverip,line[i+1]);
		 networkinfo[countern].serverport= atoi(line[i+2]);
		 inet_pton(AF_INET,networkinfo[countern].serverip,&networkinfo[countern].serveripaddr.sin_addr);
		 networkinfo[countern].servercost= INF;
		 //INcrementing the counter variables
		 i=i+2;
		 countern=countern+1;
	 }

	 //printf("\n\nThe NETWORK INFO");
	 //cout <<"\nNum server " <<myserver.numserver;

	 for(i=0;i<myserver.numserver;i++)
	 {
		char str[17];
		inet_ntop(AF_INET, &(networkinfo[i].serveripaddr.sin_addr), str, INET_ADDRSTRLEN);
		fflush(stdout);
	 }

	 //Saving the neighbor elements from the topology file
	 memset(&neighbor,0,sizeof(neighbor));

	 for(i=17;i<filemax;i++)
	 {
	//	 printf("\nContent at line %d is ::::%s",i,line[i]);
		 if(atoi(line[i]) == myserver.myid)
		 {
			 neighbor[j].serverid=atoi(line[i+1]);
			 neighbor[j].servercost=atoi(line[i+2]);

		 }//End of If Checking whether the first line is the server id

		 //saving the information for the neighbors from the all network info
		 for(int c=0;c<myserver.numserver;c++)
		 {
			 if(networkinfo[c].serverid==neighbor[j].serverid)
			 {

				 strcpy(neighbor[j].serverip,networkinfo[c].serverip);
				 neighbor[j].serverport=networkinfo[c].serverport;
				 neighbor[j].serveripaddr=networkinfo[c].serveripaddr;
				 networkinfo[c].servercost=neighbor[j].servercost;
			 }
		 }

		 i=i+2;
		 j++;


	 }//End of For ---neighbors

	// printf("\nPrinting the neighbors\n");
	 for(i=0;i<myserver.numneighbor;i++)
	 {
		 char str[17];
		 inet_ntop(AF_INET, &(neighbor[i].serveripaddr.sin_addr), str, INET_ADDRSTRLEN);
	//	 printf("\n%u \t\t %s \t\t%u\t\t%u",neighbor[i].serverid,str,neighbor[i].serverport,neighbor[i].servercost);
		 fflush(stdout);
		 int c=myserver.myid;
		 samplecostmatrix[c][neighbor[i].serverid]=neighbor[i].servercost;
	 }

	 //Initialize the server id and update counter for timer counter
	 for(i=0;i<myserver.numneighbor;i++)
	 {
		 counter[i].serverid=neighbor[i].serverid;
		 counter[i].updatecount=0;
	 }





	 //Extra To be deleted
	 printf("\nPrinting the networkinfo\n");
		 for(i=0;i<myserver.numserver;i++)
		 {
			 char str[17];
			 inet_ntop(AF_INET, &(networkinfo[i].serveripaddr.sin_addr), str, INET_ADDRSTRLEN);
			 printf("\n%u\t%s\t%u\t%u",networkinfo[i].serverid,str,networkinfo[i].serverport,networkinfo[i].servercost);
			 fflush(stdout);

		 }

	 printf("\n\nPrinting the neighbors\n");
	 for(i=0;i<myserver.numneighbor;i++)
	 {
		 char str[17];
		 inet_ntop(AF_INET, &(neighbor[i].serveripaddr.sin_addr), str, INET_ADDRSTRLEN);
		 printf("\n%u \t\t %s \t\t%u\t\t%u",neighbor[i].serverid,str,neighbor[i].serverport,neighbor[i].servercost);
		 fflush(stdout);
		 int c=myserver.myid;
		 samplecostmatrix[c][neighbor[i].serverid]=neighbor[i].servercost;
	 }
	 cout<<"\n\nSample Cost matrix\n";
	 for(int hi=0;hi<6;hi++)
	     	{
	     		for(int sj=0;sj<6;sj++)
	     		{
	     			cout<<samplecostmatrix[hi][sj]<<"\t";

	     		}
	     		cout<<"\n";
	     	}

}

/*
 * Reference:
 * 1. http://www.programminglogic.com/sockets-programming-in-c-using-udp-datagrams/
 * 2. http://www.it.uom.gr/project/client_server/socket/socket/prog/udpServer.c
 */
void startserver()
{
	int selectreturn;		//Stores the return value of select command

	int max;
	int socketindex;
	struct timeval seltime;
	seltime.tv_sec=updateinterval;
	seltime.tv_usec=0;

	struct sockaddr_in serverAddr;
	struct sockaddr_in serverStorage;
	socklen_t addr_size;
	int rbyte;

	//for multiple connections
	fd_set masterfd;      //Has all the File descriptions
	fd_set readfd;



	/*Create UDP socket*/
	clientfd = socket(PF_INET, SOCK_DGRAM, 0);
	serverfd = socket(PF_INET, SOCK_DGRAM, 0);


	if(serverfd <0)
	{
		cout << "\nSocket not created..... Exiting program";
		exit(0);
	}

	/*Configure settings in address struct*/
	memset(&serverAddr, '\0', sizeof serverAddr);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(myserver.myport);
	serverAddr.sin_addr.s_addr = myserver.myserveraddr.sin_addr.s_addr;
	//serverAddr.sin_addr.s_addr=inet_addr(myserver.myip);
	//serverAddr.sin_addr.s_addr=(INADDR_ANY);
	//memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

	/*Bind socket with address struct*/
	if(bind(serverfd, (struct sockaddr *) &serverAddr, sizeof(serverAddr))<0)
	{
		cout << "\n ERROR IN BINDING";
		exit(0);
	}
	else
	{
		cout<<"\nServer STARTED"<<endl;
			fflush(stdout);
		cout << "\n Binding successful"<<endl;
	}

	fflush(stdout);

	/*Initialize size variable to be used later on*/
	addr_size = sizeof serverStorage;

	FD_SET(serverfd,&masterfd);
	FD_SET(clientfd,&masterfd);
    FD_SET(STDIN,&masterfd);

    max= serverfd;
    while(1)
    {
    	FD_ZERO(&readfd);
        readfd=masterfd;
  //      cout << "\n$" <<endl;
  	    selectreturn = select(max+1,&readfd,NULL,NULL,&seltime);
   //     cout << "\nThe value of select return is "<<selectreturn;fflush(stdout);
        if(selectreturn>0)
        {
        	for(socketindex=0;socketindex<=max;socketindex++)
          	{
        		//Check whether the socket index is present in the readFd
                if(FD_ISSET(socketindex,&readfd))
                {
                	if(socketindex==STDIN)
  					{
                		cout <<"\nCommand Received from Terminal\n";
						fgets(command.entercmd,sizeof(command.entercmd),stdin);

						int c = commandcheck();
						if(c==0)
						{
							//char sss[]="Did not execute properly";
							cse4589_print_and_log("%s:%s\n",command.entercmd,commanderror);
						}

						fflush(stdout);
  					}//ENd of IF LOOP CHECKING command from STDIN
  					else if(socketindex==serverfd)
  					{
  					//	cout << "\n\nUPDATE---- received from other servers";
  						fflush(stdout);
  						memset(&recvpacket,0,sizeof(recvpacket));
  						rbyte = recvfrom(serverfd,&recvpacket,sizeof(recvpacket),0,(struct sockaddr *) &serverStorage, &addr_size);
  						int ssid=getserverid();
  						//Check whether packet is received from server only
  						if(rbyte>0 && ssid>0)
  						{

  							cse4589_print_and_log("RECEIVED A MESSAGE FROM SERVER %d\n",ssid);
  							fflush(stdout);
							distancevectorpackets++;
  							handlerecvdata(ssid);
   						}

  					}//End of IF checking whether data is received from serverfd

                  }//End of IF Checking that socketindex received is set in masterfd


          	}//End of FOR LOOP iterating for each socketindex



          }//End of IF CHECKING that selectreturn is greater than 0

          else if(selectreturn ==0)
          {
          	cout << "\n\t\t\tTIMEOUT occurred";


          	//Update the update count for all neighbors
          	for(int xs=0;xs<myserver.numneighbor;xs++)
          	{
          		//Update count only if first update has been received
          		if(counter[xs].firstrecv==1)
          			counter[xs].updatecount=counter[xs].updatecount+1;
          	}

          	for(int xs=0;xs<myserver.numneighbor;xs++)
          	{
          		if(counter[xs].updatecount==3)
          		{
          			//Set cost as infinity for that serverid
          			int ser=counter[xs].serverid;
          			int mser=myserver.myid;
          			samplecostmatrix[mser][ser] = INF;
          			//Update the network info also which holds the data for all servers
          			for(int i=0;i<5;i++)
          			{
          				int lis=networkinfo[i].serverid;
          				if(lis==ser)
          					networkinfo[i].servercost=INF;
          			}
          		}
          	}

          	//Send regular update packet
          	makepacket();

         	sendpacket();
/**
          	struct sockaddr_in sendAddr;
       		socklen_t addr_s;
       		int sbyte=0;
          	for(int z=0;z<myserver.numneighbor;z++)
    		{

          		//Send to all neighbors
          		//Configure settings in address struct
          		memset(&sendAddr, '\0', sizeof sendAddr);
          		sendAddr.sin_family = AF_INET;
          		sendAddr.sin_port = htons(neighbor[z].serverport);
          		sendAddr.sin_addr.s_addr = neighbor[z].serveripaddr.sin_addr.s_addr;
          		//Initialize size variable to be used later on
          		addr_s = sizeof sendAddr;

          		char str[17];
          		inet_ntop(AF_INET, &(sendAddr.sin_addr.s_addr), str, INET_ADDRSTRLEN);

          		fflush(stdout);

          		sbyte=sendto(clientfd,&updatepacket,sizeof(updatepacket),0,(struct sockaddr *)&sendAddr,addr_s);
          		if(sbyte<0)
          		{
          			perror("\nError in Sending: ");
          			clientfd = socket(PF_INET, SOCK_DGRAM, 0);
          			sendpacket();
          		}

          		else
          			cout << "\nPacket Sent to "<<str<<endl;;
          			//cout<<"\nSENDING Success\n";

          		fflush(stdout);
          	}
*/
          	//Reset the timeout value for select loop
          	seltime.tv_sec=updateinterval;
          	seltime.tv_usec=0;

          }//End of IF CHECKING that selectreturn =0


    }//End of Infinite While loop



}
//Function to send the packet
void sendpacket()
{
	struct sockaddr_in sendAddr;

	socklen_t addr_s;
	int sbyte;
	for(int z=0;z<myserver.numneighbor;z++)
    {
		//Send to all neighbors
        /*Configure settings in address struct*/
        memset(&sendAddr, '\0', sizeof sendAddr);
        sendAddr.sin_family = AF_INET;
        sendAddr.sin_port = htons(neighbor[z].serverport);
        sendAddr.sin_addr.s_addr = neighbor[z].serveripaddr.sin_addr.s_addr;
        /*Initialize size variable to be used later on*/
        addr_s = sizeof sendAddr;

        char str[17];
        inet_ntop(AF_INET, &(sendAddr.sin_addr.s_addr), str, INET_ADDRSTRLEN);

        sbyte=sendto(clientfd,&updatepacket,sizeof(updatepacket),0,(struct sockaddr *)&sendAddr,addr_s);
        if(sbyte<0)
        {
        	perror("\n::SEND TO ERROR::\n");
        	clientfd = socket(PF_INET, SOCK_DGRAM, 0);
        	sendpacket();
        }

        else
        	cout << "\nPacket Sent to "<<str<<endl;;
          	//cout<<"\nSENDING Success\n";

        fflush(stdout);
    }
}
//Function to make Update packet
void makepacket()
{
	int i=0;

	uint16_t z=0;
//	cout <<"\nNum server " <<myserver.numserver;
	updatepacket.numfield=htons(myserver.numserver);
//	cout <<"\nNum Field " << updatepacket.numfield;
	updatepacket.serverip= myserver.myserveraddr;
	updatepacket.serverport= htons(myserver.myport);


	networkinfo[0].serveripaddr=myserver.myserveraddr;
	networkinfo[0].serverport=(myserver.myport);
	networkinfo[0].serverid=(myserver.myid);
	networkinfo[0].servercost=0;
	for(i=0;i<myserver.numserver;i++)
	{
		updatepacket.sendinfo[i].ipaddr=networkinfo[i].serveripaddr;
		updatepacket.sendinfo[i].port = htons(networkinfo[i].serverport);
		updatepacket.sendinfo[i].x=htons(z);
		updatepacket.sendinfo[i].id=htons(networkinfo[i].serverid);
		updatepacket.sendinfo[i].cost=htons(networkinfo[i].servercost);
	}
/**
	//Print made packet
	cout <<"\nPrinting the packet"<<endl;
	char str[17];
	cout<<"\nNum of update fields "<<ntohs(updatepacket.numfield);
	cout<<"\nNum of Server Port "<<ntohs(updatepacket.serverport);
	inet_ntop(AF_INET, &updatepacket.serverip.sin_addr, str, INET_ADDRSTRLEN);
	cout<<"\nServer IP "<<str<<"\n";
	for(i=0;i<myserver.numserver;i++)
	{
		inet_ntop(AF_INET, &updatepacket.sendinfo[i].ipaddr.sin_addr, str, INET_ADDRSTRLEN);
		cout<<"\nServer IP ADDRESS "<<i<<" "<<str;
		cout<<"\tPort "<<" "<<ntohs(updatepacket.sendinfo[i].port);
		cout<<"\tPadding "<<" "<<ntohs(updatepacket.sendinfo[i].x);
		cout<<"\tServer ID "<<" "<<ntohs(updatepacket.sendinfo[i].id);
		cout<<"\tCost "<<" "<<ntohs(updatepacket.sendinfo[i].cost);
		fflush(stdout);
	}
*/
}
int getserverid()
{
	int s_id=-1;


	for(int i=0;i<myserver.numneighbor;i++)
	{
		char str1[17];
		char str2[17];
		inet_ntop(AF_INET, &(recvpacket.serverip.sin_addr), str1, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(neighbor[i].serveripaddr.sin_addr), str2, INET_ADDRSTRLEN);
		if(strcasecmp(str1,str2)==0)
		{
			//Means packet is received from neighbor only
			s_id=neighbor[i].serverid;
			return s_id;
		}
	}

	return s_id;
}

void sortpacket(int ida)
{
	int i=0,j=0;
	int id[5];
    int ii, jj, aa, nn, number[5];
 	nn=5;
	//Save the ids in Array id
	for(i=0;i<5;i++)
	{
		id[i]=ntohs(recvpacket.sendinfo[i].id);
	}
	//cout<<"\nID RECEIVED "<<id[0]<<":"<<id[1]<<":"<<id[2]<<":"<<id[3]<<":"<<id[4]<<endl;


	//Sorting



    for (ii = 0; ii < nn; ii++)
	number[ii]=id[ii];

    for (ii = 0; ii < nn; ++ii)
    {
        for (jj = ii + 1; jj < nn; ++jj)
        {
            if (number[ii] > number[jj])
            {
            	aa =  number[ii];
                number[ii] = number[jj];
                number[jj] = aa;
            }
        }
    }
    for (ii = 0; ii < nn; ii++)
    	id[ii]=number[ii];

    //Sorted and saved the sorted ids in Array id
	for(i=0;i<5;i++)
	{
		for(j=0;j<5;j++)
		{
			if(ntohs(recvpacket.sendinfo[j].id)==id[i])
			{
				sortedpacket.sendinfo[i].id=ntohs(recvpacket.sendinfo[j].id);
				sortedpacket.sendinfo[i].cost=ntohs(recvpacket.sendinfo[j].cost);
				sortedpacket.sendinfo[i].ipaddr=recvpacket.sendinfo[j].ipaddr;
				sortedpacket.sendinfo[i].port=ntohs(recvpacket.sendinfo[j].port);
				sortedpacket.sendinfo[i].x=ntohs(recvpacket.sendinfo[j].x);
				break;
			}
		}
	}//End of For

	sortedpacket.numfield=ntohs(recvpacket.numfield);
	sortedpacket.serverip=recvpacket.serverip;
	sortedpacket.serverport=ntohs(recvpacket.serverport);

}//End of Function

void handlerecvdata(int ids)
{

	int i=0;

	/**
	 * Update the first receive flag in count variable if it is set to 0
	 * Signifying that the data is first time received from the neighbor
	 */
	for(i=0;i<myserver.numneighbor;i++)
	{
		if(counter[i].serverid==ids)
		{
			if(counter[i].firstrecv==0)
			counter[i].firstrecv=1;
		}
	}

    //Call function to sort the ids and make sorted receive packet
	sortpacket(ids);
	//Print the sorted receive packet
	    	for(i=0;i<5;i++)
	    	{

	    		cse4589_print_and_log("%-15d%-15d\n",sortedpacket.sendinfo[i].id,sortedpacket.sendinfo[i].cost);
	    	}
	/**
	    	cout<<"\nPrinting OLD COst matrix"<<endl;
    	for(int kk=0;kk<6;kk++)
    		{
    			for(int jj=0;jj<6;jj++)
    			{

    				cout<<samplecostmatrix[kk][jj]<<"\t";
    			}
    			cout<<"\n";
    		}
    	fflush(stdout);
*/
    //Process the data and update the sample cost matrix
	for(i=1;i<6;i++)
	{
		int tsoc=sortedpacket.sendinfo[i-1].cost;
		if(sortedpacket.sendinfo[i-1].cost==0)
		{
			tsoc=INF;
		}
		else
			samplecostmatrix[ids][i]= tsoc;

	}

/**
	//cout<<"\nPrinting CHanged COst matrix"<<endl;
	for(int kk=0;kk<6;kk++)
		{
			for(int jj=0;jj<6;jj++)
			{
	//			cout<<samplecostmatrix[kk][jj]<<"\t";
			}
	//		cout<<"\n";
		}

	fflush(stdout);
*/
	//Update counter for serverid from which packet was received
	for(i=0;i<5;i++)
	{
		if(ids==counter[i].serverid)
		{
			counter[i].updatecount=0;
			break;
		}

	}

	//New costs received from the neighbor


	applybellmanford();





}

//To check the command entered and separate the command arguments
int commandcheck()
{
	//check the command saved in the command structure

	int check=0;
	int i=0;
	int lenc=0;
	char *temp[5];
	char ss[30];
	int cmdnum;
	char cmdenter[100];

	strcpy(cmdenter,command.entercmd);

	bzero(&command.cmd,sizeof(command.entercmd));
	bzero(command.argument1,sizeof(command.argument1));
	bzero(command.argument1,sizeof(command.argument2));
	bzero(command.argument1,sizeof(command.argument3));

	char *token = strtok(cmdenter, " ");
	while (token != NULL)
	{
	        temp[i]=token;
//	        printf("\n\tThe command parts are %s",temp[i]);
	        i=i+1;
	        token = strtok(NULL, " ");
	}
	cmdnum=i;
//	cout <<"\nThe total command elements are "<<cmdnum;

	strcpy(ss,temp[0]);
	strcpy(command.cmd,ss);

	if(strcasecmp(command.cmd,"update")==0)
	{
		if(cmdnum==4)
		{
			i=0;
					char sx[30];
					strcpy(sx,temp[i+3]);
					lenc=strlen(sx)-1;
					if(sx[lenc]=='\n')
						sx[lenc]='\0';

					strcpy(command.argument1,temp[i+1]);
					strcpy(command.argument2,temp[i+2]);
					strcpy(command.argument3,sx);

					//Update the cost matrix
					int servr1=0;
					int servr2=0;
					int sercst=0;
					servr1=atoi(command.argument1);
					servr2=atoi(command.argument2);
					sercst=atoi(command.argument3);
					if(sercst>0)
					{
						samplecostmatrix[servr1][servr2]=sercst;
						for(int zx=0;zx<5;zx++)
						{
							if(networkinfo[zx].serverid==servr2)
							{
								networkinfo[zx].servercost=sercst;
								break;
							}
						}
						applybellmanford();
						check=1;
					}
					else
					{
						check=0;
						strcpy(commanderror,"cost given is less than 0");
					}

					if(check==1)
						cse4589_print_and_log("%s:SUCCESS\n",command.cmd);
					else
						check=0;

		}
		else
			strcpy(commanderror,"Less Number of Arguments");
		return check;
	}
	else if(strcasecmp(command.cmd,"disable")==0 && cmdnum==2)
	{
		i=0;
		char sx[30];
		int checkflag=0;
		strcpy(sx,temp[i+1]);
		lenc=strlen(sx)-1;
		if(sx[lenc]=='\n')
			sx[lenc]='\0';

		strcpy(command.argument1,sx);

		int cmd3=atoi(command.argument1);

		for(int xx=0;xx<myserver.numserver;xx++)
		{
			//Check if the argument is a neighbor
			if(cmd3==neighbor[xx].serverid)
			{
				checkflag=1;
				break;
			}
		}
		if(checkflag==1)
		{
			struct allinfo tempneighbor[5];
			int nl=0;
			int onum=myserver.numneighbor;
			//Delete from neighbor structure list
			for(nl=0;nl<onum;nl++)
			{
				//copy to the temp
				tempneighbor[nl]=neighbor[nl];

			}
			myserver.numneighbor=myserver.numneighbor-1;
			onum=myserver.numneighbor;
			for(nl=0;nl<onum;nl++)
			{
				if(tempneighbor[nl].serverid==cmd3)
				{
					cout<<"";
				}
				else
				{
					neighbor[nl]=tempneighbor[nl];
				}
			}

			//Update cost matrix and network info
			int xd=networkinfo[0].serverid;
			samplecostmatrix[xd][cmd3]= INF;
			for(nl=0;nl<5;nl++)
			{

				if(networkinfo[nl].serverid==cmd3)
				{
					networkinfo[nl].servercost= INF;
					break;
				}
			}

			//Set check as 2
			check=2;
		}
		else
		{
			check=0;
			strcpy(commanderror,"Argument not a neighbor");
		}


		if(check==2)
			cse4589_print_and_log("%s:SUCCESS\n",command.cmd);
		else
			check=0;

		return check;
	}
	else if(cmdnum==1)
	{
		i=0;
		char sx[30];
		sprintf(sx,"%s",temp[i]);
		lenc=strlen(sx)-1;
		if(sx[lenc]=='\n')
		sx[lenc]='\0';
		strcpy(command.cmd,sx);

		strcpy(command.entercmd,command.cmd);
		if(strcasecmp(command.cmd,"step")==0)
		{
			cse4589_print_and_log("%s:SUCCESS\n",command.cmd);
			//cout <<"\nInside step command"<<endl;
			check=3;
			makepacket();
			sendpacket();

		}
		else if(strcasecmp(command.cmd,"packets")==0)
		{
			//cout <<"\nInside packets command"<<endl;
			distancevectorpackets++;

			cse4589_print_and_log("%s:SUCCESS\n",command.cmd);
			cse4589_print_and_log("%d\n",distancevectorpackets);
			distancevectorpackets=0;

			check=4;
		}
		else if(strcasecmp(command.cmd,"display")==0)
		{

			cse4589_print_and_log("%s:SUCCESS\n",command.cmd);

			for(int x=0;x<myserver.numserver;x++)
			{
				cse4589_print_and_log("%-15d%-15d%-15d\n",routingtable[x].serverid,routingtable[x].reachcost,routingtable[x].nexthop_id);
			}

			check=5;
		}
		else if(strcasecmp(command.cmd,"crash")==0)
		{
			cse4589_print_and_log("%s:SUCCESS\n",command.cmd);
			while(1)
			{

			}
			check=6;
		}
		else if(strcasecmp(command.cmd,"dump")==0)
		{
			cse4589_print_and_log("%s:SUCCESS\n",command.cmd);
			makepacket();

			cse4589_dump_packet(&updatepacket,sizeof(updatepacket));
			check=8;
		}
		else if(strcasecmp(command.cmd,"academic_integrity")==0)
		{
			//cout <<"\nInside display command"<<endl;
			cse4589_print_and_log("%s:SUCCESS\n",command.cmd);
			cse4589_print_and_log("I have read and understood the course academic integrity policy located at http://www.cse.buffalo.edu/faculty/dimitrio/courses/cse4589_f14/index.html#integrity");
			check=7;
		}
		else
			check=0;

	}

	//Determine the error reason based on the commandentered
	if(check==0)
	{
		if(strcasecmp(command.cmd,"update")==0 && cmdnum!=4)
		{
			strcpy(commanderror,"Improper number of arguments");

		}
		if(strcasecmp(command.cmd,"disable")==0 && cmdnum!=2)
			strcpy(commanderror,"Improper number of arguments");
		else
			strcpy(commanderror,"Improper Command Syntax");
	}

	return check;
}
/**
 * Reference:
 * 1. http://www.thelearningpoint.net/computer-science/c-program-distance-vector-routing-algorithm-using-bellman-ford-s-algorithm
 * 2. http://www.sanfoundry.com/cpp-program-implement-bellmanford-algorithm/
 * 3. http://www.thelearningpoint.net/computer-science/algorithms-shortest-paths-in-graphs---the-bellman-ford-algorithm
 * */

void makeedge()
{
	int i=0,j=0; 	//COunters for parsing through the sample cost matrix

	int k=0;		//For keeping the number of edges
	//Re-initialize the number of edges in graph
	edgenumber=0;


	//It is an edge if the value of the index is not infinity
	for(i=1;i<6;i++)
	{
		for(j=1;j<6;j++)
		{
			if(samplecostmatrix[i][j]==65535)
			{
//				cout<<"no Edge"<<i<<j<<"\n";
			}
			else
			{
					//Add an edge
					edge[k].source=i;
					edge[k].destination=j;
					edge[k].weight=samplecostmatrix[i][j];
		//			printf("\nEdge %d %d:%d:%d",k+1,edge[k].source,edge[k].destination,edge[k].weight);
					k++;

			}
		}
	}
	edgenumber=k;
//	cout<<"\nThe edge number is "<<edgenumber;
}
/**
 * Reference:
 * 1. http://www.thelearningpoint.net/computer-science/c-program-distance-vector-routing-algorithm-using-bellman-ford-s-algorithm
 * 2. http://www.sanfoundry.com/cpp-program-implement-bellmanford-algorithm/
 * 3. http://www.thelearningpoint.net/computer-science/algorithms-shortest-paths-in-graphs---the-bellman-ford-algorithm
 * */
void applybellmanford()
{
	 edgenumber=0;
	 int graph[5][5]={0},size[5]={0};
     int cost[5][5]={0};
     int vertices,edges,iter;
     int vertex1,vertex2,weight;
     int k=0;		//For counter of number of edges

     bzero(&routingtable,sizeof(routingtable));


     //Make edges
     makeedge();

   /* vertices represent number of vertices and edges represent number of edges in the graph. */
     vertices=5;
     edges=edgenumber;
  //   cout<<"\nThe edge number is "<<edges;
	        /* Here graph[i][j] represent the weight of edge joining i and j */
	        for(iter=0;iter<edges;iter++)
	        {
	        	vertex1=edge[k].source-1;
	        	vertex2=edge[k].destination-1;
	        	weight=edge[k].weight;
	            k++;
//	        	printf("\n%d%d%d",vertex1,vertex2,weight);
	        	graph[vertex1][size[vertex1]] = vertex2;
	        	cost[vertex1][size[vertex1]] = weight;
	        	size[vertex1]++;
	        }

	        int source;
	        source= myserver.myid;
	        source=source-1;
	        BellmanFord(graph,cost,size,source,vertices);

}
/**
 * Reference:
 * 1. http://www.thelearningpoint.net/computer-science/c-program-distance-vector-routing-algorithm-using-bellman-ford-s-algorithm
 * 2. http://www.sanfoundry.com/cpp-program-implement-bellmanford-algorithm/
 * 3. http://www.thelearningpoint.net/computer-science/algorithms-shortest-paths-in-graphs---the-bellman-ford-algorithm
 * */
void BellmanFord(int graph[][maxVertices],int cost[][maxVertices],int size[],int source,int vertices)
{

        int distance[maxVertices];
        int hop[5];
        int iter,jter,from,to;
        for(iter=0;iter<vertices;iter++)
        {
                distance[iter] = INF;
        }
        distance[source] = 0;
        /* We have to repeatedly update the distance |V|-1 times where |V| represents
           number of vertices */
        for(iter=0;iter<vertices-1;iter++)
        {
                for(from=0;from<vertices;from++)
                {
                        for(jter=0;jter<size[from];jter++)
                        {
                                to = graph[from][jter];
                                if(distance[from] + cost[from][jter] < distance[to])
                                {

                                        distance[to] = distance[from] + cost[from][jter];
                                        hop[to]=from;
                                }

                        }
                }
        }
        memset(&routingtable,0,sizeof(routingtable));
        int ro=0;int ss=0;
        for(iter=0;iter<vertices;iter++)
        {
     //           printf("\nThe shortest distance to %d is %d via %d\n",iter+1,distance[iter],hop[iter]+1);
                routingtable[ro].serverid=iter+1;
                routingtable[ro].reachcost=distance[iter];
                if(source+1==iter+1)
                	routingtable[ro].nexthop_id=source+1;
                else
                	routingtable[ro].nexthop_id=hop[iter]+1;

                if(routingtable[ro].reachcost== 65535)
                {
                	routingtable[ro].nexthop_id=-1;
                }

                //Increment the counter variable
                ro++;
        }
        /**
        cout<<"\nPrevious Network INFO"<<endl;
        for(ro=0;ro<5;ro++)
        {
        	cout<<networkinfo[ro].serverid<<":"<<networkinfo[ro].servercost<<endl;
        }
*/


//        cout<<"\nResetting Network Info after bellmanFord\n";
        for(ro=0;ro<5;ro++)
        {
        	for(ss=0;ss<5;ss++)
        	{
        		if(networkinfo[ro].serverid==routingtable[ss].serverid)
        		{
      //  			cout<<"NW"<<networkinfo[ro].serverid<<"----->RO::"<<routingtable[ss].serverid;
        			networkinfo[ro].servercost=routingtable[ss].reachcost;
        			break;
        		}
        	}

        }

/**
        cout<<"\nChanged Network INFO"<<endl;
        for(ro=0;ro<5;ro++)
        {
        	cout<<networkinfo[ro].serverid<<":"<<networkinfo[ro].servercost<<endl;
        }
*/
}
//End of Function

//End of Program
