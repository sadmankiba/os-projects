#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "mfs.h"
#include "udp.h"
#include "message.h"
#include "debug.h"

// this will send messages between the server and client that contain
// a message_t struct that is updated by other methods
int Server_To_Client(message_t *send, message_t *receive, char *server, int pnum)
{

	// open
	int sd = UDP_Open(0);
	if(sd < -1){
		// open failure
		perror("udp_send: failed to open socket.");
		return -1;
	}

	// set up sock_addr structs to be used for opening
	struct sockaddr_in sock;
	struct sockaddr_in sock1;
	int rc = UDP_FillSockAddr(&sock, server, pnum);

	// timeval struct to keep track of time passing
	struct timeval tv;
	tv.tv_usec=0; // set to 0
	tv.tv_sec=3; // set to 3

	// check to make sure fill was successful
	if(rc < 0){
                perror("upd_send: failed to find host");
                return -1;
        }

	// set value to stop trying
	int timeout = 5;
	fd_set set;
	while(1){
		// usf FDZERO on set
		FD_ZERO(&set);
		
		// set set
		FD_SET(sd,&set);

		// Write using the udp_write funcction
		UDP_Write(sd, &sock, (char*)send, sizeof(message_t));

		// make sure this was successful
		if(select(sd+1, &set, NULL, NULL, &tv)){

			// read using udp_read
			rc = UDP_Read(sd, &sock1, (char*)receive, sizeof(message_t));

			// check to make sure read was successful
			if(rc > 0){

				// close open port
				UDP_Close(sd);
				return 0;
			}
		}else{

			// wait for one less second now
			timeout --;
		}
	}
}

// important global variables
char* my_serv = NULL; // server being used
int working = 0; // make sure the server is currently working
int prt = 10000; // base port


// MFS_Init method, sets up server and port
int MFS_Init(char *hostname, int port) {
	// set proper port
	prt = port;

	// say that server is working
	working = 1;
	
	// set proper server name
	my_serv = strdup(hostname); 
	return 0;
}

// MFS_Lookup, looks up name based on given pinum and name
int MFS_Lookup(int pinum, char *name){
	// fail if name is too large
	if(strlen(name) > 28){
		return -1;
	}
	// fail if name is null
	if(name == NULL){
		return -1;
	}

	// create sending message
	message_t send;

	// set fields of the message
	send.node_num = pinum;
	strcpy((char*)&(send.name), name);
	send.msg = MFS_LOOKUP;

	// make sure server is currently working
	if(!working){
        return -1;
    }

	// create receiving message
	message_t receive;

	// send the message to the client
	int ret = Server_To_Client( &send, &receive, my_serv, prt);
	debug("In MFS_Lookup: server retcode %d, received inum %d\n", ret, receive.node_num);
		
	if(ret <= -1){
		return -1;
	}
	else{
		// return the inum of the received message
		return receive.node_num;
	}
}

// MFS_Stat method, takes in stat structure and inum used to get stats of a message
int MFS_Stat(int inum, MFS_Stat_t *m) {
	// create the meessage to be sent
	message_t send;

	// set fields of the message struct
	send.msg = MFS_STAT;
	send.node_num = inum;
	
	// check to make sure the server is functioning
	if(!working){
                return -1;
	}

	// message to ve received
	message_t receive;

	// send between clent and server
	if(Server_To_Client(&send, &receive, my_serv, prt) <= -1){
		return -1;
	}

	// filil up the struct passed with the proper info
	m->type = receive.st.type;
	m->size = receive.st.size;

	return 0;
}

// MFS_Write method, writes inum with given buffer, offset, and number of bytes
int MFS_Write(int inum, char *buffer, int offset, int nbytes){
	// message being sent
	message_t send;

	// fill the buffer field of the sending message
  	for(int i = 0; i<4096; i++){
                send.buf[i] = buffer[i];
        }

	// fill out other fields
	send.nbytes = nbytes;
	send.msg = MFS_WRITE;
	send.offset = offset;
	send.node_num = inum;

	// dont forget to check taht server is funcitoning
	if(!working){
                return -1;
        }
	// message client is receiving
	message_t receive;

	// send between server and client
	if(Server_To_Client(&send, &receive, my_serv, prt) <= -1){
		return -1;
	}

	// return the inode number of the reciever
	return receive.node_num;
}


int MFS_Read(int inum, char *buffer, int offset, int nbytes){	
	message_t send;

	send.nbytes = nbytes;
	send.node_num = inum;
	send.msg = MFS_READ;
	send.offset = offset;

	if(!working){
        return -1;
    }

	message_t receive;

	if(Server_To_Client(&send, &receive, my_serv, prt) <= -1){
		return -1;
	}

	// different than write, need to make sure the inum is not negative
	if(receive.node_num >= 0) {

		// loop through and get proper buffer
		for(int i = 0; i<4096; i++){
			// reversed this time
			buffer[i] = receive.buf[i];
		}
	}

	return receive.node_num;
}

// MFS_Creat creates a dir/file based on typ with a name and pinum specified in parameters
int MFS_Creat(int pinum, int type, char *name){
	// check if name is null
	if(name == NULL){
		return -1;
	}

	// check that name isn't too long
	if(strlen(name) > 28){
		return -1;
	}

	// message to send
	message_t send;

	// use parameters to fill the message
	send.mtype = type;
	send.msg = MFS_CREAT;
	send.node_num = pinum;
	strcpy(send.name, name);
	
	// almost forgot to make sure server is online!
	if(!working){
                return -1;
        }

	// receive the message
	message_t receive;

	// do the transaction	
	if(Server_To_Client(&send, &receive, my_serv, prt) <= -1){
		return -1;
	}

	return receive.node_num;
}

// MFS_Unlilnk method, unlinks based on pinum and name parameters
int MFS_Unlink(int pinum, char *name){
	// check name length
	if(strlen(name) > 28){
		return -1;
	}

	// check that name is valid
	if(name == NULL){
		return -1;
	}

	// sending message
	message_t send;

	// fill that message
	send.msg = MFS_UNLINK;
	strcpy(send.name, name);
	send.node_num = pinum;

	// obviously this has to be done yet again
	if(!working){
                return -1;
        }

	// receive!
	message_t receive;

	// actually send!	
	if(Server_To_Client(&send, &receive, my_serv, prt) <= -1){
		return -1;
	}

	return receive.node_num;
}

int MFS_Shutdown(){
	message_t send;
	send.msg = MFS_SHUTDOWN;
	message_t receive;

	if(Server_To_Client(&send, &receive, my_serv, prt) <= -1){
		return -1;
	}

	return 0;
}
