# Socket-Chatbot

## "chat"-like facility that enables a user at one terminal to communicate with a user at another terminal.

### Use: 
use "make" to create executable. then run ./lets-talk [my port number] [remote/local machine IP] [remote/local port number]
Example: lets-talk 6060 IP_of_machine2  6001

"!exit" will quit the connection

_____________________________________________
This project uses two main LINUX concepts: 
1. Threads - creating a threads list and assigning tasks to them. 
2. UDP 

Four threads will have access to a list ADT. The keyboard input thread will add input from the user to the list of messages to be sent to the remote client through the UDP sender thread. The UDP receiver thread will receive messages from the remote client and add them to the list of messages to be displayed on the local screen by the console output thread. The console output thread will remove messages from the list and display them on the screen.



