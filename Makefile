all:ipk-client ipk-server

ipk-client: ipk-client.cpp ipk-shared.cpp
	g++ -std=c++14 ipk-client.cpp -o ipk-client

ipk-server: ipk-server.cpp ipk-shared.cpp
	g++ -std=c++14 ipk-server.cpp -o ipk-server

clear:
	rm ipk-client ipk-server
