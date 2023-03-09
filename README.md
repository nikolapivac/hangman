# Hangman

## Description

A simple game of hangman made to exercise connecting the client and the server via sockets in Python and C++.

## Contents

server

- made in C++
- has a list of names and randomly picks an answer
- validates user's input (a letter)
- checks if the letter is guessed (and if the game is over)
- supports multiple client connections (pthreads)

client

- made in Python
- gets input (letter) from the player
- sends input to the server and gets the response back

## Instructions

- compile the server (g++ server.cpp -o server -lpthread)
- run the client python script (python3 client.py)
