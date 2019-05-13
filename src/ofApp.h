#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"
#include "ParticleSystem.h"
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


   // Particle System
   ParticleSystem *sys;
   GravityForce *grav;
   MovementForce *moveForce;
   Particle ship;

   // Models
   ofxAssimpModelLoader tractor, cornField;
   ofMesh cornMesh;
   Box shipBox, fieldBox;

   // Landing Areas
   vector<Box> landing;

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

   // Lighting
   ofLight sunLight;
   ofLight keyLight, fillLight, rimLight;
   ofMaterial planeMaterial;


   ofVec3f currentPos, box1Pos, box2Pos, oldPos, newPos;
   float box1Width, box1Height, box2Width, box2Height;

   // GUI
   bool bHide;
   ofxFloatSlider camDistance;
   ofxFloatSlider gravity;
   ofxFloatSlider move;
   ofxPanel gui;
};
