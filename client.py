import socket

HOST = "localhost"
PORT = 8000

print("Welcome to HANGMAN!")

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect((HOST, PORT))

while True:
    # Receive the guessing board from the server
    board = b""
    while b"\n" not in board:
        data = client_socket.recv(1)
        if not data:
            break
        board += data
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
    status = b""
    while b"\n" not in status:
        data = client_socket.recv(1)
        if not data:
            break
        status += data
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