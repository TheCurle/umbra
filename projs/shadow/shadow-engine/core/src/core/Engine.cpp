#include <shadow/core/Engine.h>
#include "shadow/assets/resource/ResourceManager.h"
#include "shadow/assets/resource/Resource.h"
#include "shadow/profile/Profiler.h"
#include "spdlog/spdlog.h"
#include "shadow/platform/Common.h"
#include "shadow/core/Time.h"
#include "shadow/core/CoreModule.h"

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
    void operator=(const EngineImpl &) = delete;
    EngineImpl(const EngineImpl &) = delete;

    EngineImpl(Initialization &&init) :
        prefabManager(),
        resourceManager(),
        gameRunning(false),
        smoothTimeDelta(1 / 60.f),
        timeScale(1.0f),
        paused(false),
        logFileOpen(logFile.open("Umbra.log")),
        processNextFrame(false) {

        ::ProfileFunction();

        // Back-initialize data to fake pre-startup at 60fps.
        for (float &f : deltas)
            f = 1 / 60.0f;


        spdlog::info("Starting Umbra Engine");

        if (init.workingDir) spdlog::info(std::string("Working dir: ").append(init.workingDir));

        Platform::DumpSystemData();

        if (init.fs)
            fileSystem = static_cast<std::unique_ptr<FileSystem> &&>(init.fs);
        else if (init.workingDir)
            fileSystem = FileSystem::createDiskFS(init.workingDir);
        else {
            std::string path;
            Platform::GetExecutableDirectory(path);
            fileSystem = FileSystem::createDiskFS(path);
        }

        resourceManager.init(*fileSystem);
        prefabManager.create(PrefabResource::TYPE, resourceManager);

        systemManager = SystemManager::create(*this);

        systemManager->addSystem(new CoreModule(), nullptr);

        for (auto plugin : init.plugins) {
            if (plugin[0] && systemManager->load(plugin.c_str()))
                spdlog::warn(plugin + " was not loaded.");
        }
    }

    void init() override {
        systemManager->initSystems();
    }

    ~EngineImpl() {
        prefabManager.destroy();
        systemManager.reset();
        fileSystem.reset();
        logFile.close();
        logFileOpen = false;
    }

    World& createWorld(bool main) override {
        // TODO
    }

    void destroyWorld(World& w) override {

    }

    void startGame(World& w) override {
        assert(!gameRunning);
        gameRunning = true;
        // TODO: iterate world ComponentManagers
        for(auto* system : systemManager->getSystems()) {
            system->startGame();
        }
    }

    void stopGame(World& w) override {
        assert(gameRunning);
        gameRunning = false;
        // TODO: iterate world ComponentManagers
        for(auto* system : systemManager->getSystems()) {
            system->stopGame();
        }
    }

    bool isPaused() const override {
        return paused;
    }

    void pause(bool pause) override {
        paused = pause;
    }

    void nextFrame() override {
        processNextFrame = true;
    }

    void setTimeScale(float mul) override {
        timeScale = std::max(mul, 0.001f);
    }


    void calculateSmoothedDeltas() override {
        std::array<float, 11> temp {};
        std::memcpy(temp.data(), deltas.data(), temp.size());
        std::qsort(temp.data(), temp.size(), sizeof(temp[0]), [](const void* a, const void* b) -> int {
            return *(const float*)a < *(const float*)b ? -1 : *(const float*) a > *(const float*)b ? 1 : 0;
        });

        float t = 0;
        for (size_t i = 2; i < temp.size() - 2; i++)
            t += temp[i];

        smoothTimeDelta = t / (temp.size() - 4);
        static size_t counter = SH::Profiler::MakeCounter("Smooth Time Delta (ms)", 0);
        SH::Profiler::PushCounter(counter, smoothTimeDelta * 1000);
    }

    void update(World& w) override {
        // TODO: End frame on world component managers

        ProfileFunction();

        #ifdef _WIN32
        const float memory = Platform::GetProcessMemory() / (1024 * 1024);
        static size_t processMemoryCounter = SH::Profiler::MakeCounter("Process Memory (MB) ", 0);
        SH::Profiler::PushCounter(processMemoryCounter, memory);
        #endif

        float deltaT = timer.swap() * timeScale;
        if (processNextFrame) deltaT = 1 / 30.f;

        lastTimeDeltaFrame++;
        deltas[lastTimeDeltaFrame % deltas.size()] = deltaT;

        static size_t deltaCounter = SH::Profiler::MakeCounter("Raw Frame Time (ms)", 0);
        SH::Profiler::PushCounter(deltaCounter, deltaT * 1000);

        calculateSmoothedDeltas();

        if (!paused || processNextFrame) {
            // TODO: Update world component managers
            systemManager->update(deltaT);
        }

        // TODO: input?
        fileSystem->processCallbacks();
        processNextFrame = false;
    }

    SystemManager& getSystemManager() override { return *systemManager; }
    FileSystem& getFileSystem() override { return *fileSystem; }
    ResourceManager& getResourceManager() override { return resourceManager; }

    float getLastTimeDelta() const override {
        return smoothTimeDelta / timeScale;
    }

  private:

    std::unique_ptr<FileSystem> fileSystem;
    ResourceManager resourceManager;
    std::unique_ptr<SystemManager> systemManager;
    PrefabManager prefabManager;
    SH::Timer timer;

    float timeScale;
    std::array<float, 11> deltas = {};
    size_t lastTimeDeltaFrame;
    float smoothTimeDelta;
    bool gameRunning;
    bool paused;
    bool processNextFrame;

    FileOutput logFile;
    bool logFileOpen = false;
  };

  std::unique_ptr<Engine> Engine::create(ShadowEngine::Engine::Initialization &&init) {
      return std::make_unique<EngineImpl>(static_cast<Engine::Initialization&&>(init));
  }
}