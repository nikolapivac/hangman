import socket

HOST = "localhost"
PORT = 8000

print("Welcome to HANGMAN!")

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))

while True:
    board = client_socket.recv(1024).decode().strip()
    print(board)
    
    letter = input("Enter a letter: ")
    client_socket.send(letter.encode())

    status = client_socket.recv(1024).decode().strip()
    print(status)
    if(status.startswith("You guessed")): 
        break
    elif(status.startswith("Game over.")):
        break
    else:
        continue

client_socket.close()