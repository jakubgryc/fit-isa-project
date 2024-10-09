import socket
import sys
import signal

# Global variable to keep track of the client socket
client_socket = None


def signal_handler(sig, frame):
    print("\nClient shutting down...")
    if client_socket:
        client_socket.close()
    sys.exit(0)


def tcp_client(server_port, client_port):
    global client_socket
    # Create a TCP/IP socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the client to a specific port
    client_socket.bind(("localhost", client_port))

    # Connect the socket to the server
    server_address = ("localhost", server_port)
    print(f"Connecting to server on port {server_port} from client port {client_port}")
    client_socket.connect(server_address)

    try:
        # Keep sending messages in a loop
        while True:
            message = input("Enter message to send (or type 'exit' to quit): ")
            if message.lower() == "exit":
                break
            client_socket.sendall(message.encode())


    except KeyboardInterrupt:
        pass
    finally:
        client_socket.close()


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python client.py <server_port> <client_port>")
        sys.exit(1)

    server_port = int(sys.argv[1])
    client_port = int(sys.argv[2])

    # Register the signal handler for CTRL+C
    signal.signal(signal.SIGINT, signal_handler)

    tcp_client(server_port, client_port)
