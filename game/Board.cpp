#include<iostream>
#include<vector>
#include<unordered_set>
#include<algorithm>
#include<utility>
#include<cmath>
#include<random>
#include<climits>
#include<queue>
#include<memory>

#include"Entity.h"
#include"Board.h"

#include<SFML/Graphics.hpp>

using namespace std;
using namespace sf;

bool Board::gate_placed=false;

TileType Board::genType() {
  if(gate_placed) {//Dont spawn Eggos once the gate exists
    return tiles_[no_eggo_dist_(gen_)];
  }
  return tiles_[tile_type_dist_(gen_)];
}

TileType Board::genNoBarrier() {
  return tiles_[no_barrier_dist_(gen_)];
}

Board::Board(Render* render):
  valid_moves_(4),idg_(0),ide_(0),fog_(10.5),initShuf(28)
{
  iota(begin(initShuf),end(initShuf),0);
  Render_=render;
  initialBoard();
}

void Board::newBoard() {
  //Reinitialize
  idg_=0;
  ide_=0;
  root_={};
  walk_positions_={};
  valid_moves_={nullptr,nullptr,nullptr,nullptr};
  board_={};
  gate_placed=false;

  //Create new board
  initialBoard();
}

//Board  ---------------------------------------------------
const Position* Board::insertTile(int i,int j) {
  auto[it,b]=board_.insert({i,j,idg_});
  if(not b){return nullptr;}

  const auto type(genType());
  it->type=type;
  root_.push_back(idg_++);

  if(type!=TileType::Wall and type!=TileType::Trap) {
    walk_positions_.insert(&*it);
  }
  return &*it;
}

//Add a new tile to the set board and type it to prevent walling off paths
void Board::addTile(const int i,const int j,const bool edge) {
  const auto pos=insertTile(i,j);
  if(not pos){return;}

  if(edge) {
    root_[pos->id]=ide_;
  }
  unordered_set<int> prev{};
  bool is=false;

  if(pos->type==TileType::Wall or pos->type==TileType::Trap) {
    for(int k=0;k<8;k++) {
      const auto adj=board_.find({i+di8[k],j+dj8[k]});

      if(adj!=end(board_) and (adj->type==TileType::Wall or adj->type==TileType::Trap)) {
        const int r=find(adj->id);
        if(r==ide_){is=true;}

        if(prev.count(r) or r==root_[pos->id]) {//A tile has the same root as an adjacent or current at the edge
          pos->type=genNoBarrier();
          walk_positions_.insert(pos);
        }
        prev.insert(r);
      }
    }
  }
  if(is) {//Consider rank optimization
    for(auto&r:prev) {
      if(r>0){root_[r]=ide_;}
    }
    root_[pos->id]=ide_;
  }else {
    for(auto&r:prev) {
      if(r>0){root_[r]=pos->id;}
    }
  }
}

//Generate random initial board with 11 at center
void Board::initialBoard() {
  auto[it,b]=board_.insert({0,0,idg_});
  root_.push_back(idg_++);
  it->type=TileType::Start;
  origin_=&*it;
  walk_positions_.insert(&*it);

  shuffle(begin(initShuf),end(initShuf),gen_);
  for(int k=0;k<28;k++) {
    addTile(dicsi[initShuf[k]],djcsi[initShuf[k]],initShuf[k]<16);
  }
}

//Generate more board as you step on edge tiles
void Board::addBoard(const int i,const int j) {
  root_.push_back(idg_);
  ide_=idg_++;

  shuffle(begin(idxc),end(idxc),gen_);
  for(int k=0;k<12;k++) {
    addTile(i+dics[idxc[k]],j+djcs[idxc[k]],k<idxc[k]);
  }
}

void Board::getValidMoves(Position pos) {
  for(int k=0;k<4;k++){//Check adjacent positions
    const auto adj=board_.find({pos.row+di[k],pos.col+dj[k]});

    if(adj==end(board_) or adj->type==TileType::Wall){
      valid_moves_[k]=nullptr;//Out of bounds or wall
    }else {
      valid_moves_[k]=&*adj;
    }
  }
}

void Board::movePlayer(Player* player,const Position* pos) {
  for(int k=0;k<4;k++){//Generate board
    if(!board_.count({pos->row+di[k],pos->col+dj[k]})){
      addBoard(pos->row,pos->col);
    }
  }

  if(pos->type==TileType::WalkieTalkie) {//Process powerup tiles
    if(not player->surging) {
      player->activatePsychicSurge();
      Render_->addMessage("Player Status: Psychic Surge ACTIVE!");
    }
    player->addCharges();

    clearElement(pos);

  }else if(pos->type==TileType::Eggo) {
    player->addEggo();
    Render_->addMessage("An Eggo has been collected!");
    clearElement(pos);

  }else if(pos->type==TileType::Flash) {
    player->incFlashDuration();
    setFog();
    clearElement(pos);

  }else if(pos->type==TileType::Trap) {
    if(player->surging) {//Option to remove trap if empowered
      player->toggleTrapped();

    }else {//Lose life and move to origin
      Render_->addMessage("Eleven has fallen into an Upside Down Rift!");
      Respawn(player);
      return;
    }
  }else if(pos->type==TileType::Gate) {
    player->setFoundExit();
  }
  player->setPosition(pos);

  player->decFlashDuration();
  if(not player->battery) {
    resetFog();
  }
}

//Move a Demogorgon onto a new position
void Board::moveDemogorgon(Demogorgon* enemy,const Position* pos,unordered_map<const Position*,unique_ptr<Demogorgon>>& enemies) {
  uniform_int_distribution<> idx(1,10);
  if(not enemy->rift_created and enemy->empowered and idx(gen_)>8) {//20% chance to use turn to summon rift
    if(idx(gen_)>7) {//30% chance of success
      placeTrap(enemy);
    }
    enemy->useRiftAttempt();
    Render_->addMessage("A Demogorgon is channeling a Rift!");

  }else if(pos->type==TileType::MindFlayerShard) {
    enemy->activateEmpowerment();
    clearElement(pos);
    Render_->addMessage("A Demogorgon is EMPOWERED!");
  }
  walk_positions_.insert(enemy->position);

  auto node=enemies.extract(enemy->position);
  enemy->setPosition(pos);
  node.key()=pos;
  enemies.insert(std::move(node));
}

//Randomized movement for Demogorgon
const Position* Board::randomAdvance(Demogorgon* enemy,unordered_map<const Position*,unique_ptr<Demogorgon>>& enemies) {
  const auto pos=enemy->position;
  vector<unordered_set<Position>::iterator> positions{};

  for(int k=0;k<4;k++){
    auto adj=board_.find({pos->row+di[k],pos->col+dj[k]});

    if(adj!=end(board_) and adj->type!=TileType::Wall and not enemies.count(&*adj)){
      positions.push_back(adj);//Save placement positions
    }
  }
  uniform_int_distribution<> idx(0,size(positions)-1);

  if(positions.size()) {
    return &*positions[idx(gen_)];
  }else {
    return nullptr;
  }
}

//Respawn player at origin
void Board::Respawn(Player* player) {
  player->loseLife();
  walk_positions_.insert(player->position);
  player->setPosition(origin_);
}

//Spawn n enemies a safe distance from the player
void Board::spawnEnemies(int n,unordered_map<const Position*,unique_ptr<Demogorgon>>& enemies,Player*p) {
  vector<const Position*> spawnPositions;
  auto pla=p->position;

  const int range=size(board_)/8;
  for(auto&pos:walk_positions_) {//Remove locations too close to player and those that cannot reach player
    if((abs(pla->row-pos->row)+abs(pla->col-pos->col))<range or not pos->pred) {
      continue;
    }
    spawnPositions.push_back(pos);
  }
  shuffle(begin(spawnPositions),end(spawnPositions),gen_);

  n=min(n,(int)size(spawnPositions));//Reduce n if not enough valid spawn positions
  for(int i=0;i<n;i++) {
    auto pos=spawnPositions[i];
    enemies[pos]=make_unique<Demogorgon>(Demogorgon(pos));
    walk_positions_.erase(pos);
  }
}

//Place trap at valid adjacent position
void Board::placeTrap(Demogorgon* enemy) {
  const auto pos=enemy->position;
  vector<unordered_set<Position>::iterator> positions={};

  for(int k=0;k<4;k++){
    auto adj=board_.find({pos->row+di[k],pos->col+dj[k]});

    if(adj!=end(board_) and (adj->type==TileType::ClearPath or adj->type==TileType::Start)){
      positions.push_back(adj);//Save placement positions
    }
  }
  if(not positions.empty()) {
    enemy->setRiftCreated();
    Render_->addMessage("A Demogorgon generated a new Upside Down Rift!");
    uniform_int_distribution<> idx(0,size(positions)-1);
    positions[idx(gen_)]->type=TileType::Trap;//Set random adj position to trap
  }
}

//Place the gate at a random open position
void Board::placeGate() {
  uniform_int_distribution<> idx(0,size(walk_positions_)-1);
  int r=idx(gen_);

  for(const auto pos:walk_positions_) {
    r--;if(!r) {//place gate at the position of the index
      board_.find(*pos)->type=TileType::Gate;
      gate_placed=true;
      return;
    }
  }
}

//Remove an item on the tile
void Board::clearElement(const Position* pos) {
  pos->type=TileType::ClearPath;
}

//Enable the priority queue to re-order Position objects with a comparator
struct Comparator {
  bool operator()(const Position* pos,const Position* oth) const {
    return pos->dist<oth->dist;
  }
};

//Compute shortest paths for enemies using single source from the player's position
void Board::Dijkstra(const Position* source) {
  priority_queue<const Position*,vector<const Position*>,Comparator> pq;
  pq.push(source);

  for(auto&pos:walk_positions_) {//Initialize tile distances and predecessors
    pos->dist=INT_MAX;
    pos->pred=nullptr;
  }
  source->dist=0;

  while(not pq.empty()) {//Continue until no relaxations can be performed
    const auto pos=pq.top();
    pq.pop();

    for(int k=0;k<4;k++){//Find adjacent tiles in the set board
      const auto adj=board_.find({pos->row+di[k],pos->col+dj[k]});

      if(walk_positions_.count(&*adj) and adj->dist>pos->dist+1) {//Relax if in set and has larger distance
        adj->dist=pos->dist+1;
        adj->pred=pos;

        pq.push(&*adj);
      }
    }
  }
}
