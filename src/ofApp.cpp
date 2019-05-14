#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
void ofApp::setup() {
   ofSetBackgroundColor(ofColor::black);

   // Particles & Physics Setup
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


   // texture loading
   //
   ofDisableArbTex();     // disable rectangular textures

   // load textures
   //
   string imagePath = "images/dot.png";
   if (!ofLoadImage(particleTex, imagePath)) {
      cout << "Particle Texture File: " << imagePath << " not found." << endl;
      ofExit();
   }

   // load the shader
   //
#ifdef TARGET_OPENGLES
   shader.load("shaders_gles/shader");
#else
   shader.load("shaders/shader");
#endif

   radialForce = new ImpulseRadialForce(1000);
   radialForce->setHeight(0.2);
   //cyclicForce = new CyclicForce(500);

   thrusterEmitter.sys->addForce(radialForce);
   //thrusterEmitter.sys->addForce(cyclicForce);
   thrusterEmitter.setVelocity(ofVec3f(0, 0, 0));
   thrusterEmitter.setEmitterType(RadialEmitter);
   thrusterEmitter.setGroupSize(200);
   thrusterEmitter.setRandomLife(true);
   thrusterEmitter.setLifespanRange(ofVec2f(0.2, 0.5));

   // Models
   string modelPath = "Tractor/Tractor.obj";
   if (tractor.loadModel(modelPath)) {
      tractor.setScaleNormalization(false);
      //tractor.setRotation(0, 180, 0, 1, 0);
   }
   else {
      ofLogFatalError("Can't load model: " + modelPath);
      ofExit();
   }

   modelPath = "cornMoon/cornMoon.obj";
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
   numLevels = 10;
   cout << "Generating Octree with " << numLevels << " levels." << endl;
   float startTime = ofGetElapsedTimeMillis();

   oct.create(cornMesh, numLevels);

   float endTime = ofGetElapsedTimeMillis();
   float createTime = (endTime - startTime);
   cout << "Octree Creation Time: " << createTime << " ms" << endl;

   bShowOct = false;

   colors.push_back(ofColor::white); // Colors for drawing octree levels
   colors.push_back(ofColor::blue);
   colors.push_back(ofColor::red);
   colors.push_back(ofColor::yellow);
   colors.push_back(ofColor::green);
   colors.push_back(ofColor::purple);
   colors.push_back(ofColor::orange);
   colors.push_back(ofColor::cyan);
   colors.push_back(ofColor::magenta);

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

   // Lighting
   ofEnableLighting(); // Add lighting first XD

   // setup rudimentary lighting 
   //
   initLightingAndMaterials();

   // GUI
   gui.setup();
   gui.add(move.setup("Move Force", 10, 1, 100));
   gui.add(gravity.setup("Gravity", 3, -20, 40));
   gui.add(radius.setup("Particle Radius", 5, 1, 10));
   bHide = false;
}

//--------------------------------------------------------------
void ofApp::update() {
   
   grav->set(ofVec3f(0, -gravity, 0));

   // Play Thrusters
   if (bThrust && !thrusters.isPlaying()) {
      thrusters.play();
   }

   // Update Particles
   sys->update();
   thrusterEmitter.update();
   currentPos = sys->particles[0].position;

   // Create Bounding Box
   ofVec3f min = tractor.getSceneMin() + tractor.getPosition();
   ofVec3f max = tractor.getSceneMax() + tractor.getPosition();
   shipBox = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

   // Set tractor's position to the particles position
   tractor.setPosition(currentPos.x, currentPos.y, currentPos.z);
   thrusterEmitter.setPosition(currentPos);

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
   ofBackground(ofColor::darkGrey);
   
   loadVbo();

   ofEnableDepthTest();
   shader.begin();
   theCam->begin();

   //sunLight.draw();

   // Draw Particles
   sys->draw();

   ofSetColor(ofColor::lightGoldenRodYellow);

   ofEnablePointSprites();
   particleTex.bind();
   vbo.draw(GL_POINTS, 0, (int)thrusterEmitter.sys->particles.size());
   particleTex.unbind();
   shader.end();
   ofDisablePointSprites();

   // Draw Cams
   ofFill();
   ofSetColor(ofColor::dimGrey);
   mainCam.draw();
   trackingCam.draw();
   //landingCam.draw();
   fixedCam.draw();

   //ofDisableLighting();

   // Draw Models
   ofPushMatrix();
   ofNoFill();
   ofSetColor(ofColor::white);
   tractor.drawFaces();
   cornField.drawFaces();
   ofPopMatrix();

   // Draw Bounding Boxes
   Vector3 min = shipBox.parameters[0];
   Vector3 max = shipBox.parameters[1];
   Vector3 size = max - min;
   Vector3 center = size / 2 + min;
   ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
   float w = size.x();
   float h = size.y();
   float d = size.z();
   ofDrawBox(p, w, h, d);

   // Draw Octree
   if (bShowOct) {
      ofPushMatrix();
      ofMultMatrix(cornField.getModelMatrix());
      oct.drawLeafNodes(oct.root);
      //oct.draw(oct.root, numLevels, 0, colors);
      ofPopMatrix();
   }

   // draw a grid
   //
   ofPushMatrix();
   ofRotate(90, 0, 0, 1);
   ofSetLineWidth(1);
   ofSetColor(ofColor::dimGrey);
   ofDrawGridPlane(10.0f, 16, false);
   ofPopMatrix();

   theCam->end();

   // draw the GUI
   ofDisableDepthTest();
   if (!bHide) gui.draw();

   // draw screen data
   //
   string str;
   str += "Frame Rate: " + std::to_string(ofGetFrameRate());
   ofSetColor(ofColor::white);
   ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);
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
      if (!bThrust) {
         bThrust = true;
         thrusterEmitter.start();
      }
      break;
   case OF_KEY_DOWN:
      moveForce->set(ofVec3f(0, 0, move));
      if (!bThrust) {
         bThrust = true;
         thrusterEmitter.start();
      }
      break;
   case OF_KEY_LEFT:
      moveForce->set(ofVec3f(-move, 0));
      if (!bThrust) {
         bThrust = true;
         thrusterEmitter.start();
      }
      break;
   case OF_KEY_RIGHT:
      moveForce->set(ofVec3f(move, 0));
      if (!bThrust) {
         bThrust = true;
         thrusterEmitter.start();
      }
      break;
   case ' ':
      moveForce->set(ofVec3f(0, move, 0));
      if (!bThrust) {
         bThrust = true;
         thrusterEmitter.start();
      }
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
   case 'o':
      bShowOct = !bShowOct;
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
      thrusterEmitter.stop();
      break;
   case OF_KEY_DOWN:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(sys->particles[0].velocity.x, sys->particles[0].velocity.y, 0);
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
      break;
   case OF_KEY_LEFT:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(0, sys->particles[0].velocity.y, sys->particles[0].velocity.z);
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
      break;
   case OF_KEY_RIGHT:
      moveForce->set(ofVec3f(0, 0));
      //sys->particles[0].velocity = ofVec3f(0, sys->particles[0].velocity.y, sys->particles[0].velocity.z);
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
      break;
   case ' ':
      moveForce->set(ofVec3f(0, 0, 0));
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
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

//--------------------------------------------------------------
// setup basic ambient lighting in GL  (for now, enable just 1 light)
//
void ofApp::initLightingAndMaterials() {

   static float ambient[] =
   { .5f, .5f, .5, 1.0f };
   static float diffuse[] =
   { 1.0f, 1.0f, 1.0f, 1.0f };

   static float position[] =
   { 5.0, 5.0, 5.0, 0.0 };

   static float lmodel_ambient[] =
   { 1.0f, 1.0f, 1.0f, 1.0f };

   static float lmodel_twoside[] =
   { GL_TRUE };


   glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
   glLightfv(GL_LIGHT0, GL_POSITION, position);

   glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
   glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
   glLightfv(GL_LIGHT1, GL_POSITION, position);


   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
   glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   //	glEnable(GL_LIGHT1);
   glShadeModel(GL_SMOOTH);
}

// load vertex buffer in preparation for rendering
//
void ofApp::loadVbo() {
   if (thrusterEmitter.sys->particles.size() < 1) return;

   vector<ofVec3f> sizes;
   vector<ofVec3f> points;
   for (int i = 0; i < thrusterEmitter.sys->particles.size(); i++) {
      points.push_back(thrusterEmitter.sys->particles[i].position);
      sizes.push_back(ofVec3f(radius));
   }
   // upload the data to the vbo
   //
   int total = (int)points.size();
   vbo.clear();
   vbo.setVertexData(&points[0], total, GL_STATIC_DRAW);
   vbo.setNormalData(&sizes[0], total, GL_STATIC_DRAW);
}