/*
* Authors: Harishkumar Katagal(109915793) & Gagan Nagaraju (109889036)
* FileName: odr_hkatagal.c
* 
*/

#include<unp.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <time.h>
#include "hw_addrs.h"
#include "utilities.h"
#include "odr_hkatagal.h" 



char cli_path[27];
struct routetable routeTab[25];
int routecount =0;
int stale_time = 60;
int unsockfd;
struct interfaces myodr;
struct parameters parameter;
struct packetlist *pktlst;
struct interfaces head[20];
static struct portpath porttab[100];
static struct rreqarray rreqtab[50];
struct sockaddr_un connaddr;
char odrname[128];

int main(int argc, char **argv){
	struct sockaddr	*sa;
	int prflag = 0;
	int i=0,r=0,sel=0;
	char *ptr;
	int connlen;
	
	fd_set rset;
	int maxfd=-1;
	char clientip[50];
	char msg[50];
	int port=0;
	
	int if_count=0;
	char ip_temp[50];
	struct etherpayload *buffer;
	struct data *odr_data;
	if(argc<2){
		printf("Specify stale time.\n");
		exit(0);
	}
	stale_time = atoi(argv[1]);
	//printf("Entered Stale time:%d\n",stale_time);
	setparameters();
	gethostname(odrname,128);
	unsockfd = getsockfd();
	maxfd = unsockfd;
	FD_ZERO(&rset);
	parameter.if_count = getallinterfaces(head);
	createpfsockets(head,parameter.if_count);
	//printf("\nCreated PF sockets\n");
	
	for(i=0;i<parameter.if_count;i++){
		maxfd = max(maxfd,head[i].sockfd);
		//printf("sockfd:%d\n",head[i].sockfd);
	}
	maxfd = maxfd+1;
	//printf("MAXFD:%d\n",maxfd);
	for(;;){
	//	printf("\nBack to Select\n");
		FD_SET(unsockfd,&rset);
		for(i=0;i<parameter.if_count;i++){
			FD_SET(head[i].sockfd,&rset);
		}
		if(sel = select(maxfd,&rset,NULL,NULL,NULL)<0){
			if(errno == EINTR)
				continue;
			else{
				perror("Select error.\n");
				break;
			}	
		}
		if(FD_ISSET(unsockfd,&rset)){
			odr_data = malloc(sizeof(struct data));
			//if(r = msg_recv(unsockfd, msg, clientip, &port)>0){
			if(r = odr_recv(unsockfd, odr_data, &connaddr)>0){
				//printf("Message Received: %s\n",msg);
				//printf("Server IP: %s\n",odr_data->sourceIP);
				//printf("\n Packet Received:%s\n",connaddr.sun_path);
				handlenewclient(odr_data);
			}
			else if(r<0 && errno ==EINTR)
				continue;
			else if(r<=0){
				break;
			}
		}
		for(i=0;i<parameter.if_count;i++){
			if(FD_ISSET(head[i].sockfd,&rset)){
				//printf("\n Inside Select\n");
				buffer = malloc(sizeof(struct etherpayload));
				bzero(buffer,sizeof(struct etherpayload));
				connlen = sizeof(connaddr);
				if (recvfrom(head[i].sockfd, buffer, sizeof(struct etherpayload), 0, NULL,NULL) < 0) {
					err_msg("Error in receiving Ethernet packet");
				}
				
				handlepfrequest(buffer,head[i].if_index,i);
			}
		}
	} 

	
	
	
	
	
	
	
	
}

void setparameters(){
	int i =0;
	char strtemp[LEN];
	parameter.if_count = 0;
	parameter.route_count = 0;
	parameter.rreq_count = 0;
	parameter.broadcastID = 1000;
	parameter.port_count = 2000;
	parameter.port_index = 1;	
	porttab[i].port = PORT;
	strcpy(strtemp,SERVPATH);
	memcpy((void *)porttab[i].sunpath,(void *) strtemp,strlen(strtemp));
	porttab[i].valid = 1;
	porttab[i].ts = time(NULL);
}


int getsockfd(){
	int sockfd;
	struct sockaddr_un	cliaddr;
	sockfd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
	bzero(&cliaddr, sizeof(cliaddr));
	strncpy(cli_path,ODRPATH,26);
	printf("Path : %s\n",cli_path);
	cliaddr.sun_family = AF_LOCAL;
	strcpy(cliaddr.sun_path,cli_path);
	unlink(cli_path);
	Bind(sockfd, (SA *) &cliaddr, sizeof(cliaddr));
	return sockfd;
}


int getallinterfaces(struct interfaces *head){
	int if_count = 0;
	char ip_temp[50];
	int i=0,j=0;
	struct hwa_info		*hwa, *hwahead;
	for (hwahead = hwa = Get_hw_addrs(); hwa != NULL; hwa = hwa->hwa_next) {
		
	 	/*  printf("Interface name: %s\n",hwa->if_name);
		printf("Hardware Address: %s\n",hwa->if_haddr);
		printf("Interface index: %d\n",hwa->if_index);
		printf("IP Alias: %d\n",hwa->ip_alias);   */
		
		if(strcmp(hwa->if_name,"eth0")==0){
			strncpy(myodr.if_name,hwa->if_name,strlen(hwa->if_name));
			myodr.if_index = hwa->if_index;
			bzero(&ip_temp, sizeof(ip_temp));
			Inet_ntop(AF_INET, &(((struct sockaddr_in *)hwa->ip_addr)->sin_addr),ip_temp,sizeof(ip_temp));
			strncpy(myodr.ipaddr,ip_temp,strlen(ip_temp));
			memcpy(myodr.mac, hwa->if_haddr, IF_HADDR);
			memcpy(myodr.if_haddr, hwa->if_haddr, IF_HADDR);
			/*printf("Print MAC:");
			for(j=0;j<IF_HADDR;j++){
					printf("%d:",myodr.mac[j]);
			}
			printf("\n");*/
			
		}
		
		
		if((strcmp(hwa->if_name,"eth0")!=0) && (strcmp(hwa->if_name,"lo")!=0)){
			strncpy(head[if_count].if_name,hwa->if_name,strlen(hwa->if_name));
			head[if_count].if_index = hwa->if_index;
			bzero(&ip_temp, sizeof(ip_temp));
			Inet_ntop(AF_INET, &(((struct sockaddr_in *)hwa->ip_addr)->sin_addr),ip_temp,sizeof(ip_temp));
			strncpy(head[if_count].ipaddr,ip_temp,strlen(ip_temp));
			memcpy(head[if_count].mac, hwa->if_haddr, IF_HADDR);
			memcpy(head[if_count].if_haddr, hwa->if_haddr, IF_HADDR);
			/*printf("\nInterface\n");
			for(i=0;i<IF_HADDR;i++){
				printf("%d\t",head[if_count].mac[i]);
			}*/	
			if_count++;
		}
	}
	return if_count;
}

void createpfsockets(struct interfaces *head, int if_count){
	int i=0;
	struct sockaddr_ll socket_address;
	socket_address.sll_family   = PF_PACKET;
	socket_address.sll_protocol = htons(PROTOCOL);
	socket_address.sll_hatype   = ARPHRD_ETHER;
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	socket_address.sll_halen    = ETH_ALEN;	
	
	for(i=0;i<if_count;i++){
		socket_address.sll_ifindex = head[i].if_index;
		head[i].sockfd = Socket(PF_PACKET,SOCK_RAW,htons(PROTOCOL));
		memcpy(socket_address.sll_addr,head[i].if_haddr,IF_HADDR);
		Bind(head[i].sockfd,(SA*)&socket_address,sizeof(socket_address));
	}
	return;
}


void handlenewclient(struct data *odr_data){
	//Check if route exits in the routing table
	//If exits use the route
	//Otherwise create a new RREQ and send 
	int flag;
	int index = -1;
	struct odrpacket *odr_packet;
	int srcport = PORT;
	flag = checkforroute(odr_data->destIP, &index);
	//printf("Source Port before if: %d\n",odr_data->srcPort);
	if(odr_data->srcPort != PORT){
		//printf("Source Port odr data: %d\n",odr_data->srcPort);
		odr_data->srcPort = updateportpath();
		//printf("Source Port after assigning: %d\n",odr_data->srcPort);
	}
	
	if(odr_data->flag == 1 || flag == 0 || flag == 1){
		//Generate RREQs
		odr_packet = malloc(sizeof(struct odrpacket));
		buildodrpacket(odr_data,odr_packet,0,srcport);
		//printf("Type after return:%d\n",odr_packet->type);
		//add odrpacket to the list
		//addpackettolist(odr_packet);
		updaterreqtable(odr_packet);
		buildandBroadcast(head,parameter.if_count,odr_packet,-1);
		return;
	}
	if(flag == 2){
		//Generate Application message and forward the packet on next hop
		printf("Message Received from client/server\n");
		odr_packet = malloc(sizeof(struct odrpacket));
		buildodrpacket(odr_data,odr_packet,2,srcport);
		sendToNextHop(odr_packet,routeTab[index].it_ind);
	}
	
}



/*
* Handles the pf packets
*/
void handlepfrequest(struct etherpayload *ethpl, int ifindex, int it_ind){
	//Type 0 is RREQ
	//Type 1 is RREP
	//Type 2 is application data
	int flag = -1;
	int index = -1;
	int destFlag = -1;
	int rreqFlag = -1;
	struct odrpacket *datapacket;
	
	printincomingpkt(ethpl);
	if(ethpl->packet.type == 0){
		//printf("\nNew RREQ received\n");
		//printpayload(ethpl);
		//printf("RREQ RECEIVE END\n\n");
		destFlag = checkifdest(ethpl,ifindex,it_ind);
		if(destFlag == 1){
			//generate RREP and send back
			updatesourceroutetable(ethpl,ifindex, it_ind);
			datapacket = malloc(sizeof(struct odrpacket));
			rreqFlag = updaterreqtable(&(ethpl->packet));
			findrreq(&(ethpl->packet),datapacket);
			if(rreqFlag == 2){
				return;
			}		
			//printroutetable();
			if(ethpl->packet.rrepsent == 0){
				sendBackRREP(ethpl, it_ind,1);
			}
			return;
		}		
		updatesourceroutetable(ethpl,ifindex, it_ind);
		//Check if the packet is already present in list
		rreqFlag = updaterreqtable(&(ethpl->packet));
		if(rreqFlag == 2){
			return;
		}
		flag = checkforroute(ethpl->packet.destIP, &index);
		
		if(ethpl->packet.forcedDisc == 1 || flag == 0 || flag == 1){
			//Insert sourceIP into route table			
			//broadcast RREQ's
			//printroutetable();
			updatesourceroutetable(ethpl,ifindex, it_ind);
			ethpl->packet.hopcount = ethpl->packet.hopcount+1;
			buildandBroadcast(head,parameter.if_count,&(ethpl->packet),ifindex);
			
		}
		if(flag == 2){
			//Update the route table for sourceIP
			//generate RREP and send back
			datapacket = malloc(sizeof(struct odrpacket));
			memcpy((void *)datapacket,&(ethpl->packet),sizeof(struct odrpacket));
			datapacket->hopcount +=1;
			updatesourceroutetable(ethpl,ifindex, it_ind);
			//printroutetable();
			if(ethpl->packet.rrepsent == 0){
				sendBackRREP(ethpl, it_ind,1);
			}
			datapacket->rrepsent = 1;
			buildandBroadcast(head,parameter.if_count,datapacket,ifindex);
			return;
			
		}
		
	}
	else if(ethpl->packet.type == 1){
		//printf("\nNew RREP received\n");
		//printpayload(ethpl);
		//printf("RREP RECEIVE END\n\n");
		handleRREPRequest(ethpl,ifindex,it_ind);
		//printroutetable();
	}
	else if(ethpl->packet.type == 2){
		//printf("\nNew Application data received\n");
		//printpayload(ethpl);
		//printf("New App End\n\n");
		handleAppData(ethpl,ifindex,it_ind);
		//printroutetable();
	}
	else{
		printf("Invalid packet:%d\n",ethpl->packet.type);
	}

}



void buildodrpacket(struct data *odr_data, struct odrpacket *odrpkt, int type, int srcport){
	odrpkt->type = type;
	//printf("Src IP:%s\n",odr_data->sourceIP);
	memcpy(odrpkt->srcIP,odr_data->sourceIP,IPLEN);
	odrpkt->srcPort = odr_data->srcPort;
	//printf("\nSrc Build port:%d\n",odrpkt->srcPort);
	memcpy(odrpkt->destIP,odr_data->destIP,IPLEN);
	//printf("\nBuild Dest IP: %s\n",odrpkt->destIP);
	odrpkt->destPort = odr_data->destPort;
	//printf("\nDest Build port:%d\n",odrpkt->destPort);
	odrpkt->hopcount = 1;
	parameter.broadcastID += 20;
	odrpkt->broadcastID = parameter.broadcastID;
	odrpkt->rrepsent = 0;
	odrpkt->forcedDisc = odr_data->flag;
	//odrpkt->act_data = malloc(sizeof(struct data));
	memcpy((void *)&(odrpkt->act_data),(void *)odr_data,sizeof(struct data));
}

void handleAppData(struct etherpayload *ethpl, int ifindex, int it_ind){
	char paths[LEN];	
	updatesourceroutetable(ethpl,ifindex, it_ind);
	if((strcmp(ethpl->packet.destIP,myodr.ipaddr))==0){
		getPath(paths,ethpl->packet.destPort);
		odr_send(unsockfd, &(ethpl->packet.act_data),paths);
	}
	else{
		sendToNextHop(&(ethpl->packet),it_ind);
	}
}


void handleRREPRequest(struct etherpayload *ethpl, int ifindex, int it_ind){
	int flag =-1;
	struct odrpacket *datapacket;
	updatesourceroutetable(ethpl,ifindex, it_ind);
	datapacket = malloc(sizeof(struct odrpacket));
	flag = findrreq(&(ethpl->packet),datapacket);
	
	if(flag == 1){
		if((strcmp(ethpl->packet.destIP,myodr.ipaddr))==0){
			buildappmsg(datapacket);
			sendToNextHop(datapacket,it_ind);
			//printf("RREP RECEIVED BUT MATCHING\n");
		}
		else{
			ethpl->packet.hopcount +=1;
			sendToNextHop(&(ethpl->packet),it_ind);
			//printf("RREP RECEIVED BUT NOT MATCHING\n");
		}
	}
	else{
		//printf("RREP RECEIVED BUT CAN't FIND PACKET\n");
	}
}

void buildappmsg(struct odrpacket *datapacket){
	datapacket->type = 2;
	//printf("buildapp\n");
	//printf("IP build:%s\n",datapacket->act_data.destIP);
	return;
}
void printincomingpkt(struct etherpayload *ethpl){
	int j=0;
	char dest[10],src[10];
	printf("\n\n\n******************************************\n\n");
	printf("Incoming Message Type:");
	if(ethpl->packet.type == 0){
		printf("RREQ\n");
	}
	else if(ethpl->packet.type == 1){
		printf("RREP\n");
	}
	if(ethpl->packet.type == 2){
		printf("APPLICATION PAYLOAD\n");
	}
	printf("ODR at node %s: received - frame hdr - src:%s dest",odrname,odrname);
	for(j=0;j<IF_HADDR;j++){
		printf(":%02x",ethpl->destMAC[j]);
	}
	//printf("before one\n");
	gethostnames(ethpl->packet.destIP,dest);
	gethostnames(ethpl->packet.srcIP,src);
	//printf("after two\n");
	printf("\n");
	printf("ODR msg payload - message type:%d src: %s dest: %s\n",ethpl->packet.type,src,dest);
	//ethpl->packet.srcIP,ethpl->packet.destIP);
	printf("\n******************************************\n");
}


void printoutgoingpkt(struct etherpayload *ethpl){
	int j=0;
	char dest[10],src[10];
	printf("\n\n\n******************************************\n\n");
	printf("Sending Message Type:");
	if(ethpl->packet.type == 0){
		printf("RREQ\n");
	}
	else if(ethpl->packet.type == 1){
		printf("RREP\n");
	}
	if(ethpl->packet.type == 2){
		printf("APPLICATION PAYLOAD\n");
	}
	printf("ODR at node %s: sending - frame hdr - src:%s dest",odrname,odrname);
	for(j=0;j<IF_HADDR;j++){
		printf(":%02x",ethpl->destMAC[j]);
	}
	//printf("before one\n");
	gethostnames(ethpl->packet.destIP,dest);
	gethostnames(ethpl->packet.srcIP,src);
	//printf("after two\n");
	printf("\n");
	printf("ODR msg payload - message type:%d src: %s dest: %s\n",ethpl->packet.type,src,dest);
	//ethpl->packet.srcIP,ethpl->packet.destIP);
	printf("\n******************************************\n");
}

void gethostnames(char *ip, char *name){
	struct hostent *hptr;
	struct in_addr addr;
	inet_pton(AF_INET,ip,&addr);
	if((hptr = gethostbyaddr(&addr,sizeof(addr),AF_INET))==NULL){
		perror("Error: Invalid IP address. Terminating.");
		exit(0);
	}
	memcpy(name,hptr->h_name,10);
}

int findrreq(struct odrpacket *odrpkt, struct odrpacket *datapacket){
	int i =0;
	if(parameter.rreq_count == 0)
		return 0;
	for(i =0 ;i<parameter.rreq_count;i++){
		if(((strcmp(odrpkt->destIP,rreqtab[i].srcIP))==0) && (rreqtab[i].broadcastID == odrpkt->broadcastID)){
			memcpy((void *)datapacket,(void *)&(rreqtab[i].packet),sizeof(struct odrpacket));
			rreqtab[i].rrepseen = 1;
			return 1;
		}
	}
	return 0;
}




int checkifdest(struct etherpayload *ethpl, int ifindex, int it_ind){
	//printf("\nODR IP:%s\n",myodr.ipaddr);
	//printf("\nDestination IP:%s\n",ethpl->packet.destIP);
	if((strcmp(ethpl->packet.destIP,myodr.ipaddr))==0){
		//printf("\nDestination found\n");
		//Send RREP Back
		
		return 1;
	}
	return 0;
}


void sendBackRREP(struct etherpayload *ethpl, int it_ind, int hop_count){
	//printf("InsideSendBackStart\n");
	struct odrpacket rrep_pkt;
	ethpl->packet.hopcount = hop_count;
	buildRREPpacket(ethpl,&rrep_pkt);
	sendToNextHop(&rrep_pkt,it_ind);	
	//printf("InsideSendBackEnd\n");
}

void sendToNextHop(struct odrpacket *odrpkt, int it_ind){
	//printf("InsideNextHoPStart\n");
	int i=0;
	int send_result =0;
	int index = -1;
	int flag = -1;
	uint8_t broadMAC[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	
	struct sockaddr_ll socket_address;
	
	struct etherpayload *payload = malloc(sizeof(struct etherpayload));
	
	flag = checkforroute(odrpkt->destIP,&index);
	if(index < 0){
		printf("\nRoute Entry Not found in routing table: %d   %d\n",index, flag);
	}
	it_ind = routeTab[index].it_ind;
	socket_address.sll_family   = PF_PACKET;
	socket_address.sll_protocol = htons(PROTOCOL);
	socket_address.sll_hatype   = ARPHRD_ETHER;
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	socket_address.sll_halen    = ETH_ALEN;	
	memcpy(socket_address.sll_addr,head[it_ind].mac,IF_HADDR);
	memcpy((void *)&payload->packet,(void*)odrpkt,sizeof(struct odrpacket));
	memcpy((void*)payload->destMAC, (void *)routeTab[index].nextMAC, IF_HADDR);
	
	//memcpy((void*)payload->destMAC, (void *)broadMAC, IF_HADDR);
	memcpy((void*)payload->srcMAC, (void *)head[it_ind].mac, IF_HADDR);
	payload->protocol = htons(PROTOCOL);
	socket_address.sll_ifindex = routeTab[index].if_index;
	//payload->packet.if_index = head[i].if_index;
	//printf("Just before send:%d\n",payload->packet.type);
	//printf("\nSending on sockfd:%d\n",head[it_ind].sockfd);
	//printpayload(payload);
	send_result = sendto(head[routeTab[index].it_ind].sockfd, payload, sizeof(struct etherpayload), 0,(struct sockaddr*)&socket_address, sizeof(socket_address));
	printoutgoingpkt(payload);
	if(send_result == -1){
		perror("Send Error");
	}
	//printf("InsideNextHoPEnd\n");
	
	
}

void printpayload(struct etherpayload *payload){
	int j=0;
	// Source MAC
	printf("\n\n---------Send to next hop---------\n");
	printf("Source MAC:");
		for(j=0;j<IF_HADDR;j++){
				printf("%d:",payload->srcMAC[j]);
		}
		printf("\n");
	
	//Destination MAC
	printf("\nDestination MAC:");
		for(j=0;j<IF_HADDR;j++){
				printf("%d:",payload->destMAC[j]);
		}
		printf("\n");
	//Message Type
	printf("Message Type:%d\n",payload->packet.type);
	//Source IP
	printf("Source IP:%s\n",payload->packet.srcIP);
	//Destination IP
	printf("Destination IP:%s\n",payload->packet.destIP);
	printf("\n-------------------------------\n\n");
}
	
	


void buildRREPpacket(struct etherpayload *ethpl, struct odrpacket *rrep_pkt){
	
	rrep_pkt->type = 1;
	//printf("InsidebuildRREPStart\n");
	memcpy(rrep_pkt->srcIP,ethpl->packet.destIP,IPLEN);
	//printf("InsidebuildSourceIP\n");
	rrep_pkt->srcPort = ethpl->packet.destPort;
	memcpy(rrep_pkt->destIP,ethpl->packet.srcIP,IPLEN);
	//printf("InsidebuilddestIP\n");
	rrep_pkt->destPort = ethpl->packet.srcPort;
	rrep_pkt->hopcount = ethpl->packet.hopcount;
	rrep_pkt->broadcastID = ethpl->packet.broadcastID;
	rrep_pkt->rrepsent = 1;
	rrep_pkt->forcedDisc = ethpl->packet.forcedDisc;
	//printf("Insidebuildbeforeactdata\n");
	//rrep_pkt->act_data = (struct data *)malloc(sizeof(struct data));
	struct odrpacket *temppkt = &(ethpl->packet);
	memcpy((void *)&(rrep_pkt->act_data),(void *)&(temppkt->act_data),sizeof(struct data));
	//printf("InsidebuildRREPEnd\n");
}




void printroutetable(){
	int i=0;
	int j=0;
	printf("Print Route count:%d\n",parameter.route_count);
	for(i=0;i<parameter.route_count;i++){
		printf("************************%d******************\n",i);
		printf("Destination IP: %s\n",routeTab[i].destIP);
		printf("HW Address of hop:");
		for(j=0;j<IF_HADDR;j++){
				printf("%d:",routeTab[i].nextMAC[j]);
		}
		printf("\nInterface Index: %d\n",routeTab[i].if_index);
		printf("\nInterface Location: %d\n",routeTab[i].it_ind);
		printf("Number of hops:%d\n",routeTab[i].nohops);
		printf("Time: %lld\n", (long long) routeTab[i].ts);
		printf("************************%d******************\n",i);
	}
}


/*
* 0 - New Entry made
* 1 - broadcast id is greater and update made 
* 2 - Broadcast id is lesser and no update made
* 
*/

int updaterreqtable(struct odrpacket *odrpkt){
	int i=0;
	for(i=0;i<parameter.rreq_count;i++){
		if((strcmp(odrpkt->srcIP, rreqtab[i].srcIP)) == 0){
			if(odrpkt->broadcastID > rreqtab[i].broadcastID){
				rreqtab[i].broadcastID = odrpkt->broadcastID;
				memcpy((void*)&(rreqtab[i].packet),(void *)odrpkt,sizeof(struct odrpacket));
				rreqtab[i].rrepseen = 0;
				return 1;
			}
			else{
				return 2;
			}
		}
	}
	i = parameter.rreq_count;
	memcpy(rreqtab[i].srcIP,odrpkt->srcIP,LEN);
	rreqtab[i].broadcastID = odrpkt->broadcastID;
	memcpy((void*)&(rreqtab[i].packet),(void *)odrpkt,sizeof(struct odrpacket));
	rreqtab[i].rrepseen = 0;
	parameter.rreq_count++;
	return 0;
}


void updatesourceroutetable(struct etherpayload *ethpl, int ifindex, int it_ind){
	int i=0;
	int flag=-1;
	int index=-1;
	int j=0;
	if((strcmp(ethpl->packet.srcIP,myodr.ipaddr)) == 0)
		return;
	flag = checkforroute(ethpl->packet.srcIP, &index);
	//printf("\n\nInside Route Update\n");
	//printpayload(ethpl);
	//printf("\n\n");
	if(flag == 0){
		i = parameter.route_count;
		memcpy(routeTab[i].destIP,ethpl->packet.srcIP,IPLEN);
		memcpy(routeTab[i].nextMAC,ethpl->srcMAC,IF_HADDR);
		//printip
		/*printf("Inside Print MAC:");
		for(j=0;j<IF_HADDR;j++){
				printf("%d:",routeTab[i].nextMAC[j]);
		}
		printf("\n");*/
		routeTab[i].if_index = ifindex;
		routeTab[i].nohops = ethpl->packet.hopcount;
		routeTab[i].it_ind = it_ind;
		routeTab[i].ts = time(NULL);
		parameter.route_count++;
	}
	else if(flag == 1 || ethpl->packet.forcedDisc == 1){
		i = index;
		memcpy(routeTab[i].destIP,ethpl->packet.srcIP,IPLEN);
		memcpy(routeTab[i].nextMAC,ethpl->srcMAC,IF_HADDR);
		routeTab[i].if_index = ifindex;
		routeTab[i].nohops = ethpl->packet.hopcount;
		routeTab[i].it_ind = it_ind;
		routeTab[i].ts = time(NULL);		
	}
	else if(flag == 2){
		//reconfirm route
		i = index;
		if(ethpl->packet.hopcount <= routeTab[i].nohops){
			memcpy(routeTab[i].destIP,ethpl->packet.srcIP,IPLEN);
			memcpy(routeTab[i].nextMAC,ethpl->srcMAC,IF_HADDR);
			routeTab[i].if_index = ifindex;
			routeTab[i].nohops = ethpl->packet.hopcount;
			routeTab[i].it_ind = it_ind;
			routeTab[i].ts = time(NULL);
		}
	}
	//printf("Update Route count:%d\n",parameter.route_count);
}

/*
* 0 - Route does not exist;
* 1 - Route exists but stale;
* 2 - Route exists.
*/

int checkforroute(char destIP[IPLEN], int *index){
	int i=0;
	double temp;
	if(parameter.route_count == 0){
		return 0;
	}
	else{
		for(i=0;i<parameter.route_count;i++){
			if(strcmp(routeTab[i].destIP,destIP)==0){
				// check time 
				if(difftime(time(NULL),routeTab[i].ts) > stale_time){
					*index = i;
					return 1;
				}
				else{
					*index = i;
					return 2;
				}
			}
		}
	}
	return 0;	
}


void buildandBroadcast(struct interfaces *head, int if_count,struct odrpacket *odrpkt, int if_index){
	int i=0;
	int send_result =0;
	uint8_t broadMAC[] = {0xff,0xff,0xff,0xff,0xff,0xff};
	char buffer[ETH_FRAME_LEN];
	struct sockaddr_ll socket_address;
	
	struct etherpayload *payload = malloc(sizeof(struct etherpayload));
	
	socket_address.sll_family   = PF_PACKET;
	socket_address.sll_protocol = htons(PROTOCOL);
	socket_address.sll_hatype   = ARPHRD_ETHER;
	socket_address.sll_pkttype  = PACKET_OTHERHOST;
	socket_address.sll_halen    = ETH_ALEN;	
	memcpy(socket_address.sll_addr,broadMAC,IF_HADDR);
	memcpy((void *)&payload->packet,(void*)odrpkt,sizeof(struct odrpacket));
	for(i=0;i<if_count;i++){
		if(head[i].if_index == if_index){
			continue;
		}
		memcpy((void*)payload->destMAC, (void *)broadMAC, IF_HADDR);
		memcpy((void*)payload->srcMAC, (void *)head[i].mac, IF_HADDR);
		payload->protocol = htons(PROTOCOL);
		socket_address.sll_ifindex = head[i].if_index;
		//payload->packet.if_index = head[i].if_index;
		//printf("Just before send:%d\n",payload->packet.type);
		send_result = sendto(head[i].sockfd, payload, sizeof(struct etherpayload), 0,(struct sockaddr*)&socket_address, sizeof(socket_address));
		printoutgoingpkt(payload);
		if(send_result == -1){
			perror("Broadcast Error");
		}
	}
	
}


int updateportpath(){
	int i;
	int j=0;
	i = parameter.port_index;
	for(j=0;j<i;j++){
		if((strcmp(connaddr.sun_path,porttab[j].sunpath))==0){
			return porttab[j].port;
		}
	}
	porttab[i].port = parameter.port_count;
	parameter.port_count = parameter.port_count + 15;
	memcpy((void *)porttab[i].sunpath,(void *)connaddr.sun_path,strlen(connaddr.sun_path));
	porttab[i].valid = 1;
	porttab[i].ts = time(NULL);
	parameter.port_index++;
	return porttab[i].port;
}

void getPath(char* paths, int ports){
	int i=0;
	int j=0;
	i = parameter.port_index;
	for(j=0;j<i;j++){
		if(porttab[j].port == ports){
			//printf("Path found:%s\n",porttab[j].sunpath);
			memcpy((void *)paths,(void *)porttab[j].sunpath,sizeof(porttab[j].sunpath));
			return;
		}
	}
}


int deletefromlist(struct odrpacket *odrpkt, struct odrpacket *datapacket){
	struct packetlist *prev;
	struct packetlist *templst;
	if(pktlst == NULL){
		return 0;
	}
	templst = pktlst;
	prev = NULL;
	//printf("Printing delete first\n");
	if(pktlst!=NULL){
		if((strcmp(pktlst->packet.srcIP,odrpkt->destIP) == 0) && (pktlst->packet.broadcastID == odrpkt->broadcastID)){
			memcpy((void *)datapacket,(void*)&(pktlst->packet), sizeof(struct odrpacket));
			pktlst = pktlst->next;
			return 1;
		}
	}	
	while(templst!= NULL){
		if((strcmp(templst->packet.srcIP,odrpkt->destIP) == 0) && (templst->packet.broadcastID == odrpkt->broadcastID)){
			memcpy((void *)datapacket,(void*)&(pktlst->packet), sizeof(struct odrpacket));
			prev->next = templst->next;
			templst = templst->next;
			return 1;
		}	
		prev = templst;
		templst = templst->next;
	}
	return 0;
}

void addpackettolist(struct odrpacket *odrpkt){
	struct packetlist *tempkt;
	struct packetlist *templst;
	if(pktlst == NULL){
		pktlst = (struct packetlist *)malloc(sizeof(struct packetlist));
		memcpy((void *)&(pktlst->packet),(void*)odrpkt, sizeof(struct odrpacket));
		pktlst->next = NULL;
		
		return;
	}
	templst = pktlst;
	while(templst->next!=NULL){
		templst = templst->next;
	}
	tempkt = malloc(sizeof(struct packetlist));
	memcpy((void *)&(tempkt->packet),(void*)odrpkt, sizeof(struct odrpacket));
	tempkt->next = NULL;
	templst->next = tempkt;
	return;
	
}



