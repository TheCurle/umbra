#pragma once

#include <memory>
#include <vector>
#include "shadow/assets/management/delegate.h"

namespace ShadowEngine {

    /**
     * Manage Engine Systems.
     * Manages the lifecycle, loading and destruction of necessary systems.
     * Looks up and loads all necessary systems as required.
     */

    struct SystemManager {
        virtual ~SystemManager() = default;

        static std::unique_ptr<SystemManager> create(struct Engine& eng);
        static void createAllSystems(Engine& eng);

        virtual void initSystems() = 0;

        virtual struct EngineSystem* load(const char* path) = 0;
        virtual void unload(struct EngineSystem* system) = 0;

        virtual void addSystem(EngineSystem* system, void* library) = 0;
        virtual EngineSystem* getSystem(const char* name) = 0;
        virtual void* getLibrary(EngineSystem* system) const = 0;

        virtual const std::vector<EngineSystem*>& getSystems() = 0;
        virtual const std::vector<void*>& getLibraries() = 0;

        virtual void update(float deltaTime) = 0;
        virtual Delegate<void(void*)>& onLibraryLoaded() = 0;
    };

    /**
     * ComponentManager is an Engine System that manages all Components of a specific type in the game world.
     * Separately from the Systems that operate over the Components themselves, these are what make sure the Components' data is readily accessible.
     */
    struct ComponentManager {
        virtual ~ComponentManager();

        virtual void init() {}
        virtual void serialize(struct OutputMemoryStream& serializer) = 0;
        virtual void deserialize(struct InputMemoryStream& serialize, const struct EntityMap& map, uint32_t version) = 0;

        virtual void preReload(OutputMemoryStream& serializer) {}
        virtual void postReload(InputMemoryStream& serializer) {}

        virtual EngineSystem& getSystem() const = 0;
        virtual void update(float timeDelta) = 0;
        virtual void lateUpdate(float timeDelta) {}

        virtual struct World& getWorld() = 0;
        virtual void startGame() {}
        virtual void stopGame() {}

        virtual uint32_t getVersion() const { return -1; };
    };

    /**
     * Engine Systems are core objects that manage some kind of state about the game.
     * Component Systems manage the data of all game objects (entities).
     */
    struct EngineSystem {
        virtual ~EngineSystem();

        // The System is just being loaded. No order is guaranteed.
        virtual void init() {}
        // All systems have been inited, every System gets a second chance in case it depends on something else.
        virtual void initOver() {}
        // Update the system. Parameter is the amount of time passed since last update.
        virtual void update(float) {}
        virtual const char* getName() const = 0;
        virtual uint32_t getVersion() const { return 0; };
        // Save the state of the System.
        virtual void serialise(OutputMemoryStream& serializer) const = 0;
        // Load the state of the System from a previous save.
        virtual bool deserialize(uint32_t version, InputMemoryStream& serializer) = 0;
        virtual void systemAdded(EngineSystem& system) {}

        virtual void createManagers(World&) {}
        virtual void startGame() {}
        virtual void stopGame() {}

    };
}