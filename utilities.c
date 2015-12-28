/*
* Authors: Harishkumar Katagal(109915793) & Gagan Nagaraju (109889036)
* FileName: utilities.c
* 
*/

#include "unp.h"
#include "utilities.h"


void msg_send(int sockfd, char *destnIP, int destPort, int flag, char* msg){
	struct data	datasend;
	memcpy(datasend.destIP, destnIP, IPLEN);
	printf("\nSending SourceIP:%s\n",datasend.sourceIP);
	printf("\nSending destIP:%s\n",datasend.destIP);
	datasend.destPort = destPort;
	datasend.flag = flag;
	strncpy(datasend.msg,msg,strlen(msg));
	Sendto(sockfd,&datasend,sizeof(datasend),0,NULL,NULL);
}

/* msg_send function */
void Msg_Send(int sockfd, char *srcIP, char *destnIP, int srcPort, int destPort, int flag, char* msg){
	struct data	datasend;
	char path[27];
	struct sockaddr_un cliaddr;
	bzero(&cliaddr,sizeof(cliaddr));
	memcpy(datasend.sourceIP, srcIP, IPLEN);
	memcpy(datasend.destIP, destnIP, IPLEN);
	//printf("\nSending SourceIP:%s\n",datasend.sourceIP);
	//printf("\nSending destIP:%s\n",datasend.destIP);
	datasend.destPort = destPort;
	datasend.srcPort = srcPort;
	datasend.flag = flag;
	cliaddr.sun_family = AF_LOCAL;
	strncpy(path,ODRPATH,19);
	strcpy(cliaddr.sun_path,path);	
	bzero(&datasend.msg,sizeof(datasend.msg));
	memcpy((void*)datasend.msg,(void*)msg,strlen(msg));
	Sendto(sockfd,&datasend,sizeof(datasend),0,(SA*)&cliaddr,sizeof(cliaddr));
}

/* msg_recv function */
int msg_recv(int sockfd, char *msg, char* srcIP, int* port){
	struct data	datarecv;
	int size;
	bzero(msg,sizeof(msg));
	Recvfrom(sockfd,&datarecv, sizeof(datarecv),0,NULL,NULL);
	strncpy(msg,datarecv.msg,strlen(datarecv.msg));
	//printf("\nData received1:%s\n",datarecv.sourceIP);
	strncpy(srcIP,datarecv.sourceIP,strlen(datarecv.sourceIP));
	//printf("\nData received2:\n");
	*port = datarecv.srcPort;
	//printf("\nData received3:\n");
	size = strlen(msg);
	//printf("\nData received4:\n");
	return size;
	
}

int odr_recv(int sockfd, struct data *odr_data,struct sockaddr_un *connaddr){
	struct data	datarecv;
	int size;
	int connlen= sizeof(struct sockaddr_un);
	Recvfrom(sockfd,&datarecv, sizeof(datarecv),0,(SA*)connaddr,&connlen);
	strncpy(odr_data->msg,datarecv.msg,strlen(datarecv.msg));
	//printf("\nData received1:%s\n",datarecv.sourceIP);
	strncpy(odr_data->sourceIP,datarecv.sourceIP,strlen(datarecv.sourceIP));
	strncpy(odr_data->destIP,datarecv.destIP,strlen(datarecv.destIP));
	odr_data->flag = datarecv.flag;
	//printf("\nData received2:\n");
	odr_data->srcPort = datarecv.srcPort;
	odr_data->destPort = datarecv.destPort;
	//printf("\nData received3:\n");
	size = strlen(datarecv.msg);
	//printf("\nData received4:\n");
	return size;
	
}

void odr_send(int sockfd, struct data *odr_data, char *paths){
	struct data	datasend;
	//char path[35];
	struct sockaddr_un cliaddr;
	bzero(&cliaddr,sizeof(cliaddr));
	printf("Here\n");
	printf("ODR CLI:%s\n",odr_data->sourceIP);
	memcpy(datasend.sourceIP, odr_data->sourceIP, IPLEN);
	memcpy(datasend.destIP, odr_data->destIP, IPLEN);
	//printf("\nSending SourceIP:%s\n",datasend.sourceIP);
	//printf("\nSending destIP:%s\n",datasend.destIP);
	datasend.srcPort = odr_data->srcPort;
	datasend.destPort = odr_data->destPort;
	datasend.flag = odr_data->flag;
	cliaddr.sun_family = AF_LOCAL;
	//strncpy(path,SERVPATH,28);
	//strcpy(cliaddr.sun_path,paths);	
	memcpy(cliaddr.sun_path,paths,strlen(paths));
	printf("Server path:%s\n",cliaddr.sun_path);
	strncpy(datasend.msg,odr_data->msg,strlen(odr_data->msg));
	Sendto(sockfd,&datasend,sizeof(datasend),0,(SA*)&cliaddr,sizeof(cliaddr));
}
