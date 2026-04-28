#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
from pathlib import Path
from typing import Any, Dict, List
from typing import Iterator, Optional

from tools.project import (
    Library,
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0
VERSIONS = ["G4QE01"]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20251118"
config.dtk_tag = "v1.8.0"
config.objdiff_tag = "v3.5.1"
config.sjiswrap_tag = "v1.2.2"
config.wibo_tag = "0.7.0"

# Project
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "--strip-local-absolute",
    "-I include",
    "-I src",
    f"-I build/{config.version}/include",
    f"--defsym BUILD_VERSION={version_num}",
]
config.ldflags = [
    "-fp hardware",
    "-nodefaults",
    "-warn off",
]
if args.debug:
    config.ldflags.append("-g")  # Or -gdwarf-2 for Wii linkers
if args.map:
    config.ldflags.append("-mapunused")
    # config.ldflags.append("-listclosure") # For Wii linkers

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Progress
config.progress_use_fancy = True
config.progress_code_fancy_frac = 293
config.progress_code_fancy_item = "Trophies"
config.progress_data_fancy_frac = 51
config.progress_data_fancy_item = "Event Matches"

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = None

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.

cflags_base = [
    "-nowraplines",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-cwd source",
    "-Cpp_exceptions off",
    "-fp_contract on",
    "-nosyspath",
    "-O4,p",
    "-multibyte",
    "-nodefaults",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-RTTI off",
    "-str reuse",
    "-sym on",
    "-use_lmw_stmw on",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
    '-pragma "supress_warnings on"',
]

# Debug flags
if args.debug:
    # Or -sym dwarf-2 for Wii compilers
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

# Metrowerks library flags
cflags_runtime = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-str reuse,readonly",
    # "-gccinc",
    "-common off",
    "-inline auto",
]

cflags_runtime_MSL_C = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    "-inline auto, deferred",
    "-maxerrors 1",
    "-nosyspath",
    "-fp_contract off",
    "-use_lmw_stmw on",
    "-multibyte",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-RTTI off",
    "-char signed",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-common off",
    f"-i build/{config.version}/include",
    f"-DBUILD_VERSION={version_num}",
    f"-DVERSION_{config.version}",
]

cflags_dolphin = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-char signed",
    # "-O4,p",
    "-sym on",
    "-inline auto",
    f"-DVERSION={version_num}",
    "-D__GEKKO__",
    "-DSDK_REVISION=2",
    # "-DSDK_REVISION=1",
]

cflags_ode = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-O4,p",
    # "-inline off",
    "-inline auto, deferred",
    "-maxerrors 1",
    "-nosyspath",
    "-fp_contract on",
    "-multibyte",
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-RTTI off",
    "-char signed",
    "-use_lmw_stmw on",
    # "-str reuse,pool,readonly",
    "-common off",
    "-cwd source",
    "-str reuse",
    "-sym on"
]

cflags_musyx = [
    "-proc gekko",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i extern/musyx/include",
    # "-i libc",
    "-inline auto,depth=4",
    # "-inline auto",
    "-O4,p",
    "-fp hard",
    "-enum int",
    "-sym on",
    "-Cpp_exceptions off",
    "-str reuse,pool,readonly",
    "-fp_contract off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

cflags_musyx_debug = [
    "-proc gecko",
    "-fp hard",
    "-nodefaults",
    "-nosyspath",
    "-i include",
    "-i extern/musyx/include",
    # "-i libc",
    "-g",
    "-sym on",
    "-D_DEBUG=1",
    "-fp hard",
    "-enum int",
    "-Cpp_exceptions off",
    "-DMUSY_TARGET=MUSY_TARGET_DOLPHIN",
]

# # REL flags
# cflags_rel = [
#     *cflags_base,
#     "-sdata 0",
#     "-sdata2 0",
# ]

cflags_nl = [
    *cflags_base,
    "-proc gekko",
    "-nodefaults",
    "-nosyspath",
    "-i include",
]

cflags_odemuexi = [
    *cflags_base,
    "-inline deferred"
]

cflags_trk_minnow_dolphin = [
    *cflags_base,
    "-use_lmw_stmw on",
    "-rostr",
    "-str reuse",
    "-gccinc",
    "-common off",
    "-inline deferred",
    "-char signed",
    "-sdata 0",
    "-sdata2 0",   
    "-sdatathreshold 0",
]

includes_base = [
    "include",
    # "include/libc",
    "include/PowerPC_EABI_Support/MSL_C/MSL_Common/", #instead of libc, which is a copy of it...
]

system_includes_base = [
    "include",
    f"build/{config.version}/include",
]

config.linker_version = "GC/1.3.2"

Objects = List[Object]


def Lib(
    lib_name: str,
    objects: Objects,
    mw_version: str = config.linker_version,
    cflags=cflags_base,
    fix_epilogue=True,
    fix_trk=False,
    includes: List[str] = includes_base,
    system_includes: List[str] = system_includes_base,
    src_dir: Optional[str] = None,
    category: Optional[str] = None,
) -> Library:
    def make_includes(includes: List[str]) -> Iterator[str]:
        return map(lambda s: f"-i {s}", includes)

    lib = {
        "lib": lib_name,
        # "mw_version": f"GC/1.2.5{'n' if fix_epilogue else ''}",
        "mw_version": mw_version, 
        "cflags": [
            *cflags,
            *make_includes(includes),
            "-I-",
            *make_includes(system_includes),
        ],
        "host": False,
        "progress_category": category,
        "objects": objects,
    }

    if src_dir is not None:
        lib["src_dir"] = src_dir

    return lib


def RuntimeLib(lib_name: str, objects: Objects) -> Library:
    return Lib(
        lib_name,
        objects,
        cflags=cflags_runtime,
        fix_epilogue=False,
        category="runtime",
    )


def RuntimeLib_MSL_C(lib_name: str, objects: Objects) -> Library:
    return Lib(
        lib_name,
        objects,
        mw_version="GC/2.5",
        cflags=[
            *cflags_runtime_MSL_C,
            "-fp_contract on", 
            "-inline auto,deferred", 
            "-str pool,readonly"
        ],
        category="runtime",
    )

def NLLib(lib_name: str, objects: Objects) -> Library:
    return Lib(
        lib_name,
        objects,
        includes=[
            *includes_base,
            "src/ode",
            "extern/musyx/include",
        ],
        system_includes=[
            *system_includes_base,
        ],
        mw_version="GC/2.0",
        # mw_version="GC/1.3.2",
        # mw_version="GC/1.2.5n",
        cflags=[
            *cflags_base,
            "-pool off",
            "-DdNODEBUG=ON",
            "-DdIDESINGLE",
            "-DdSINGLE=1",
            "-DdTHREADING_INTF_DISABLED",
            "-DHAVE_MALLOC_H=1",
            "-DdODE_SMStricker_Patch"
        ],            
        category="game",
    )

def GameLib(lib_name: str, objects: Objects) -> Library:
    return Lib(
        lib_name,
        objects,
        includes=[
            *includes_base,
            "src/ode",
            "extern/musyx/include",
        ],
        system_includes=[
            *system_includes_base,
        ],
        mw_version="GC/2.5",
        # mw_version="GC/2.0",
        # mw_version="GC/1.3.2",
        # mw_version="GC/1.2.5n",
        cflags=[
            *cflags_base,
            "-pool off",
            "-DdNODEBUG=ON",
            "-DdIDESINGLE",
            "-DdSINGLE=1",
            "-DdTHREADING_INTF_DISABLED",
            "-DHAVE_MALLOC_H=1",
            "-DdODE_SMStricker_Patch"
            "-DMUSY_VERSION_MAJOR=2",
            "-DMUSY_VERSION_MINOR=0",
            "-DMUSY_VERSION_PATCH=3",
            # '-pragma "supress_warnings on"',
        ],            
        category="game",
    )

def GameLib13(lib_name: str, objects: Objects) -> Library:
    return Lib(
        lib_name,
        objects,
        includes=[
            *includes_base,
            "src/ode",
            "extern/musyx/include",
        ],
        system_includes=[
            *system_includes_base,
        ],
        # mw_version="GC/2.0",
        mw_version="GC/1.3",
        # mw_version="GC/1.2.5n",
        cflags=[
            *cflags_base,
            "-pool off",
            "-DdNODEBUG=ON",
            "-DdIDESINGLE",
            "-DdSINGLE=1",
            "-DdTHREADING_INTF_DISABLED",
            "-DHAVE_MALLOC_H=1",
            "-DdODE_SMStricker_Patch"
            "-DMUSY_VERSION_MAJOR=2",
            "-DMUSY_VERSION_MINOR=0",
            "-DMUSY_VERSION_PATCH=3",
        ],            
        category="game",
    )


def ODELib(lib_name: str, objects: Objects, cflags=cflags_ode) -> Library:
    return Lib(
        lib_name,
        objects,
        includes=[
            *includes_base,
            "include/ode",
        ],
        system_includes=[
            *system_includes_base,
            "include/PowerPC_EABI_Support/MSL_C++/MSL_Common/",
        ],
        # mw_version="GC/1.3.2",
        mw_version="GC/2.0",
        cflags=[
            *cflags,
            "-DdNODEBUG=ON",
            "-DdIDESINGLE",
            "-DdSINGLE=1",
            "-DdTHREADING_INTF_DISABLED",
            "-DHAVE_MALLOC_H=1",
        ],        
        category="third_party",
    )


def DolphinLib_O3(lib_name: str, objects: Objects, cflags=cflags_dolphin) -> Library:
    return Lib (
        lib_name,
        objects,
        includes=[
            *includes_base,
            "src/Dolphin",
        ],
        system_includes=[
            *system_includes_base,
            "include/Dolphin",
        ],
        mw_version="GC/1.2.5n",
        cflags=[
            *cflags,
            "-O3,p",
        ],        
        category="sdk",
    )

def DolphinLib(lib_name: str, objects: Objects, cflags=cflags_dolphin) -> Library:
    return Lib (
        lib_name,
        objects,
        includes=[
            *includes_base,
            "src/Dolphin",
        ],
        system_includes=[
            *system_includes_base,
            "include/Dolphin",
        ],
        mw_version="GC/1.2.5n",
        # mw_version="GC/1.3.2",
        cflags=[
            *cflags,
            "-O4,p",
        ],        
        category="sdk",
    )

def DolphinLib132(lib_name: str, objects: Objects, cflags=cflags_dolphin) -> Library:
    return Lib (
        lib_name,
        objects,
        includes=[
            *includes_base,
            "src/Dolphin",
        ],
        system_includes=[
            *system_includes_base,
            "include/Dolphin",
        ],
        # mw_version="GC/1.2.5n",
        mw_version="GC/1.3.2",
        cflags=[
            *cflags,
            "-O4,p",
        ],        
        category="sdk",
    )


def DolphinTrkLib(lib_name: str, objects: Objects, cflags=cflags_trk_minnow_dolphin) -> Library:
    src_dir = None
    includes = includes_base
    system_includes = system_includes_base
    return Lib(
        lib_name,
        objects,
        mw_version="GC/1.3.2",
        src_dir=src_dir,
        cflags=cflags,
        includes=includes,
        system_includes=system_includes,
        category="sdk",
    )

def MusyxLib(lib_name: str, objects: Objects, debug=False, major=2, minor=0, patch=3) -> Library:
    cflags = cflags_musyx if not debug else cflags_musyx_debug
    return Lib (
        lib_name,
        objects,
        src_dir="extern/musyx/src",
        includes=[
            *includes_base,
            "src/Dolphin",
        ],
        mw_version="GC/1.3.2",

        cflags=[
            *cflags,
            f"-DMUSY_VERSION_MAJOR={major}",
            f"-DMUSY_VERSION_MINOR={minor}",
            f"-DMUSY_VERSION_PATCH={patch}",
        ],
        category="third_party",
    )


Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False


config.libs = [
    RuntimeLib(
        "Runtime.PPCEABI.H",
        [
            Object(Matching, "PowerPC_EABI_Support/Runtime/__init_cpp_exceptions.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/Runtime/__mem.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/Runtime/__va_arg.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/Runtime/global_destructor_chain.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/Runtime/ptmf.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/Runtime/runtime.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/Runtime/Gecko_ExceptionPPC.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/Runtime/NMWException.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "PowerPC_EABI_Support/Runtime/GCN_mem_alloc.c", extra_cflags=["-inline auto"]),
        ],
    ),
    RuntimeLib_MSL_C(
        "MSL_C.PPCEABI.bare.H",
        [
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/extras.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/alloc.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/ansi_files.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/abort_exit.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/errno.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/ansi_fp.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/arith.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/buffer_io.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/ctype.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/locale.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/direct_io.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/mbstring.c", extra_cflags=["-inline auto", "-inline noauto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/mem.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/mem_funcs.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/misc_io.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/printf.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/scanf.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/string.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/strtold.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/strtoul.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/char_io.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/wchar_io.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/float.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/signal.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/file_io.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/file_pos.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/qsort.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/wctype.c", extra_cflags=["-inline auto"]),

            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/wcstoul.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/wscanf.c", extra_cflags=["-inline auto"]),

            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/e_acos.c", extra_cflags=["-inline auto", "-inline off"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/e_fmod.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/e_pow.c", extra_cflags=["-inline auto", "-inline off"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/e_rem_pio2.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/k_cos.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/k_rem_pio2.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/k_sin.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/k_tan.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_atan.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_ceil.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_copysign.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_floor.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_frexp.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_ldexp.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_modf.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_sin.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/s_tan.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/w_acos.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/w_fmod.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/w_pow.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/e_sqrt.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/MSL_Common/w_sqrt.c", extra_cflags=["-inline auto"]),

            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/uart_console_io_gcn.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/critical_regions.gamecube.c", extra_cflags=["-inline auto"]),
            Object(Matching, "PowerPC_EABI_Support/MSL/MSL_C/PPC_EABI/math_ppc.c", extra_cflags=["-inline auto"]),
        ],
    ),
    GameLib(
        "SMS (Super Mario Strikers)",
        [
            Object(NonMatching, "Game/main.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/ComUpdateTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/FrontEndTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/GameRenderTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/Sys/clock.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Sys/debug.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/Sys/simpleparser.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Sys/eventman.cpp", extra_cflags=["-inline auto", "-O4 -inline deferred"]),
            Object(Matching, "Game/Sys/geventdst.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Sys/tweak.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Sys/FloatingPointExceptions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Sys/CallStackDumper.cpp", extra_cflags=["-inline auto", "-inline deferred"]),    
            Object(NonMatching, "Game/Core/mtRandom.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(NonMatching, "Game/Sys/PlatStream.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Sys/GCStream.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Sys/movie.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Sys/THPSimple.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Sys/gcmemcard.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Game Objects
            Object(NonMatching, "Game/Game.cpp", extra_cflags=["-inline auto", "-inline deferred"]),    
            Object(NonMatching, "Game/GameInfo.cpp", extra_cflags=["-inline auto", "-inline deferred"]),    
            Object(NonMatching, "Game/GameTweaks.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/CharacterTweaks.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/ScriptTuning.cpp", extra_cflags=["-inline auto", "-inline auto,deferred"]),

            # Game/Transitions
            Object(NonMatching, "Game/Transitions/ScreenTransitionManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Transitions/ColourBlendScreenTransition.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Transitions/ScriptedTransition.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Transitions/TransLight.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Transitions/TransitionSequence.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Transitions/ModelTransition.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Camera
            Object(Matching, "Game/CameraLoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Camera/CameraMan.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Camera/FaceCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Camera/GoalCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Camera/ShootToScoreCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Camera/TopDownCamera.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Camera/kickoffcam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Camera/MatrixEffectCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Camera/FollowCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Camera/ReplayCamera.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Camera/AnimViewerCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Camera/rumblefilter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Camera/DebugCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Camera/GameplayCam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Camera/animcam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(NonMatching, "Game/Replay.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/ReplayChoreo.cpp", extra_cflags=["-inline auto", "-inline deferred"]),    
            Object(NonMatching, "Game/ReplayManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/RenderSnapshot.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(NonMatching, "Game/Ball.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/Net.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Field.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Character.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/CharacterTemplate.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/CharacterEffects.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/Player.cpp", extra_cflags=["-inline auto"]),

            Object(NonMatching, "Game/Goalie.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/GoalieFatigue.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(NonMatching, "Game/Team.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Formation.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FormationDefines.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/TweaksBase.cpp", extra_cflags=["-inline auto"]),

            Object(NonMatching, "Game/NisPlayer.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Game Render
            Object(NonMatching, "Game/Render/FlareHandler.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/Nis.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Render/CameraGuy.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/SkinAnimatedNPC.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Render/depthoffield.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Render/Wiper.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/Bowser.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/ChainChomp.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Render/GraphicsLoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/Indicators.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/ShootToScoreArrow.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/Render/ShootToScoreMeter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/Jumbotron.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Render/SkinAnimatedMovableNPC.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/NPCManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/SidelineExplodable.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/ElectricFence.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # related to character (should probably be in Game/Render)
            Object(Matching, "Game/PoseNode.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/PoseAccumulator.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SHierarchy.cpp", extra_cflags=["-inline auto"]),

            Object(NonMatching, "Game/SAnim.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SAnim/AnimRetargeter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SAnim/pnSAnimController.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SAnim/pnBlender.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SAnim/pnSingleAxisBlender.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SAnim/pnFeather.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AnimInventory.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Blinker.cpp", extra_cflags=["-inline auto"]),   

            # Net 
            Object(NonMatching, "Game/Render/NetMesh.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/NetMeshEdge.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/NetMeshModelLoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Terrain / Environment
            Object(Matching, "Game/TrophyInfo.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/TerrainTypes.cpp", extra_cflags=["-inline auto"]),

            # Audio
            Object(NonMatching, "Game/Audio/CharacterAudio.cpp", extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/GameAudio.cpp", extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/audio.cpp", extra_cflags=["-inline auto"]),   
            Object(Matching, "Game/Audio/SebringSoundDefines.cpp" , extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/SoundEventScript.cpp" , extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/WorldAudio.cpp", extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/AudioEventHandler.cpp", extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/AudioLoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/AudioScriptEventMgr.cpp", extra_cflags=["-inline auto", "-inline deferred"]),  
            Object(NonMatching, "Game/Audio/AudioStream.cpp", extra_cflags=["-inline auto", "-inline deferred"]),  
            Object(NonMatching, "Game/Audio/CrowdMood.cpp", extra_cflags=["-inline auto", "-inline deferred"]),  
            Object(NonMatching, "Game/Audio/StreamTrack.cpp", extra_cflags=["-inline auto", "-inline deferred"]),   
            Object(NonMatching, "Game/Audio/PriorityStream.cpp", extra_cflags=["-inline auto", "-inline deferred"]),   

            # Physics
            Object(NonMatching, "Game/Physics.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/RayCollider.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/PhysicsAIBall.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/Physics/PhysicsBox.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/PhysicsShell.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/PhysicsColumn.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/PhysicsCapsule.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/PhysicsFakeBall.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/Physics/PhysicsBall.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/PhysicsSphere.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/PhysicsFinitePlane.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/PhysicsGroundPlane.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/PhysicsPlane.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/PhysicsRoundedCorner.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/PhysicsWall.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Equivalent, "Game/PhysicsNPC.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/PhysicsObject.cpp", extra_cflags=["-inline auto", "-inline noauto, deferred"]),
            Object(NonMatching, "Game/PhysicsCompositeObject.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/Physics/PhysicsCharacterBase.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Physics/PhysicsCharacter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Physics/CollisionSpace.cpp" , extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/PhysicsTransform.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Physics/PhysicsWorld.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Physics/LoadablePhysicsMesh.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Physics/CharacterPhysicsElement.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Physics/PhysicsNet.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Physics/PhysicsBanana.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Physics/PhysicsGoalie.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # World
            Object(NonMatching, "Game/World.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/WorldManager.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/BasicStadium.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/World/WorldLoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/World/worldanim.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Pad
            Object(Matching, "Game/Pad/FlickDetection.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Tasks
            Object(Matching, "Game/WorldUpdateTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/ParticleUpdateTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/TweakerTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/TestTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/ProfileTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/PlatPadUpdateTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FixedUpdateTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/DispatchEventsTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/ResetTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/TransitionTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/BeginFrameTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/EndFrameTask.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Triggers
            Object(Matching, "Game/Triggers/BinaryTriggerFile.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/MarioTriggers.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/WorldTriggers.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/CharacterTriggers.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Triggers/AnimTagScript.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Triggers/SebringAnimScript.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Triggers/AnimTrigger.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            

            Object(Matching, "Game/PadMonkey.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/PadActions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/RumbleActions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Render
            Object(NonMatching, "Game/Render/Presentation.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Equivalent, "Game/Render/CrowdManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Render/NPCLoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/RenderShadow.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/StaticModelExplodable.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Render/AnimatedModelExplodable.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/GameObjectLighting.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Font/fontmanager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            
            # AI
            Object(Matching, "Game/AI/AILoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AI/AIPlay.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/AiUtil.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Powerups.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AI/AISandbox.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/HeadTrack.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AIPad.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/DecisionEntity.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AI/ScriptAction.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AI/FilteredRandom.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/ShotMeter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/AvoidController.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Fielder.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/FielderDesires.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/FielderActions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AI/Fuzzy.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AI/FuzzyVariant.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/GoalieActions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/GoalieLooseBall.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/GoalieSave.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Scripts/CommonScript.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/SpaceSearch.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Variant.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Scripts/FormationScript.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/ScriptCaching.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Scripts/ScriptQuestions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Scripts/Plays/DefaultDefensive.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Scripts/Plays/DefaultOffensive.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Scripts/Plays/DefaultLoose.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/AI/Scripts/RootScript.cpp", extra_cflags=["-inline deferred"]),
            Object(Matching, "Game/AI/Scripts/ScriptDefines.cpp", extra_cflags=["-inline auto", "-inline deferred"]),            

            # Frontend / Scenes
            Object(Matching, "Game/GameSceneManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/BaseSceneHandler.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/BaseGameSceneManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(NonMatching, "Game/OverlayManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/OverlayHandlerHUD.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/OverlayHandlerInGameText.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/OverlayHandlerSTSX2.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/OverlayHandlerDemo.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/OverlayHandlerGoal.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/FE/feTweenFuncs.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feOptionsSubMenus.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feScene.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feRender.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/feFontResource.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/FE/feSceneResource.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/FE/fePackage.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/FE/feSoundKeyframeTrigger.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/feTextureResource.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feScrollText.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/feAsyncImage.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feAnimation.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/FELoader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feNSNMessenger.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/fePopupMenu.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/feManager.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feCaptainComponent.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/FE/feTweener.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feSceneManager.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/feLibObject.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feChooseSideComponent.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feAnimModelManager.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feButtonComponent.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feSidekickGridComponent.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/FEAudio.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/fePresentation.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/feMapMenu.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feResourceManager.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feCaptainGridComponent.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feInput.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feScrollingTicker.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/feMusic.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feInGameMessengerManager.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/feHelpFuncs.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/feSlideMenu.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/LidOpenMessage.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/BraggingRights.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/Cup/CupTickerManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/Overlay/OverlayHandlerSummary.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/FE/Overlay/OverlayHandlerWinner.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(NonMatching, "Game/FE/tlSlide.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/tlComponent.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/tlInstance.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/tlComponentInstance.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "Game/FE/tlTextInstance.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/FE/tlTextInstance_runtime.cpp", extra_cflags=["-inline auto"]),

            Object(Matching, "Game/SH/SHBackground.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHChooseCaptains.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHChooseCup.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHChooseSides.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHCredits.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHCrossFader.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHCupCheater.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHCupChooseCaptain.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHCupHub.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SH/SHCupOptions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHCupTrophy.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHHealthWarning.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHLesson.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHLessonSelect.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHLoading.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SH/SHLoadingTransition.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHMainMenu.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHMilestoneTrophy.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHMoviePlayer.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHOptions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHPause.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SH/SHPauseOptions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHPausePostGame.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHProgressiveScan.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SH/SHQuickGameplayOptions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHSaveLoad.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHSkillSelect.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHSpoils.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHStadiumSelect.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHSuperTeam.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHTitleScreen.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHTournSetParams.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/SH/SHTournTeamSetup.cpp", extra_cflags=["-inline auto", "-inline deferred"]),            

            # GFX
            Object(NonMatching, "Game/Drawable/DrawableNetMesh.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Drawable/DrawableCharacter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Drawable/DrawableBall.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/Drawable/DrawablePowerup.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Game/Drawable/DrawableExplosionFragment.cpp", extra_cflags=["-inline auto"]),
            # Object(Matching, "Game/Drawable/DrawableTmModel.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            # Object(NonMatching, "Game/Drawable/DrawableSkinModel.cpp", extra_cflags=["-inline auto"]),

            # Interpreter
            Object(NonMatching, "Game/InterpreterCore.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # GL
            Object(Matching, "Game/GL/GLMaterial.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/GL/GLTextureAnim.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/GL/GLVertexAnim.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/GL/GLMeshWriter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/GL/GLInventory.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/GL/GLRenderBuffer.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/GL/GLSkinMesh.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/GL/gluMeshWriter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/GL/gluSkinMesh.cpp", extra_cflags=["-inline auto", "-inline off"]),
            Object(NonMatching, "Game/GL/ShaderSkinMesh.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # FX
            Object(NonMatching, "Game/ObjectBlur.cpp", extra_cflags=["-inline auto"]),

            # AnimProps
            Object(Matching, "Game/AnimProps/globalanimproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/AnimProps/goalieanimproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # SoundProps
            Object(Matching, "Game/SoundProps/birdogensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/birdograsssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/birdoconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/birdometalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/birdorubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/birdowoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),    

            Object(Matching, "Game/SoundProps/bowsergensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/bowsergrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/bowserconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/bowsermetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/bowserrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/bowserwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/crittergensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/crittergrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/critterconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/crittermetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/critterrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/critterwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/critterrobotsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/daisygensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/daisygrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/daisyconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/daisymetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/daisyrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/daisywoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/dkgensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/dkgrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/dkconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/dkmetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/dkrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/dkwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/hambrosgensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/hambrosgrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/hambrosconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/hambrosmetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/hambrosrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/hambroswoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/koopagensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/koopagrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/koopaconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/koopametalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/kooparubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/koopawoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/luigigensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/luigigrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/luigiconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/luigimetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/luigirubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/luigiwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/mariogensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/mariograsssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/marioconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/mariometalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/mariorubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/mariowoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/peachgensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/peachgrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/peachmetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/peachconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/peachrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/peachwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/supergensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/supergrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/superconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/supermetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/superrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/superwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/toadgrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/toadconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/toadwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/toadrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/toadmetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/toadgensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/waluigiconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/waluigigensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/waluigigrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/waluigimetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/waluigirubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/waluigiwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/warioconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/wariogensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/wariograsssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/wariometalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/wariorubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/wariowoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/yoshiconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/yoshigensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/yoshigrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/yoshimetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/yoshirubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/yoshiwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/stadbattlesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadbowsersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadconcretesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadcratersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadgensoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadgrasssoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadkongasoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadmetalsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadpalacesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadpipesoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadrubbersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadundersoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/stadwoodsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            Object(Matching, "Game/SoundProps/crowdsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/SoundProps/powerupsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),  
            Object(Matching, "Game/SoundProps/worldsoundproperties.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Effects
            Object(Matching, "Game/Effects/efList.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Effects/EffectsTemplate.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Effects/EffectsGroup.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Effects/ParticleSystem.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Effects/EmissionController.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Effects/EmissionManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Effects/PhotoFlashEffect.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # DB
            Object(NonMatching, "Game/DB/SaveLoad.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/DB/UserOptions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/DB/StatsTracker.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/DB/Simmer.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/DB/CustomTournament.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Debug
            Object(NonMatching, "Game/Debug/FrameCounter.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Debug/ShapeRender.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Debug/TimeRegions.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Loader
            Object(Matching, "Game/Loader/LoadingManager.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
        ],
    ),
    GameLib(
        "NL (Next Level Library)",
        [
            Object(NonMatching, "NL/nlBind.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlAVLTree.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlBundleFile.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlConfig.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/nlDebug.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/nlDebugFile.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/nlEndian.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlFile.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlFileGC.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlFont.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlLocalization.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/nlMain.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "NL/nlMath.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/MemAlloc.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlMemory.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlPrint.cpp"), # problem with stdarg.h .. but should otherwise be linkable
            Object(Matching, "NL/nlSlotPool.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlString.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/nlTask.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlTextBox.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/nlTextEscape.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/nlTicker.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/nlTimer.cpp", extra_cflags=["-inline auto"]),     

            # Ext/Platform
            Object(Matching, "NL/plat/globalpad.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            # Object(NonMatching, "NL/plat/platpad.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "NL/plat/platpad.cpp", extra_cflags=["-inline auto"]),
            # Ext/Math
            Object(Matching, "NL/plat/platvmath.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "NL/plat/platqmath.cpp", extra_cflags=["-inline auto"]),
            # Ext/Audio
            Object(NonMatching, "NL/plat/plataudio.cpp", extra_cflags=["-inline auto"]),

            # Ext/GC
            Object(NonMatching, "NL/gc/gcSwizzler.cpp", extra_cflags=["-inline auto", "-inline deferred"]),

            # Ext/Game-Specific?
            Object(Matching, "NL/StatsGatherer.cpp", extra_cflags=["-inline auto", "-inline deferred"]),     
            Object(Matching, "NL/math.cpp", extra_cflags=["-inline auto", "-inline deferred"]),     
            Object(NonMatching, "NL/utility.cpp", extra_cflags=["-inline auto", "-inline deferred"]),     

            # Ext/GL
            Object(NonMatching, "NL/glx/glxSwap.cpp", extra_cflags=["-inline auto", "-inline off"]),
            Object(NonMatching, "NL/glx/glxFont.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxMatrix.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxMemory.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxTexture.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxSend.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxLoadModel.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxGX.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxDisplayList.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/glx/glxTarget.cpp", extra_cflags=["-inline auto"]),
            
            Object(Matching, "NL/gl/gl.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glDraw2.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "NL/gl/glDraw3.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "NL/gl/glFont.cpp", extra_cflags=["-inline auto", "-inline off"]),
            Object(Matching, "NL/gl/glMatrix.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/gl/glMatrixStack.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/gl/glMemory.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glModify.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glRenderList.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/gl/glStat.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "NL/gl/glState.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/gl/glStruct.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/gl/glTarget.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glView.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/gl/glUserData.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glModel.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "NL/gl/glTexture.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glConstant.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glAppAttach.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "NL/gl/glPlat.cpp", extra_cflags=["-inline auto"]),
        ],
    ),

    GameLib13(
        "NL (Next Level Library)",
        [
            Object(Matching, "Game/Drawable/DrawableObj.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(NonMatching, "Game/Drawable/DrawableModel.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Drawable/DrawableTmModel.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "Game/Drawable/DrawableSkinModel.cpp", extra_cflags=["-inline auto"]),
        ],
    ),

    ODELib(
        "Open Dynamics Engine (ODE)",
        [
            Object(NonMatching, "ode/NLGAdditions.cpp", extra_cflags=["-inline auto", "-inline auto, deferred"]),
            Object(Matching, "ode/collision_kernel.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "ode/collision_space.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "ode/collision_std.cpp", extra_cflags=["-inline auto", "-fp_contract on"]),
            Object(NonMatching, "ode/collision_transform.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "ode/collision_util.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "ode/error.cpp", extra_cflags=["-inline auto", "-inline off"]),
            Object(Matching, "ode/joint.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "ode/memory.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "ode/ode.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
            Object(Matching, "ode/matrix.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "ode/mass.cpp", extra_cflags=["-inline auto"]),
            Object(Matching, "ode/obstack.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "ode/quickstep.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "ode/rotation.cpp", extra_cflags=["-inline auto", "-inline off"]),
            Object(Matching, "ode/util.cpp", extra_cflags=["-inline auto"]),

            Object(NonMatching, "ode/ext/dColumn.cpp", extra_cflags=["-inline auto", "-inline off"]),
            Object(NonMatching, "ode/ext/dFinitePlane.cpp", extra_cflags=["-inline auto"]),
            Object(NonMatching, "ode/ext/dRoundedCorner.cpp", extra_cflags=["-inline auto"]),

            Object(NonMatching, "ode/odemath.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
        ],
    ),
    # ODELib20(
    #     "Open Dynamics Engine (ODE) MWCC 2.0",
    #     [
    #         Object(NonMatching, "ode/odemath.cpp", extra_cflags=["-inline auto", "-inline deferred"]),
    #     ],
    # ),
    DolphinLib(
        "THP",
        [
            Object(Matching, "Dolphin/thp/THPDec.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/thp/THPAudio.c", extra_cflags=["-inline auto"]),
        ]
    ),
    DolphinLib_O3(
        "Dolfin SDK",
        [
            # Dolphin/OS
            Object(NonMatching, "Dolphin/exi/EXIBios.c", extra_cflags=["-inline auto"]),
        ]
    ),
    DolphinLib(
        "Dolfin SDK",
        [
            # Dolphin/OS
            Object(NonMatching, "Dolphin/os/OS.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSAlarm.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSAlloc.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSArena.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSAudioSystem.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSCache.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSContext.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSError.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSExec.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSFont.c", extra_cflags=["-inline auto", "-char unsigned"]),
            Object(Matching, "Dolphin/os/OSInterrupt.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSLink.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSMemory.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSMutex.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSReboot.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSReset.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSResetSW.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSRtc.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSSync.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSThread.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/OSTime.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/__start.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/os/__ppc_eabi_init.cpp", extra_cflags=["-inline auto"]),

            # Dolfin/AI
            Object(Matching, "Dolphin/ai/ai.c", extra_cflags=["-inline auto"]),
            
            # Dolfin/AR
            Object(Matching, "Dolphin/ar/ar.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/ar/arq.c", extra_cflags=["-inline auto"]),

            # Dolphin/BASE
            Object(Matching, "Dolphin/base/PPCArch.c", extra_cflags=["-inline auto"]),

            # Dolphin/CARD
            Object(Matching, "Dolphin/card/CARDBios.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDUnlock.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDRdwr.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDBlock.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDDir.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDCheck.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDMount.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDFormat.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDOpen.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDCreate.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDRead.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDWrite.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDDelete.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDStat.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/card/CARDNet.c", extra_cflags=["-inline auto"]), 

            # Dolfin/DB
            Object(Matching, "Dolphin/db/db.c", extra_cflags=["-inline auto"]),

            # Dolfin/DSP
            Object(Matching, "Dolphin/dsp/dsp.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dsp/dsp_debug.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dsp/dsp_task.c", extra_cflags=["-inline auto"]),

            # Dolphin/DVD
            Object(Matching, "Dolphin/dvd/dvdlow.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dvd/dvdfs.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dvd/dvd.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dvd/dvdqueue.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dvd/dvderror.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dvd/dvdidutils.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dvd/dvdFatal.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/dvd/fstload.c", extra_cflags=["-inline auto"]),         

            # Dolphin/EXI
            # Object(NonMatching, "Dolphin/exi/EXIBios.c", extra_cflags=["-inline auto", "-O3,p"]),
            Object(Matching, "Dolphin/exi/EXIUart.c", extra_cflags=["-inline auto"]),

            # Dolphin/GX
            Object(Matching, "Dolphin/gx/GXInit.c", extra_cflags=["-inline auto", "-opt nopeephole"]),
            Object(Matching, "Dolphin/gx/GXFifo.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXAttr.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXMisc.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXGeometry.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXFrameBuf.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXLight.c", extra_cflags=["-inline auto", "-fp_contract off"]),
            Object(Matching, "Dolphin/gx/GXTexture.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXBump.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXTev.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXPixel.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXDisplayList.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/gx/GXTransform.c", extra_cflags=["-inline auto", "-fp_contract off"]),
            Object(Matching, "Dolphin/gx/GXPerf.c", extra_cflags=["-inline auto"]),

            # Dolphin/MTX
            Object(Matching, "Dolphin/mtx/mtx.c", extra_cflags=["-inline auto", "-char signed"]),
            Object(NonMatching, "Dolphin/mtx/mtx44.c", extra_cflags=["-inline auto", "-char signed"]),
            Object(NonMatching, "Dolphin/mtx/quat.c", extra_cflags=["-inline auto", "-char signed", "-fp_contract off"]),

            # Dolphin/PAD
            Object(Matching, "Dolphin/pad/Padclamp.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/pad/Pad.c", extra_cflags=["-inline auto"]),

            # Dolphin/SI
            Object(Matching, "Dolphin/si/SIBios.c", extra_cflags=["-inline auto"]),
            Object(Matching, "Dolphin/si/SISamplingRate.c", extra_cflags=["-inline auto"]),

            # Dolphin/VI
            Object(Matching, "Dolphin/vi/vi.c", extra_cflags=["-inline auto"]),
        ],
    ),

    DolphinLib132(
        "VM",
        [
            # Dolphin/vm.a
            Object(Matching, "Dolphin/vm.a/VM.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Dolphin/vm.a/VMPageReplacement.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Dolphin/vm.a/VMMapping.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "Dolphin/vm.a/VMBase.c", extra_cflags=["-inline auto"]),
        ],
    ),    


    DolphinLib(
        "amcstubs",
        [
            Object(Matching, "Dolphin/AmcStub/AmcExi2Stubs.c", extra_cflags=["-inline auto"]),
        ],
    ),    
    DolphinLib(
        "OdemuExi2",
        [
            Object(Matching, "Dolphin/OdemuExi2/DebuggerDriver.c", extra_cflags=["-inline auto"]),
        ],
        cflags=cflags_odemuexi,
    ),

    DolphinLib(
        "OdenotStub",
        [
            Object(Matching, "Dolphin/OdenotStub/odenotstub.c", extra_cflags=["-inline auto"]),
        ],
    ),        
    MusyxLib(
        "Musyx",
        [
            Object(Matching, "musyx/runtime/seq.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/synth.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/seq_api.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/snd_synthapi.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "musyx/runtime/stream.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/synthdata.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/synthmacros.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/synthvoice.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/synth_ac.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/synth_dbtab.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/synth_adsr.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "musyx/runtime/synth_vsamples.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/s_data.c", extra_cflags=["-inline auto"]),
            Object(NonMatching, "musyx/runtime/hw_dspctrl.c",  extra_cflags=["-inline auto", "-sdatathreshold 8"]),
            Object(Matching, "musyx/runtime/hw_volconv.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/snd3d.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/snd_init.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/snd_math.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/snd_midictrl.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/snd_service.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/hardware.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/dsp_import.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/hw_aramdma.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/hw_dolphin.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/hw_memory.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/CheapReverb/creverb_fx.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/CheapReverb/creverb.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/StdReverb/reverb_fx.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/StdReverb/reverb.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/Delay/delay_fx.c", extra_cflags=["-inline auto"]),
            Object(Matching, "musyx/runtime/Chorus/chorus_fx.c", extra_cflags=["-inline auto"]),
        ]
    ),        
    DolphinTrkLib(
        "TRK_MINNOW_DOLPHIN",
        [
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/main_TRK.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/mutex_TRK.c", extra_cflags=["-inline auto"]),      
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/mem_TRK.c", extra_cflags=["-inline auto"]),

            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/mpc_7xx_603e.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/CircleBuffer.c", extra_cflags=["-inline auto"]),

            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/dolphin_trk.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/dolphin_trk_glue.c", extra_cflags=["-inline auto"]),
            
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/target_options.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/targcont.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/targsupp.c", extra_cflags=["-inline auto"]),
            
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/notify.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/flush_cache.c", extra_cflags=["-inline auto"]),

            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/dispatch.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/serpoll.c", extra_cflags=["-inline auto", "-sdata 8"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/mainloop.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/nubevent.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/nubinit.c", extra_cflags=["-inline auto"]),

            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/usr_put.c", extra_cflags=["-inline auto"]),

            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/support.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/UDP_Stubs.c", extra_cflags=["-inline auto"]),

            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/msg.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/msgbuf.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/msghndlr.c", extra_cflags=["-inline auto"]),

            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/MWTrace.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/MWCriticalSection_gc.cpp", extra_cflags=["-inline auto"]),

            # NonMatchin...
            Object(NonMatching, "SDK/TRK_MINNOW_DOLPHIN/main.c", extra_cflags=["-inline auto", "-sdatathreshold 8"]),
            Object(NonMatching, "SDK/TRK_MINNOW_DOLPHIN/main_gdev.c", extra_cflags=["-inline auto", "-sdatathreshold 8"]),
            Object(NonMatching, "SDK/TRK_MINNOW_DOLPHIN/__exception.s", extra_cflags=["-inline auto"]),

            Object(NonMatching, "SDK/TRK_MINNOW_DOLPHIN/targimpl.c", extra_cflags=["-inline auto"]),
            Object(Matching, "SDK/TRK_MINNOW_DOLPHIN/mslsupp.c", extra_cflags=["-inline auto", "-enum int"]),
        ]
    ),
]


# Optional callback to adjust link order. This can be used to add, remove, or reorder objects.
# This is called once per module, with the module ID and the current link order.
#
# For example, this adds "dummy.c" to the end of the DOL link order if configured with --non-matching.
# "dummy.c" *must* be configured as a Matching (or Equivalent) object in order to be linked.
def link_order_callback(module_id: int, objects: List[str]) -> List[str]:
    # Don't modify the link order for matching builds
    if not config.non_matching:
        return objects
    if module_id == 0:  # DOL
        return objects + ["dummy.c"]
    return objects

# Uncomment to enable the link order callback.
# config.link_order_callback = link_order_callback


# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game Code"),
    ProgressCategory("sdk", "Dolphin SDK Code"),
    ProgressCategory("third_party", "Third Party"),
    ProgressCategory("runtime", "Gekko Runtime Code"),
]
config.print_progress_categories = args.verbose
config.progress_each_module = args.verbose

# Optional extra arguments to `objdiff-cli report generate`
config.progress_report_args = [
    # Marks relocations as mismatching if the target value is different
    # Default is "functionRelocDiffs=none", which is most lenient
    # "--config functionRelocDiffs=data_value",
]

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
