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
#include <sys/types.h>

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
		 << "./ipk-client -h host -p port [-r|-w] soubor\n"
		 << "  -h <host>\n"
		 << "      (IP adresa nebo fully-qualified DNS name) identifikace serveru jakozto koncoveho bodu komunikace klienta\n"
		 << "  -p <port>\n"
		 << "      cilove cislo portu\n"
		 << "  -r <soubor>\n"
		 << "      klient stahne soubor ze serveru\n"
		 << "  -w <soubor>\n"
		 << "      klient nahraje soubor na server\n"
	;
	exit(1);
}

int main(int argc, char *argv[])
{

	if (argc == 1) { // Called without parameters
		printHelp(); // exit(1)
	}

	const char * host = "";
	uint16_t port;
	bool port_set = false;
	string read = "";
	string write = "";

	int opt;
	while ((opt = getopt(argc, argv, "h:p:r:w:")) != -1) {
		switch (opt) {
			case 'h':
				host = optarg;
				break;
			case 'p': {
				char *endptr;
				long port_raw = strtol(optarg, &endptr, 10);
				if (*endptr != '\0' || port_raw < 0 || port_raw > 65535) {
					cerr << "Hodnota parametru -p musi byt cele kladne cislo v intervalu <0;65535>\n";
					printHelp(); // exit(1)
				}
				port = (uint16_t) port_raw;
				port_set = true;
				} break;
			case 'r':
				read = optarg;
				if (!write.empty()) {
					cerr << "Parametr -r nelze pouzit zaroven s parametrem -w\n";
					printHelp(); // exit(1)
				}
				break;
			case 'w':
				write = optarg;
				if (!read.empty()) {
					cerr << "Parametr -w nelze pouzit zaroven s parametrem -r\n";
					printHelp(); // exit(1)
				}
				break;
			case '?':
				if (optopt == 'h' || optopt == 'p' || optopt == 'r' || optopt == 'w') {
					cerr << "Chybi hodnota u argumentu -" << (char) optopt << '\n';
				}
				else {
					cerr << "Neznamy parametr -" << (char) optopt << '\n';
				}
			default:
				printHelp(); // exit(1)
		}
	}

	if (host == "" || !port_set || (write.empty() && read.empty())) {
		cerr << "Chyby povinne parametry\n";
		printHelp(); // exit(1)
	}

	/* DNS lookup for host address */
	struct hostent *server = gethostbyname(host);
	if (server == NULL) {
		cerr << "CHYBA: adresa serveru nenalezena\n";
		exit(2);
	}

	/* Create socket */
	int client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_INET = IPv4, SOCK_STREAM = sequenced, reliable
	if (!client_sock) {
		cerr << "CHYBA: nelze vytvorit socket\n";
		exit(2);
	}

	struct sockaddr_in server_addr;
	memset((char *) &server_addr, 0, sizeof(server_addr)); // Null undefined values

	/* Create connection with server */
	memcpy((char *) &server_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port); // convert to uint16_t

	if (!connect(client_sock, (const struct sockaddr *) &server_addr, sizeof(server_addr))) {
		cerr << "CHYBA: nelze navazat spojeni se serverem\n";
		exit(2);
	}

	/* Close connection */
	close(client_sock);

	return 0;
}
