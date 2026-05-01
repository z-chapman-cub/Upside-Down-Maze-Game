#ifndef RENDER_H
#define RENDER_H

#include<vector>
#include<string>
#include<unordered_map>

#include "Entity.h"

#include<SFML/Graphics.hpp>

using namespace std;
using namespace sf;

//For re-used objects when rendering
class Render {
public:
  explicit Render();

  //METHODS
  void defaultView();

  //Output methods
  void fogSprite(string str,Position pos,bool fog);
  void drawSprite(string str,Position pos);
  void slideSprite(string str,float x,float y);
  void setPosition(Sprite& sprite,Position pos);
  void setScale(Sprite& sprite);
  void drawMessage(Text& txt,string message,int x,int y);
  void displayMessages();//Show messages
  void addMessage(string message);

  //FIELDS
  RenderWindow window;
  RectangleShape player;
  View camera;

  vector<string> messages;//Circular queue of game messages
  int mi;//Message index

  static constexpr int window_const=120;
	static constexpr float sprite_const=0.22f;
  static unordered_map<string,Texture> textures;

  Font pixlet;

private:

  vector<string> entities_={//Asset names
    "Wall", "ClearPath", "ValidPath", "Eleven", "Demogorgon", "Eggo", "WalkieTalkie",
    "MindFlayerShard", "Trap", "Gate",
    "Lives", "Banish", "Teleport", "Start", "Flash", "Empowered"
  };
};

#endif //RENDER_H