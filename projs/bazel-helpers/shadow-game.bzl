
load("//projs/bazel-helpers:windows_dll_library.bzl", "windows_dll_library")
load("//projs/bazel-helpers:copy_to_directory.bzl", "copy_to_directory")

load("//projs/bazel-helpers:copy_utils.bzl", "copy_paths", "copy_to_dir_cmd", "copy_to_dir_bash")
load("@bazel_skylib//lib:paths.bzl", _paths = "paths")

script_template = """\
Game: {{
    dll: {game_name}_lib.dll
}}
"""

def wrap(ctx):

    name = ctx.label.name
    subdir = name + "/"

    launcher = ctx.actions.declare_file(subdir + ctx.label.name + "_0.exe")
    ctx.actions.run(
        outputs = [launcher],
        inputs = ctx.attr._launcher.files.to_list(),
        executable = ctx.attr._copy.files.to_list()[0],
        arguments = [launcher.path, ctx.attr._launcher.files.to_list()[0].path],
        mnemonic = "CopyLauncher",
    )
    return launcher

def _foo_binary_impl(ctx):
    
    ##################################################################################
    ############################# Copy files to output ###############################
    ##################################################################################

    #if not ctx.attr.srcs and not ctx.attr.prefix_mapped_srcs:
    #    fail("srcs and prefix_mapped_srcs must not be empty in copy_to_directory %s" % ctx.label)

    output = ctx.actions.declare_directory(ctx.attr.name+"-package")

    # Gather a list of src_path, dst_path pairs
    copy_paths_list = []

    #for target, prefix_map in ctx.attr.prefix_mapped_srcs.items():
    #    parts = prefix_map.split(":")
    #    if 2 != len(parts):
    #        fail("prefix_map %s for label %s is malformed" % (prefix_map, target))
    #    files = target[DefaultInfo].files.to_list()
    #    for src_file in files:
    #        src_path, output_path, src_file = copy_paths(ctx, src_file, parts[0], parts[1])
    #        if None != src_path:
    #            dst_path = _paths.normalize("/".join([output.path, output_path]))
    #            copy_paths.append((src_path, dst_path, src_file))

    for src_file in ctx.files.srcs:
        src_path, output_path, src_file = copy_paths(ctx, src_file)
        if None != src_path:
            dst_path = _paths.normalize("/".join([output.path, output_path]))
            copy_paths_list.append((src_path, dst_path, src_file))

    for src_file in ctx.files._engine:
        src_path, output_path, src_file = copy_paths(ctx, src_file)
        if None != src_path:
            dst_path = _paths.normalize("/".join([output.path, output_path]))
            copy_paths_list.append((src_path, dst_path, src_file))

    #exe = ctx.actions.declare_file(ctx.attr.name+"/shadow-runtime.exe")

    res = []

    if ctx.attr.is_windows:
        res = copy_to_dir_cmd(ctx, copy_paths_list, output)
    else:
        res = copy_to_dir_bash(ctx, copy_paths_list, output, ctx.attr.allow_symlink)



    exe = wrap(ctx)


    script = ctx.actions.declare_file("%s-start" % ctx.label.name)
    script_content = script_template.format(
        game_name = ctx.label.name,
    )
    ctx.actions.write(script, script_content)

    # executable = res[0]["file"]
    return [
        DefaultInfo(files = depset([output]), executable = exe ),
    ]
    

shadow_game_bundle = rule(
    implementation = _foo_binary_impl,
    executable = True,
    attrs = {
        "deps": attr.label_list(),

        "allow_symlink": attr.bool(default = False),
        "downcase": attr.bool(default = False),
        "is_windows": attr.bool(mandatory = True),
        "prefix_mapped_srcs": attr.label_keyed_string_dict(allow_files = True),
        "replace_prefixes": attr.string_dict(default = {}),
        "srcs": attr.label_list(allow_files = True),

        "_engine": attr.label(default = Label("//projs/shadow/shadow-runtime")),
        "_launcher": attr.label(default = Label("//projs/bazel-helpers/wraper:launcher.exe")),
        "_copy": attr.label(default = Label("//projs/bazel-helpers/copy")),
        #"lib": attr.label(default = Label("//projs/shadow/shadow-runtime")),
    },
)

def shadow_game(
        name,
        srcs = [],
        deps = [],
        hdrs = [],
        visibility = None,
        prefix_mapped_srcs = {},
        **kwargs):

    _is_windows = select({
        "@bazel_tools//src/conditions:host_windows": True,
        "//conditions:default": False,
    })

    shadow_game_bundle(
        name = name,
        is_windows = _is_windows,
        #lib = name + "_lib",

        srcs = [
            name + "_lib.dll"
        ],
        prefix_mapped_srcs = prefix_mapped_srcs
    )


    #copy_to_directory(
    #    name = name + "-game",
    #    srcs = [
    #        "//projs/shadow/shadow-runtime",
    #    ],
    #)

    windows_dll_library(
        name = name + "_lib",
        srcs = srcs,
        hdrs = hdrs,
        # Define COMPILING_DLL to export symbols during compiling the DLL.
        copts = ["/DCOMPILING_DLL"],
    )