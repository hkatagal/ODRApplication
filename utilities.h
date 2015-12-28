/*
* Authors: Harishkumar Katagal(109915793) & Gagan Nagaraju (109889036)
* FileName: utilities.h
* 
*/
#ifndef UTILITY_H_
#define UTILITY_H_

#include "unp.h"

#define ODRPATH "/tmp/odrpath_7747"
#define SERVPATH "/tmp/server_hkatagal_7747"
#define LEN 200
#define IPLEN 50
#define PORT 7747
#define CLI_PORT 7748





struct data{
	char sourceIP[IPLEN];
	char destIP[IPLEN];
	int srcPort;
	int destPort;
	int flag;
	char msg[LEN];
};

void Msg_Send(int sockfd, char *srcIP, char *destnIP, int srcPort, int destPort, int flag, char* msg);
void msg_send(int sockfd, char *destnIP, int destPort, int flag, char* msg);
int msg_recv(int sockfd, char *msg, char* srcIP, int* port);
int odr_recv(int sockfd, struct data *odr_data,struct sockaddr_un *connaddr);
void odr_send(int sockfd, struct data *odr_data, char *paths);

#endif
