/**
 * Project: IPK - client/server
 * Author: Vladan Kudlac
 * Created: 5.3.2018
 * Version: 0.2
 */

/* Error codes: */
#define EXIT_ARG 1 // error when parsing arguments
#define EXIT_NET 2 // network error
#define EXIT_IOE 3 // I/O error
#define EXIT_SYS 4 // system error (fork)
#define EXIT_COM 5 // communication error (invalid request / error response)

/* Requirements */
#include <iostream> // IO operations
#include <fstream> // files IO
#include <unistd.h> // fork, getpid, getopt
#include <cstring> // memset, memcpy
#include <sys/types.h> // for BSD systems
#include <sys/socket.h> // accept, bind, recv
#include <netinet/in.h> // INADDR_ANY, htons
#include <netdb.h> // struct hostent

//#ifdef _WIN32
//#include <afxres.h>
//#endif

using namespace std;
