# kalaha
A terminal based implementation in C of the Kalah[a] board game, also known as Mancala.

As there exist lots of variations of the game, this is an implementation of just one of them (see rules below).

## About the game
Kalaha (sometimes known as Kalah or Mancala) is a two player board game. The board has the following setup:
![Initial playing board](assets/init_board.jpg "Initial playing board")
- There are 6 small pits for each player (referred to as just pits). Initially, they are each filled with 6 beads.
- There is one big pit per player on the to the left and above the pits. It is called a player's home.
- Turns are being taken in a clockwise direction

The turn always starts by taking all the beads from any of the pits that belong to the player (but not the home). Then, the beads are being placed one by one out of the hand into each pit clockwise, including the player's own home but excluding the other player's home.
- If the last bead was placed in an empty pit, the turn is over
- If the last bead was placed in a non-empty pit, the player takes all the beads from that pit again and continues this process
- If the last bead was placed in the player's home, they get an extra turn where they can again choose which pit to play.

The game ends whenever either either player has no beads left. The winner is the person who has the greatest sum of the number of beads in their home and the number of beads other player has left in their pits.

## Usage
Build the game by running `make`. Run with `./game.out`.

## TODO:
- Finish this doc
- (in progress) Write documentation for everything lol
- (in progress) Difficulty settings
- Player names