#include <cstdlib>
#include <iostream>
#include <ctime>
#include <string>
#include <cctype>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <fstream>
#include <vector>

using namespace std;

constexpr int PORT = 8000;
constexpr int MAX_CLIENTS = 8;

bool sendMessageToClient(int socket, const char *message, size_t len)
{
    int status = send(socket, message, len, 0);
    if (status < 0)
    {
        cout << "Error: Failed to send message to client.\n"
             << endl;
        return false;
    }
    return true;
}

string receiveLetterFromClient(int socket)
{
    char buffer[1];
    bzero(buffer, 1);
    int status = recv(socket, buffer, 1, 0);
    if (status < 1)
    {
        return "";
    }
    else
    {
        string letter;
        letter = buffer[0];
        return letter;
    }
}

void readNamesFromFile(vector<string> &names)
{
    ifstream file("names.txt");
    string name;

    while (file >> name)
    {
        names.push_back(name);
    }

    file.close();
}

string pickRandomName()
{
    vector<string> names;
    readNamesFromFile(names);
    srand(time(0));

    int randomNum = rand() % names.size();
    string answer = names[randomNum];

    return answer;
}

void hangmanGame(int client_socket)
{
    int lives = 5;
    string answer = pickRandomName();
    int answerLength = answer.length();

    // initialize the guessing string (board)
    string guessing(answerLength, '_');

    const char *board;
    string letter;
    string message;

    // game loop
    while (lives > 0 && guessing != answer)
    {
        // send current guessing board
        board = (guessing + "\n").c_str();
        if (!sendMessageToClient(client_socket, board, strlen(board)))
            break;

        // receive letter from client
        letter = receiveLetterFromClient(client_socket);
        if (letter == "")
        {
            cout << "Error: Failed to receive letter from client\n"
                 << endl;
            break;
        }

        // check if the user entered a letter
        if (!isalpha(letter[0]))
        {
            message = "Please enter a letter.\n";
            if (!sendMessageToClient(client_socket, message.c_str(), strlen(message.c_str())))
                break;
            cout << "Client " << client_socket << " didn't enter a letter\n";
            continue;
        }

        letter = tolower(letter[0]);
        // check if user has already guessed this letter
        if (guessing.find(letter) != string::npos)
        {
            message = "You already guessed this letter. Try again.\n";
            if (!sendMessageToClient(client_socket, message.c_str(), strlen(message.c_str())))
                break;
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
                message = "Game over. You failed to guess the answer - " + answer + "!\n";
                if (!sendMessageToClient(client_socket, message.c_str(), strlen(message.c_str())))
                    break;
                cout << "Client " << client_socket << " lost all lives\n";
                close(client_socket);
            }
            else
            {
                message = "Incorrect. You have " + to_string(lives) + " lives left.\n";
                if (!sendMessageToClient(client_socket, message.c_str(), strlen(message.c_str())))
                    break;

                cout << "Client " << client_socket << " guessed wrong and lost a life\n";
            }
        }
        else
        {
            if (guessing == answer)
            {
                message = "You guessed the answer - " + answer + "!\n";
                if (!sendMessageToClient(client_socket, message.c_str(), strlen(message.c_str())))
                    break;
                cout << "Client " << client_socket << " won\n";
                close(client_socket);
            }
            else
            {
                message = "Correct! You have " + to_string(lives) + " lives left.\n";
                if (!sendMessageToClient(client_socket, message.c_str(), strlen(message.c_str())))
                    break;
                cout << "Client " << client_socket << " guessed the right letter\n";
            }
        }
    }

    close(client_socket);
}

int main()
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;

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
            return -1;
        }
        cout << "Accepted client " << client_socket << endl;

        // Creating a new thread and starting the game for each client
        try
        {
            thread(hangmanGame, client_socket).detach();
        }
        catch (...)
        {
            cout << "Error creating new thread" << endl;
            close(client_socket);
            return -1;
        }
        cout << "Thread for client " << client_socket << " created" << endl;
    }

    close(server_socket);
    return 0;
}