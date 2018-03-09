all:ipk-client ipk-server

ipk-client:
	g++ -std=c++14 ipk-client.cpp -o ipk-client

ipk-server:
	g++ -std=c++14 ipk-server.cpp -o ipk-server

clear:
	rm ipk-client ipk-server
