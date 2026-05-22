#include<iostream>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<algorithm>
#include<utility>
#include<cmath>
#include<random>
#include<climits>
#include<queue>
#include<cfloat>

#include "Game.h"
#include "Entity.h"
#include "Render.h"

#include<SFML/Graphics.hpp>

using namespace std;
using namespace sf;

//Player and enemy interact
void Game::Combat(Demogorgon* enemy) {
  if(enemy->empowered) {
    Board_->Respawn(player);
    Render_->addMessage("A Demogorgon has caught Eleven!");
  }else {
    if(player->surging) {
      player->toggleCombat();
    }else {
      Board_->Respawn(player);
      Render_->addMessage("A Demogorgon has caught Eleven!");
    }
  }
}

//GAME
Game::Game(Board* board,Render* render):
  player(nullptr),turn(GameState::PlayerTurn),dt(0)
{
  Render_=render;
  Board_=board;
}

//Constructor
void Game::newGame() {
  //Reinitialize
  turn=GameState::PlayerTurn;
  enemies.clear();
  Render_->messages={"","",""};
  Render_->mi=0;

  //Create new player and enemies
  player=new Player(Board_->origin_);
  Board_->Dijkstra(player->position);//Ensure spawned enemy can reach player
  Board_->spawnEnemies(1,enemies,player);
}

//The player is on a trap
void Game::Trapped(const Event& e) {
  if(e.key.code==Keyboard::Y) {//Remove gate
    player->useCharge();
    Board_->clearElement(player->position);

    if(!player->tele_charges) {
      player->deactivatePsychicSurge();
    }
    player->toggleTrapped();
    Render_->addMessage("A Rift has been sealed");
    turn=GameState::EnemyTurn;

  }else if(e.key.code==Keyboard::N){//Fall into gate
    player->loseLife();
    Render_->addMessage("Eleven has fallen into an Upside Down Rift!");
    player->setPosition(Board_->origin_);
    player->toggleTrapped();

    turn=GameState::EnemyTurn;
  }
}

//The player is battling a Demogorgon
void Game::inCombat(const Event& e) {
  if(e.key.code==Keyboard::Y) {//Banish an enemy
    Banish(player->position);
    player->toggleCombat();
    turn=GameState::EnemyTurn;

  }else if(e.key.code==Keyboard::N){
    Board_->Respawn(player);
    Render_->addMessage("A Demogorgon has caught Eleven!");
    player->toggleCombat();
    turn=GameState::EnemyTurn;

  }else if(e.key.code==Keyboard::T) {
    player->toggleTeleporting();
  }
}

//The player is toggling a spell cast
void Game::castSpell(const Event& e) {
  if(e.key.code==Keyboard::B) {//Banish
    if(player->banishing) {
      player->toggleBanishing();
    }else {
      auto pos=player->position;
      for(auto&[key,enemy]:enemies) {
        auto adj=enemy->position;
        if(abs(pos->row-adj->row)+abs(pos->col-adj->col)==1) {
          if(player->teleporting) {//Swap from teleporting to banishing
            player->toggleTeleporting();
          }
          player->toggleBanishing();break;
        }
      }
      if(not player->banishing) {
        Render_->addMessage("No Demogorgons are in range!");
      }
    }
  }else if(e.key.code==Keyboard::T){//Teleport
    if(player->banishing) {//Swap from banishing to teleporting
      player->toggleBanishing();
    }
    player->toggleTeleporting();
  }
}

//Process the user's input
void Game::playerInput(const Event& e) {
  //Mouse actions
  if(player->teleporting and Mouse::isButtonPressed(Mouse::Left)) {
    Teleport();

  //Keyboard actions
  }else if(e.type==Event::KeyPressed) {
    if(player->combat and player->surging) {
      inCombat(e);

    }else if(player->trapped) {
      Trapped(e);

    }else {//Move player or cast spell
      if(inputToPos.count(e.key.code)) {//Move player
        if(player->banishing) {//Banish
          Banish(Board_->valid_moves_[inputToPos.at(e.key.code)]);
          player->toggleBanishing();
          if(not player->combat){turn=GameState::EnemyTurn;}

        }else if(not player->teleporting) {
          auto move=Board_->valid_moves_[inputToPos.at(e.key.code)];
          if(not move) {
            Render_->addMessage("Invalid move");
          }else {
            playerTurn(move);
            if(not player->combat){turn=GameState::EnemyTurn;}
          }
        }

      }else if(player->surging) {
        castSpell(e);

      }else if(e.key.code==Keyboard::B or e.key.code==Keyboard::T) {
        Render_->addMessage("You have no ability charges");
      }
    }
  }
}

//The player takes their turn
void Game::playerTurn(const Position* pos) {
  for(auto&[key,enemy]:enemies) {//Compute stalemate before move to position
    if(enemy->position==pos) {
      if(Stalemate(enemy.get())){player->toggleStalemate();}
      break;
    }
  }
  if(not player->stalemate) {//If not stalemated take turn
    Board_->movePlayer(player,pos);

    for(auto&[key,enemy]:enemies) {
      if(enemy->position==pos) {
        Combat(enemy.get());
      }
    }
  }else {
    player->toggleStalemate();
  }

  if(not Board_->gate_placed and player->eggos==3) {
    Render_->addMessage("Eleven has collected enough Eggos! The Gate has appeared!");
    Board_->placeGate();
  }
}

//The enemies take their turns
void Game::enemyTurns() {
  if(not iterating) {
    keys.clear();
    for(auto&[key,ptr]:enemies) {
      keys.push_back(key);
    }
    ite=begin(keys);
    iterating=true;
  }

  while(ite!=end(keys)){
    auto it=enemies.find(*ite);
    if(it!=end(enemies)) {
      Board_->Dijkstra(player->position);
      demogorgonTurn(it->second.get());

      if(player->combat) {//Each enemy may attack separately
        turn=GameState::PlayerTurn;
        return;
      }
    }
    ++ite;
  }
  iterating=false;
  turn=GameState::PlayerTurn;
}

//A Demogorgon's turn
void Game::demogorgonTurn(Demogorgon* enemy) {
  const auto prd=enemy->position->pred;

  if(prd and not enemies.count(prd)) {//Move to predecessor from Dijkstras if no collision
    if(prd==player->position) {//Stalemate
      if(Stalemate(enemy)){return;}
      Combat(enemy);
    }
    Board_->moveDemogorgon(enemy,prd,enemies);

  }else {//If there is no path to the player pick a random move without collisions
    auto pra=Board_->randomAdvance(enemy,enemies);

    if(pra) {
      Board_->moveDemogorgon(enemy,pra,enemies);
    }
  }
}

bool Game::Stalemate(Demogorgon* enemy) {
  if(enemy->empowered and player->surging) {
    player->deactivatePsychicSurge();
    enemy->deactivateEmpowerment();
    Render_->addMessage("A Psychic Stalemate occurred!");
    return true;
  }
  return false;
}

//if land on same square prompt to banish before lose life
void Game::displayBanish() const {
  if(not player->banishing){return;}
  auto pla=player->position;

  for(auto&[key,enemy]:enemies) {//optimize for no redraw since you wont move
    auto pos=enemy->position;
    if(abs(pos->col-pla->col)+abs(pos->row-pla->row)>1){continue;}
    Sprite sprite(Render::textures["Banish"]);

    Render_->setPosition(sprite,*pos);
    Render_->setScale(sprite);
    sprite.setColor(Color(255,255,255,128));
    Render_->window.draw(sprite);
  }
  Render_->defaultView();

  Text text;
  text.setFont(Render_->pixlet);

  text.setString("Choose a Demogorgon to banish");
  text.setCharacterSize(80);
  text.setFillColor(Color::White);
  text.setPosition(50,200);
  Render_->window.draw(text);
}

//Display positions within 2 tiles to the player
void Game::displayTeleport(Vector2f mos) const {
  if(not player->teleporting){return;}

  auto pos=player->position;
  float minDist=FLT_MAX,
      xm=mos.x/Render::window_const,
      ym=mos.y/Render::window_const;
  int cursor=0;

  for(int k=0;k<8;k++) {//Display valid
    int x=tpi[k]+pos->col,y=tpj[k]+pos->row;
    auto adj=Board_->board_.find({y,x});
    if(adj==end(Board_->board_) or adj->type==TileType::Wall) {
      continue;
    }
    Sprite sprite(Render::textures[("Teleport")]);
    sprite.setPosition(Render::window_const*x,Render::window_const*y);

    sprite.setColor(Color(255,255,255,64));
    Render_->setScale(sprite);
    Render_->window.draw(sprite);

    int dist=abs(xm-x)+abs(ym-y);
    if(dist<minDist) {
      minDist=dist;
      cursor=k;
    }
  }

  Sprite sprite(Render::textures["Teleport"]);

  //Display nearest to cursor
  Vector2f pf((tpi[cursor]+pos->col)*Render::window_const,
    (tpj[cursor]+pos->row)*Render::window_const);

  sprite.setPosition(pf);
  sprite.setColor(Color(255,255,255,128));
  sprite.setScale(Render::sprite_const,Render::sprite_const);
  Render_->window.draw(sprite);
}

//Remove enemy from board
void Game::Banish(const Position* pos) {
  auto it=enemies.find(*ite);
  if(it!=end(enemies) and pos==it->first) {//Safely remove enemies from container
    enemies.erase(it);
  }else {
    enemies.erase(pos);
  }

  player->useCharge();
  Board_->walk_positions_.insert(pos);
  Render_->addMessage("A Demogorgon has been banished!");
}

//Teleport player to selected position if valid
void Game::Teleport() {
  auto pos=player->position;
  float minDist=FLT_MAX,
      xm=mos.x/Render::window_const,
      ym=mos.y/Render::window_const;
  int cursor=0;

  for(int k=0;k<8;k++) {//Get nearest to cursor
    int x=tpi[k]+pos->col,y=tpj[k]+pos->row;

    int dist=abs(xm-x)+abs(ym-y);
    if(dist<minDist) {
      minDist=dist;
      cursor=k;
    }
  }

  auto new_pos=Board_->board_.find({tpj[cursor]+pos->row,tpi[cursor]+pos->col});
  if(new_pos==end(Board_->board_) or new_pos->type==TileType::Wall){return;}

  player->useCharge();
  player->toggleTeleporting();
  if(player->combat) {//Exit combat
    player->toggleCombat();
  }if(player->banishing) {//Exit banishing
    player->toggleBanishing();
  }
  playerTurn(&*new_pos);//Move as your entire turn

  turn=GameState::EnemyTurn;
}

bool Game::isGameOver() const {
  return player->lives<=0 or player->exit;
}

//Conclusion and play again or not
bool Game::gameOver() {
  while(Render_->window.isOpen()) {
    displayEnd();

    Event e;
    if(Render_->window.pollEvent(e)) {
      if(e.type==Event::KeyPressed) {//Restart the game
        if(e.key.code==Keyboard::Y) {
          Board_->newBoard();
          newGame();
          return false;
        }else if(e.key.code==Keyboard::N) {//Close the game
          Render_->window.close();
          return true;
        }
      }
    }
    Render_->window.display();
  }
  return true;
}

//Follow player with camera
void Game::centerCamera() {
  Vector2f camTarget=player->getCamera();//Get player coordinate in view/world units (SFML)
  Vector2f current=Render_->camera.getCenter();

  float smooth=0.5f;//Smooth constant
  current+=(camTarget-current)*smooth*dt;

  Render_->camera.setCenter(current);
  Render_->window.setView(Render_->camera);
}

//DISPLAY ---------------------------------------------
//Invoke all display methods
void Game::Display() {
  Render_->window.clear();
  restartClock();

  Board_->printBoard(*player->position);
  Board_->printValid();
  Board_->printEntities(player,enemies);

  displayBanish();//move these to display status

  mos=Render_->window.mapPixelToCoords(Mouse::getPosition(Render_->window));
  displayTeleport(mos);
  displayStatus();//moves view relative to player
  Render_->displayMessages();

  Render_->window.display();
}

//Print out information messages
void Game::displayStatus() {
  int msgRows=0;

  Render_->window.setView(Render_->window.getDefaultView());

  //PLAYER
  //ITEM ROWS
  int itemRows=0;

  int lives=player->lives;
  for(int i=0;i<lives;i++) {//display lives in a row at the margin
    Sprite sprite(Render::textures["Lives"]);
    sprite.setPosition(1820-110*i,1820);
    sprite.setScale(Render::sprite_const,Render::sprite_const);
    Render_->window.draw(sprite);
  }

  int eggos=player->eggos;
  for(int i=0;i<eggos;i++) {//display lives in a row at the margin
    Sprite sprite(Render::textures["Eggo"]);

    sprite.setPosition(1820-110*i,1720-itemRows*100);
    sprite.setScale(Render::sprite_const,Render::sprite_const);
    Render_->window.draw(sprite);
  }
  if(eggos){itemRows++;}

  int charges=player->tele_charges;
  for(int i=0;i<charges;i++) {
    Sprite sprite(Render::textures["WalkieTalkie"]);

    sprite.setPosition(1820-110*i,1720-itemRows*100);
    sprite.setScale(Render::sprite_const,Render::sprite_const);
    Render_->window.draw(sprite);
  }
  if(charges){itemRows++;}

  int flash=player->battery;
  for(int i=0;i<flash;i++) {
    Sprite sprite(Render::textures["Flash"]);

    sprite.setPosition(1820-110*i,1720-itemRows*100);
    sprite.setScale(Render::sprite_const,Render::sprite_const);
    Render_->window.draw(sprite);
  }

  if(player->surging) {
    Text bantxt,teltxt;
    bantxt.setFont(Render_->pixlet),teltxt.setFont(Render_->pixlet);

    bantxt.setString("B. Banish Demogorgon");
    bantxt.setCharacterSize(80);
    bantxt.setFillColor(Color::White);
    bantxt.setPosition(50,0);
    Render_->window.draw(bantxt);

    teltxt.setString("T. Teleport");
    teltxt.setCharacterSize(80);
    teltxt.setFillColor(Color::White);
    teltxt.setPosition(50,100);
    Render_->window.draw(teltxt);
  }

  //key to select ability, left click to use
  if(player->trapped) {
    Text bantxt,teltxt;
    bantxt.setFont(Render_->pixlet),teltxt.setFont(Render_->pixlet);

    bantxt.setString("Disarm the trap? (1 ability charge)");
    bantxt.setCharacterSize(80);
    bantxt.setFillColor(Color::White);
    bantxt.setPosition(50,300);
    Render_->window.draw(bantxt);

    teltxt.setString("Press: Y/N");
    teltxt.setCharacterSize(80);
    teltxt.setFillColor(Color::White);
    teltxt.setPosition(50,400);
    Render_->window.draw(teltxt);

    msgRows+=2;
  }

  //key to select ability, left click to use
  if(player->teleporting) {
    Text text;
    text.setFont(Render_->pixlet);

    text.setString("Choose a tile to teleport to with your mouse");
    text.setCharacterSize(80);
    text.setFillColor(Color::White);
    text.setPosition(50,200);
    Render_->window.draw(text);

    msgRows+=1;
  }

  //key to select ability, left click to use
  if(player->combat) {
    Text bantxt,teltxt;
    bantxt.setFont(Render_->pixlet),teltxt.setFont(Render_->pixlet);

    bantxt.setString("Banish the Demogorgon (1 ability charge)? (You may teleport)");
    bantxt.setCharacterSize(80);
    bantxt.setFillColor(Color::White);
    bantxt.setPosition(50,300);
    Render_->window.draw(bantxt);

    teltxt.setString("Press: Y/N");
    teltxt.setCharacterSize(80);
    teltxt.setFillColor(Color::White);
    teltxt.setPosition(50,400);
    Render_->window.draw(teltxt);

    msgRows+=2;
  }
}

//Display entities in the game
void Board::printEntities(const Player*p,unordered_map<const Position*,unique_ptr<Demogorgon>>& enemies) {
  const auto pla=p->position;
  Render_->slideSprite("Eleven",p->render_col,p->render_row);

  p->render_row+=(pla->row-p->render_row)/30;
  p->render_col+=(pla->col-p->render_col)/30;

  for(auto&[key,enemy]:enemies){
    const auto pos=enemy->position;
    const int dx=abs(pos->row-pla->row),dy=abs(pos->col-pla->col);

    if(sqrt(dx*dx+dy*dy)<fog_) {//Only show enemies within vision radius
      Render_->slideSprite(enemy->empowered?"Empowered":"Demogorgon",
                     enemy->render_col,enemy->render_row);
    }
    enemy->render_row+=(pos->row-enemy->render_row)/30;
    enemy->render_col+=(pos->col-enemy->render_col)/30;
  }
}

//Display the board tiles
void Board::printBoard(Position pos){
  for(auto&tile:board_){
    const int dx=abs(tile.row-pos.row),dy=abs(tile.col-pos.col);

    if(tile.type!=TileType::Wall and tile.type!=TileType::ClearPath) {
      Render_->fogSprite("ClearPath",tile,sqrt(dx*dx+dy*dy)>=fog_);
    }
    Render_->fogSprite(square_to_str[tile.type],tile,sqrt(dx*dx+dy*dy)>=fog_);
  }
}

//Highlight valid adjacent squares for the player
void Board::printValid(){
  for(auto&pos:valid_moves_){
    if(not pos){continue;}
    Sprite sprite(Render::textures[("ValidPath")]);

    sprite.setPosition(Render::window_const*pos->col,Render::window_const*pos->row);
    sprite.setColor(Color(255,255,255,128));
    Render_->setScale(sprite);
    Render_->window.draw(sprite);
  }
}

void Game::displayEnd() const {
  Render_->defaultView();
  Text txt;
  txt.setFont(Render_->pixlet);

  if(player->exit) {
    txt.setString("Congratulations, Eleven!");
    txt.setFillColor(Color::White);
    txt.setCharacterSize(140);
    txt.setPosition(350,450);
    Render_->window.draw(txt);

    txt.setString("You found the Gate and sealed it!");
    txt.setFillColor(Color::White);
    txt.setCharacterSize(140);
    txt.setPosition(150,550);
    Render_->window.draw(txt);

    txt.setString("Play again? Y/N");
    txt.setFillColor(Color::White);
    txt.setCharacterSize(120);
    txt.setPosition(650,800);
    Render_->window.draw(txt);

  }else {
    txt.setString("Game Over.");
    txt.setFillColor(Color::Red);
    txt.setCharacterSize(200);
    txt.setPosition(580,550);
    Render_->window.draw(txt);

    txt.setString("Play again? Y/N");
    txt.setFillColor(Color::White);
    txt.setCharacterSize(120);
    txt.setPosition(650,800);
    Render_->window.draw(txt);
  }
}
