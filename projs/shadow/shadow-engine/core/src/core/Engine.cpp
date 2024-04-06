#include <shadow/core/Engine.h>
#include "shadow/assets/resource/ResourceManager.h"
#include "shadow/assets/resource/Resource.h"
#include "shadow/profile/Profiler.h"
#include "spdlog/spdlog.h"
#include "shadow/platform/Common.h"

namespace ShadowEngine {

  struct PrefabManager final : ResourceTypeManager {
    explicit PrefabManager() : ResourceTypeManager() { }

    Resource* createResource(const Path& path) override {
        return new PrefabResource(path, *this);
    }

    void destroyResource(Resource& res) override {
        free(dynamic_cast<PrefabResource*>(&res));
    }
  };

  struct EngineImpl : Engine {
    void operator=(const EngineImpl&) = delete;
    EngineImpl(const EngineImpl&) = delete;

    EngineImpl(Initialization&& init) :
        prefabManager(), resourceManager(), gameRunning(false), smoothTimeDelta(1/60.f), timeScale(1.0f), paused(false), nextFrame(false) {

        ::ProfileFunction();

        // Back-initialize data to fake pre-startup at 60fps.
        for (float& f : deltas)
            f = 1/60.0f;

        logFileOpen = logFile.open("Umbra.log");

        spdlog::info("Starting Umbra Engine");

        if (init.workingDir) spdlog::info(std::string("Working dir: ").append(init.workingDir));

        Platform::DumpSystemData();

        if (init.fs.get())
            fileSystem = static_cast<std::unique_ptr<FileSystem>&&>(init.fs);
        else if(init.workingDir)
            fileSystem = FileSystem::createDiskFS(init.workingDir);
        else {
            std::string path;
            Platform::GetExecutableDirectory(path);
            fileSystem = FileSystem::createDiskFS(path);
        }

        resourceManager.init(*fileSystem);
        prefabManager.create(PrefabResource::TYPE, resourceManager);

        systemManager = SystemManager::create(*this);

        systemManager->addSystem(createCoreSystem(), nullptr);




    }

  private:

    std::unique_ptr<FileSystem> fileSystem;
    ResourceManager resourceManager;
    std::unique_ptr<SystemManager> systemManager;
    PrefabManager prefabManager;

    float timeScale;
    float deltas[11] = {};
    float smoothTimeDelta;
    bool gameRunning;
    bool paused;
    bool nextFrame;

    FileOutput logFile;
    bool logFileOpen = false;
  };
}