#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
void ofApp::setup() {
   ofSetBackgroundColor(ofColor::black);

   // Particle Physics Setup
   sys = new ParticleSystem();
   grav = new GravityForce();
   moveForce = new MovementForce();

   currentPos = glm::vec3(0, 100, 0);
   oldPos = currentPos;

   ship.position = currentPos;
   ship.lifespan = 60 * 60 * 1000;
   ship.velocity = ofVec3f(0, 0);
   ship.radius = 1;
   ship.damping = 0.9995;

   sys->add(ship);
   sys->setLifespan(10000000);
   sys->addForce(grav);
   sys->addForce(moveForce);

   // Models
   string modelPath = "Tractor/Tractor.obj";
   if (tractor.loadModel(modelPath)) {
      tractor.setScaleNormalization(false);
      ofVec3f min = tractor.getSceneMin() + tractor.getPosition();
      ofVec3f max = tractor.getSceneMax() + tractor.getPosition();

      // Create Bounding Box
   }
   else {
      ofLogFatalError("Can't load model: " + modelPath);
      ofExit();
   }

   modelPath = "cornMoon/cornMoon.obj";
   //modelPath = "geo/mars-low.obj";
   if (cornField.loadModel(modelPath)) {
      cornField.setScaleNormalization(false);
      cornField.setPosition(-100, 32, 80);
      cornField.setScale(2, 1, 2);
      cornMesh = cornField.getMesh(0);
   }
   else {
      ofLogFatalError("Can't load model: " + modelPath);
      ofExit();
   }

   // Landing Fields


   // Octree


   // Camera
   mainCam.setDistance(100);
   mainCam.setNearClip(10);
   mainCam.setFov(65.5);
   ofEnableSmoothing();
   ofEnableDepthTest();

   theCam = &mainCam;

   trackingCam.setGlobalPosition(glm::vec3(100, 25, 0));
   trackingCam.lookAt(glm::vec3(0, 0, 0));

   landingCam.setGlobalPosition(currentPos);
   landingCam.lookAt(glm::vec3(0, 0, 0));

   fixedCam.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y + 50, currentPos.z + 50));
   fixedCam.lookAt(currentPos);

   // Sound
   // Take Me Home, Country Roads
   // Album: Fallout 76 (Original Trailer Soundtrack) by Copilot Music + Sound
   string soundPath = "sounds/CountryRoads.mp3";
   if (countryRoads.load(soundPath)) {
      countryRoads.setLoop(true);
      countryRoads.setVolume(0.8f);
      countryRoads.play();
   }
   else {
      ofLogFatalError("Can't load sound: " + soundPath);
      ofExit();
   }

   soundPath = "sounds/sfx_vehicle_engineloop.wav";
   if (thrusters.load(soundPath)) {
      thrusters.setMultiPlay(false);
      thrusters.setVolume(0.2f);
      bThrust = false;
   }
   else {
      ofLogFatalError("Can't load sound: " + soundPath);
      ofExit();
   }

   // GUI
   gui.setup();
   gui.add(move.setup("Move Force", 10, 1, 100));
   gui.add(gravity.setup("Gravity", 3, -20, 40));
   bHide = false;
   //	gui.add(camDistance.setup("Camera Distance", 20, 10, 50));
}

//--------------------------------------------------------------
void ofApp::update() {
   
   grav->set(ofVec3f(0, -gravity, 0));

   // Play Thrusters
   if (bThrust && !thrusters.isPlaying()) {
      thrusters.play();
   }

   sys->update();
   currentPos = sys->particles[0].position;

   // Set tractor's position to the particles position
   tractor.setPosition(currentPos.x, currentPos.y, currentPos.z);

   // Set fixedCam as a trailing cam
   fixedCam.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y + 50, currentPos.z + 50));
   fixedCam.lookAt(currentPos);

   // Set trackingCam to follow the tractor from a fixed position
   trackingCam.lookAt(currentPos);

   // Set landingCam to tractor's position looking down
   landingCam.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y, currentPos.z));
   landingCam.lookAt(glm::vec3(currentPos.x, currentPos.y - 50, currentPos.z));

   checkCollision();
   checkLanding();
}

//--------------------------------------------------------------
void ofApp::draw() {
   ofBackground(ofColor::black);
   
   ofEnableDepthTest();
   theCam->begin();

   // Draw Cams
   ofSetColor(ofColor::dimGrey);
   mainCam.draw();
   trackingCam.draw();
   //landingCam.draw();
   fixedCam.draw();

   // Draw Models
   ofPushMatrix();
   //ofEnableLighting(); // Add lighting first XD
   ofSetColor(ofColor::white);
   tractor.drawFaces();
   cornField.drawFaces();
   
   ofPopMatrix();

   // Draw Bounding Boxes

   // draw a grid
   //
   ofPushMatrix();
   ofRotate(90, 0, 0, 1);
   ofSetLineWidth(1);
   ofSetColor(ofColor::dimGrey);
   ofDrawGridPlane(10.0f, 16, false);
   ofPopMatrix();

   // Draw Particle
   sys->draw();

   theCam->end();

   // draw the GUI
   ofDisableDepthTest();
   if (!bHide) gui.draw();
}

// Use ship bounding box
void ofApp::checkCollision() {
   newPos = sys->particles[0].position;

   if (newPos.y <= 0) {
      newPos = oldPos;
      sys->particles[0].position = newPos;
      sys->particles[0].velocity.y = 0;
   }

   oldPos = newPos;
}

// Check ship's position with landing areas
void ofApp::checkLanding() {

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
   switch (key) {
   case OF_KEY_UP:
      moveForce->set(ofVec3f(0, 0, -move));
      if (!bThrust) bThrust = true;
      break;
   case OF_KEY_DOWN:
      moveForce->set(ofVec3f(0, 0, move));
      if (!bThrust) bThrust = true;
      break;
   case OF_KEY_LEFT:
      moveForce->set(ofVec3f(-move, 0));
      if (!bThrust) bThrust = true;
      break;
   case OF_KEY_RIGHT:
      moveForce->set(ofVec3f(move, 0));
      if (!bThrust) bThrust = true;
      break;
   case ' ':
      moveForce->set(ofVec3f(0, move, 0));
      if (!bThrust) bThrust = true;
      break;
   case 'C':
   case 'c':
      if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
      else mainCam.enableMouseInput();
      break;
   case 'F':
   case 'b':
      break;
   case 'f':
      ofToggleFullscreen();
      break;
   case 'h':
      bHide = !bHide;
      break;
   case OF_KEY_F1:
      theCam = &mainCam;
      break;
   case OF_KEY_F2:
      theCam = &fixedCam;
      break;
   case OF_KEY_F3:
      theCam = &landingCam;
      break;
   case OF_KEY_F4:
      theCam = &trackingCam;
      break;
   default:
      break;
   }

}


//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
   switch (key) {
   case OF_KEY_UP:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(sys->particles[0].velocity.x, sys->particles[0].velocity.y, 0);
      bThrust = false;
      thrusters.stop();
      break;
   case OF_KEY_DOWN:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(sys->particles[0].velocity.x, sys->particles[0].velocity.y, 0);
      bThrust = false;
      thrusters.stop();
      break;
   case OF_KEY_LEFT:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(0, sys->particles[0].velocity.y, sys->particles[0].velocity.z);
      bThrust = false;
      thrusters.stop();
      break;
   case OF_KEY_RIGHT:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(0, sys->particles[0].velocity.y, sys->particles[0].velocity.z);
      bThrust = false;
      thrusters.stop();
      break;
   case ' ':
      moveForce->set(ofVec3f(0, 0, 0));
      bThrust = false;
      thrusters.stop();
      break;
   default:
      break;
   }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
