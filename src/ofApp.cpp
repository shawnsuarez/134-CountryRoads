#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
   ofSetBackgroundColor(ofColor::black);

   // Particle Physics Setup
   sys = new ParticleSystem();
   grav = new GravityForce();
   moveForce = new MovementForce();

   currentPos = glm::vec3(0, 50, 0);
   oldPos = currentPos;

   ship.position = currentPos;
   ship.lifespan = 60 * 60 * 1000;
   ship.velocity = ofVec3f(0, 0);
   ship.radius = 10;

   sys->add(ship);
   sys->setLifespan(10000000);
   sys->addForce(grav);
   sys->addForce(moveForce);

   // Models


   // Landing Fields


   // Octree


   // Camera
   mainCam.setDistance(30);
   mainCam.setNearClip(10);
   mainCam.setFov(65.5);
   ofEnableSmoothing();
   ofEnableDepthTest();

   theCam = &mainCam;

   cam1.setGlobalPosition(glm::vec3(100, 25, 0));
   cam1.lookAt(glm::vec3(0, 0, 0));

   cam2.setGlobalPosition(currentPos);
   cam2.lookAt(glm::vec3(0, 0, 0));

   // Sound

   // GUI
   gui.setup();
   gui.add(move.setup("Move Force", 30, 1, 100));
   gui.add(gravity.setup("Gravity", 9, -20, 20));
   bHide = false;
   //	gui.add(camDistance.setup("Camera Distance", 20, 10, 50));
}

//--------------------------------------------------------------
void ofApp::update() {
   
   grav->set(ofVec3f(0, -gravity, 0));

   sys->update();
   currentPos = sys->particles[0].position;

   // Set tractor's position to the particles position



   // Set mainCam as a trailing cam
   mainCam.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y + 50, currentPos.z + 50));
   mainCam.lookAt(currentPos);

   // Set cam1 to follow the tractor from a fixed position
   cam1.lookAt(currentPos);

   // Set cam2 to tractor's position looking down
   cam2.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y - 10, currentPos.z));
   cam2.lookAt(glm::vec3(currentPos.x, currentPos.y - 50, currentPos.z));

   checkCollision();
   checkLanding();
}

//--------------------------------------------------------------
void ofApp::draw() {
   ofBackground(ofColor::black);
   
   ofEnableDepthTest();
   theCam->begin();

   // draw simple box object
   //
   //ofNoFill();
   //ofDrawBox(30);

   //	ofPushMatrix();
   //	ofTranslate(mainCam.getPosition());
   //	ofDrawSphere(2.0, 5.0);
   //	ofPopMatrix();

   // Draw Cams
   ofSetColor(ofColor::dimGrey);
   mainCam.draw();
   cam1.draw();
   cam2.draw();

   // Draw Models

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
      break;
   case OF_KEY_DOWN:
      moveForce->set(ofVec3f(0, 0, move));
      break;
   case OF_KEY_LEFT:
      moveForce->set(ofVec3f(-move, 0));
      break;
   case OF_KEY_RIGHT:
      moveForce->set(ofVec3f(move, 0));
      break;
   case ' ':
      moveForce->set(ofVec3f(0, move, 0));
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
      theCam = &cam1;
      break;
   case OF_KEY_F3:
      theCam = &cam2;
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
      break;
   case OF_KEY_DOWN:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(sys->particles[0].velocity.x, sys->particles[0].velocity.y, 0);
      break;
   case OF_KEY_LEFT:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(0, sys->particles[0].velocity.y, sys->particles[0].velocity.z);
      break;
   case OF_KEY_RIGHT:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(0, sys->particles[0].velocity.y, sys->particles[0].velocity.z);
      break;
   case ' ':
      moveForce->set(ofVec3f(0, 0, 0));
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
