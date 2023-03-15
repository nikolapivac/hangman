#include <cstdlib>
#include <iostream>
#include <ctime>
#include <string>
#include <cctype>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

using namespace std;

const int PORT = 8000;
const int MAX_CLIENTS = 8;

void *hangmanGame(void *arg)
{
    int client_socket = *(int *)arg;
    int lives = 5;
    string names[10] = {
        "anastasia",
        "andrew",
        "bradley",
        "bonnie",
        "christian",
        "fiona",
        "george",
        "oliver",
        "gladys",
        "pamela"};

    // setting the random generator with current time
    // so its a different random number each time the program runs
    srand(time(0));
    int randomNum = rand() % (sizeof(names) / sizeof(names[0]));

    // pick an answer randomly
    string answer = names[randomNum];
    int answerLength = answer.length();

    // initialize the guessing string
    string guessing;
    for (int i = 0; i < answerLength; i++)
    {
        guessing += "_";
    }

    const char *message;
    char buffer[1024];
    string letter;
    string tempMessage;

    // game loop
    while (lives > 0 && guessing != answer)
    {
        bzero(buffer, 1024);
        message = (guessing + "\n").c_str();
        int status = send(client_socket, message, strlen(message), 0);
        if (status < 0)
        {
            cout << "Error: Failed to send guessing string to client\n"
                 << endl;
            close(client_socket);
            pthread_exit(NULL);
        }

        status = recv(client_socket, buffer, 1, 0);
        if (status < 1)
        {
            cout << "Error: Failed to recieve data from client\n"
                 << endl;
            close(client_socket);
            pthread_exit(NULL);
        }
        letter = buffer[0];

        // check if the user entered a letter
        if (!isalpha(letter[0]))
        {
            message = "Please enter a letter.\n";
            send(client_socket, message, strlen(message), 0);
            cout << "Client " << client_socket << " didn't enter a letter\n";
            continue;
        }

        letter = tolower(letter[0]);
        // check if user has already guessed this letter
        if (guessing.find(letter) != string::npos)
        {
            message = "You already guessed this letter. Try again.\n";
            send(client_socket, message, strlen(message), 0);
            cout << "Client " << client_socket << " has already guessed this letter\n";
            continue;
        }

        // compare every letter of the answer to user's input letter
        bool found = false;
        for (int i = 0; i < answerLength; i++)
        {
            if (answer[i] == letter[0])
            {
                guessing[i] = letter[0];
                found = true;
            }
        }

        // if the letter is incorrect, take 1 life
        if (!found)
        {
            --lives;
            if (lives == 0)
            {
                tempMessage = "Game over. You failed to guess the answer - " + answer + "!\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
                cout << "Client " << client_socket << " lost all lives\n";
                close(client_socket);
            }
            else
            {
                tempMessage = "Incorrect. You have " + to_string(lives) + " lives left.\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
                cout << "Client " << client_socket << " guessed wrong and lost a life\n";
            }
        }
        else
        {
            if (guessing == answer)
            {
                tempMessage = "You guessed the answer - " + answer + "!\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
                cout << "Client " << client_socket << " won\n";
                close(client_socket);
            }
            else
            {
                tempMessage = "Correct! You have " + to_string(lives) + " lives left.\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
                cout << "Client " << client_socket << " guessed the right letter\n";
            }
        }
    }

    close(client_socket);
    pthread_exit(NULL);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    pthread_t thread_id;

    // Creating the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        cout << "Error creating server socket" << endl;
        return -1;
    }
    cout << "Server socket created" << endl;

    // Set the server address and port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    // Bind the server socket to the address and port
    if (::bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        cout << "Error binding server socket" << endl;
        return -1;
    }
    cout << "Server socket bind complete" << endl;

    // Listening on the server socket
    if (listen(server_socket, MAX_CLIENTS) == -1)
    {
        cout << "Error listening for clients on server socket" << endl;
        return -1;
    }
    cout << "Listening..." << endl;

    // Accepting client connections as they come in, and creating a new thread for each client
    while (true)
    {
        socklen_t client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == -1)
        {
            cout << "Error accepting client connection" << endl;
            close(client_socket);
            exit(1);
        }
        cout << "Accepted client " << client_socket << endl;

        // Creating a new thread and starting the game for each client
        if (pthread_create(&thread_id, NULL, hangmanGame, (void *)&client_socket) != 0)
        {
            cout << "Error creating new thread" << endl;
            close(client_socket);
            exit(1);
        }
        cout << "Thread for client " << client_socket << " created" << endl;
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}