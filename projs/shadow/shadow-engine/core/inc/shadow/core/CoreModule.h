#pragma once

#include "Module.h"
#include "system/EngineSystem.h"

class CoreModule : public SH::Module, public ShadowEngine::EngineSystem {
  SHObject_Base(CoreModule)
  public:
    CoreModule() : Module() {}

    void PreInit() override {

    }

    void Init() override {

    }

    void Update(int frame) override {

    }

    void Destroy() override {

    }

    const char* getName() const override { return "Core"; }
    virtual void serialise(ShadowEngine::OutputMemoryStream& serializer) const {}
    virtual bool deserialize(uint32_t version, ShadowEngine::InputMemoryStream& serializer) { return true; }

};
