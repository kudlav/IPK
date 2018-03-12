/**
 * Project: IPK - client/server
 * Author: Vladan Kudlac
 * Created: 5.3.2018
 * Version: 0.3
 */

#include "ipk-shared.cpp"

/**
 * Print usage of this program and exit with code EXIT_ARG (error when parsing arguments).
 */
void printHelp()
{
	cerr << "Pouziti:\n"
			"./ipk-client -h host -p port [-r|-w] soubor\n"
			"  -h <host>\n"
			"      (IP adresa nebo fully-qualified DNS name) identifikace serveru jakozto koncoveho bodu komunikace klienta\n"
			"  -p <port>\n"
			"      cilove cislo portu\n"
			"  -r <soubor>\n"
			"      klient stahne soubor ze serveru\n"
			"  -w <soubor>\n"
			"      klient nahraje soubor na server\n"
			"\n"
			"Navratove kody:\n"
			"  0    ok\n"
			"  1    chyba pri zpracovani argumentu\n"
			"  2    chyba sitoveho rozhranni (např: nelze vytvorit socket, priradit port, vytvorit spojeni)\n"
			"  3    chyba prace se soubory\n"
			"  5    chyba komunikace (server odpovedel chybou)"
	;
	exit(EXIT_ARG);
}

/**
 * Parse arguments from CLI. If an error occurs inside function, program ends with code EXIT_ARG
 *
 * @param argc Number arguments from CLI
 * @param argv Array with CLI arguments
 * @param port Pointer where port number will be stored
 * @param host Pointer where host string will be stored
 * @param path_in Pointer where path of input file will be stored
 * @param path_out Pointer where path of output file will be stored
 */
void parseArguments(int argc, char *argv[], uint16_t* port, string* host, string* path_in, string* path_out) {
	if (argc == 1) { // Called without parameters
		printHelp(); // exit(EXIT_ARG)
	}
	int opt;
	bool port_set = false;
	while ((opt = getopt(argc, argv, "h:p:r:w:")) != -1) {
		switch (opt) {
			case 'h':
				*host = optarg;
				break;
			case 'p': {
				char *endptr;
				long port_raw = strtol(optarg, &endptr, 10);
				if (*endptr != '\0' || port_raw < 0 || port_raw > 65535) {
					cerr << "CHYBA: hodnota parametru -p musi byt cele kladne cislo v intervalu <0;65535>\n";
					printHelp(); // exit(EXIT_ARG)
				}
				*port = (uint16_t) port_raw;
				port_set = true;
			} break;
			case 'r':
				(*path_out) = optarg;
				if (!(*path_in).empty()) {
					cerr << "CHYBA: parametr -r nelze pouzit zaroven s parametrem -w\n";
					printHelp(); // exit(EXIT_ARG)
				}
				break;
			case 'w':
				(*path_in) = optarg;
				if (!(*path_out).empty()) {
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

	if ((*host).empty() || !port_set || ((*path_out).empty() && (*path_in).empty())) {
		cerr << "CHYBA: chyby povinne parametry\n";
		printHelp(); // exit(EXIT_ARG)
	}
}

/**
 * Main function of ipk-client.
 *
 * @param argc Number of CLI arguments
 * @param argv Array with CLI arguments
 * @return See return codes above
 */
int main(int argc, char *argv[])
{
	/* Parse arguments */
	uint16_t port;
	string host = "";
	string path_in = "";
	string path_out = "";
	parseArguments(argc, argv, &port, &host, &path_in, &path_out);

	/* DNS lookup for host address */
	struct hostent *server = gethostbyname(host.c_str());
	if (server == nullptr) {
		cerr << "CHYBA: adresa serveru nenalezena\n";
		exit(EXIT_NET);
	}

	/* Create socket */
	int client_sock = socket(AF_INET, SOCK_STREAM, 0); // AF_INET = IPv4, SOCK_STREAM = sequenced, reliable
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
	fstream file;
	if (!path_out.empty()) {
		file.open(path_out.c_str(), ios::out | ios::trunc | ios::binary);
	}
	else {
		file.open(path_in.c_str(), ios::in | ios::binary);
	}

	if (!file.is_open()) {
		cout << "CHYBA: soubor nelze otevrit\n";
		exit(EXIT_IOE);
	}

	/* Read/write request to the server */
	string data;
	if (!path_out.empty()) {
		data = "RECV " + path_out;
	}
	else {
		data = "SEND " + path_in;
	}
	write(client_sock, data.c_str(), data.size());

	/* Confirmation from server */
	data.clear();
	data.resize(DATA_SIZE);
	recv(client_sock, &data[0], data.size(), 0);
	if (data.find("200 OK") == string::npos) {
		cerr << "CHYBA: server odpovedel chybou: " << data;
		exit(EXIT_COM);
	}

	/* File transfer */
	int loaded;
	data.clear();
	data.resize(DATA_SIZE);
	if (!path_out.empty()) {
		while ((loaded = read(client_sock, &data[0], data.size())) > 0) {
			if (loaded < DATA_SIZE) {
				data.resize((unsigned int) loaded);
			}
			file << data;
			data.clear();
			data.resize(DATA_SIZE);
		}
	}
	else {
		do {
			file.read(&data[0], data.size());
			write(client_sock, data.c_str(), (unsigned int) file.gcount());
		}
		while (!file.eof());
	}

	/* Close file */
	file.close();

	/* Close connection */
	close(client_sock);

	return EXIT_SUCCESS;
}
