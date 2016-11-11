#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

#include "ld24529.h"
#include "CraigUtils.h"
#include "Event.h"
#include "ObjInfo.h"
#include "Params.h"
#include "Random.h"
#include "Window.h"

#ifdef _MSC_VER
using namespace epl;
#endif
using namespace std;
using String = std::string;

Initializer<ld24529> __ld24529_initializer;

String ld24529::species_name(void) const
{
	if (health() > 5.0) return "Alage";
	return "ld24529";
}

String ld24529::player_name(void) const {
	return "ld24529";
}

Action ld24529::encounter(const ObjInfo& info)
{
	if (info.species == species_name()) {
		/* don't be cannibalistic */
		set_course(info.bearing + M_PI);
		return LIFEFORM_IGNORE;
	}
	else {
		hunt_event->cancel();
		SmartPointer<ld24529> self = SmartPointer<ld24529>(this);
		hunt_event = new Event(0.0, [self](void) { self->hunt(); });
		return LIFEFORM_EAT;
	}
}

void ld24529::initialize(void) {
	LifeForm::add_creator(ld24529::create, "ld24529");
}

/*
* REMEMBER: do not call set_course, set_speed, perceive, or reproduce
* from inside the constructor!!!!
* you must wait until the object is actually alive
*/
ld24529::ld24529() {
	SmartPointer<ld24529> self = SmartPointer<ld24529>(this);
	new Event(0, [self](void) { self->startup(); });
}

ld24529::~ld24529() {}

void ld24529::startup(void) {
	set_course(drand48() * 2.0 * M_PI);
	set_speed(2 + 5.0 * drand48());
	SmartPointer<ld24529> self = SmartPointer<ld24529>(this);
	hunt_event = new Event(0, [self](void) { self->hunt(); });
}

void ld24529::spawn(void) {
	SmartPointer<ld24529> child = new ld24529;
	reproduce(child);
}


Color ld24529::my_color(void) const {
	return BLUE;
}

SmartPointer<LifeForm> ld24529::create(void) {
	return new ld24529;
}


void ld24529::hunt(void) {
	const String fav_food = "Algae";

	hunt_event = nullptr;
	if (health() == 0.0) { return; } // we died

	double r = 20.0;
	ObjList prey = perceive(r);
	while (prey.size()==0 && r<50) {
		r = r + 10.0;
		prey = perceive(r);
	}

	double best_d = HUGE;
	bool FoundAlage = false;
	double time;
	double MinHealth=500.0;
	double MinCourse;
	double MinDistance;
	double MinAllyHealth = 500.0;
	double MinAllyCourse;
	double MinAllyDistance;
	for (ObjList::iterator i = prey.begin(); i != prey.end(); ++i) {
		if ((*i).species == fav_food && (*i).health<5.0) {
			FoundAlage = true;
			if (best_d > (*i).distance) {
				set_speed(3.0+5.0*drand48());
				set_course((*i).bearing);
				best_d = (*i).distance;
			}
		}
		else {
			if (MinHealth > (*i).health) {
				MinHealth = (*i).health;
				MinCourse = (*i).bearing;
				MinDistance = (*i).distance;
			}
		}
	}

	
	if (FoundAlage) {
		time = best_d / get_speed();
		SmartPointer<ld24529> self = SmartPointer<ld24529>(this);
		hunt_event = new Event(time, [self](void) { self->hunt(); });
		if(health()>6.0) spawn();
		return;
	}

	if ( MinHealth < health()/2.0 ) {
		set_speed(3.0 + 5.0*drand48());
		set_course(MinCourse);
		time = MinDistance / max_speed;
		SmartPointer<ld24529> self = SmartPointer<ld24529>(this);
		hunt_event = new Event(time, [self](void) { self->hunt(); });
		if (health()>8.0) spawn();
		return;
	}

	set_speed(2.0 + 5.0*drand48());
	set_course(get_course() + M_PI*(0.8+0.2*drand48()));
	SmartPointer<ld24529> self = SmartPointer<ld24529>(this);
	if (health() > 1.0) {
		hunt_event = new Event(8.0, [self](void) { self->hunt(); });
	}
	else {
		set_speed(5 + 5.0*drand48());
		set_course(get_course() + M_PI*(0.9 + 0.1*drand48()));
		hunt_event = new Event(8.0, [self](void) { self->hunt(); });
	}
}
