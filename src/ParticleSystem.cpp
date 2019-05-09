
// Kevin M.Smith - CS 134 SJSU

#include "ParticleSystem.h"

void ParticleSystem::add(const Particle &p) {
	particles.push_back(p);
}

void ParticleSystem::addForce(ParticleForce *f) {
	forces.push_back(f);
}

void ParticleSystem::setLifespan(float l) {
   for (int i = 0; i < particles.size(); i++) {
      particles[i].lifespan = l;
   }
}

void ParticleSystem::reset() {
   for (int i = 0; i < forces.size(); i++) {
      forces[i]->applied = false;
   }
}

void ParticleSystem::update() {
	// check if empty and just return
	if (particles.size() == 0) return;

	vector<Particle>::iterator p = particles.begin();
	vector<Particle>::iterator tmp;

	// check which particles have exceed their lifespan and delete
	// from list.  When deleting multiple objects from a vector while
	// traversing at the same time, we need to use an iterator.
	//
	/*while (p != particles.end()) {
		if (p->lifespan != -1 && p->age() > p->lifespan) {
			tmp = particles.erase(p);
			p = tmp;
		}
		else p++;
	}*/

	// update forces on all particles first 
	//
	for (int i = 0; i < particles.size(); i++) {
		for (int k = 0; k < forces.size(); k++) {
         if (!forces[k]->applied)
			   forces[k]->updateForce( &particles[i] );
		}
	}

   // update all forces only applied once to "applied"
   // so they are not applied again.
   //
   /*for (int i = 0; i < forces.size(); i++) {
      if (forces[i]->applyOnce)
         forces[i]->applied = true;
   }*/

	// integrate all the particles in the store
	//
	for (int i = 0; i < particles.size(); i++)
		particles[i].integrate();

}

//  draw the particle cloud
//
void ParticleSystem::draw() {
	for (int i = 0; i < particles.size(); i++) {
		particles[i].draw();
	}
}

// Gravity Force Field 
//
GravityForce::GravityForce(const ofVec3f &g) {
	gravity = g;
}

void GravityForce::updateForce(Particle * particle) {
	//
	// f = mg
	//
	particle->forces += gravity * particle->mass;
}

// Movement Forces
MovementForce::MovementForce(const ofVec3f & m)
{
   movement = m;
}

void MovementForce::updateForce(Particle * particle)
{
   particle->forces += movement;
}