# EC420
Tiny, header-only, Entity-Component framework in 420 lines of C++

```cpp
Entity entity = world.createEntity();

world.addComponents(entity, A());

world.addComponents(entity, B(), C());

world.removeComponent<B>(entity);

world.destroyEntity(entity);
```

```cpp
// std::vector of entities with multiple components
world.entitiesWithComponents<A,B,C>();
	
// std::vector of all instances of a single component
world.components<D>();
```
