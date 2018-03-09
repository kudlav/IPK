/*
 * Error codes:
 * 0 = ok
 * 1 = error when parsing arguments
 * 2 = internal error
 */

/* Requirements */
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h> // for BSD systems

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#ifdef _WIN32
#include <afxres.h> // TODO WIN only!!
#endif

using namespace std;

void printHelp()
{
	cerr << "Pouziti:\n"
		 << "./ipk-server -p <port>\n"
		 << "  -p <port>\n"
		 << "      cislo portu, na kterem server nasloucha\n"
	;
	exit(1);
}

int main(int argc, char *argv[])
{
	if (argc == 1) { // Called without parameters
		printHelp(); // exit(1)
	}

	uint16_t port;

	int opt;
	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
			case 'p': {
				char *endptr;
				long port_raw = strtol(optarg, &endptr, 10);
				if (*endptr != '\0' || port_raw < 0 || port_raw > 65535) {
					cerr << "Hodnota parametru -p musi byt cele kladne cislo v intervalu <0;65535>\n";
					printHelp(); // exit(1)
				}
				port = (uint16_t) port_raw;
				} break;
			case '?':
				if (optopt == 'p') {
					cerr << "Chybi hodnota u argumentu -p\n";
				}
				else {
					cerr << "Neznamy parametr -" << (char) optopt << '\n';
				}
			default:
				printHelp(); // exit(1)
		}
	}

	/* Create socket */
	int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_INET = IPv4, SOCK_STREAM = sequenced, reliable
	if (!server_sock) {
		cerr << "CHYBA: nelze vytvorit socket\n";
		exit(2);
	}

	struct sockaddr_in server_addr;
	memset((char *) &server_addr, 0, sizeof(server_addr)); // Null undefined values

	/* Bind socket to all interfaces */
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port); // convert to uint16_t

	if (!bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
		cerr << "CHYBA: nelze priradit socket k sitovemu rozhranni\n";
		return 2;
	}

	/* Listen for connections on a socket */
	if (!listen(server_sock , 1)) { // MAX 1 WAITTING CLIENT
		cerr << "CHYBA: socket se nepodarilo nastavit pro prijmani prichozich pozadavku\n";
		return 2;
	}

	/* Handle waitting connections from a client */
	struct sockaddr_in client_addr;
	int client_addr_length = sizeof(client_addr);
	while (true) {
		/* Accept connection */
		int client_sock = accept(server_sock, (struct sockaddr*) &client_addr, (socklen_t*) &client_addr_length);
		if (!client_sock) {
			cerr << "CHYBA: prichozi pozadavek nelze prijmout\n";
			return 2;
		}
		/* Close connection */
		close(client_sock);
	}
}
