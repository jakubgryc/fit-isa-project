from socket import *

app_addr = "", 4747
app_socket = socket(AddressFamily.AF_INET, SocketKind.SOCK_DGRAM)
app_socket.bind(app_addr)

while True:
    mess, sen_addr = app_socket.recvfrom(2048)
    print(str(mess.decode()), end=" ")
    print(sen_addr)
