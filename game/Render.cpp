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
  messages(3),mi(0),
  quad(Vector2f(1920,1920)),
  lightPos(scene_const,scene_const),
  lightsRender(scene_const,scene_const),
  numLights(1),smooth(0.5f),dt(0)
{
  window.setFramerateLimit(240);
  camera.setCenter(scene_const,scene_const);//Center camera on player

  for(auto&ent:entities_) {//Load assets into map
    textures[ent].loadFromFile("emoji/"+ent+".png");
  }
  pixlet.loadFromFile("fonts/Pixellettersfull-BnJ5.ttf");//Load font

  lightTexture.create(1920,1920);
  lightShader.loadFromFile("light.frag",Shader::Fragment);
  lightShader.setUniform("radius",500.0f);
  light.setTexture(lightTexture.getTexture());

  sceneTexture.create(1920,1920);
  scene.setTexture(sceneTexture.getTexture());

  scene.setPosition(0,1920);
  scene.setScale(1,-1);
  light.setPosition(0,1920);
  light.setScale(1,-1);

  quad.setFillColor(Color::White);
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

  sceneTexture.draw(sprite);
}

void Render::drawSprite(string str,Position pos) {
  Sprite sprite(textures[str]);
  setPosition(sprite,pos);
  setScale(sprite);

  sceneTexture.draw(sprite);
}

void Render::slideSprite(string str,float x,float y) {
  Sprite sprite(textures[str]);
  sprite.setPosition(window_const*x+scene_const,window_const*y+scene_const);
  setScale(sprite);

  sceneTexture.draw(sprite);
}

void Render::setPosition(Sprite& sprite,Position pos) {
  sprite.setPosition(window_const*pos.col+scene_const,window_const*pos.row+scene_const);
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
    auto message=messages[mic];
    mic=(mic+1)%3;
    if(message!="") {
      drawMessage(txt,message,x,y);
      y+=80;//
    }
  }
}

//Add message to circular queue
void Render::addMessage(string message) {
  messages[mi]=message;
  mi=(mi+1)%3;
}

void Render::lightingDisplay(Position pos) {
  sceneTexture.setView(sceneCam);
  lightTexture.setView(sceneCam);

  for(int i=1;i<numLights;i++) {
    lightsShifted[i]+=(Glsl::Vec2(lights[i].x-pos.col*window_const,
      lights[i].y-pos.row*window_const)-lightsShifted[i])*smooth*dt;
  }

  lightShader.setUniformArray(
    "lightPos",
    lightsShifted.data(),
    numLights
  );
  lightShader.setUniform(
    "lightCount",
    numLights
  );

  lightTexture.clear();
  lightTexture.draw(quad,&lightShader);
  lightTexture.display();
}

void Render::Multiply() {
  window.draw(scene);

  window.draw(light,BlendMultiply);

  displayMessages();

  sceneTexture.clear();
}

void Render::addLight(float x,float y) {
  lights[numLights]=Glsl::Vec2(x*window_const+scene_const,y*window_const+scene_const);
  lightsShifted[numLights++]=Glsl::Vec2(x*window_const+scene_const-pla.col*window_const,y*window_const+scene_const-pla.row*window_const);
}

void Render::addPlayerLight(float x,float y){
  lights[0]=Glsl::Vec2(x*window_const+scene_const,y*window_const+scene_const);
  lightsShifted[0]=Glsl::Vec2(x*window_const+scene_const,y*window_const+scene_const);
}

void Render::clearLights() {
  numLights=1;
}
