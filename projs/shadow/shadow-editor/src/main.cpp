#include "shadow/core/ShadowApplication.h"
#include "../inc/EditorModule.h"

#include <iostream>
#include <iomanip>
#include <cstdlib>

int main(int argc, char *argv[]) {
    std::cout << "argc == " << argc << '\n';

    for (int ndx{}; ndx != argc; ++ndx) {
        std::cout << "argv[" << ndx << "] == " << std::quoted(argv[ndx]) << '\n';
    }
    std::cout << "argv[" << argc << "] == " << static_cast<void *>(argv[argc]) << '\n';
    /*...*/

    SH::ShadowApplication app(argc, argv);
    app.GetModuleManager().AddAssembly({
        .id="assembly:/shadow-editor",
        .path="shadow-editor",
        .type=SH::AssemblyType::EXE});
    app.GetModuleManager().LoadModulesFromAssembly("assembly:/shadow-editor");
    app.Init();
    app.Start();

    return argc == 3 ? EXIT_SUCCESS : EXIT_FAILURE; // optional return value
}

extern "C" {
void EXPORT assembly_entry(SH::ModuleManager &m) {
    m.AddDescriptors({
                         .id="module:/editor",
                         .name = "Editor",
                         .class_name = "EditorModule",
                         .assembly="assembly:/shadow-editor",
                         .dependencies={"module:/platform/sdl2"},
                     });
}
}