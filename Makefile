


server:
	g++ server.cpp server_utils.cpp die.cpp -o server


subscriber:
	g++ client.cpp client_utils.cpp die.cpp -o subscriber

clean: 
	rm server subscriber