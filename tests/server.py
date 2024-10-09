import socket
import sys
import signal
import threading

# Global variable to keep track of the server socket
server_socket = None


# Function to handle individual client connections
def handle_client(connection, client_address):
    print(f"New connection from {client_address}")

    try:
        # Handle communication with the client in a loop
        while True:
            data = connection.recv(1024)
            if data:
                print(f"Received from {client_address}: {data.decode()}")
            else:
                print(f"Client {client_address} disconnected.")
                break
    except:
        print(f"Error with client {client_address}")
    finally:
        connection.close()


def signal_handler(sig, frame):
    print("\nServer shutting down...")
    if server_socket:
        server_socket.close()
    sys.exit(0)


def tcp_server(server_port):
    global server_socket
    # Create a TCP/IP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to the port
    server_address = ("localhost", server_port)
    print(f"Starting server on port {server_port}")
    server_socket.bind(server_address)

    # Listen for incoming connections
    server_socket.listen(5)  # Allow up to 5 connections in the queue

    while True:
        print("Waiting for a connection...")
        try:
            connection, client_address = server_socket.accept()

            # Start a new thread to handle the client
            client_thread = threading.Thread(
                target=handle_client, args=(connection, client_address)
            )
            client_thread.daemon = True  # Thread will exit when the main program exits
            client_thread.start()

        except KeyboardInterrupt:
            break


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python server.py <server_port>")
        sys.exit(1)

    server_port = int(sys.argv[1])

    # Register the signal handler for CTRL+C
    signal.signal(signal.SIGINT, signal_handler)

    tcp_server(server_port)
