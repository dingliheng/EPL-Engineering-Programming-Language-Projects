/* main test simulator */
#include <iostream>
#include "CraigUtils.h"
#include "Window.h"
#include "tokens.h"
#include "ObjInfo.h"
#include "QuadTree.h"
#include "Params.h"
#include "LifeForm.h"
#include "Event.h"
#include "Random.h"

using namespace std;

template <typename T>
void bound(T& x, const T& min, const T& max) {
    assert(min < max);
    if (x > max) { x = max; }
    if (x < min) { x = min; }
}


ObjInfo LifeForm::info_about_them(SmartPointer<LifeForm> neighbor) {
    ObjInfo info;
    
    info.species = neighbor->species_name();
    info.health = neighbor->health();
    info.distance = pos.distance(neighbor->position());
    info.bearing = pos.bearing(neighbor->position());
    info.their_speed = neighbor->speed;
    info.their_course = neighbor->course;
    return info;
}

void LifeForm::update_position() {
    if(!is_alive) return;
    double delta_time=Event::now()-update_time;
    if(delta_time<=0.001) return;
	update_time = Event::now();
    energy-= movement_cost(speed, delta_time);
    Point oldPoint=pos;
    double delta_distance = speed*delta_time;
    pos.xpos=pos.xpos+delta_distance*cos(course);
    pos.ypos=pos.ypos+delta_distance*sin(course);
	if (space.is_out_of_bounds(pos)) {
		pos = oldPoint;
		die();
		return;
	}
    space.update_position(oldPoint,pos);
    if(energy<min_energy) {
        die();
        return;
    }

}


void LifeForm::border_cross() {
    if(!is_alive) return;
    update_position();
    check_encounter();
    compute_next_move();
}

void LifeForm::compute_next_move(void) {
    if(!is_alive) return;
    update_position();
    if(border_cross_event!=nullptr) {
        border_cross_event->cancel();
    }
    double distanceToEdge = space.distance_to_edge(pos, course);
    if(speed>0) {
        double deltaTime= distanceToEdge/speed+Point::tolerance;
        SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
        border_cross_event = new Event(deltaTime, [self](void) { self->border_cross(); });
    }
}

void LifeForm::region_resize() {
    //if(!is_alive) return;
    compute_next_move();
}

void LifeForm::set_course(double newCourse) {
    if(!is_alive) return;
    update_position();
    course=newCourse;
    compute_next_move();
}

void LifeForm::set_speed(double newSpeed) {
    if(!is_alive) return;
    update_position();
    speed=newSpeed;
    if(newSpeed>max_speed)
        speed=max_speed;
    if(newSpeed<0)
        speed=0;
    compute_next_move();
}

void LifeForm::check_encounter() {
    if(!is_alive) return;
    SmartPointer<LifeForm> enemy=space.closest(pos);
	enemy->update_position();
    double distance=pos.distance(enemy->pos);
    if(distance<encounter_distance) {
        energy-=encounter_penalty;
        enemy->energy-=encounter_penalty;
        if(energy<min_energy) {
            die();
        }
        if(enemy->energy<min_energy) {
            enemy->die();
        }
        resolve_encounter(enemy);
    }
}

void LifeForm::resolve_encounter(SmartPointer<LifeForm> enemy) {
    
    if(!is_alive || !enemy->is_alive) return;
    Action my_action = encounter(info_about_them(enemy));
    Action enemy_action = enemy->encounter(enemy->info_about_them(this));
    bool my_eating=false, enemy_eating=false;
    if(my_action==LIFEFORM_EAT ) {
        double mychances=eat_success_chance(this->energy, enemy->energy);
        double myrand = epl::drand48();
        if(myrand<mychances) my_eating=true;
    }
    if(enemy_action==LIFEFORM_EAT ) {
        double enemy_chances=eat_success_chance(this->energy, enemy->energy);
        double enemy_rand = epl::drand48();
        if(enemy_rand<enemy_chances) enemy_eating=true;
    }
    
    //eating rules
    if(my_eating==true && enemy_eating==false) {
        this->eat(enemy);
		enemy->die();
    }
    else if(my_eating==false && enemy_eating==true) {
        enemy->eat(this);
		this->die();
    }
    else if (my_eating==true && enemy_eating==true) {
		if (this->speed >= enemy->speed) {
			this->eat(enemy);
			enemy->die();
		}
		else { enemy->eat(this); this->die(); }
    }
    
}

void LifeForm::eat(SmartPointer<LifeForm> enemy) {
    if(!is_alive) return;
    energy-=eat_cost_function(energy,enemy->energy);
    SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
    (void) new Event(digestion_time, [self,enemy](void) { self->gain_energy(enemy->energy); });
}

void LifeForm::gain_energy(double enemy_energy) {
    if(!is_alive) return;
    energy+=enemy_energy*eat_efficiency;
}

void LifeForm::age() {
    if(!is_alive) return;
    SmartPointer<LifeForm> self = SmartPointer<LifeForm>(this);
	(void) new Event(age_frequency, [self](void) { self->energy -= age_penalty; self->age(); });
}

ObjList LifeForm::perceive(double radius) {
    std::vector<ObjInfo> prey_list;
	std::vector<SmartPointer<LifeForm>> v;
    if(!is_alive) return prey_list;
    double r;
    r=min(radius,max_perceive_range);
    r=max(radius,min_perceive_range);
    energy-=perceive_cost(r);
    v=space.nearby(pos,r);
	for (SmartPointer<LifeForm> a : v) {
		prey_list.push_back(info_about_them(a));
	}
    return prey_list;
}

void LifeForm::reproduce(SmartPointer<LifeForm> child) {
	if (!is_alive) return;
	double delta_time = Event::now() - reproduce_time;
	if (delta_time < min_reproduce_time) {
		child->die();
		return;
	}
	energy = (energy/2) * (1.0 - reproduce_cost);
	if (energy < min_energy) {
		die();
		child->die();
		return;
	}
	child->energy = energy;
	child->pos.xpos=pos.xpos+ epl::drand48() * reproduce_dist*sqrt(2)*2;
	child->pos.ypos = pos.ypos + epl::drand48() * reproduce_dist*sqrt(2) * 2;
	space.insert(child, child->pos, [child]() { child->region_resize(); });
	child->is_alive = true;
	(void) new Event(age_frequency, [child](void) { child->age(); });
    return;
}
