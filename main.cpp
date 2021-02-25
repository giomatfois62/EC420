#include "ecs.h"

#include <iostream>
#include <map>

using namespace ecs;
using namespace std;

// macro to define simple components (no drawUI function)
#define MAKE_COMPONENT(COMPONENT, VALUETYPE) \
class COMPONENT : public Component<COMPONENT> { \
public:\
	VALUETYPE value; \
	static constexpr const char name[] = #COMPONENT; \
	static void destroy(ECS &world, Entity id) { \
		world.removeComponent<COMPONENT>(id); \
	} \
};

MAKE_COMPONENT(A, int); // component with name/value type
MAKE_COMPONENT(B, float);
MAKE_COMPONENT(C, std::string);

// custom data types
struct Struct {
	int a;
	float b;
	std::string c;
};

MAKE_COMPONENT(D, Struct);

// explicit component class declaration
class E : public Component<E> {
public:
	int value; // component value
	
	static constexpr const char *name = "E"; //component name
	
	// custom component drawUI function to use with ImGUI or similar 
	static void drawUI(ECS &world, Entity id) { 
		E &e = world.component<E>(id);
		
		// draw UI
		cout << "Called custom drawUI of E on Entity " << id << endl;
	}

	static void create(ECS &world, Entity id) { // optional custom create function
		world.addComponents(id, E());

		cout << "Called custom (re)create E on Entity " << id << endl;
	}

	static void destroy(ECS &world, Entity id) { // optional custom destroy function
		world.removeComponent<E>(id);

		cout << "Called custom destroy E on Entity " << id << endl;
	}
};

void printComponents()
{
	// static component register holds component descriptions: label, type ID and static functions
	cout << ecs::ComponentRegister.size() << " registered components\n";

	for (const auto &componentDescription : ecs::ComponentRegister)
		cout << "Name: " << componentDescription.label 
			<< " Type: " << componentDescription.type << endl;
}

int main()
{
	srand(time(nullptr));
	
	printComponents();

	ECS world;

	size_t COUNT = 100000;	
	
	// crete entities and add components
	Entity entity;
	for (size_t i = 0; i < COUNT; ++i) {
		entity = world.createEntity();
	
		float rnd = rand()%ComponentRegister.size();

		if (rnd <= A::type())
			world.addComponents(entity, A()); // few entities have this
		if (rnd <= B::type())
			world.addComponents(entity, B());
		if (rnd <= C::type())
			world.addComponents(entity, C());
		if (rnd <= D::type())
			world.addComponents(entity, D());
		if (rnd <= E::type())
			world.addComponents(entity, E()); // all entities have this
	}
	
	entity = rand()%COUNT;
	cout << "Random Entity " << entity << endl;
	
	// static component functions usage (manipulate entities using only component IDs)
	for (auto &componentDescription : ComponentRegister) {
		componentDescription.drawUI(world, entity);
		componentDescription.create(world, entity);
		componentDescription.destroy(world, entity);
	}
	
	// remove a component from an entity
	world.removeComponent<E>(entity);

	// delete an entity
	world.destroyEntity(entity);
	
	// check entities count
	cout << world.entities().size() << " Entities created so far" << endl;
	
	// std::vector of entities with multiple components
	cout <<  world.entitiesWithComponents<A,B,C>().size() 
		<< " Entities with Components A, B, C" << endl;
	
	// std::vector of all instances of a component type
	cout << world.components<D>().size() 
		<< " Entities with Components D" << endl;
	
	return 0;
}
