Super Mario Strikers  
[![Build Status]][actions] [![Code Progress]][progress] [![Data Progress]][progress] [![Link Progress]][progress] [![Discord Badge]][discord]
=============

<!--
Replace with your repository's URL.
-->
[Build Status]: https://github.com/yannicksuter/smstrikers-decomp/actions/workflows/build.yml/badge.svg
[actions]: https://github.com/yannicksuter/smstrikers-decomp/actions/workflows/build.yml
<!--
decomp.dev progress badges
See https://decomp.dev/api for an API overview.
-->
[Code Progress]: https://decomp.dev/yannicksuter/smstrikers-decomp.svg?mode=shield&measure=code&label=Code
[Data Progress]: https://decomp.dev/yannicksuter/smstrikers-decomp.svg?mode=shield&measure=data&label=Data
[Link Progress]: https://decomp.dev/yannicksuter/smstrikers-decomp.svg?mode=shield&measure=complete_code_percent&label=Linked
[progress]: https://decomp.dev/yannicksuter/smstrikers-decomp
<!--
Replace with your Discord server's ID and invite URL.
-->
[Discord Badge]: https://img.shields.io/discord/727908905392275526?color=%237289DA&logo=discord&logoColor=%23FFFFFF
[discord]: https://discord.gg/hKx3FJJgrV

A work-in-progress decompilation of Super Mario Strikers for GameCube.

This repository does **not** contain any game assets or assembly whatsoever. An existing copy of the game is required.

Supported versions:

- `G4QE01`: Rev 0 (USA)

Decompilation
=============

Decompilation is the process of reverse-engineering compiled machine code back into human-readable source code. Unlike disassembly, which produces assembly language, decompilation aims to reconstruct high-level code (like C or C++) that closely matches what the original developers wrote. This process involves analyzing the binary executable, understanding its structure and behavior, and translating it back into source code that compiles to produce identical machine code. In this project, the goal is not just a close match, but a **100% match**—the decompiled source code must compile to produce byte-for-byte identical machine code to the original. This is why diffing (see the [Diffing](#diffing) section below) is an essential piece of the process, as it allows us to verify that our decompiled code produces exactly the same binary output as the original game. Decompilation projects like this one enable deeper understanding of game mechanics, facilitate modding and preservation, and serve as valuable learning resources for understanding how games were built.

For interesting discoveries and insights found during the decompilation process, check out [Fun Facts](FunFacts.md).

Progress
========

![progress overview](https://decomp.dev/projects/989774797.svg?mode=overview&version=G4QE01)

Track the project decompilation progress and explore the interactive graph on [decomp.dev](https://decomp.dev/yannicksuter/smstrikers-decomp).


Contributing
============

Everybody can and is warmly welcome to contribute to this project! Whether you're experienced with decompilation or just getting started, your contributions are valuable.

**How to contribute:**

1. **Make a pull request** – Simply create a pull request with your changes.
2. **Join our Discord server** – Connect with other contributors to exchange ideas, coordinate work, or get help: [Discord Server](https://discord.gg/hKx3FJJgrV)

**Partial matches welcome:**
Don’t worry if your function or class translation isn’t a perfect match yet — partial matches are still valuable. They can help others understand the general structure of the codebase, and getting all the details 100% correct becomes easier with more routine and context.

**Code style:**
Please run [`clang-format`](https://clang.llvm.org/docs/ClangFormat.html) to ensure minimal, consistent formatting before committing.

**Tooling tip:**
You can use `class_gen.py` (found under `tools/scripts`) to speed up the basic setup of C++ class structures from object files:

```bash
./tools/scripts/class_gen.py --obj-file ./build/G4QE01/obj/GameAudio.o
```

This will create a C++ source and header file. After generation:

* Clean up and fix `#include` statements.
* Remove function definitions from the generated templates.
* Move the files to the proper location in the `src` and `include` hierarchy.

**Dumping DWARF info:**
If you have legally acquired the game yourself and placed the original debug ELF at `orig/G4QE01/MarioSoccerR.elf`, you can dump its DWARF debug info into a human-readable `dwarf.txt` (used by some helper scripts, e.g. `auto_class_gen.py`):

```bash
./tools/scripts/dump_dwarf.sh
```

The script writes `dwarf.txt` to the project root. Run `python configure.py && ninja` at least once beforehand so that `build/tools/dtk` is available.

**Learning together:**
I'm learning myself throughout this process, so I'm very open to adapting and improving things for this project. Don't hesitate to suggest better approaches or point out areas that could be enhanced.

Dependencies
============

Windows
--------

On Windows, it's **highly recommended** to use native tooling. WSL or msys2 are **not** required.  
When running under WSL, [objdiff](#diffing) is unable to get filesystem notifications for automatic rebuilds.

- Install [Python](https://www.python.org/downloads/) and add it to `%PATH%`.
  - Also available from the [Windows Store](https://apps.microsoft.com/store/detail/python-311/9NRWMJP3717K).
- Download [ninja](https://github.com/ninja-build/ninja/releases) and add it to `%PATH%`.
  - Quick install via pip: `pip install ninja`

macOS
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages):

  ```sh
  brew install ninja
  ```

- Install [wine-crossover](https://github.com/Gcenx/homebrew-wine):

  ```sh
  brew install --cask --no-quarantine gcenx/wine/wine-crossover
  ```

After OS upgrades, if macOS complains about `Wine Crossover.app` being unverified, you can unquarantine it using:

```sh
sudo xattr -rd com.apple.quarantine '/Applications/Wine Crossover.app'
```

Linux
------

- Install [ninja](https://github.com/ninja-build/ninja/wiki/Pre-built-Ninja-packages).
- For non-x86(_64) platforms: Install wine from your package manager.
  - For x86(_64), [wibo](https://github.com/decompals/wibo), a minimal 32-bit Windows binary wrapper, will be automatically downloaded and used.

Building
========

- Clone the repository (including the `musyx` submodule):

  ```sh
  git clone --recursive https://github.com/yannicksuter/smstrikers-decomp
  ```

  If you've already cloned the repository without --recursive, initialize the submodule manually:

  ```sh
  git submodule update --init --recursive
  ```

- To update the repository and its submodules in one go:

  ```sh
  git pull --recurse-submodules
  ```

- Copy your game's disc image to `orig/G4QE01`.
  - Supported formats: ISO (GCM), RVZ, WIA, WBFS, CISO, NFS, GCZ, TGC
  - After the initial build, the disc image can be deleted to save space.

- Configure:

  ```sh
  python configure.py
  ```

  To use a version other than `G4QE01` (USA, Rev 0), specify it with `--version`.

- Build:

  ```sh
  ninja
  ```

Diffing
=======

Once the initial build succeeds, an `objdiff.json` should exist in the project root.

Download the latest release from [encounter/objdiff](https://github.com/encounter/objdiff). Under project settings, set `Project directory`. The configuration should be loaded automatically.

Select an object from the left sidebar to begin diffing. Changes to the project will rebuild automatically: changes to source files, headers, `configure.py`, `splits.txt` or `symbols.txt`.

Acknowledgements
================

This project wouldn’t be possible without the collective knowledge, tools, and support of the broader decompilation community. Huge thanks to contributors of other GameCube decomp projects, the teams behind [decomp.dev](https://decomp.dev/) and [decomp.me](https://decomp.me/), and the incredibly helpful discussions happening on Discord. These resources have been invaluable for solving problems, speeding up setup, and staying motivated throughout the process.
