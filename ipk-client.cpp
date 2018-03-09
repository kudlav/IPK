/**
 * Project: IPK - client/server
 * Author: Vladan Kudlac
 * Created: 5.3.2018
 */

/* Error codes: */
#define EXIT_ARG 1 // error when parsing arguments
#define EXIT_NET 2 // network error
#define EXIT_IOE 3 // I/O error
#define EXIT_SYS 4 // system error (fork)

/* Requirements */
#include <iostream> // IO operations
#include <fstream> // files IO
#include <unistd.h>
#include <cstring> // memset, memcpy
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
	exit(EXIT_ARG);
}

int main(int argc, char *argv[])
{
	if (argc == 1) { // Called without parameters
		printHelp(); // exit(EXIT_ARG)
	}

	uint16_t port;
	bool port_set = false;
	string host = "";
	string read = "";
	string save = "";

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
					cerr << "CHYBA: hodnota parametru -p musi byt cele kladne cislo v intervalu <0;65535>\n";
					printHelp(); // exit(EXIT_ARG)
				}
				port = (uint16_t) port_raw;
				port_set = true;
				} break;
			case 'r':
				read = optarg;
				if (!save.empty()) {
					cerr << "CHYBA: parametr -r nelze pouzit zaroven s parametrem -w\n";
					printHelp(); // exit(EXIT_ARG)
				}
				break;
			case 'w':
				save = optarg;
				if (!read.empty()) {
					cerr << "CHYBA: parametr -w nelze pouzit zaroven s parametrem -r\n";
					printHelp(); // exit(EXIT_ARG)
				}
				break;
			case '?':
				if (optopt == 'h' || optopt == 'p' || optopt == 'r' || optopt == 'w') {
					cerr << "CHYBA: chybi hodnota u argumentu -" << (char) optopt << '\n';
				}
				else {
					cerr << "CHYBA: neznamy parametr -" << (char) optopt << '\n';
				}
			default:
				printHelp(); // exit(EXIT_ARG)
		}
	}

	if (host.empty() || !port_set || (save.empty() && read.empty())) {
		cerr << "CHYBA: chyby povinne parametry\n";
		printHelp(); // exit(EXIT_ARG)
	}

	/* DNS lookup for host address */
	struct hostent *server = gethostbyname(host.c_str());
	if (server == NULL) {
		cerr << "CHYBA: adresa serveru nenalezena\n";
		exit(EXIT_NET);
	}

	/* Create socket */
	int client_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_INET = IPv4, SOCK_STREAM = sequenced, reliable
	if (client_sock < 0) {
		cerr << "CHYBA: nelze vytvorit socket\n";
		exit(EXIT_NET);
	}

	struct sockaddr_in server_addr;
	memset((char *) &server_addr, 0, sizeof(server_addr)); // Null undefined values

	/* Create connection with server */
	memcpy((char *) &server_addr.sin_addr.s_addr, server->h_addr, (size_t) server->h_length);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port); // convert to uint16_t

	if (connect(client_sock, (const struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		cerr << "CHYBA: nelze navazat spojeni se serverem\n";
		exit(EXIT_NET);
	}

	/* Open file */
	ofstream file;
	if (!save.empty()) {
		file.open(save.c_str(), ios::out | ios::trunc | ios::binary);
	}
	else {
		file.open(read.c_str(), ios::out | ios::trunc | ios::binary);
	}

	if (!file.is_open()) {
		cout << "CHYBA: soubor nelze otevrit\n";
		exit(EXIT_IOE);
	}

	// ### HERE WILL BE THE TRANSFER ### //
	string data;
	data = "Hello, it's me, pid:" + to_string(getpid());
	write(client_sock, data.c_str(), data.size());

	/* Close file */
	file.close();

	/* Close connection */
	close(client_sock);

	return EXIT_SUCCESS;
}
