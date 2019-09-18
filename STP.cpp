#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <string>
#include <queue>

using namespace std;

class bridge {
public:
	int root;
	int distance;
	int id;
	vector<char> adj_lan; // connect with which lan
	unordered_map<char, bool> port;
	pair<char, int> LB;   //from which lan and bridge
	bridge()
	{
		id = -1;
		root = -1;
		distance = 0;
	}
};
class message {
public:
	int root;
	int distance;
	bridge from;     // from which bridge
	int target;     
	char lan;        // from which lan
	message()
	{
		root = -1;
		distance = -1;
		target = -1;
		lan = '\0';
	}
};
/*
struct messagecom
{
	bool operator() (message const &a, message const &b){
		if (a.target < b.target) return true;
		else return false;
	}
};
*/
void update(message& a, vector<bridge>& re_b) {
	int root = a.root;
	int dis = a.distance;
	bridge from = a.from;
	//int target = a.target;
	char lan = a.lan;
	//message re_message;
	
	for (int i = 0; i < re_b.size(); i++) {
		bridge b = re_b[i];
		if (root < b.root) {
			re_b[i].root = root;
			re_b[i].distance = dis + 1;
			re_b[i].LB = make_pair(lan, from.id);
			re_b[i].port[lan] = true;
		}
		else if (root == b.root && dis + 1 < re_b[i].distance) {
			re_b[i].root = root;
			re_b[i].distance = dis + 1;
			re_b[i].LB = make_pair(lan, from.id);
			re_b[i].port[lan] = true;
		}
		else if (root == b.root && dis + 1 == re_b[i].distance && from.id < re_b[i].LB.second) {
			re_b[i].root = root;
			re_b[i].distance = dis + 1;
			re_b[i].LB = make_pair(lan, from.id);
			re_b[i].port[lan] = true;
		}
		else if (root == b.root && dis == re_b[i].distance && from.id < re_b[i].id) {
			re_b[i].port[lan] = false;
			int n = re_b[i].port.size();
			if (n <= 2) {
				for (auto it = re_b[i].port.begin(); it != re_b[i].port.end(); it++) {
					it->second = false;
				}
			}
		}
		else if (root == b.root && dis + 1 == re_b[i].distance) {
			re_b[i].port[lan] = false;
			int n = re_b[i].port.size();
			if (n <= 2) {
				for (auto it = re_b[i].port.begin(); it != re_b[i].port.end(); it++) {
					it->second = false;
				}
			}
		}
		/*
		else {
			re_b[i].port[lan] = false;
			int n = re_b[i].port.size();
			if (n <= 2) {
				for (auto it = re_b[i].port.begin(); it != re_b[i].port.end(); it++) {
					it->second = false;
				}
			}
		}
		*/
	}
	//return re_message; 
}
class lan
{
public:
	char id;
	int DP;
	vector<int> adj_bridge;
	vector<int> hosts;
	lan()
	{
		id = '\0';
		DP = -1;
	}
};

int main()
{
    vector<bridge> b_l;     //bridge to lan
	unordered_map<char, vector<int>> l_b;     //lan to bridge
	//lan l_b;
	
	ifstream fin("test.txt");      //read input graph
	string s;
	while (getline(fin, s)) {
		int i = 0;
		int id = 0;
		bridge tmp;
		while (s[i] != ' ') {
			id = id * 10 + s[i] - '0';
			i++;
		}
		tmp.id = id;
		tmp.root = id;
		for (; i < s.size(); i++) {
			if (s[i] != ' ') {
				tmp.adj_lan.push_back(s[i]);
				tmp.port[s[i]] = true;
				l_b[s[i]].push_back(id);
			}
		}
		b_l.push_back(tmp);
	}

	// bridge lan graph finished and Initialize the start state finished

	for (int i = 0; i < b_l.size(); i++) {
		cout << "bridge" << " " << i + 1 << " " << "configuration" << " " << b_l[i].root << "," << b_l[i].distance << "," << b_l[i].id << endl;
	}


	



	//queue<message> send, received;
	queue<int> order;
	string m;
	cout << "myprogram LANfilename";
	getline(cin, m);
	for (int i = 0; i < m.size(); i++) {
		if (m[i] == ' ') continue;
		int id = 0;
		while (m[i] >= '0' && m[i] <= '9') {
			id = id * 10 + (m[i] - '0');
			i++;
		}
		order.push(id);
	}

	// sending order finished


	while (!order.empty()) {
		int index = order.front();
		order.pop();
		message m;                     // generated the send message
		m.root = b_l[index - 1].root;
		m.distance = b_l[index - 1].distance;
		m.from = b_l[index - 1];
		queue<char> re_lan;                //follow the order generate the reveived lan
		for (int i = 0; i < b_l[index - 1].adj_lan.size(); i++) {
			re_lan.push(b_l[index - 1].adj_lan[i]);
		}
		while (!re_lan.empty()) {
			char lan = re_lan.front();
			re_lan.pop();
			m.lan = lan;                         //finish the send message
			vector<bridge> re_b;                  //generate the received bridge
			vector<int> replace;                  //the update bridage need to be replace after update
			for (auto it = l_b[lan].begin(); it != l_b[lan].end(); it++) {
				//cout << *it << endl;
				if (*it != index) {
					bridge t = b_l[*it - 1];
					replace.push_back(*it - 1);
					re_b.push_back(t);
				}
			}
			update(m, re_b);

			// update finish, replace the original data
			
			for (int i = 0; i < replace.size(); i++) {
				b_l[replace[i]] = re_b[i];
			}

		}
	}
/*

	while (!order.empty()) {
		int index = order.front();
		order.pop();
		message m;
 		m.root = b_l[index - 1].id;
		m.distance = b_l[index - 1].distance;
		m.from = b_l[index - 1];
		vector<char> re_lan;
		for (int i = 0; i < b_l[index - 1].adj_lan.size(); i++) {
			re_lan.push_back(b_l[index - 1].adj_lan[i]);
		}
		vector<bridge> re_b;
		vector<int> replace;
		for (int i = 0; i < re_lan.size(); i++) {
			char lan = re_lan[i];
			for (auto j : l_b[lan]) {
				if (j != index) {
					bridge t = b_l[j - 1];
					replace.push_back(j - 1);
					re_b.push_back(t);
				}
			}
		}
		update(m, re_b);
		// finish update, replace the original data

		for (int i = 0; i < replace.size(); i++) {
			b_l[replace[i]] = re_b[i];
		}
	}
	*/


	// all update finish follow the given sending order, display the result
	
	for (int i = 0; i < b_l.size(); i++) {
		cout << "bridge" << " " << i + 1 << " " <<"configuration" << " " << b_l[i].root << "," << b_l[i].distance << "," << b_l[i].id << endl;
	}
	for (int i = 0; i < b_l.size(); i++) {
		for (auto it = b_l[i].port.begin(); it != b_l[i].port.end(); it++) {
			bool state = it->second;
			string action = state ? "open" : "close";
			cout << "bridge" << " " << i + 1 << " " << "port" << " " << it->first << " " << action << endl;
		}
	}
	
	cin.get(); 


}