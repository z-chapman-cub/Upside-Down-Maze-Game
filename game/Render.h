#ifndef RENDER_H
#define RENDER_H

#include <array>
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
  void lightingDisplay(Position pos);
  void Multiply();
  void addLight(float x,float y);
  void addPlayerLight(float x,float y);
  void clearLights();
  void restartClock(){dt=clock.restart().asSeconds();}

  //FIELDS
  RenderWindow window;
  RectangleShape player;
  View camera;

  vector<string> messages;//Circular queue of game messages
  int mi;//Message index

  static constexpr int window_const=120;
	static constexpr float sprite_const=0.22f;
  static constexpr int scene_const=960;
  static unordered_map<string,Texture> textures;

  Font pixlet;

  RenderTexture sceneTexture,lightTexture,allTexture;
  Shader lightShader;
  RectangleShape quad;
  Sprite scene,light,all;

  float lightSmooth;
  Glsl::Vec2 lightPos,lightsRender;

  View sceneCam;

  array<Glsl::Vec2,100> lights,lightsShifted;
  int numLights;

  //Camera smooth constant
  float smooth;
  float dt;
  Clock clock;

  Position pla;

private:

  vector<string> entities_={//Asset names
    "Wall", "ClearPath", "ValidPath", "Eleven", "Demogorgon", "Eggo", "WalkieTalkie",
    "MindFlayerShard", "Trap", "Gate",
    "Lives", "Banish", "Teleport", "Start", "Torch", "Empowered"
  };
};

#endif //RENDER_H