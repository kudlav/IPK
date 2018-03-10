/**
 * Project: IPK - client/server
 * Author: Vladan Kudlac
 * Created: 5.3.2018
 * Version: 0.1
 */

/* Error codes: */
#define EXIT_ARG 1 // error when parsing arguments
#define EXIT_NET 2 // network error
#define EXIT_IOE 3 // I/O error
#define EXIT_SYS 4 // system error (fork)
#define EXIT_COM 5 // communication error (invalid request)

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
		 << "./ipk-server -p <port>\n"
		 << "  -p <port>\n"
		 << "      cislo portu, na kterem server nasloucha\n"
	;
	exit(EXIT_ARG);
}

int main(int argc, char *argv[])
{
	if (argc == 1) { // Called without parameters
		printHelp(); // exit(EXIT_ARG);
	}

	uint16_t port;

	int opt;
	while ((opt = getopt(argc, argv, "p:")) != -1) {
		switch (opt) {
			case 'p': {
				char *endptr;
				long port_raw = strtol(optarg, &endptr, 10);
				if (*endptr != '\0' || port_raw < 0 || port_raw > 65535) {
					cerr << "CHYBA: hdnota parametru -p musi byt cele kladne cislo v intervalu <0;65535>\n";
					printHelp(); // exit(EXIT_ARG);
				}
				port = (uint16_t) port_raw;
				} break;
			case '?':
				if (optopt == 'p') {
					cerr << "CHYBA: chybi hodnota u argumentu -p\n";
				}
				else {
					cerr << "CHYBA: neznamy parametr -" << (char) optopt << '\n';
				}
			default:
				printHelp(); // exit(EXIT_ARG);
		}
	}

	/* Create socket */
	int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); // AF_INET = IPv4, SOCK_STREAM = sequenced, reliable
	if (server_sock < 0) {
		cerr << "CHYBA: nelze vytvorit socket\n";
		exit(EXIT_NET);
	}

	struct sockaddr_in server_addr;
	memset((char *) &server_addr, 0, sizeof(server_addr)); // Null undefined values

	/* Bind socket to all interfaces */
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port); // convert to uint16_t
	int result = bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr));
	if (result < 0) {
		cerr << "CHYBA: nelze priradit socket k sitovemu rozhranni\n";
		exit(EXIT_NET);
	}

	/* Listen for connections on a socket */
	if (listen(server_sock , 1) < 0) { // MAX 1 WAITTING CLIENT
		cerr << "CHYBA: socket se nepodarilo nastavit pro prijmani prichozich pozadavku\n";
		exit(EXIT_NET);
	}

	cout << "Sever bezi.\n";

	/* Handle waitting connections from a client */
	struct sockaddr_in client_addr;
	int client_addr_length = sizeof(client_addr);
	while (true) {
		/* Accept connection */
		int client_sock = accept(server_sock, (struct sockaddr*) &client_addr, (socklen_t*) &client_addr_length);
		if (client_sock < 0) {
			cerr << "CHYBA: prichozi pozadavek nelze prijmout\n";
			continue;
		}

		/* CONNECTION HANDLING */

		pid_t pid = fork();
		if (pid < 0) { // FORK: ERROR
			cerr << "CHYBA: nepodarilo se vytvorit proces pro zpracovani pozadavku\n";
			exit(EXIT_SYS);
		}
		else if (pid == 0) { // FORK: CHILD (processing)
			/* Child - close parent socket (just for child process) */
			close(server_sock);

			pid = getpid();
			cout << "New connection, PID:" << pid << '\n';

			/* Wait for client request */
			string path_out = "";
			string path_in = "";
			string data = "";
			data.resize(1024);
			recv(client_sock, &data[0], data.size(), 0);
			if (data.find("RECV") != string::npos) { // Download file from server
				path_in = data.substr(5);
			}
			else if (data.find("SEND") != string::npos) { // Upload file to server
				path_out = data.substr(5);
			}
			else {
				data = "400 BAD_REQUEST";
				write(client_sock, data.c_str(), data.size());
				cerr << "CHYBA: klient zaslal neplatny pozadavek: " << data << '\n';
				exit(EXIT_COM);
			}

			/* Open file */
			fstream file;
			if (!path_out.empty()) {
				file.open(path_out.c_str(), ios::out | ios::trunc | ios::binary);
			}
			else {
				file.open(path_in.c_str(), ios::in | ios::binary);
			}
			if (!file.is_open()) {
				data = "403 FILE_ERROR";
				write(client_sock, data.c_str(), data.size());
				cerr << "CHYBA: soubor nelze otevrit\n";
				exit(EXIT_IOE);
			}

			data = "200 OK";
			write(client_sock, data.c_str(), data.size());

			/* File transfer */
			data.clear();
			data.resize(1024);
			if (!path_out.empty()) {
				while (read(client_sock, &data[0], data.size()) > 0) {
					file << data;
					data.clear();
					data.resize(1024);
				}
			}
			else {
				do {
					file.read(&data[0], data.size());
					write(client_sock, data.c_str(), data.size());
					data.clear();
					data.resize(1024);
				}
				while (!file.eof());
			}

			/* Close file */
			file.close();

			/* Child - close connection */
			close(client_sock);

			cout << "End connection, PID:" << pid << '\n';

			exit(EXIT_SUCCESS);
		}
		else { // FORK: MASTER (control)
			/* Master - close connection */
			close(client_sock);
		}
	}

		/* END OF CONNECTION HANDLING */
}
