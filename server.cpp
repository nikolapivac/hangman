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
        message = guessing.c_str();
        send(client_socket, message, strlen(message), 0);

        int bytes_recieved = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes_recieved == -1)
        {
            cout << "Error: Failed to recieve data from client\n"
                 << endl;
            close(client_socket);
            return NULL;
        }
        letter = buffer[0];

        // check if the user entered an empty string
        if (letter.length() == 0)
        {
            message = "You didn't enter anything. Please enter a letter.\n";
            cout << "Client entered empty string\n";
            send(client_socket, message, strlen(message), 0);
            continue;
        }

        // check if the user entered multiple characters
        if (letter.length() > 1)
        {
            message = "You entered multiple characters. Please enter only one letter.\n";
            cout << "Client entered multiple characters\n";
            send(client_socket, message, strlen(message), 0);
            continue;
        }

        // check if the user entered a letter
        if (!isalpha(letter[0]))
        {
            message = "Please enter a letter.\n";
            cout << "Client didn't enter a letter\n";
            send(client_socket, message, strlen(message), 0);
            continue;
        }

        letter = tolower(letter[0]);
        // check if user has already guessed this letter
        if (guessing.find(letter) != string::npos)
        {
            message = "You already guessed this letter. Try again.\n";
            cout << "Client has already guessed this letter\n";
            send(client_socket, message, strlen(message), 0);
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
                cout << "Client lost all lives\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
            }
            else
            {
                tempMessage = "Incorrect. You have " + to_string(lives) + " lives left.\n";
                cout << "Client guessed wrong and lost a life\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
            }
        }
        else
        {
            if (guessing == answer)
            {
                tempMessage = "You guessed the answer - " + answer + "!\n";
                cout << "Client won\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
            }
            else
            {
                tempMessage = "Correct! You have " + to_string(lives) + " lives left.\n";
                cout << "Client guessed the right letter\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
            }
        }
    }
    close(client_socket);
    return NULL;
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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the server socket to the server address
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
            continue;
        }
        cout << "Accepted client" << endl;

        // Creating a new thread and starting the game for each client
        if (pthread_create(&thread_id, NULL, hangmanGame, (void *)&client_socket) != 0)
        {
            cout << "Error creating new thread" << endl;
            continue;
        }
        cout << "Thread for client created" << endl;

        // Detach the thread so it can run independently
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}