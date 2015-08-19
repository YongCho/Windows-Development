### How to run

Run developer command prompt.
Cd into Server folder.
run cl main.cpp.
run main.exe <listening port number>. It will start listening on that port.

Run another developer command prompt.
Cd into Client folder.
run cl main.cpp
run main.exe <server address> <server port>. Since the server program in on the same computer, give 127.0.0.1 for the server address. The port number should be the same as the listening port on the server. It will connect to the server and send a small data. Both the server and the client will terminate then.


### How to open in VS 2013
1.
Open VS 2013.
Open Project...
Select WinSock2 HowTo.sln which contains both Server and Client projects.

2.
Right click on WinSock2 HowTo.sln
Open with -> Microsoft Visual Studio 2013
