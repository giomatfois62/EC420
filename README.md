# EC420
Tiny, header-only, Entity-Component framework in 420 lines of C++(17) 


User defined classes derived from class Component<T> are automatically registered at compile time, without any macro or runtime method calls.  
Components of the same type are stored in std::vectors and kept packed through the program execution, to avoid cache miss.  
To compile the example main program run  
```cpp
g++ main.cpp --std=c++17
```
Below some code samples using the framework (taken from the main.cpp in repo).

* Example Component class declaration
```cpp
class E : public ecs::Component<E> {
public:
	int value; // component value
	
	static constexpr const char *name = "E"; // component name
	
	// custom component drawUI function to use with ImGUI or similar to show and modify component data
	static void drawUI(ECS &world, Entity id) { 
		E &e = world.component<E>(id);
		
		// draw UI
		cout << "Called custom drawUI of E on Entity " << id << endl;
	}

	// optional custom create function
	static void create(ECS &world, Entity id) { 
		world.addComponents(id, E()); // same as default implementation

		cout << "Called custom (re)create E on Entity " << id << endl;
	}

	// optional custom destroy function
	static void destroy(ECS &world, Entity id) { 
		world.removeComponent<E>(id); // same as default implementation

		cout << "Called custom destroy E on Entity " << id << endl;
	}
};
```
* Entities creation and manipulation
```cpp
int main(int argc, char** argv)
{
	ecs::ECS world;

	Entity entity = world.createEntity();

	world.addComponents(entity, A());

	world.addComponents(entity, B(), C());

	world.removeComponent<B>(entity);

	world.destroyEntity(entity);
	
	// remove all entities, also done on destructor call. 
	world.cleanUp() 

	return 0;
}
```
* Query group of entities having one or multiple components
```cpp
// std::vector<Entity> of entities with multiple components
world.entitiesWithComponents<A,B,C>();
	
// std::vector<D> of all instances of a single component
world.components<D>();
```
* A static component register holds all declared component descriptions: label, type ID and static functions
```cpp
for (auto &componentDescription : ecs::ComponentRegister) {
	cout << componentDescription.label; \\ component name (const char*)
	cout << componentDescription.type; \\ component ID (uint16_t)
	
	// static component functions usage (allows to manipulate entities using only component IDs)
	componentDescription.drawUI(world, entity); 
	componentDescription.create(world, entity);
	componentDescription.destroy(world, entity);
}
```
