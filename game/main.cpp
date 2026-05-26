#include<iostream>
#include<queue>

#include"Game.h"
#include "Entity.h"
#include "Render.h"

#include<SFML/Graphics.hpp>

using namespace std;

/*
 *Upside Down Maze
 *
 *Escape a dangerous maze as Eleven inspired by the popular
 *series Stranger Things while fending off Demogorgons and
 *collecting powerups in a board style game.
 */

int main(){
  auto* render=new Render();
  auto* board=new Board(render);
  auto* game=new Game(board,render);

  game->newGame();
  Clock turnClock;
  const float turnDelay=0.2f;
  board->getValidMoves(*game->player->position);

  //MAIN LOOP
  while(render->window.isOpen()) {
    game->Display();//Display graphics

    if(game->isGameOver()) {//Game over
      if(game->gameOver()) {
        break;
      }
    }
    game->centerCamera();//Follow player with camera

    if(turnClock.getElapsedTime().asSeconds()>=turnDelay) {//Wait between turns
      if(game->turn==GameState::PlayerTurn) {
        Event e;
        while(game->turn==GameState::PlayerTurn and render->window.pollEvent(e)) {
          if(e.type==Event::Closed) {//Window x pressed
            render->window.close();
          }
          board->getValidMoves(*game->player->position);
          game->playerInput(e);
        }
      }else if(not game->player->combat) {
        game->enemyTurns();

        if(size(game->enemies)<size(board->board_)/18) {//Respawn banished enemies and more as explore board
          board->spawnEnemies(2,game->enemies,game->player);
        }
      }
      turnClock.restart();
    }
  }

  //Free memory
  delete render;
  delete game;
  delete board;
  return 0;
}




