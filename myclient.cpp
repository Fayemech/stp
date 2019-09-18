#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and alarm() */
#include <errno.h>      /* for errno and EINTR */
#include <signal.h>     /* for sigaction() */
#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
#include <queue>
#include <sstream>
#include <iomanip>

using namespace std;

#define ECHOMAX         255     /* Longest string to echo */
#define TIMEOUT_SECS    10       /* Seconds between retransmits */
#define MAXTRIES        20       /* Tries before giving up */


int tries = 0;   /* Count of times sent - GLOBAL for signal-handler access */

void DieWithError(string errorMessage);   /* Error handling function */
void CatchAlarm(int ignored);            /* Handler for SIGALRM */

struct element
{
	char dest;
	int dist;
};

struct distance_vector_
{
	char sender;
	int num_of_dests;
	struct element content[6];
};


struct element_rt
{
	char dest;
	int dis;
	char nexthop;
};

struct routingtalbe
{
	struct element_rt content[6];
};

struct element_nt
{
	char dest;
	int dis;
	char ip[10];
};

struct neighbortable
{
	char node;
	int portnum;
	struct element_nt content[6];
};

/*
class neighbortable {
public:
	char node;
	int portnum;
	unordered_map<char, pair<int, string>> table;
	neighbortable()
	{
		node = '\0';
	}
};

class routingtable {
public:
	char node;
	unordered_map<char, pair<int, char>> table;
	routingtable()
	{
		node = '\0';
	}
};

void initialize(routingtable &route, vector<pair<char, string>> &neighbor) {
	ifstream fin("test.txt");      //read input graph
	string s;
	while (getline(fin, s)) {
		int line = 0;
		int i = 0;
		if (line == 0) {
			char tmp;
			while (s[i] != ' ') {
				tmp += s[i];
				i++;
			}
			route.node = tmp;
			line++;
			route.table[tmp].first = 0;
			route.table[tmp].second = tmp;
		}
		else {
			char node;
			int dis;
			string ip;
			while (i < s.size() && s[i] != ' ') {
				node += s[i];
				i++;
			}
			while (i < s.size() && s[i] == ' ') {
				i++;
			}
			while (i < s.size() && s[i] != ' ') {
				dis = dis * 10 + s[i];
				i++;
			}
			while (i < s.size() && s[i] == ' ') {
				i++;
			}
			int index = 0;
			while (i < s.size&& s[i] != ' ') {
				ip += s[i];
				i++;
			}
			neighbor.push_back(make_pair(node, ip));
			route.table[node].first = dis;
			route.table[node].second = node;
		}
	}
}

bool bellmanford(distance_vector_&a, routingtable &b) {
	bool sign = false;
	for (int i = 0; i < a.num_of_dests; i++) {
		char sender = a.sender;
		char node = a.content[i].dest;
		int dis = a.content[i].dist;
		if (b.table.find(node) == b.table.end() || b.table[node].first > (b.table[sender].first + dis)) {
			sign = true;
			b.table[node].first = (b.table[sender].first + dis);
			b.table[node].second = sender;
		}
	}
	return sign;
}

*/

int main(int argc, char *argv[])
{
	int socket_rec;
	int socket_sed;
	struct sockaddr_in echoServAddr_rec;  /* Echo server address */
	struct sockaddr_in echoServAddr_sed ; /* Echo server address */
	unsigned short echoServPort;          /* Echo server port */
	char *servIP;                   /*IP address of server*/
	unsigned int cliAddrLen;         /* Length of incoming message */
	struct sigaction myAction;       /* For setting signal handler */
	struct sockaddr_in echoClntAddr; /* Client address */
	int respStringLen;

	ifstream infile;
	string config;

	distance_vector_ disvec;         
	distance_vector_ rev_disvec;
	routingtalbe mytable;
	neighbortable neighbor;

	

	if (argc != 2)    
	{
		fprintf(stderr, "error", argv[0]);
		return 0;
	}

	config = argv[1];

	infile.open(config.c_str()); //read configfile to initialize table



	int line = 0;
	int i = 0;
	string s;
	char node;
	int dis;
	char ip[10] = {'\0'};
	int index = 0;
	while (getline(infile, s)) {
		stringstream ss(s);
		if (line == 0) {
			ss >> neighbor.node;
			line++;
		}
		else {
			if (line == 1) {
				ss >> neighbor.portnum;
				line++;
			}
			else {
				if (ss >> node >> dis >> ip) {
					neighbor.content[index].dest = node;
					neighbor.content[index].dis = dis;
					for (int j = 0; j < 9; j++) {
						neighbor.content[index].ip[j] = ip[j];
					}
					neighbor.content[index].ip[9] = '\0';
					index++;
				}
				else {
					break;
				}
			}
		}
	}

	

	infile.close();      //neighbor table finish, now copy to routing table

	for (int i = 0; i < index; i++) {
		mytable.content[i].dest = neighbor.content[i].dest;
		mytable.content[i].dis = neighbor.content[i].dis;
		mytable.content[i].nexthop = neighbor.content[i].dest;
	}
	for (int i = index; i < 6; i++) {
		mytable.content[i].dest = '!';
		mytable.content[i].dis = 999;
		mytable.content[i].nexthop = '!';
 	}
	//print the routingtable

	cout << "routing table" << endl;

	for (int i = 0; i < 6; i++) {
		cout << mytable.content[i].dest << setw(5) << mytable.content[i].dis << setw(5) << mytable.content[i].nexthop << endl;
	}

	//creat dv


	disvec.sender = neighbor.node;
	disvec.num_of_dests = index;

	for (int i = 0; i < index; i++) {
		disvec.content[i].dest = neighbor.content[i].dest;
		disvec.content[i].dist = neighbor.content[i].dis;
	}

	//set up socket and others

	if ((socket_rec = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket_rec() failed");

	if ((socket_sed = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		DieWithError("socket_sed() failed");

	/* Set signal handler for alarm signal */
	myAction.sa_handler = CatchAlarm;
	if (sigfillset(&myAction.sa_mask) < 0) /* block everything in handler */
		DieWithError("sigfillset() failed");
	myAction.sa_flags = 0;

	if (sigaction(SIGALRM, &myAction, 0) < 0)
		DieWithError("sigaction() failed for SIGALRM");

	//server address for receiving
	echoServPort = neighbor.portnum;
	memset(&echoServAddr_rec, 0, sizeof(echoServAddr_rec));   /* Zero out structure */
	echoServAddr_rec.sin_family = AF_INET;                /* Internet address family */
	echoServAddr_rec.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	echoServAddr_rec.sin_port = htons(echoServPort);      /* Local port */

	//sserver address for sending

	memset(&echoServAddr_sed, 0, sizeof(echoServAddr_sed));      /* Zero out structure */
	echoServAddr_sed.sin_family = AF_INET;
	/* servIP was chang base on neighbor ip */
	echoServAddr_sed.sin_port = htons(echoServPort);         /* Local port */

	
	if (bind(socket_rec, (struct sockaddr *) &echoServAddr_rec, sizeof(echoServAddr_rec)) < 0)
		DieWithError("bind() failed");

	//send init dv to all neighbor to start

	for (int i = 0; i < index; i++) {
		servIP = neighbor.content[i].ip;
		echoServAddr_sed.sin_addr.s_addr = inet_addr(servIP); // servIP
		//send
		if (sendto(socket_sed, &disvec, sizeof(disvec), 0, (struct sockaddr *) &echoServAddr_sed, sizeof(echoServAddr_sed)) != sizeof(disvec))
		{
			DieWithError("sendto() sent a different number of bytes than expected");
		}
	}

	//after start, run forever;

	for (;;) {
		//send periodically
		alarm(TIMEOUT_SECS);
		cliAddrLen = sizeof(echoClntAddr);

		while ((respStringLen = recvfrom(socket_rec, &rev_disvec, sizeof(rev_disvec), 0,
			(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0) 
		{
			if (errno == EINTR)     /*Alarm went off  */
			{
				cout << "periodically send dv" << endl;
				for (int i = 0; i < index; i++)
				{
					servIP = neighbor.content[i].ip;
					echoServAddr_sed.sin_addr.s_addr = inet_addr(servIP);

					if (sendto(socket_sed, &disvec, sizeof(disvec), 0, (struct sockaddr *) &echoServAddr_sed, sizeof(echoServAddr_sed)) != sizeof(disvec))
					{
						DieWithError("sendto() sent a different number of bytes than expected");
					}
				}

				alarm(TIMEOUT_SECS); /*reset timeout back to 5sec. So, if nothing received again, resend the DV within 5 sec.*/
			}
			else {
				DieWithError("recvfrom() failed");
			}	
		}
		/* recvfrom() got something --  cancel the timeout */
		alarm(0);

		//print what we received

		cout << "distance vector from " << rev_disvec.sender <<endl;
		for (int i = 0; i < rev_disvec.num_of_dests; i++) {
			cout << rev_disvec.content[i].dest << setw(5) << rev_disvec.content[i].dist << endl;
		}

		//update the routingtable
		//try to find where the rev_disvec came from;
		int routindex = 0;
		while (mytable.content[routindex].dest != rev_disvec.sender) {
			routindex++;
		}
		//cout << "routindex" << routindex;
		int l;
		bool sign = false; //use to determinde do we update the routing talbe, which means do we need to send it to neighbor
		char cur;
		
		for (int n = 0; n < rev_disvec.num_of_dests; n++) {
			
			cur = rev_disvec.content[n].dest;
			if (cur == neighbor.node) {
				continue;
			}
			l = 0;
			bool find = false;
			//while (mytable.content[l].dest != cur && mytable.content[l].dest != '!' && l < 6) {
			//	l++;
			//}
			//if (l < 6 && mytable.content[l].dest != '!') find = true;
			while (mytable.content[l].dest != '!' && !find) {
				if (mytable.content[l].dest == cur) {
					find = true; //find the cur dest
				}
				else {
					l++;
				}
			}
			if (!find) {
				//not in, we need to add it also change the sign to true;
				sign = true;
				mytable.content[l].dest = cur;
				mytable.content[l].dis = mytable.content[routindex].dis + rev_disvec.content[n].dist;
				mytable.content[l].nexthop = rev_disvec.sender;

				//modify our disvec

				disvec.content[disvec.num_of_dests].dest = cur; // add at the end of content;
				disvec.content[disvec.num_of_dests].dist = mytable.content[l].dis;
				disvec.num_of_dests++;
			}
			else {
				//already in, we need to compare the dis, remove the longer one;
				if (mytable.content[l].dis > mytable.content[routindex].dis + rev_disvec.content[n].dist) {
					sign = true;     //modify it so we need to send to neighbor.
					mytable.content[l].dis = mytable.content[routindex].dis + rev_disvec.content[n].dist;
					mytable.content[l].nexthop = rev_disvec.sender;

					//modify our disvec
					//find which lane need to modify in the disvec first
					int t = 0;
					while (t < disvec.num_of_dests && disvec.content[t].dest != cur) {
						t++;
					}
					disvec.content[t].dist = mytable.content[l].dis;


				}
			}
		}
		if (sign) {
			//we update it, so we need send disvec to others, but print first;
			cout << "routing table update" << endl;
			for (int k = 0; k < 6; k++) {
				cout << mytable.content[k].dest << setw(5) << mytable.content[k].dis << setw(5) << mytable.content[k].nexthop << endl;
			}
			for (int k = 0; k < index; k++) {
				servIP = neighbor.content[k].ip;
				echoServAddr_sed.sin_addr.s_addr = inet_addr(servIP);
				if (sendto(socket_sed, &disvec, sizeof(disvec), 0, (struct sockaddr *) &echoServAddr_sed, sizeof(echoServAddr_sed)) != sizeof(disvec))
				{
					DieWithError("sendto() sent a different number of bytes than expected");
				}
			}
		}
	}
	close(socket_rec);
	close(socket_sed);

	

}


void CatchAlarm(int ignored)     /* Handler for SIGALRM */
{
	tries += 1;
}


void DieWithError(string errorMessage)
{
	cout << "\n" << errorMessage << endl;
	exit(1);
}