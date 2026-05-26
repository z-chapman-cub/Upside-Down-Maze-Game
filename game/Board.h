#ifndef BOARD_H
#define BOARD_H

#include<random>
#include<memory>
#include<unordered_set>

#include "Render.h"
#include "Entity.h"

using namespace std;

//Occupancy of each tile of the board
enum class TileType {
	Wall, ClearPath, Eggo, WalkieTalkie,
	MindFlayerShard, Trap, Gate, Torch, Start
};

class Board {
public:
	Board(Render* render);

	void newBoard();

	void Respawn(Player* player);
	void spawnEnemies(int n,unordered_map<const Position*,unique_ptr<Demogorgon>>& enemies,Player* player);

	//Find the tiles the player can move to for display
	void getValidMoves(Position pos);
	void movePlayer(Player* player,const Position* pos);
	void moveDemogorgon(Demogorgon* enemy,const Position* pos,unordered_map<const Position*,unique_ptr<Demogorgon>>& enemies);
	const Position* randomAdvance(Demogorgon*enemy,unordered_map<const Position*, unique_ptr<Demogorgon>>&enemies);

	void Dijkstra(const Position* source);

	// You probably want to implement these
	void placeGate();
	void placeTrap(Demogorgon* enemy);
	void clearElement(const Position* pos);
	void setFog(){fog_=15.5;}
	void resetFog(){fog_=10.5;}

	//Display methods
	void printBoard(Position pos);
	void printEntities(const Player*p,unordered_map<const Position*,unique_ptr<Demogorgon>>& enemies);
	void printValid();

	void initialBoard();
	void addBoard(int i,int j);
	void addTile(int i,int j,bool edge);
	const Position* insertTile(int i,int j);

	unordered_set<Position> board_;
	unordered_set<const Position*> walk_positions_;

	vector<const Position*> valid_moves_;//Where the player may move
	const Position* origin_;
	Render* Render_;

	static bool gate_placed;

private:
	//Random generation
	inline static thread_local std::mt19937 gen_{std::random_device{}()};
	uniform_int_distribution<> reveal_enemy_chance_;

	inline static vector<TileType> tiles_{TileType::Wall,TileType::ClearPath,TileType::Eggo,
		TileType::WalkieTalkie,TileType::MindFlayerShard,TileType::Trap,TileType::Torch};

	//Default
	inline static discrete_distribution<> tile_type_dist_ {0.752,0.012,0.004,0.012,0.012,0.008,0.020};
	//No barriers
	inline static discrete_distribution<> no_barrier_dist_{0.000,0.944,0.004,0.012,0.012,0.008,0.020};
	//No Eggos
	inline static discrete_distribution<> no_eggo_dist_   {0.756,0.012,0.000,0.012,0.012,0.008,0.020};

	static TileType genType();
	static TileType genNoBarrier();

	vector<int> root_;
	int idg_;//Position id
	int ide_;//Edge id

	float fog_;//Fog of war current radius

	//Convert tile types to strings
	unordered_map<TileType,string> square_to_str{
		  {TileType::Wall,"Wall"},
			{TileType::ClearPath,"ClearPath"},
			{TileType::Eggo,"Eggo"},
			{TileType::WalkieTalkie,"WalkieTalkie"},
			{TileType::MindFlayerShard,"MindFlayerShard"},
			{TileType::Trap,"Trap"},
			{TileType::Gate,"Gate"},
			{TileType::Torch,"Torch"},
			{TileType::Start,"Start"},
		};

	//4-directional checks
	static constexpr int di[4]{-1, 1, 0, 0},
											 dj[4]{0, 0,-1, 1};
	//8-directional checks
	static constexpr int di8[8]{0, 0,1,1, 1,-1,-1,-1},
											 dj8[8]{1,-1,0,1,-1, 0, 1,-1};

	//Reveal circle with edge tiles placed at front
	static constexpr int dics[12]{-2,-1,-1,0,0,1,1,2,-1,0,0,1},
											 djcs[12]{0,-1,1,-2,2,-1,1,0,0,-1,1,0};

	//Initial board with edge tiles placed at front
	static constexpr int dicsi[28]{2,2,-2,-2,-3,-2,-2,-1,-1,0,0,1,1,2,2,3,-2,-1,-1,0,0,1,1,2,-1,0,0,1},
	                     djcsi[28]{2,-2,2,-2,0,-1,1,-2,2,-3,3,-2,2,-1,1,0,0,-1,1,-2,2,-1,1,0,0,-1,1,0};

	//To shuffle reveal indices
	int idxc[12]{0,1,2,3,4,5,6,7,8,9,10,11};
	vector<int> initShuf;

	//CONSTANT METHODS
	//Map roots together
	int find(const int x) {
		if(x<0){return x;}
		if(root_[x]!=x) {
			root_[x]=find(root_[x]);
		}
		return root_[x];
	}
};



#endif //BOARD_H
