/*
* Authors: Harishkumar Katagal(109915793) & Gagan Nagaraju (109889036)
* FileName: odr_hkatagal.h
* 
*/


#ifndef ODR_H_
#define ODR_H_

#include "unp.h"
#include "utilities.h"
#define	IF_HADDR	 6
#define IF_NAME 16
#define PROTOCOL 0x7747 //Protocol number



struct routetable{
	char destIP[IPLEN];
	uint8_t nextMAC[IF_HADDR];
	uint32_t if_index;
	uint32_t nohops;
	int it_ind;
	time_t ts;
	//struct routetable *next;
};

struct interfaces{
	int sockfd;
	char    if_name[IF_NAME];	/* hardware address */
	int     if_index;		/* interface index */
	char ipaddr[IPLEN];	/* IP address */
	uint8_t mac[IF_HADDR];
	char    if_haddr[IF_HADDR];
};


struct odrpacket{
	int type;
	char srcIP[IPLEN];
	int srcPort;
	char destIP[IPLEN];
	int destPort;
	int hopcount;
	int broadcastID;
	int rrepsent;
	int forcedDisc;
	struct data act_data;
};

struct etherpayload{
	uint8_t destMAC[IF_HADDR];
	uint8_t srcMAC[IF_HADDR];
	uint16_t protocol;
	//char text[15];
	struct odrpacket packet;
};


struct portpath{
	int port;
	char sunpath[LEN];
	int valid;
	time_t ts;
};

struct packetlist{
	//struct etherpayload payload;
	struct odrpacket packet;
	struct packetlist *next;
};


struct rreqarray{
	char srcIP[IPLEN];
	int broadcastID;
	int rrepseen;
	struct odrpacket packet;
};


struct parameters{
	int if_count;
	int broadcastID;
	int route_count;
	int port_count;
	int port_index;
	int rreq_count;
	struct payloadlist *list;
};

void setparameters();

int getsockfd();
int getallinterfaces(struct interfaces *head);
void createpfsockets(struct interfaces *head, int if_count);
void buildandBroadcast(struct interfaces *head, int if_count,struct odrpacket *odrpkt, int if_index);
void handlenewclient(struct data *odr_data);
void buildodrpacket(struct data *odr_data, struct odrpacket *odrpkt, int type, int srcport);
void handlepfrequest(struct etherpayload *ethpl, int ifindex, int it_ind);
int checkforroute(char destIP[IPLEN], int *index);
void printroutetable();
void updatesourceroutetable(struct etherpayload *ethpl, int ifindex, int it_ind);

int checkifdest(struct etherpayload *ethpl, int ifindex, int it_ind);
void sendBackRREP(struct etherpayload *ethpl, int it_ind, int hop_count);
void sendToNextHop(struct odrpacket *odrpkt, int it_ind);
void buildRREPpacket(struct etherpayload *ethpl, struct odrpacket *rrep_pkt);
void addpackettolist(struct odrpacket *odrpkt);
void handleRREPRequest(struct etherpayload *ethpl, int ifindex, int it_ind);
int deletefromlist(struct odrpacket *odrpkt, struct odrpacket *datapacket);
void buildappmsg(struct odrpacket *datapacket);
void handleAppData(struct etherpayload *ethpl, int ifindex, int it_ind);
void getPath(char* paths, int ports);
int updateportpath();
int findrreq(struct odrpacket *odrpkt, struct odrpacket *datapacket);
int updaterreqtable(struct odrpacket *odrpkt);
void printpayload(struct etherpayload *payload);
void printoutgoingpkt(struct etherpayload *ethpl);
void gethostnames(char *ip, char *name);
void printincomingpkt(struct etherpayload *ethpl);

#endif
