# Escape the Upside Down Maze (In-Development)
Escape a dangerous maze as Eleven inspired by the popular series Stranger Things while fending off Demogorgons and
collecting powerups in a board style game. Originally given as a homework assignment for my Intensive Programming Workshop
class, I turned the assignment into an experiment with graph algorithms. The board is dynamically generated as 
the player moves into unexplored territory while always guaranteeing a path using Disjoint Set Union. Also, the enemies
use Dijkstra's single-source shortest path algorithm to move closer to the player. Turn logic and event processing
is written around what the SFML libraries provide.

## How to Run
From the /maze directory compile and run with "make".

Preview:
![Demo Gif](maze.gif)
