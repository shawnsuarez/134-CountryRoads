#pragma once
//  Kevin M. Smith - CS 134 SJSU

#include "ofMain.h"
#include "Particle.h"


//  Pure Virtual Function Class - must be subclassed to create new forces.
//
class ParticleForce {
protected:
public:
   bool applyOnce = false;
   bool applied = false;
   virtual void updateForce(Particle *) = 0;
};

class ParticleSystem {
public:
	void add(const Particle &);
	void addForce(ParticleForce *);
	void update();
   void setLifespan(float);
   void reset();
	void draw();
	vector<Particle> particles;
	vector<ParticleForce *> forces;
};



// Some convenient built-in forces
//
class GravityForce: public ParticleForce {
private:
	ofVec3f gravity;
public:
   GravityForce() {};
	GravityForce(const ofVec3f & gravity);
	void updateForce(Particle *);
   void set(const ofVec3f &gravity) { this->gravity = gravity; };
};

class TurbulenceForce : public ParticleForce {
   ofVec3f tmin, tmax;
public:
   void set(const ofVec3f &min, const ofVec3f &max) { tmin = min; tmax = max; }
   TurbulenceForce(const ofVec3f & min, const ofVec3f &max);
   void updateForce(Particle *);
};

class ImpulseRadialForce : public ParticleForce {
   float magnitude;
   float height = .2;
public:
   void set(float mag) { magnitude = mag; }
   void setHeight(float h) { height = h; }
   ImpulseRadialForce(float magnitude);
   void updateForce(Particle *);
};

class CyclicForce : public ParticleForce {
   float magnitude;
public:
   void set(float mag) { magnitude = mag; }
   CyclicForce(float magnitude);
   void updateForce(Particle *);
};

class MovementForce : public ParticleForce {
private:
   ofVec3f movement;
public:
   MovementForce() {};
   MovementForce(const ofVec3f & movement);
   void updateForce(Particle *);
   void set(const ofVec3f &movement) { this->movement = movement; };
};