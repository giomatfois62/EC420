# EC420
Tiny, header-only, Entity-Component framework in 420 lines of C++

'''cpp
	Entity entity = world.createEntity();

	world.addComponents(entity, A());

	world.addComponents(entity, B(), C());

	world.removeComponent<B>(entity);

	world.destroyEntity(entity);
'''
