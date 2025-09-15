import socket 
import threading 
import difflib

bind_ip = "0.0.0.0" 
bind_port = 4242

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind((bind_ip, bind_port)) 
# we tell the server to start listening with 
# a maximum backlog of connections set to 5
server.listen(5) 

print(f"[+] Listening on port {bind_ip} : {bind_port}")                            


buf=""
#client handling thread
def handle_client(client_socket): 
    #printing what the client sends 
    while (True):
        request = client_socket.recv(1024) 
        print(f"[+] Recieved: {request}") 
        #sending back the packet
        #print("Sending response: Ping received") 
        client_socket.send("Ping received\n".encode())
    client_socket.close()

# When a client connects we receive the 
# client socket into the client variable, and 
# the remote connection details into the addr variable
client, addr = server.accept() 
print(f"[+] Accepted connection from: {addr[0]}:{addr[1]}")
client.send("Connection established!".encode())
while (True):
    request = client.recv(1024)
    if (request):
        print(f"[+] Recieved: {request}") 
        client.send("Ping received\n".encode())
        for i in request.decode():
            if i != '\r':
                buf += i
            else:
                break
        if buf.upper() == 'CLOSE':
            print("Closing connection!")
            client.close()
            break
        else:
            print("Length of what you typed is", len(buf))
            print("Maintaining connection...")

        buf = ""


client_handler = threading.Thread(target=handle_client, args=(client,))
client_handler.start()
