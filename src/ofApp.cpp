#include "ofApp.h"
#include "Util.h"

//--------------------------------------------------------------
void ofApp::setup() {
   ofSetBackgroundColor(ofColor::black);

   // Particles & Physics Setup
   sys = new ParticleSystem();
   grav = new GravityForce();
   moveForce = new MovementForce();
   turb = new TurbulenceForce(ofVec3f(-0.5, 0, -0.5), ofVec3f(0.5, 0, 0.5));

   currentPos = glm::vec3(0, 30, 0);

   ship.position = currentPos;
   ship.velocity = ofVec3f(0, 0);
   ship.radius = 1;
   ship.damping = 0.9995;

   sys->add(ship);
   sys->setLifespan(10000000);
   sys->addForce(grav);
   sys->addForce(moveForce);
   sys->addForce(turb);

   // Thruster Setup
   radialForce = new ImpulseRadialForce(1000);
   radialForce->setHeight(0.2);
   //cyclicForce = new CyclicForce(500);

   thrusterEmitter.sys->addForce(radialForce);
   //thrusterEmitter.sys->addForce(cyclicForce);
   thrusterEmitter.setVelocity(ofVec3f(0, -100, 0));
   thrusterEmitter.setEmitterType(DirectionalEmitter);
   thrusterEmitter.setGroupSize(200);
   thrusterEmitter.setRandomLife(true);
   thrusterEmitter.setLifespanRange(ofVec2f(0.05, 0.1));
   thrusterEmitter.setRate(20);

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

   // Models
   string modelPath = "Tractor/Tractor.obj";
   if (tractor.loadModel(modelPath)) {
      tractor.setScaleNormalization(false);
   }
   else {
      ofLogFatalError("Can't load model: " + modelPath);
      ofExit();
   }

   modelPath = "cornMoon/cornMoon.obj";
   //modelPath = "geo/mars-low.obj";
   if (cornField.loadModel(modelPath)) {
      cornField.setScaleNormalization(false);
   }
   else {
      ofLogFatalError("Can't load model: " + modelPath);
      ofExit();
   }

   bWireframe = false;
   bBoundingBox = false;

   // Landing Fields


   // Octree
   numLevels = 8;
   cout << "Generating Octree with " << numLevels << " levels." << endl;
   float startTime = ofGetElapsedTimeMillis();

   oct.create(cornField.getMesh(0), numLevels);

   float endTime = ofGetElapsedTimeMillis();
   float createTime = (endTime - startTime);
   cout << "Octree Creation Time: " << createTime << " ms" << endl;

   selectedPoint = ofVec3f(0, 0, 0);

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

   trackingCam.setGlobalPosition(glm::vec3(0, 25, 25));
   trackingCam.lookAt(glm::vec3(0, 0, 0));

   landingCam.setGlobalPosition(currentPos);
   landingCam.lookAt(glm::vec3(0, 0, 0));

   fixedCam.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y + 25, currentPos.z + 25));
   fixedCam.lookAt(currentPos);

   bShowCams = false;

   // Sound
   // Take Me Home, Country Roads
   // Album: Fallout 76 (Original Trailer Soundtrack) by Copilot Music + Sound
   string soundPath = "sounds/CountryRoads.mp3";
   if (countryRoads.load(soundPath)) {
      countryRoads.setLoop(true);
      countryRoads.setVolume(0.8f);
      //countryRoads.play();
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
   ofEnableLighting();

   // setup rudimentary lighting 
   //
   initLightingAndMaterials();

   sunLight.setup();
   sunLight.enable();
   sunLight.setAreaLight(100, 100);
   sunLight.setAmbientColor(ofFloatColor(0.1, 0.1, 0.1));
   sunLight.setDiffuseColor(ofFloatColor(100, 100, 100));
   sunLight.setSpecularColor(ofFloatColor(1, 1, 1));
   sunLight.setPosition(glm::vec3(0, 150, 0));

   // GUI
   gui.setup();
   gui.add(move.setup("Move Force", 10, 1, 100));
   gui.add(gravity.setup("Gravity", 0, -20, 40));
   gui.add(radius.setup("Particle Radius", 5, 1, 10));
   bHide = false;
   bShowPoint = false;
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

   // Create Ship Bounding Box
   ofVec3f min = tractor.getSceneMin() + tractor.getPosition();
   ofVec3f max = tractor.getSceneMax() + tractor.getPosition();
   shipBox = Box(Vector3(min.x, min.y, min.z), Vector3(max.x, max.y, max.z));

   // Set tractor's position to the particles position
   tractor.setPosition(currentPos.x, currentPos.y, currentPos.z);
   thrusterEmitter.setPosition(currentPos);

   // Set fixedCam as a trailing cam
   fixedCam.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y + 25, currentPos.z + 25));
   fixedCam.lookAt(currentPos);

   // Set trackingCam to follow the tractor from a fixed position
   trackingCam.lookAt(currentPos);

   // Set landingCam to tractor's position looking down
   landingCam.setGlobalPosition(glm::vec3(currentPos.x, currentPos.y + 20, currentPos.z));
   landingCam.lookAt(glm::vec3(currentPos.x, currentPos.y - 50, currentPos.z));

   checkCollision();
   checkLanding();
   checkAltitude();
}

//--------------------------------------------------------------
void ofApp::draw() {
   ofBackground(ofColor::darkGrey);
   
   loadVbo();

   ofEnableDepthTest();
   shader.begin();
   theCam->begin();

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
   if (bShowCams) {
      ofFill();
      ofSetColor(ofColor::dimGrey);
      mainCam.draw();
      trackingCam.draw();
      landingCam.draw();
      fixedCam.draw();
   }

   // Draw Models
   ofPushMatrix();
   ofNoFill();
   ofSetColor(ofColor::white);
   if (bWireframe) {
      tractor.drawWireframe();
      cornField.drawWireframe();
   }
   else {
      tractor.drawFaces();
      cornField.drawFaces();
   }
   ofPopMatrix();

   // Draw Ship Bounding Box
   if (bBoundingBox) {
      Vector3 min = shipBox.parameters[0];
      Vector3 max = shipBox.parameters[1];
      Vector3 size = max - min;
      Vector3 center = size / 2 + min;
      ofVec3f p = ofVec3f(center.x(), center.y(), center.z());
      float w = size.x();
      float h = size.y();
      float d = size.z();
      ofDrawBox(p, w, h, d);
   }

   if (bShowPoint && bPointSelected) {
      ofSetColor(ofColor::blue);
      ofDrawSphere(selectedPoint, 5);
   }

   // Draw Octree
   if (bShowOct) {
      ofPushMatrix();
      ofMultMatrix(cornField.getModelMatrix());
      //oct.drawLeafNodes(oct.root);
      //oct.draw(oct.root, numLevels, 0, colors);
      oct.draw(oct.root, 3, 0, colors);
      ofPopMatrix();
   }

   // draw a grid
   //
   /*ofPushMatrix();
   ofRotate(90, 0, 0, 1);
   ofSetLineWidth(1);
   ofSetColor(ofColor::dimGrey);
   ofDrawGridPlane(10.0f, 16, false);
   ofPopMatrix();*/

   theCam->end();

   // Draw GUI
   ofDisableDepthTest();
   if (!bHide) gui.draw();

   string str;
   str += "Frame Rate: " + std::to_string(ofGetFrameRate());
   ofSetColor(ofColor::white);
   ofDrawBitmapString(str, ofGetWindowWidth() - 170, 15);

   str = "Altitude: " + to_string(altitude);
   ofDrawBitmapString(str, ofGetWindowWidth() - 170, 55);

   str = "Controls: \n UP_ARROW: FORWARD \n DOWN_ARROW: BACK \n";
   str += " LEFT_ARROW: LEFT \n RIGHT_ARROW : RIGHT \n";
   str += " SPACE: UP \n CTRL: DOWN \n";
   str += " F1: Free Cam \n F2: Fixed Cam \n F3: Landing Cam \n F4: Tracking Cam \n";
   str += " X: Show Cams \n P: Show Point \n R: Reset";
   ofDrawBitmapString(str, ofGetWindowWidth() - 170, 85);
}

// Check terrain collision using Octree and ship bounding box
void ofApp::checkCollision() {
   ofVec3f contactPt = currentPos;
   ofVec3f vel = sys->particles[0].velocity;
   if (vel.y > 0) return;

   TreeNode node;
   if (oct.intersect(contactPt, oct.root, node)) {
      cout << "Collision" << endl;
      sys->particles[0].velocity.y = -sys->particles[0].velocity.y * 0.5;
   }
}

// Check ship's position with landing areas
void ofApp::checkLanding() {

}

// Check ship altitude using ray intersection
void ofApp::checkAltitude() {
   ofVec3f rayPoint = currentPos + ofVec3f(0, 10, 0);
   ofVec3f rayDir = ofVec3f(currentPos.x, currentPos.y - 1000, currentPos.z);
   rayDir.normalize();
   Ray ray = Ray(Vector3(rayPoint.x, rayPoint.y, rayPoint.z),
      Vector3(rayDir.x, rayDir.y, rayDir.z));

   TreeNode rtn;
   if (oct.intersect(ray, oct.root, rtn)) {
      bPointSelected = true;
      selectedPoint = ofVec3f(rtn.box.center().x(), rtn.box.center().y(), rtn.box.center().z());

      altitude = currentPos.y - selectedPoint.y;
   }
   else {
      bPointSelected = false;
   }
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
   case OF_KEY_CONTROL:
      moveForce->set(ofVec3f(0, -move, 0));
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
   case 'b':
      bBoundingBox = !bBoundingBox;
      break;
   case 'C':
   case 'c':
      if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
      else mainCam.enableMouseInput();
      break;
   case 'F':
   case 'f':
      ofToggleFullscreen();
      break;
   case 'h':
      bHide = !bHide;
      break;
   case 'o':
      bShowOct = !bShowOct;
      break;
   case 'p':
      bShowPoint = !bShowPoint;
      break;
   case 'r':
      sys->particles[0].position = glm::vec3(0, 30, 0);
      sys->particles[0].velocity = glm::vec3(0, 0, 0);
      break;
   case 'w':
      bWireframe = !bWireframe;
      break;
   case 'x':
      bShowCams = !bShowCams;
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
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
      break;
   case OF_KEY_DOWN:
      moveForce->set(ofVec3f(0, 0));
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
      break;
   case OF_KEY_LEFT:
      moveForce->set(ofVec3f(0, 0));
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
      break;
   case OF_KEY_RIGHT:
      moveForce->set(ofVec3f(0, 0));
      bThrust = false;
      thrusters.stop();
      thrusterEmitter.stop();
      break;
   case OF_KEY_CONTROL:
      moveForce->set(ofVec3f(0, 0, 0));
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
   glEnable(GL_LIGHT1);
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