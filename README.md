# Hangman

## Description

A simple game of hangman made to exercise connecting the client and the server via subprocess in Python.

## Contents

server

- made in C++
- has a list of names and randomly picks an answer
- validates user's input (a letter)
- checks if the letter is guessed (and if the game is over)

client

- made in Python
- using subprocess (Popen)
- gets input (letter) from the player
- sends input to the server and gets the response back

## Instructions

- compile the server (g++ server.cpp -o server)
- run the client python script (python3 client.py)
