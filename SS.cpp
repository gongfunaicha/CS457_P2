// Main.cpp

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <cstdint>
#include <inttypes.h>

using namespace std;
										//test order call tree
int server();
int set_up_serv_port();

int client(string * ip, string * port, vector<string> * packet, int * server_socket);
void check_packet(string * inc, vector<string> * packet, bool * more_inc, bool * fetch_file);
void ip_port(string * ip, string * port, string * line, int * vector_size, vector<string> * packet);
void traceback(int * server_socket, int * client_socket);
void *handle_connection(void * c_sock);

void sendFile(string website, int server_socket);


int main(int argc, char* argv[]) {
	int o = 0; //used to check function return values.
	o = server();
	if(o == -1){ return -1;}
	return 0;
}

//calls the server functions one by one.
int server() {
	int o = 0; //check for errors.
	o = set_up_serv_port();
	if(o == -1){ return -1;}
	return 0;
}
  
//server functions in order
int set_up_serv_port(){ 					//returns socket descriptor if successful, -1 if it isnt't.
											//use stream socket (uses tcp port)
	int o = 0;							// used to check for bind errors.
	int sock = 0;							// used as the return value for socket call.
	sock = socket(AF_INET, SOCK_STREAM, 0); //s is the socket descriptor if this is 
										      //successful. else it could not create
	struct addrinfo hints, *s_info;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if (sock == -1) {
		cout << "Error: could not create socket. Program exiting. \n";
		return -1;
	}
	
	char name[150];
	gethostname(name, sizeof(name));			//host name is used to get ip address of server.
	  
	struct sockaddr_in server_ip;				//struct to hold the ip address of server
	server_ip.sin_family=AF_INET;
	  
	int port = 0; 							//hardcoded a port between 49152 and 65535. (could loop until it finds a free port)
	for(int i = 50000; i < 64000; i++){
		port = i;  
		server_ip.sin_port = htons(port);		//htons makes sure that the port number is converted to big.
		server_ip.sin_addr.s_addr = INADDR_ANY; //INADDR_ANY gets the address of the server
		o = bind(sock, (struct sockaddr *) &server_ip, sizeof(server_ip)); //binds the socket to the server's ip address--for some stupid reason it requires that you typecast the sockaddr_in to sock_addr.
		if(o == -1) {
			continue;
		} else {
			break;
		}
	}
	if(o == -1) {
		cout << "Error: could not bind to port " << port << " Program exiting. \n";
		return -1;
	}
	cout << "Waiting for a connection on ";

	  
															//get the address of computer to print.
	getaddrinfo(name, NULL, &hints, &s_info);
	struct sockaddr_in *printout = (struct sockaddr_in *)s_info->ai_addr;
	void * a = &(printout->sin_addr);
	char s[100];
	inet_ntop(s_info->ai_family, a, s, sizeof(s));
	printf("%s", s);
	  
	printf("%s", inet_ntoa(server_ip.sin_addr));
	cout  << " port " << port << "\n";
	  
	  
	struct sockaddr_in client_ip1;//up to five sock addrs can be used.							
	struct sockaddr_in client_ip2;
	struct sockaddr_in client_ip3;
	struct sockaddr_in client_ip4;
	struct sockaddr_in client_ip5;
	int connect_sock1 = 0, connect_sock2 = 0, connect_sock3 = 0, connect_sock4 = 0, connect_sock5 = 0;
	int conn_number = 0;
	pthread_t threads[5];
	socklen_t len = sizeof(client_ip1);
	int ret_code = 0;
	listen(sock, 5);									  //sets up the TCP port, listen for connections
	  
	while(true){										  //when a new connection arrives, dispatch a new thread to handle it.
		if(conn_number >= 5){
			cout << "Error: max number of connections reached";
			break;
		}
		switch(conn_number){								  //socklen_t is usually an int that holds the size of the client's sockaddr_in struct.
		case 1: {
					connect_sock1 = accept(sock, (struct sockaddr *) &client_ip1,  &len);  //when a connection comes in, writes ip info into client_addr, returns a new FD that is used for just this connection.
					ret_code = pthread_create(&threads[conn_number], NULL, handle_connection, (void *)(intptr_t)connect_sock1);
					break;
				}
		case 2: {
					connect_sock2 = accept(sock, (struct sockaddr *) &client_ip2,  &len);
					ret_code = pthread_create(&threads[conn_number], NULL, handle_connection, (void *)(intptr_t)connect_sock2);
					break;
				}
				
	    case 3: {
					connect_sock3 = accept(sock, (struct sockaddr *) &client_ip3,  &len);
					ret_code = pthread_create(&threads[conn_number], NULL, handle_connection, (void *)(intptr_t)connect_sock3);
					break;
				}
	    case 4: {
					connect_sock4 = accept(sock, (struct sockaddr *) &client_ip4,  &len);
					ret_code = pthread_create(&threads[conn_number], NULL, handle_connection, (void *)(intptr_t)connect_sock4);
					break;
				}
	    case 5: {
					connect_sock5 = accept(sock, (struct sockaddr *) &client_ip5,  &len);
					ret_code = pthread_create(&threads[conn_number], NULL, handle_connection, (void *)(intptr_t)connect_sock5);
					break;
				}	
		}
		
		//for checking on if statement: false = 0; true = 1
		if(connect_sock1 == -1 || connect_sock2 == -1 || connect_sock3 == -1 || connect_sock4 == -1 || connect_sock5 == -1 || ret_code) {
			cout << "Error: could not accept connection. \n";
			return -1;
		} else {
			conn_number++;
		}
	}
	return 0;
}
	  
	  
//*********************THIS SECTION OF CODE IS MULTITHREADED.*******************************
void *handle_connection(void * c_sock){	  
	 
	char incoming[140];
	vector<string> packet;
	int connect_sock;
	connect_sock = (intptr_t)c_sock;
	bool more_inc = true;	
	bool fetch_file = false;
	while(more_inc) {
	    bzero(incoming, 140);
	    recv(connect_sock, incoming, sizeof(incoming), 0); 				// block to recieve message into the buffer from client.
																		//do stuff with incoming. (need to take the buffer, pull the chars off of it, and put them into a string)
	    string inc(incoming, (sizeof(incoming)/sizeof(*incoming)));		//turn this into a string
	    check_packet(&inc, &packet, &more_inc, &fetch_file);							//check to see whether it is a url or a ip addr/port pair. if it is not the end of the stepping stones, go on to send out to next reciever.	
	}
     
    if(fetch_file) { //if this is true, time to call nick's method. else move to send to next computer in chain.
		
		//**************************************************************************************************
		
		//NICK, THIS IS WHERE YOU CAN PLUG IN YOUR METHOD OR CREATE AN INSTANCE OF YOUR PROGRAM
		
		sendFile(packet.at(0), connect_sock);
		
	} else {
		srand(time(NULL));
		unsigned int rando = rand() % packet.size();
		if(rando == 0) {
			rando++;
		} if(rando == packet.size() - 1) {				//make sure it doesn't send EOF as the line.
			rando--;
		}
		string ip = "";
		string port = "";
		string line = packet[rando];
		packet.erase(packet.begin() + rando);
		int packet_size = packet.size();
		ip_port(&ip, &port, &line, &packet_size, &packet);				//parses the chosen line into an ip and a port number
		client(&ip, &port, &packet, &connect_sock);			//sends the file and url to the next stop in the chain.
	}
	return 0;
}
  
  
int client(string * ip, string * port, vector<string> * packet, int * server_socket){ //testing will be on localhost, ip: 127.0.0.1
	struct sockaddr_in my_ip;			//set up the &%$$#$% sockaddr_in struct again... 
	int p_no = strtol(port->c_str(), NULL, 10);
	my_ip.sin_family=AF_INET;
	my_ip.sin_port = htons(p_no);
	const char *c = ip->c_str();
	my_ip.sin_addr.s_addr = inet_addr(c);
	  
	int cli_sock = socket(AF_INET, SOCK_STREAM, 0);		//open the port
	if(cli_sock == -1) {
		cout << "Error: could not create socket. Program exiting.";
		return -1;
	}
	cout << "Connecting to server...";
	cout << (*ip) << " " << (*port);
	int success = connect(cli_sock, (struct sockaddr *) &my_ip, sizeof(my_ip));
	if(success < 0) {
		cout << "Error: could not connect to given ip/port. Program exiting. \n";
		return -1;
	}
	cout << "Connected! \n";
	  
	char outgoing[140];
	  
	//*****read from beginning to end of vector and fire away to next in chain.
	for(unsigned int i = 0; i < packet->size(); i++) {
		bzero(outgoing, 140);
	    string temp = packet->at(i);
	    strncpy(outgoing, temp.c_str(), sizeof(outgoing));
	    outgoing[sizeof(outgoing) -1] = 0;
	    
	    send(cli_sock, outgoing, sizeof(outgoing), 0);
	}
	
	traceback(server_socket, &cli_sock);					//this handles the pass back of information.
	return 0;
}
  
void check_packet(string * inc, vector<string> * packet, bool * more_inc, bool * fetch_file) {		//checks an incoming string to see if it is a url or part of the file. puts it into the correct place in the vector.
	string temp = inc->substr(0, 2);
	string end = inc->substr(0, 3);
	if(!temp.compare("U:")) {
		//then it is a URL, make sure it is at the front. (assuming proper format it will always be in front.)
		packet->push_back((*inc));
	} else if(!temp.compare("P:")) {
		//then it is an ip/ port combo, make sure it is at the back.
		packet->push_back((*inc));
	} else if(!temp.compare("**")) {
		//send to final method that retrieves the file and ports it thru the connection.
		(*fetch_file) = true;
	} else if(!end.compare("EOF")) {
		(*more_inc) = false;
		packet->push_back((*inc));
	}
}
  
void ip_port(string * ip, string * port, string * line, int * vector_size, vector<string> * packet) 
	size_t ip_start = line->find("S");
	ip_start--;
	(*port) = line->substr(2, ip_start); 
	size_t ip_end = line->find("/");
	ip_end--;
	for(unsigned int i = 0; i < 3; i++) {
		ip_start++;
	}
	(*ip) = line->substr(ip_start, ip_end);
	cout << "Port: " << (*port);
	cout << " IP address: " << (*ip);
	if((*vector_size) == 2) {
		packet->insert(packet->begin() + 1, "**");		//add special character meaning all connections are set up.
	}
}
 
void traceback(int * server_socket, int * client_socket) {			//server and client socket are used to pass information back through the chain, then destroys the connection.
	int to_recieve = 0;
	int net_recieve = 0;
	int total_length = 0;

	recv(*client_socket, &net_recieve, sizeof(net_recieve) , 0); 				//block to recieve the total file size first.
	total_length = ntohl(net_recieve);
	send(*server_socket, (const char*)&net_recieve, sizeof(net_recieve), 0);
	
	while(total_length != 0) {
	    recv(*client_socket, &net_recieve, sizeof(net_recieve), 0);
	    to_recieve = ntohl(net_recieve);							//packet length
		send(*server_socket, (const char*)&net_recieve, sizeof(net_recieve), 0);		//send it to the next in the chain.
		if(to_recieve == 0) {
			break;
		} else {
			total_length -= to_recieve;
		}
		char packet_body[to_recieve];
		bzero(packet_body, to_recieve);
		recv(*client_socket, packet_body, sizeof(packet_body), 0);
		send(*server_socket, packet_body, sizeof(packet_body), 0);																//do stuff with incoming. (need to take the buffer, pull the chars off of it, and put them into a string)  
	}
	
	pthread_exit(NULL);			//terminates this thread.
}

int downloadFile(string website) {
	string command = "wget " + website + " -O wget.txt";
	system((const char*)command.c_str());
	return 0;
}

void sendFile(string website, int sockfd) {
	downloadFile(website);
	
	int LENGTH = 512;
	string fs_name_str = "wget.txt";
	const char* fs_name = fs_name_str.c_str();
    char sdbuf[LENGTH]; 


	//Send the file name
    int n = write(sockfd, fs_name, strlen(fs_name));
    if(n < 0)
		printf("Error: sending filename");
            

    printf("[Client] Sending %s to the Server... ", fs_name);
    FILE* fs = fopen(fs_name, "r");
    if(fs == NULL) {
        printf("ERROR: File %s not found.\n", fs_name);
        exit(1);
    }

    bzero(sdbuf, LENGTH); 
    int fs_block_sz, errno = 0;
    while((fs_block_sz = fread(sdbuf, sizeof(char), LENGTH, fs)) > 0) {
        if(send(sockfd, sdbuf, fs_block_sz, 0) < 0) {
            fprintf(stderr, "ERROR: Failed to send file %s. (errno = %d)\n", fs_name, errno);
            break;
        }
        bzero(sdbuf, LENGTH);
    }
    
    remove(fs_name);
    
    
    printf("Ok File %s from Client was Sent!\n", fs_name);
}