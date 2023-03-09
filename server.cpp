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

// ----------------------------------------------------------------
// Server otvara socket u main-u i bind-a ga na adresu i port. Zatim slusa
// i ceka da se klijenti spoje. Za svakog klijenta otvara novi thread
// i pokrece igru (hangmanGame funkcija). U toj funkciji se komunicira sa
// klijentom preko send/recv metoda. Prvo server salje klijentu stanje igre
// (na pocetku je to string "_" znakova, npr. za ime fiona ce poslat _____ tako
// da klijent zna koliko ime ima slova). Zatim server prima od klijenta slovo i
// izvrsava validaciju (je li klijent unio jedno slovo, je li uopce slovo, itd.)
// Nakon toga se provjerava je li slovo pogodjeno, ako je, potrebno je izmijenit onaj
// guessing string (ako je klijent pogodio "i", za fionu ce dobit nazad _i___) i konacno
// se klijentu salje jedna od 4 poruke, ovisno o tome je li pogodio, promasio, pobijedio
// ili izgubio. Ja kad pokrenem server, uspjesno mi se spoji klijent, posalje mu se inicijalni
// guessing string i pita me za unos slova na klijentu. Kad unesem slovo, server sve odradi i
// uspjesno posalje novi guessing string i poruku. Onda bi klijent trebao pitat za novo slovo
// ali ne pita, nego se klijent zamrzne. Tek kad ubijem servera, onda klijent pita za novo
// slovo. Pretpostavljam da je u pitanju nacin na koji se cita
// ili salje slovo (recv), jer se ne radi o zasebnim porukama nego o stream-u podataka, ali
// ne znam kako to prilagodit za ovaj moj slucaj.
//
// Kad ispravim taj problem trebalo bi jos dodat mozda nekakvu identifikaciju
// (username ili nesto slicno) za klijenta, ako ih se vise spoji
// da server zna koji mu je sta poslao.
// ----------------------------------------------------------------

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
        int status = send(client_socket, guessing.c_str(), strlen(guessing.c_str()), 0);
        if (status < 0)
        {
            cout << "Error: Failed to send guessing string to client" << endl;
            close(client_socket);
            return NULL;
        }

        status = recv(client_socket, buffer, 1024, 0);
        if (status == -1)
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
            send(client_socket, message, strlen(message), 0);
            cout << "Client entered empty string\n";
            continue;
        }

        // check if the user entered multiple characters
        if (letter.length() > 1)
        {
            message = "You entered multiple characters. Please enter only one letter.\n";
            send(client_socket, message, strlen(message), 0);
            cout << "Client entered multiple characters\n";
            continue;
        }

        // check if the user entered a letter
        if (!isalpha(letter[0]))
        {
            message = "Please enter a letter.\n";
            send(client_socket, message, strlen(message), 0);
            cout << "Client didn't enter a letter\n";
            continue;
        }

        letter = tolower(letter[0]);
        // check if user has already guessed this letter
        if (guessing.find(letter) != string::npos)
        {
            message = "You already guessed this letter. Try again.\n";
            send(client_socket, message, strlen(message), 0);
            cout << "Client has already guessed this letter\n";
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
                cout << "Client lost all lives\n";
                close(client_socket);
            }
            else
            {
                tempMessage = "Incorrect. You have " + to_string(lives) + " lives left.\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
                cout << "Client guessed wrong and lost a life\n";
            }
        }
        else
        {
            if (guessing == answer)
            {
                tempMessage = "You guessed the answer - " + answer + "!\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
                cout << "Client won\n";
                close(client_socket);
            }
            else
            {
                tempMessage = "Correct! You have " + to_string(lives) + " lives left.\n";
                message = tempMessage.c_str();
                send(client_socket, message, strlen(message), 0);
                cout << "Client guessed the right letter\n";
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
        cout << "Accepted client" << endl;

        // Creating a new thread and starting the game for each client
        if (pthread_create(&thread_id, NULL, hangmanGame, (void *)&client_socket) != 0)
        {
            cout << "Error creating new thread" << endl;
            close(client_socket);
            exit(1);
        }
        cout << "Thread for client created" << endl;
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}