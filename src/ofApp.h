#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "Particle.h"
#include "box.h"
#include "Octree.h"

class ofApp : public ofBaseApp {

public:
   void setup();
   void update();
   void draw();
   void checkCollision();
   void checkLanding();

   void keyPressed(int key);
   void keyReleased(int key);
   void mouseMoved(int x, int y);
   void mouseDragged(int x, int y, int button);
   void mousePressed(int x, int y, int button);
   void mouseReleased(int x, int y, int button);
   void mouseEntered(int x, int y);
   void mouseExited(int x, int y);
   void windowResized(int w, int h);
   void dragEvent(ofDragInfo dragInfo);
   void gotMessage(ofMessage msg);
   void initLightingAndMaterials();
   void loadVbo();


   // Particle System
   ParticleSystem *sys;
   GravityForce *grav;
   MovementForce *moveForce;
   Particle ship;

   ParticleEmitter thrusterEmitter;
   ImpulseRadialForce *radialForce;
   CyclicForce *cyclicForce;

   // textures
   //
   ofTexture  particleTex;

   // shaders
   //
   ofVbo vbo;
   ofShader shader;

   // Models
   ofxAssimpModelLoader tractor, cornField;
   ofMesh cornMesh;
   Box shipBox, fieldBox;

   // Landing Areas
   vector<Box> landings;

   // Octree
   Octree oct;
   int numLevels;
   vector<ofColor> colors;
   bool bShowOct;

   // Cameras
   ofEasyCam mainCam;
   ofCamera landingCam, trackingCam, fixedCam;
   ofCamera *theCam;

   // Sounds
   ofSoundPlayer countryRoads, thrusters;
   bool bThrust;

   ofVec3f currentPos, box1Pos, box2Pos, oldPos, newPos;
   float box1Width, box1Height, box2Width, box2Height;

   // GUI
   bool bHide;
   ofxFloatSlider camDistance;
   ofxFloatSlider gravity;
   ofxFloatSlider move;
   ofxFloatSlider radius;
   ofxPanel gui;
};
