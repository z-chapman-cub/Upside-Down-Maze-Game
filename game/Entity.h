#ifndef ENTITY_H_
#define ENTITY_H_

#include<SFML/Graphics.hpp>

#include<string>

enum class TileType;

using namespace std;
using namespace sf;

//Store information on board positions
struct Position {
  int row;
  int col;
  int id;

  mutable TileType type;

  mutable int dist;
  mutable const Position* pred;

  bool operator==(const Position& other) const {
      return row == other.row and col == other.col;
  }
};

template<>
struct std::hash<Position> {
  size_t operator()(const Position& p) const noexcept {
    return hash<int>()(p.row)^(hash<int>()(p.col)<<1);
  }
};

class Entity {
public:
  explicit Entity(const Position* pos);
  virtual ~Entity() = default;

  [[nodiscard]] virtual string getName() const {return "Entity";}

  Vector2f getCamera() const;

  void setPosition(const Position* pos) {position=pos;}

  const Position* position;
  mutable float render_row;
  mutable float render_col;

private:
};

class Player: public Entity {
public:
  explicit Player(const Position* pos);
  [[nodiscard]] string getName() const override {return "Player";}

  //METHODS
  void toggleCombat() {combat=!combat;}
  void toggleBanishing() {banishing=!banishing;}
  void toggleTrapped() {trapped=!trapped;}
  void incFlashDuration();
  void decFlashDuration();
  void setFoundExit(){exit=true;}

  //Spells
  void toggleTeleporting(){teleporting=!teleporting;}

  void addEggo();
  void addCharges();
  void useCharge();

  void toggleStalemate(){stalemate=!stalemate;}
  void activatePsychicSurge(){surging=true;}
  void deactivatePsychicSurge();
  void loseLife();

  //FIELDS
  int lives;
  int eggos;
  int tele_charges;
  int battery;

  bool teleporting;
  bool banishing;
  bool combat;
  bool trapped;
  bool surging;
  bool stalemate;
  bool exit;

private:
};

class Demogorgon: public Entity {
public:
  explicit Demogorgon(const Position* pos);
  [[nodiscard]] string getName() const override {return "Demogorgon";}

  //METHODS
  void activateEmpowerment();
  void deactivateEmpowerment();
  void useRiftAttempt();
  void setRiftCreated(){rift_created=true;}
  void toggleStalemate(){stalemate=!stalemate;}

  //FIELDS
  bool empowered;
  bool stalemate;
  bool rift_created;

private:
  int rift_attempts_;
};

#endif // ENTITY_H_
