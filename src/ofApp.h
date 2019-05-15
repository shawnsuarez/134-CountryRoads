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
   void checkAltitude();

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
   TurbulenceForce *turb;
   Particle ship;

   ParticleEmitter thrusterEmitter;
   ImpulseRadialForce *radialForce;
   CyclicForce *cyclicForce;

   ofTexture  particleTex;
   ofVbo vbo;
   ofShader shader;

   // Models
   ofxAssimpModelLoader tractor, cornField, corn;
   vector<ofxAssimpModelLoader> corns;
   ofMesh cornMesh;
   Box shipBox;
   float altitude;
   ofVec3f currentPos;
   bool bWireframe;
   bool bBoundingBox;

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
   bool bShowCams;

   // Sounds
   ofSoundPlayer countryRoads, thrusters;
   bool bThrust;

   // Lighting
   ofLight sunLight;

   // GUI
   bool bHide;
   ofxFloatSlider camDistance;
   ofxFloatSlider gravity;
   ofxFloatSlider move;
   ofxFloatSlider radius;
   ofxPanel gui;

   // Test
   bool bPointSelected;
   bool bShowPoint;
   ofVec3f selectedPoint;
};
