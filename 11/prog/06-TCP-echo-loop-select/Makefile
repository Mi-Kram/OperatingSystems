all: TCPEchoClientLoop TCPEchoServer-Select
TCPEchoClientLoop: TCPEchoClientLoop.c DieWithError.c
	gcc TCPEchoClientLoop.c DieWithError.c -o TCPEchoClientLoop
TCPEchoServer-Select: TCPEchoServer-Select.c DieWithError.c
	gcc TCPEchoServer.h TCPEchoServer-Select.c DieWithError.c \
	HandleTCPClient.c AcceptTCPConnection.c CreateTCPServerSocket.c \
	-o TCPEchoServer-Select
