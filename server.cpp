#include <cstdlib>
#include <iostream>
#include <ctime>
#include <string>
#include <cctype>

using namespace std;

int main()
{
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

    // game loop
    while (lives > 0 && guessing != answer)
    {
        cout << guessing << endl;

        string letter;
        getline(cin, letter);

        // check if the user entered an empty string
        if (letter.length() == 0)
        {
            cout << "You didn't enter anything. Please enter a letter." << endl;
            continue;
        }

        // check if the user entered multiple characters
        if (letter.length() > 1)
        {
            cout << "You entered multiple characters. Please enter only one letter." << endl;
            continue;
        }

        // check if the user entered a letter
        if (!isalpha(letter[0]))
        {
            cout << "Please enter a letter."
                 << endl;
            continue;
        }

        letter = tolower(letter[0]);
        // check if user has already guessed this letter
        if (guessing.find(letter) != string::npos)
        {
            cout << "You already guessed this letter. Try again."
                 << endl;
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
                cout << "Game over. You failed to guess the answer - " << answer << "!" << endl;
            }
            else
            {
                cout << "Incorrect. You have " << lives << " lives left." << endl;
            }
        }
        else
        {
            if (guessing == answer)
            {
                cout << "You guessed the answer - " << answer << "!" << endl;
            }
            else
            {
                cout << "Correct! You have " << lives << " lives left." << endl;
            }
        }
    }

    return 0;
}