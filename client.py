import socket

HOST = "localhost"
PORT = 8000

print("Welcome to HANGMAN!")

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))

while True:
    # Receive the guessing board from the server
    board = client_socket.recv(1024).decode()
    print(board)

    # Get the letter from the user and send it to the server
    letter = input("Enter a letter: ")
    client_socket.send(letter.encode() + b"\n")

    # Recieve the message from the server
    status = client_socket.recv(1024).decode()
    print(status)

    # If the message is a final one (win/lose), end the game
    if(status.startswith("You guessed")): 
        break
    elif(status.startswith("Game over.")):
        break
    else:
        continue

client_socket.close()