from subprocess import Popen, PIPE, STDOUT

hangman = Popen(["./server"], stdin=PIPE, stdout=PIPE, stderr=STDOUT)

print("Welcome to HANGMAN!")

while True:
    board = hangman.stdout.readline().decode().strip()
    print(board)
    
    letter = input("Enter a letter: ")
    hangman.stdin.write(letter.encode() + b"\n")
    hangman.stdin.flush()

    status = hangman.stdout.readline().decode().strip()
    print(status)
    if(status.startswith("You guessed")): 
        break
    elif(status.startswith("Game over.")):
        break
    else:
        continue


hangman.stdin.close()
hangman.stdout.close()
hangman.kill()
