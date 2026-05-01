#include<SFML/Graphics.hpp>

#include "Render.h"

unordered_map<string,Texture> Render::textures{};

//CONSTRUCT
Render::Render():
  window(
    VideoMode(1920,1920),
    "Upside Down Maze",
    Style::Titlebar|Style::Close
  ),
  player({50.f,50.f}),
  camera(FloatRect(0,0,1920,1920)),
  messages(3),mi(0)
{
  window.setFramerateLimit(240);
  camera.setCenter(1920/window_const/2,1920/window_const/2);//Center camera on player

  for(auto&ent:entities_) {//Load assets into map
    textures[ent].loadFromFile("emoji/"+ent+".png");
  }
  pixlet.loadFromFile("fonts/Pixellettersfull-BnJ5.ttf");//Load font
}

//MEMBERS
void Render::defaultView() {
  window.setView(window.getDefaultView());
}

void Render::fogSprite(string str,Position pos,bool fog) {
  Sprite sprite(textures[str]);
  setPosition(sprite,pos);
  setScale(sprite);
  if(fog){sprite.setColor({100,100,100});}
  window.draw(sprite);
}

void Render::drawSprite(string str,Position pos) {
  Sprite sprite(textures[str]);
  setPosition(sprite,pos);
  setScale(sprite);
  window.draw(sprite);
}

void Render::slideSprite(string str,float x,float y) {
  Sprite sprite(textures[str]);
  sprite.setPosition(window_const*x,window_const*y);
  setScale(sprite);
  window.draw(sprite);
}

void Render::setPosition(Sprite& sprite,Position pos) {
  sprite.setPosition(window_const*pos.col,window_const*pos.row);
}

void Render::setScale(Sprite& sprite) {
  sprite.setScale(sprite_const,sprite_const);
}

void Render::drawMessage(Text& txt,string message,int x,int y) {
  txt.setString(message);
  txt.setCharacterSize(70);
  txt.setFillColor(Color::White);
  txt.setPosition(x,y);
  window.draw(txt);
}

//Display messages from circular queue
void Render::displayMessages() {
  defaultView();
  Text txt;
  txt.setFont(pixlet);

  int x=50,y=1650,mic=mi;
  for(int i=0;i<3;i++) {
    auto message=messages[mic++%3];//fix
    if(message==""){continue;}
    drawMessage(txt,message,x,y);
    y+=80;//
  }
}

//Add message to circular queue
void Render::addMessage(string message) {
  messages[mi++]=message;
  if(mi==(int)size(messages)){mi=0;}
}
