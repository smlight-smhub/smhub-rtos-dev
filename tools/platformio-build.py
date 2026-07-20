#
# Copyright (c) 2026 SMLIGHT
# All rights reserved.
#
# PlatformIO build script for framework-sg2000-rtos.
#
from os.path import join

Import("env")
platform = env.PioPlatform()

FRAMEWORK_DIR = platform.get_package_dir("framework-sg2000-rtos")
assert FRAMEWORK_DIR

import os
import glob
from os.path import join

build_dir = env.subst("$BUILD_DIR")
gen_metal_dir = join(build_dir, "generated", "metal")
os.makedirs(gen_metal_dir, exist_ok=True)

# --- Generate Nanopb RPC files ---
proto_file = join(FRAMEWORK_DIR, "proto", "rpc.proto")
proto_dir = join(FRAMEWORK_DIR, "proto")
gen_dir = join(FRAMEWORK_DIR, "src", "gen")
os.makedirs(gen_dir, exist_ok=True)
nanopb_gen = join(FRAMEWORK_DIR, "lib", "nanopb", "generator", "nanopb_generator.py")
if os.path.exists(proto_file):
    project_dir = env.subst("$PROJECT_DIR")
    venv_python = join(project_dir, ".venv", "bin", "python")
    if not os.path.exists(venv_python):
        venv_python = env.subst("$PYTHONEXE") if env.subst("$PYTHONEXE") else "python3"
    env.Execute(
        f"{venv_python} {nanopb_gen} -I {proto_dir} --output-dir={gen_dir} {proto_file}"
    )

def get_version(filepath):
    v = {"MAJOR": "1", "MINOR": "0", "PATCH": "0"}
    if os.path.exists(filepath):
        with open(filepath, "r") as f:
            for line in f:
                if "VERSION_MAJOR" in line: v["MAJOR"] = line.split("=")[1].strip()
                if "VERSION_MINOR" in line: v["MINOR"] = line.split("=")[1].strip()
                if "VERSION_PATCH" in line: v["PATCH"] = line.split("=")[1].strip()
    return v

libmetal_v = get_version(join(FRAMEWORK_DIR, "lib", "libmetal", "VERSION"))

replacements = {
    "@PROJECT_SYSTEM@": "generic",
    "@PROJECT_SYSTEM_UPPER@": "GENERIC",
    "@PROJECT_PROCESSOR@": "riscv",
    "@PROJECT_PROCESSOR_UPPER@": "RISCV",
    "@PROJECT_MACHINE@": "cv181x",
    "@PROJECT_MACHINE_UPPER@": "CV181X",
    "@PROJECT_VERSION_MAJOR@": libmetal_v["MAJOR"],
    "@PROJECT_VERSION_MINOR@": libmetal_v["MINOR"],
    "@PROJECT_VERSION_PATCH@": libmetal_v["PATCH"],
    "@PROJECT_VERSION@": f"{libmetal_v['MAJOR']}.{libmetal_v['MINOR']}.{libmetal_v['PATCH']}",
    "#cmakedefine HAVE_STDATOMIC_H": "#define HAVE_STDATOMIC_H 1",
    "#cmakedefine HAVE_FUTEX_H": "/* #undef HAVE_FUTEX_H */",
    "#cmakedefine HAVE_PROCESSOR_ATOMIC_H": "/* #undef HAVE_PROCESSOR_ATOMIC_H */",
    "#cmakedefine HAVE_PROCESSOR_CPU_H": "/* #undef HAVE_PROCESSOR_CPU_H */",
}

metal_src = join(FRAMEWORK_DIR, "lib", "libmetal", "lib")
if os.path.exists(metal_src):
    for filepath in glob.glob(join(metal_src, "**/*.h"), recursive=True):
        rel_path = os.path.relpath(filepath, metal_src)
        dest_path = join(gen_metal_dir, rel_path)
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)

        with open(filepath, "r") as f:
            content = f.read()

        for old, new in replacements.items():
            if old in content:
                content = content.replace(old, new)

        if os.path.exists(dest_path):
            with open(dest_path, "r") as f:
                if f.read() == content:
                    continue

        with open(dest_path, "w") as f:
            f.write(content)

# --- Generate OpenAMP Version Header ---
openamp_v_in = join(FRAMEWORK_DIR, "lib", "open-amp", "lib", "version.h.in")
openamp_v_out = join(build_dir, "generated", "version_def.h")

if os.path.exists(openamp_v_in):
    with open(openamp_v_in, "r") as f:
        content = f.read()
    for old, new in replacements.items():
        if old in content:
            content = content.replace(old, new)

    if not (os.path.exists(openamp_v_out) and open(openamp_v_out).read() == content):
        with open(openamp_v_out, "w") as f:
            f.write(content)

env.Append(
    CPPPATH=[
        join(build_dir, "generated"),
        join(FRAMEWORK_DIR, "include"),
        join(FRAMEWORK_DIR, "src"),
        join(FRAMEWORK_DIR, "lib", "smhub-hal", "include"),
        join(FRAMEWORK_DIR, "lib", "smhub-hal", "hal", "cv181x", "include"),
        join(FRAMEWORK_DIR, "lib", "smhub-hal", "common", "include"),
        join(FRAMEWORK_DIR, "lib", "smhub-hal", "common", "include", "riscv64"),
        join(FRAMEWORK_DIR, "lib", "smhub-hal", "driver", "include"),
        join(FRAMEWORK_DIR, "lib", "smhub-hal", "arch", "include"),
        join(FRAMEWORK_DIR, "lib", "FreeRTOS-Kernel", "include"),
        join(FRAMEWORK_DIR, "lib", "FreeRTOS-Kernel", "portable", "GCC", "RISC-V"),
        join(FRAMEWORK_DIR, "lib", "open-amp", "lib", "include"),
        join(FRAMEWORK_DIR, "lib", "nanopb"),
    ],
    CPPDEFINES=[
        "CONFIG_64BIT",
        "METAL_INTERNAL",
        ("DEFAULT_LOGGER_ON", "0"),
        ("VQ_RX_EMPTY_NOTIFY", "0"),
        ("VIRTIO_DRIVER_SUPPORT", "1"),
        ("VIRTIO_DEVICE_SUPPORT", "1"),
        ("VIRTIO_USE_DCACHE", "1"),
    ],
    CXXFLAGS=["-std=gnu++20"],
)

env.BuildSources(
    join("$BUILD_DIR", "FrameworkFreeRTOS"),
    join(FRAMEWORK_DIR, "src"),
    src_filter=["+<*.c>", "+<gen/*.c>"],
)

env.BuildSources(
    join("$BUILD_DIR", "FrameworkHAL"),
    join(FRAMEWORK_DIR, "lib", "smhub-hal"),
    src_filter=[
        "+<common/src/*.c>",
        "+<driver/src/*.c>",
        "+<hal/cv181x/src/*.c>",
        "+<arch/src/*.S>",
        "+<arch/src/*.c>",
    ],
)

env.BuildSources(
    join("$BUILD_DIR", "FrameworkFreeRTOS-Kernel"),
    join(FRAMEWORK_DIR, "lib", "FreeRTOS-Kernel"),
    src_filter=[
        "+<*.c>",
        "+<portable/MemMang/heap_4.c>",
        "+<portable/GCC/RISC-V/port.c>",
        "+<portable/GCC/RISC-V/portASM.S>",
    ],
)

env.BuildSources(
    join("$BUILD_DIR", "FrameworkLibmetal"),
    join(FRAMEWORK_DIR, "lib", "libmetal", "lib"),
    src_filter=[
        "+<*.c>",
        "-<system/*>",
        "+<system/generic/*.c>",
        "-<processor/*>",
        "+<processor/riscv/*.c>",
    ],
)

env.BuildSources(
    join("$BUILD_DIR", "FrameworkOpenAMP"),
    join(FRAMEWORK_DIR, "lib", "open-amp", "lib"),
)

env.BuildSources(
    join("$BUILD_DIR", "FrameworkNanopb"),
    join(FRAMEWORK_DIR, "lib", "nanopb"),
    src_filter=[
        "-<*>",
        "+<pb_encode.c>",
        "+<pb_decode.c>",
        "+<pb_common.c>",
    ],
)
