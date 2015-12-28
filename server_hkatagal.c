/*
* Authors: Harishkumar Katagal(109915793) & Gagan Nagaraju (109889036)
* FileName: server_hkatagal.c
* 
*/


#include "unp.h"
#include <time.h>
#include "utilities.h"

int getsockfd();
char cli_path[35];
int main(int argc, char **argv){
	int sockfd;
	char servername[128];
	int maxfd=-1;
	fd_set rset;
	int r=0;
	char msg[50];
	char srcIP[IPLEN];
	struct in_addr addr;
	struct hostent *hptr;
	int port;
	time_t ts;
	char buf[512];
	int sel;
	char **pptr;
	char str[INET_ADDRSTRLEN];
	const char *serverip, *destIP;
	
	gethostname(servername,128);
	printf("\nServer Running on: %s\n",servername);
	if((hptr = gethostbyname(servername))==NULL){
		perror("Error: Invalid host name. Terminating.");
		exit(0);
	}
	pptr = hptr->h_addr_list;
	destIP = Inet_ntop(hptr->h_addrtype,*pptr,str,sizeof(str));
	//printf("The server IP is: %s\n",destIP);
	sockfd = getsockfd();
	//printf("Socket Created:%d\n",sockfd);
	maxfd = sockfd+1;
	FD_ZERO(&rset);
	for(;;){
	/*	printf("\nBack to Select\n");
		FD_SET(sockfd,&rset);
		if(sel = select(maxfd,&rset,NULL,NULL,NULL)<0){
			if(errno == EINTR)
				continue;
			else{
				perror("Select error.\n");
				break;
			}	
		}
		if(FD_ISSET(sockfd,&rset)){*/
			r = msg_recv(sockfd,msg,srcIP,&port);
			if(r<0){
				perror("Receive Error\n");
			}
			
			inet_pton(AF_INET,srcIP,&addr);
			if((hptr = gethostbyaddr(&addr,sizeof(addr),AF_INET))==NULL){
				perror("Error: Invalid IP address. Terminating.");
				exit(0);
			}
			printf("\n\nserver at node %s responding to request from %s\n\n",servername,hptr->h_name);
			//printf("The client host is : %s\n",hptr->h_name);
			bzero(buf,sizeof(buf));
			ts = time(NULL);
			snprintf(buf,sizeof(buf),"%.24s\r\n",ctime(&ts));
			Msg_Send(sockfd, destIP, srcIP,7747, port, 0, buf);
			
		//}
	}
}

int getsockfd(){
	int sockfd;
	struct sockaddr_un	cliaddr;
	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
	bzero(&cliaddr, sizeof(cliaddr));
	memcpy(cli_path,SERVPATH,28);
	printf("Path : %s\n",cli_path);
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path,cli_path);
	unlink(cli_path);
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	return sockfd;
}
