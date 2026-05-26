#ifndef GAME_H_
#define GAME_H_

#include <memory>
#include<SFML/Graphics.hpp>

#include<random>
#include<vector>
#include<unordered_set>
#include<unordered_map>

#include "Board.h"

enum class GameState {
	PlayerTurn,EnemyTurn
};

class Game {
public:
	Game(Board*board,Render*render);

	//METHODS
	//Game creation
	void newGame();
	bool isGameOver() const;
	bool gameOver();

	//Input
	void playerInput(const Event&e);

	//Turns
	void playerTurn(const Position* pos);
	void enemyTurns();
	void demogorgonTurn(Demogorgon* enemy);

	//Interaction logic
	void Combat(Demogorgon* enemy);
	bool Stalemate(Demogorgon* enemy);
	void Trapped(const Event& e);
	void inCombat(const Event& e);
	void castSpell(const Event& e);

	//Spells
	void Banish(const Position* pos);
	void Teleport();

	//Display methods
	void Display();//Show all
	void displayStatus();
	void displayBanish() const;
	void displayTeleport(Vector2f mos) const;
	void displayEnd() const;

	void centerCamera();

	//FIELDS
	//Entities that take turns
	Player* player;

	//Enemies and their iteration
	unordered_map<const Position*,unique_ptr<Demogorgon>> enemies;
	vector<const Position*>::iterator ite;
	vector<const Position*> keys;
	bool iterating=false;

	//Outer logic
	Vector2<float> mos;//Mouse position
	GameState turn;

	//Helpers
	const unordered_map<const Keyboard::Key,int> inputToPos=
	{{Keyboard::W,0},{Keyboard::S,1},{Keyboard::A,2},{Keyboard::D,3},
		{Keyboard::Up,0},{Keyboard::Down,1},{Keyboard::Left,2},{Keyboard::Right,3}};

	//Map for teleport indicators
	static constexpr int tpi[8]{0,0,-1,-1,1,1,-2,2},
											 tpj[8]{-2,2,-1,1,-1,1,0,0};

private:
	Render* Render_;
	Board* Board_;
};

#endif // GAME_H_
