import socket

HOST = "localhost"
PORT = 8000

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))

def receiveMessageFromServer(socket):
    buffer = b""
    while b"\n" not in buffer:
        data = socket.recv(1)
        if not data:
            break
        buffer += data
    return buffer

print("Welcome to HANGMAN!")
username = input("Enter your username: ")
client_socket.send(username.encode())

while True:
    # Receive the guessing board from the server
    board = receiveMessageFromServer(client_socket)
    print(board.decode())

    # Get the letter from the user and send it to the server
    letter = input("Enter a letter: ")
    if not letter:
        print("You didn't enter anything.\n")
        break
    elif len(letter) > 1:
        print("You entered multiple letters.\n")
        break
    client_socket.send(letter.encode())

    # Recieve the message from the server
    status = receiveMessageFromServer(client_socket)
    print(status.decode())

    check = status.decode()
    # If the message is a final one (win/lose), end the game
    if(check.startswith("You guessed")): 
        break
    elif(check.startswith("Game over.")):
        break
    else:
        continue

client_socket.close()