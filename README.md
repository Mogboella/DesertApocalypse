# Desert Colony (OpenGL)

This project shows an OpenGL real-time scene of a desert colony under siege: a leviathan-sized sand worm erupts from the dunes while fighter ships, ruins, vegetation, and smaller worms animate across a heightmap-driven terrain with a dynamic day/night sky.

## Scene Highlights
- Heightmap terrain built with custom desert/terrain shaders and gradient skybox tinting.
- Animated hierarchical models for the leviathan mother sand worm and smaller worms, with fighter ships orbiting the leviathan spawn.
- Dynamic lighting (directional + togglable point/spot lights), day/night transition, and head-bobbed first-person camera tied to terrain height.
- Asset packaging in CMake post-build steps copies shaders, textures, and models into the build output for immediate execution.

## Repository Layout
- `code/src` - Application entry point, renderer, environment/terrain manager, animation, and scene setup.
- `code/assets` - Models, textures, and GLSL shaders referenced at runtime (see Asset Setup below).
- `external` - Vendored GLAD, GLFW, GLM, and Assimp sources used by CMake.
- `scripts` - Convenience scripts for building (`build.sh`) and running (`run.sh`).

## Asset Setup (not in repo)
Models and large textures are **not versioned** for licensing/size reasons. Download them from the credits below and place them at these exact paths relative to the repo root:

- Textures (put files directly in `code/assets/textures/`)
  - `dune_heightmap-1.jpg`
  - `sand_dark.jpg`

- Models (place files/folders in `code/assets/models/`)
  - `ice-worm/source/AnimatedWorm/AnimatedWorm.fbx` plus its accompanying `source/AnimatedWorm/` texture files.
  - `worm_monster/scene.gltf` with its `worm_monster/textures/` folder.
  - `gas_mask/scene.gltf` with its textures folder.
  - `a_road_trip_in_2016_-_whale_class_transport.glb`
  - `v-19_torrent_-_star_wars_-_clone_wars.glb`
  - `ruined_city.glb`
  - `ruined_city2.glb` (or the skyscraper asset you download; keep the filename consistent).
  - `cactus.glb`
  - `cherry_tree.glb`
  - `mechanical_girl.glb`

The CMake post-build step copies `code/assets/models`, `code/assets/textures`, and `code/assets/shaders` next to the executable. If you add new assets, keep their relative paths stable or update the paths in `code/src/main.cpp` and `CMakeLists.txt` accordingly.

## Build (macOS)
Prerequisites: CMake >= 3.10, a C++11 compiler, Xcode Command Line Tools (for `xcrun`/OpenGL), zlib, and Assimp available to CMake. GLAD/GLFW/GLM ship in `external/`.

```bash
chmod +x scripts/build.sh scripts/run.sh
./scripts/build.sh    # wipes and recreates build/, configures Release, then makes mydesertcolony_main
```

If you prefer manual steps: `mkdir -p build && cd build && cmake .. && make -j`. On Linux/Windows you may need to point CMake to your OpenGL/Assimp installations and adjust any framework flags.

## Run
```bash
./scripts/run.sh      # rebuilds then runs ./build/mydesertcolony_main
```
Run from the repo root so relative asset paths resolve; the build step copies shaders, textures, and models next to the executable.

## Controls
- Look with the mouse; scroll adjusts camera zoom.
- Move: `W/A/S/D`, ascend/descend: `E/Q`.
- Camera presets: `1`..`4` for curated viewpoints.
- Toggle day/night: `N`; toggle point lights: `P`; toggle spotlights: `L`.
- Exit: `Esc`. Camera stays a fixed offset above the terrain and adds walking bob when moving.

## Asset Credits
### Textures & Heightmaps
- `dune_heightmap-1.jpg` (terrain heightmap) — Acala92 - Reddit User, Google-Drive-Dunes: https://drive.google.com/drive/folders/1DuO_WLyqQIA8U2lAupCW59xnwM57E-v6 (2022).
- `sand_dark.jpg` (sand texture) — Poliigon rippled sand texture: https://www.poliigon.com/texture/rippled-wet-sand-texture/6997.

### Models
- Ice Worm (mother worm): gavinpgamer1 — https://skfb.ly/oUyNt (Sketchfab, 2023).
- Worm Monster (attacking worms): CR!STALLL — https://skfb.ly/oVNZZ (Sketchfab, 2023).
- Soldier (gas mask figure): Mr_bruh, Chaos_insurgency_ci_from_scp__cb — https://skfb.ly/oUu2E (Sketchfab, 2024).
- Whale class transport: jronn, A Road Trip In 2016 — https://skfb.ly/R8GH (Sketchfab).
- V-19 Torrent starfighter: AirStudions — https://skfb.ly/oPHqB (Sketchfab).
- Cherry Tree: local.yany — https://skfb.ly/pzOOB (Sketchfab).
- Mechanical Girl: Chenchanchong — https://skfb.ly/pB8KZ (Sketchfab).
- Ruined City: eucalyp555 — https://skfb.ly/o68xn (Sketchfab).
- Post-apocalyptic buildings (ruined skyscraper cluster): Helindu — https://skfb.ly/otBXq (Sketchfab).
- Cactus: upeglnf951 — https://skfb.ly/oGy9w (Sketchfab).

*This project is my submission for the TCD Computer Graphics Individual Project 2025/26 MSc Computer Science - AR/VR*
