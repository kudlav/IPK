#include <iostream>
#include <unistd.h>

using namespace std;

int main(int argc, char *argv[]) {
	cout << "Hello, World!" << endl;
	cerr << "Erorr!!!!!" << endl;

	getopt(argc, argv, "h:p:r:w:");

	return 0;
}