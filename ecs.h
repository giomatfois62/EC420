#ifndef ECS_H
#define ECS_H

#include <cstdint>
#include <utility>
#include <vector>
#include <queue>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <map>
#include <functional>
#include <limits>
#include <cassert>

namespace ecs {

typedef uint16_t ComponentType;
typedef size_t Entity;

template <class T>
class _Component {
public:	
	static ComponentType type() { return T::m_type; }
	
	Entity id() { return m_id; }

	void setId(Entity id) { m_id = id; }
	
private:
	const static ComponentType m_type;
	Entity m_id = 0;
};

class ECS;
typedef std::function<void(ECS &world, Entity id)> ComponentFunction;

struct ComponentDescription {
	const char* label;
	ComponentType type;
	
	ComponentFunction create;
	ComponentFunction destroy;
	ComponentFunction drawUI;
};

inline std::vector<ComponentDescription> ComponentRegister; // c++17

template<class T>
ComponentType registerComponent() {
	ComponentRegister.push_back({
		T::name,
		static_cast<ComponentType>(ComponentRegister.size()),
		T::create,
		T::destroy,
		T::drawUI
	});
	
	assert(ComponentRegister.size() < std::numeric_limits<ComponentType>::max());

	return ComponentRegister.size()-1;
};

template <class T>
const ComponentType _Component<T>::m_type = registerComponent<T>();

class BaseContainer {
public:
	virtual ~BaseContainer() = default;	
	
	virtual size_t size() = 0;
	virtual void clear() = 0;
	virtual std::pair<size_t, size_t> remove(size_t index) = 0;
};

template<class T>
class Container : public BaseContainer {
public:	
	Container() { m_items.resize(1); }
	
	size_t size() override { return m_items.size(); }

	T& itemAt(size_t index) { return m_items[index]; }
	
	T& operator[](size_t index) { return m_items[index]; }
	
	std::vector<T>& items() { return m_items; }
	
	size_t insert(const T &item)
	{
		m_items.push_back(item);
		
		return m_items.size() - 1;
	}
	
	std::pair<size_t, size_t> remove(size_t index) override
	{
		if (index < m_items.size() - 1) {
			std::swap(m_items[index], m_items[m_items.size() - 1]);
			m_items.pop_back();
			
			return std::make_pair(m_items[index].id(), index);
		} else {
			m_items.pop_back();

			return std::make_pair(0, 0);
		}
	}
	
	void clear() override { m_items.resize(1); }
	
private:
	std::vector<T> m_items;
};

template<class T>
class SparseContainer {
public:
	SparseContainer() { m_items.resize(1); }
	
	size_t realSize() { return m_items.size(); }
	
	size_t size() { return m_items.size() - m_free.size(); }
	
	T& itemAt(size_t index) { return m_items[index]; }
	
	T& operator[](size_t index) { return m_items[index]; }
	
	std::vector<T>& data() { return m_items; }
	
	size_t insert(const T &item)
	{
		size_t itemIndex;
		
		if (!m_free.empty()) {
			itemIndex = m_free.front();
			m_free.pop();
			m_items[itemIndex] = item;
		} else {
			itemIndex = m_items.size();
			m_items.push_back(item);
		}
		
		return itemIndex;
	}
	
	void remove(size_t index)
	{
		if (index < m_items.size() - 1) {
			m_free.push(index);
			m_items[index] = T();
		} else {
			m_items.pop_back();
		}
	}
	
	std::vector<T> &items() {return m_items;}
	
	void clear() { m_items.resize(1); }
	
private:
	std::vector<T> m_items;
	std::queue<size_t> m_free;
};

class ComponentStorage {
	public:
		ComponentStorage() { m_storage.resize(ComponentRegister.size()); }
		~ComponentStorage() { clear(); }
		
		BaseContainer* operator[](size_t index) { return m_storage[index]; }

		template <typename T>
		Container<T>* get()
		{		
			ComponentType type(T::type());
		
			if (m_storage[type] == nullptr)
				m_storage[type] = new Container<T>();
		
			return static_cast<Container<T>*>(m_storage[type]);
		}

		BaseContainer* get(ComponentType type)
		{		
			return m_storage[type];
		}
	
		void clear()
		{
			for(BaseContainer* b : m_storage)
				delete b;
		
			m_storage.clear();
		}
	
	private:
		std::vector<BaseContainer*> m_storage;
};

typedef std::vector<size_t> ComponentList;
typedef SparseContainer<ComponentList> EntityList;

class ECS {
	public:
		ECS()
		{
			m_entitiesWith.resize(ComponentRegister.size());
		}
		
		template <class T>
		void addComponents(Entity id, T&& component)
		{
			ComponentType type(T::type());
			
			if (m_entities[id].size() <= type)
				m_entities[id].resize(type + 1, 0);

			component.setId(id);
			size_t index = m_entities[id][type];
			
			if (index > 0) {
				m_components.get<T>()->itemAt(index) = component;
			}else {
				m_entities[id][type] = m_components.get<T>()->insert(component);
				
				m_entitiesWith[type].insert(id);
			}
		}
		
		template <class T1, class T2, class ...Args>
		void addComponents(Entity id, T1&&c1, T2&&c2, Args...args)
		{
			addComponents(id, c1);
			addComponents(c2, args...);
		}

		template <class T, typename... Targs>
		void createComponent(Entity id, Targs... args)
		{
			addComponents(id, T(args...));
		}

		template <class T>
		void removeComponent(Entity id)
		{
			ComponentType type(T::type());

			auto pair = m_components.get(type)->remove(componentIndex(id, type));
		
			if (pair.first > 0)
				m_entities[pair.first][type] = pair.second;

			m_entities[id][type] = 0;
			
			m_entitiesWith[type].erase(id);
		}
	
		template <class T>
		const std::vector<T>& components()
		{		
			return m_components.get<T>()->items();
		}

		template <class T>
		T& componentWithIndex(size_t index)
		{
			return m_components.get<T>()->itemAt(index);
		}

		template <class T>
		T& component(Entity id)
		{	
			return componentWithIndex<T>(componentIndex(id, T::type()));
		}
	
		Entity createEntity()
		{	
			return m_entities.insert(ComponentList());
		}
	
		void destroyEntity(Entity id)
		{
			for(size_t i = 0; i < m_entities[id].size(); ++i) {
				if (m_entities[id][i] == 0)
					continue;
				
				auto pair = m_components.get(i)->remove(componentIndex(id, i));
					
				if (pair.first > 0)
					m_entities[pair.first][i] = pair.second; 
			}		
			
			m_entities.remove(id);
		}
		
		template<typename T>
		std::set<size_t>  entitiesWithComponent()
		{
			return m_entitiesWith[T::type()];
		}
		
		template<typename... Targs>
		std::vector<size_t> entitiesWithComponents()
		{
			std::vector<ComponentType> list;
			readComponents<Targs...>(list);
			
			std::sort(list.begin(), list.end(), 
				[this](auto c1,auto c2) {
					return m_components[c1]->size() < m_components[c2]->size();
			});
			
			std::vector<size_t> entities = {0};
			
			for (Entity id : m_entitiesWith[list[0]]) {
					if (hasComponents<Targs...>(id))
						entities.push_back(id);
			}	
			
			return entities;
		}
	
		void cleanUp()
		{
			m_entities.clear();
			m_components.clear();
		}

		size_t componentIndex(Entity id, ComponentType type)
		{	
			if (m_entities[id].size() <= type)
				m_entities[id].resize(type + 1, 0);
		
			return m_entities[id][type];
		}
		
		template<class T> 
		bool hasComponents(Entity id)
		{
			return m_entities[id][T::type()];
		}
		
		template<class T1, class T2, class ...Args> 
		bool hasComponents(Entity id)
		{
			if (!hasComponents<T1>(id))
				return false;
			
			return hasComponents<T2, Args...>(id);
		}
		
		EntityList& entities()
		{
			return m_entities;
		}

	private:
		EntityList m_entities;
		ComponentStorage m_components;
		std::vector<std::set<size_t>> m_entitiesWith;
		
		template<class T> 
		void readComponents(std::vector<ComponentType> &list)
		{
			list.push_back(T::type());
		}
		
		template<class T1, class T2, class ...Args> 
		void readComponents(std::vector<ComponentType> &list)
		{
			readComponents<T1>(list);
			readComponents<T2, Args...>(list);
		}
};
	
static void drawUI(ECS &world, ComponentType type, Entity id)
{
	ComponentRegister[type].drawUI(world, id);
}
	
static void create(ECS &world, ComponentType type, Entity id)
{
	ComponentRegister[type].create(world, id);
}

static void destroy(ECS &world, ComponentType type, Entity id)
{
	ComponentRegister[type].destroy(world, id);
}

static const char *label(ComponentType type)
{
	return ComponentRegister[type].label;
}

template <class T>
class Component : public _Component<T> {
public:
	static constexpr const char *name = "Unnamed Component";
 
	static void drawUI(ECS &world, Entity id) 
	{
		
	}

	static void create(ECS &world, Entity id) 
	{ 
		world.addComponents(id,T()); 
	}
	
	static void destroy(ECS &world, Entity id) 
	{ 
		world.removeComponent<T>(id); 
	} 
};

} // namespace ecs

#endif
