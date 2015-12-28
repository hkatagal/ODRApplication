/*
* Authors: Harishkumar Katagal(109915793) & Gagan Nagaraju (109889036)
* FileName: client_hkatagal.c
* 
*/



#include "unp.h"
#include "utilities.h"


int getsockfd();
char cli_path[27];
int main(int argc, char **argv){
	int vm_select;
	int sockfd;
	struct sockaddr_un	cliaddr, servaddr,addr2;
	char path[27];
	int len=0;
	char servername[5];
	struct hostent *hptr;
	char **pptr;
	const char *serverip, *clientip;
	char str[INET_ADDRSTRLEN], str1[INET_ADDRSTRLEN];
	char cliname[128];
	char msg[50];
	int temp=0;
	int port = CLI_PORT;
	int srcPort = CLI_PORT;
	int destPort = PORT;
	char c;
	struct timeval timer;
	fd_set rset, allset;
	int maxfd=-1;
	int nready;
	int time_count = 0;
	
	
	
	gethostname(cliname,128);
	//printf("\nHostName: %s\n",cliname);
	sockfd = getsockfd();
	bzero(&servaddr, sizeof(servaddr));	
	
	servaddr.sun_family = AF_LOCAL;
	strncpy(path,ODRPATH,19);
	//unlink(path);
	strcpy(servaddr.sun_path,path);	
	//printf("\n ODRPATH: %s\n",servaddr.sun_path);
	len = sizeof(addr2);
	Getsockname(sockfd, (SA *) &addr2, &len);
	temp = atoi(cliname+2);
	sprintf(cliname,"vm%d",temp,4);
	printf("\nClient is on: %s\n",cliname);
	if((hptr = gethostbyname(cliname))==NULL){
		perror("Error: Invalid host name. Terminating.");
		exit(0);
	}
	pptr = hptr->h_addr_list;
	clientip = Inet_ntop(hptr->h_addrtype,*pptr,str,sizeof(str));
	printf("The client IP is: %s\n",clientip);
	printf("bound name = %s, returned len = %d\n", addr2.sun_path, len);
	for(;;){
		printf("\n***************************************************\n");
		printf("\nEnter the server number that you want to connect to.\nEnter a number between: 1 to 10. Enter 0 to exit\n");
		if((scanf("%d",&vm_select)) == 1){
			c = getchar();
			if((vm_select < 0 ) || (vm_select > 10)){
				printf("\nError: Invalid server number.\n");
				continue;
			}
			else if(vm_select == 0){
				printf("\nExiting client\n");
				printf("\n***************************************************\n");
				return 0;
			}
			else{
				sprintf(servername,"vm%d",vm_select,4);
				printf("\nSelected server vm%d\t%s.\n",vm_select,servername);
				if((hptr = gethostbyname(servername))==NULL){
					perror("Error: Invalid host name. Terminating.");
					exit(0);
				}
				pptr = hptr->h_addr_list;
				printf("\nclient at node %s sending request to server at %s.\n\n",cliname,servername);
				//printf("The client IP before: %s\n",clientip);
				serverip = Inet_ntop(hptr->h_addrtype,*pptr,str1,sizeof(str1));
				//printf("The server IP is: %s\n",serverip);
				//printf("The client IP after: %s\n",clientip);
				//Connect(sockfd,(SA*)&servaddr,sizeof(servaddr));
				strncpy(msg,"Time",5);
				Msg_Send(sockfd, clientip, serverip, srcPort, destPort, 0, msg);
				timer.tv_sec = 10;
				timer.tv_usec = 0;
				FD_ZERO(&rset);
				maxfd = sockfd+1;
		start:	for(;;){
					FD_ZERO(&rset);
					FD_SET(sockfd,&rset);
					maxfd = sockfd+1;
					timer.tv_sec = 5;
					timer.tv_usec = 0;
					if(nready = select(maxfd,&rset,NULL,NULL,&timer)<0){
						if(errno == EINTR)
							continue;
						else{
							perror("Select error\n");
							break;
						}	
					}
					if(FD_ISSET(sockfd,&rset)){
						bzero(msg,sizeof(msg));
						msg_recv(sockfd,msg,serverip,&port);
						printf("\nclient at node %s : received from server at %s, time: %s\n\n",cliname,servername,msg);
						break;
				//printf("Time:%s\n",msg);
					}
					else{
						time_count++;
						bzero(msg,sizeof(msg));
						strncpy(msg,"Time",5);
						if(time_count==1){
							printf("\nclient at node %s : timeout on response from server at %s.\n",cliname,servername);
							printf("\nclient at node %s sending request again to server at %s.\n\n",cliname,servername);
							Msg_Send(sockfd, clientip, serverip, srcPort, destPort, 1, msg);	
							//goto start;
						}
						else{
							printf("\nclient at node %s : timeout on response from server at %s.\n",cliname,servername);
							printf("\ngiving up on server at: %s\n",servername);
							break;
						}
					}
				}
			}
			printf("\n***************************************************\n");
		}
		else {
			c = getchar();
			printf("\nError: Invalid input.\n");
			printf("\n***************************************************\n");
			return 0;
			
		}
		
	}
	unlink(cli_path);
}

/*
* Creates socket
*/

int getsockfd(){
	int sockfd;
	struct sockaddr_un	cliaddr;
	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
	bzero(&cliaddr, sizeof(cliaddr));
	strncpy(cli_path,"/tmp/cli_hkatagal_XXXXXX",26);
	if((mkstemp(cli_path))<0){
		perror("Error: mkstemp creation error\n");
		exit(0);
	}
	unlink(cli_path);
	printf("Path : %s\n",cli_path);
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path,cli_path);
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	return sockfd;
}
