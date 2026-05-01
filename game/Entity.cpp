#include<iostream>

#include "Entity.h"
#include "Game.h"

using namespace std;

//ENTITY
Entity::Entity(const Position* pos):
  position(pos),render_row(pos->row),render_col(pos->col)
{}

//CONSTRUCTORS
Player::Player(const Position* pos):
  Entity(pos),lives(3),eggos(0),tele_charges(0),battery(0),
  teleporting(false),banishing(false),combat(false),trapped(false),
  surging(false),stalemate(false),exit(false)
{}

Demogorgon::Demogorgon(const Position* pos):
  Entity(pos),empowered(false),stalemate(false),
  rift_created(false),rift_attempts_(0)
{}

//METHODS
//PLAYER
void Player::addEggo() {
  eggos++;
}

void Player::addCharges() {
  tele_charges+=2;
}

void Player::useCharge() {
  tele_charges--;
  if(not tele_charges) {
    surging=false;
  }
}

void Player::loseLife() {
  if(lives) {
    lives--;
  }
}

//Deactivate surge before having used all charges
void Player::deactivatePsychicSurge() {
  surging=false;
  tele_charges=0;
}

void Player::incFlashDuration() {
  battery+=3;
}

void Player::decFlashDuration() {
  if(not battery){return;}
  battery--;
}

//DEMOGORGON
void Demogorgon::activateEmpowerment() {
  empowered=true;
  rift_attempts_=3;
}

void Demogorgon::useRiftAttempt() {
  rift_attempts_--;
  if(not rift_attempts_) {
    deactivateEmpowerment();
  }
}

void Demogorgon::deactivateEmpowerment() {
  rift_attempts_=0;
  empowered=false;
}

Vector2f Entity::getCamera() const {
  return {static_cast<float>(position->col*Render::window_const),
    static_cast<float>(position->row*Render::window_const)};
}
