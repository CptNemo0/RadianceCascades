# The Bouncers

> A deterministic 2D physics battle simulation that renders itself into mass-producible videos, built on the project's Radiance Cascades global-illumination engine.

**The Bouncers** are circle "fighters" that bounce around a square arena with full energy conservation. Each fighter has a **class** (starting with **Fire** and **Ice**) and equal starting health. Every few seconds a **buff** spawns; the first fighter to grab it arms their class ability, which triggers on the next **clash** with an enemy:

- **Fire** — on pickup the fighter is engulfed in flame. The next clash makes the enemy **burn**: damage over time, ticking on a fixed interval.
- **Ice** — on pickup the enemy is **frozen** for ~2 s. If the ice fighter clashes into them during that window, the enemy is knocked flying and takes burst damage.

More classes will follow once Fire and Ice are solid.

The point isn't only to _watch_ a match — it's to **mass-produce videos of them**. A match runs headless, writes **every frame** to disk, and emits a **timestamped event log** (wall hits, buff pickups, clashes, ability uses, damage ticks, deaths). A downstream tool turns the log into an audio track; `ffmpeg` then muxes frames + audio into a finished video. Matches are authored as small **`.match` JSON scenarios** that an LLM can generate and feed to the program.

The simulation renders **unlit** sprites into the scene; the existing **Radiance Cascades** pipeline then lights it. See **[RADIANCE_CASCADES.md](RADIANCE_CASCADES.md)** for the engine internals (render-graph nodes, JFA/SDF, the RC/Cached-RC/GI lighting methods, build presets, and the profiler).

> **Status: roadmap / in development.** This document is the implementation plan. The sections below describe the target design and the milestone-by-milestone changes; not all of it exists yet.

---

## How it fits on the engine

The engine is a node-based render graph. Today the lit-scene front-end is **Canvas → Fire → UVColorspace → JFA → SDF → {GI | RC | Cached RC | Comparison}**, where the "scene" is a single hand-drawn RGBA texture.

The Bouncers replace the hand-drawn front-end with a simulation-driven one:

```
Simulation (CPU)  ──snapshot──►  BouncersNode ──► UVColorspace ──► JFA ──► SDF ──► RC ──► frame
     │                              (sprites, unlit)
     └──events──►  event log (JSONL)  ──►  audio tool  ──►  ffmpeg mux
```

- **Simulation** is pure CPU (no OpenGL): Position-Based Dynamics, collisions, class abilities, status effects, buff spawning, win conditions. It is unit-testable in isolation.
- Each step it produces a **POD snapshot** (fighter transforms, status visuals, particles, global effects) and emits **events**.
- **`BouncersNode`** draws the snapshot's sprites into the **scene color texture** plus a parallel **R8 material-type texture**, with no lighting. The rest of the RC pipeline lights the result.
- **Run modes:** the first target is an **offline recorder** — sim and render run in **lockstep** (one step per frame), so output is stable. A threaded live-preview mode (sim thread + SPSC ring buffer) comes later.

### The material model (R8 type texture)

The RC engine currently has **no material concept** — a single color texture doubles as both emission and occlusion. To support **transparent, reflective, and refractive** objects we add **one R8 texture** that travels alongside the color texture through UVColorspace → JFA → SDF. Each texel's number selects a surface type:

| value | type        | RC behaviour                                |
| ----: | ----------- | ------------------------------------------- |
|     0 | empty       | ray passes                                  |
|     1 | opaque      | terminate, return color (current behaviour) |
|     2 | emissive    | accumulate emission                         |
|     3 | translucent | attenuate + tint, keep marching             |
|     4 | reflective  | spawn reflected ray, blend by reflectivity  |
|     5 | refractive  | Snell-bend the ray (experimental)           |

Continuous parameters (opacity, reflectivity, IOR) come from uniforms or a tiny per-type lookup; surface **normals are taken from the SDF gradient** (free — we already build the SDF). The list is extensible.

> ⚠️ **Refraction bends rays, which breaks RC's straight-ray interval/merge assumption.** It is the riskiest item, scoped last and treated as a thesis-worthy approximation rather than a drop-in.

---

## `.match` scenario format

A `.match` file is JSON (parsed with **rapidjson**) describing one fight. Indicative shape:

```jsonc
{
  "version": 1,
  "arena": { "size": 1024, "restitution": 1.0 },
  "seed": 12345,
  "fps": 60,
  "duration_cap_s": 60,
  "fighters": [
    {
      "class": "fire",
      "pos": [0.3, 0.5],
      "vel": [0.6, 0.2],
      "radius": 28,
      "health": 100,
    },
    {
      "class": "ice",
      "pos": [0.7, 0.5],
      "vel": [-0.6, -0.2],
      "radius": 28,
      "health": 100,
    },
  ],
  "buffs": { "interval_s": 5.0, "first_spawn_s": 3.0 },
}
```

Victory is last-fighter-standing, or highest health at `duration_cap_s`.

---

## Planned usage

```sh
# headless render of a scenario to a frame folder + event log + manifest
RadianceCascades.exe --headless --match matches/duel.match --out out/duel --fps 60 \
    --on-complete "python tools/make_video.py"
```

On match end the program flushes state, finalizes the JSONL event log, writes a **`manifest.json`** (frames dir, fps, resolution, log path, audio cues), then invokes `--on-complete` with the manifest path so the downstream audio + `ffmpeg` stage can run. The same binary still launches as the interactive engine when no match/headless flags are given.

---

## Roadmap

Surgical, milestone-by-milestone changes. `＋` = new file, `~` = modified file. The **offline recorder works end-to-end before** the threaded live mode and the risky refraction work.

### M0 — Build & config foundations

- [x] `~ CMakeLists.txt`: FetchContent **rapidjson** (header-only); vendor **`stb_image.h`**; enable `tests/` via CTest; link threads.
- [ ] `＋ sources/app_config.h`: POD `AppConfig` { `headless`, `match_path`, `out_dir`, `fps`, `seed`, `on_complete_cmd`, `record` }.
- [ ] `＋ sources/cli.{h,cc}`: tiny argv parser → `AppConfig` (`--match`, `--out`, `--headless`, `--fps`, `--seed`, `--on-complete`).
- [ ] `~ sources/main.cc`: parse args; add the `stb_image` implementation define (mirroring the existing `stb_image_write` block); branch interactive vs record.

### M1 — Headless, fixed-dt clock, seeded RNG

- [ ] `~ sources/app.{h,cc}`: `headless_` → `GLFW_VISIBLE=false`, skip `Ui` + mouse observers; **fixed-dt clock** (`time_ = frame_index_ * (1/fps)`, reusing the measuring precedent) so events are frame-stamped; headless loop runs until the match signals end.
- [ ] `＋ sources/rng.h` + `~ sources/utility.cc`: replace the global `std::mt19937 gen(1000)` with a seedable `Rng` service seeded from `AppConfig.seed` (convenience for repeatable test runs — not a hard reproducibility guarantee); route `Random*`/noise through it.

### M2 — Asset pipeline & sprite rendering

- [ ] `~ sources/texture.{h,cc}`: `Texture::FromFile(path)` via stb_image (alpha, premultiply, sRGB→linear).
- [ ] `＋ sources/asset_manager.{h,cc}`: path→`Texture` cache / atlas.
- [ ] `＋ sources/sprite_batch.{h,cc}`: instanced-quad batch (per-instance transform, uv-rect, tint) — many sprites per draw. Does **not** overload the `Surface` singleton.
- [ ] `＋ shaders/sprite.{vs,fs}` + `~ ShaderManager` enum/`ParseShaderType` + `shaders.list`: sprite shader writes scene color **and** the material-type channel.

### M3 — Simulation core (pure CPU, headless-testable) — `sources/sim/`

- [ ] `＋ sim/body.h` — circle (pos, vel, radius, inv_mass).
- [ ] `＋ sim/pbd_solver.{h,cc}` — predict positions → circle-circle & circle-wall constraints → solve iterations → derive velocities; restitution for energy conservation; knockback = applied impulse.
- [ ] `＋ sim/fighter.{h,cc}` — id, class enum, health, body, status list, armed-buff state.
- [ ] `＋ sim/status_effect.{h,cc}` — generic duration/tick effects: Burn (DoT), Freeze, Knockback. Extensible.
- [ ] `＋ sim/class_ability.{h,cc}` — per-class strategy. Fire: pickup arms flame → clash applies Burn. Ice: pickup freezes enemy 2 s → clash within window ⇒ knockback + damage.
- [ ] `＋ sim/buff.h` — buff entity, spawn cadence, pickup detection.
- [ ] `＋ sim/match_rules.h` — victory (last alive / health at time cap) + time cap.
- [ ] `＋ sim/events.h` — `SimEvent` { type, frame, fighter ids, **position** (pan), **magnitude** (intensity) }; types: WallHit, BuffSpawn, BuffPickup, Clash, AbilityUse, DamageTick, Freeze, Knockback, Death, MatchEnd.
- [ ] `＋ sim/simulation.{h,cc}` — owns fighters/buffs; `Step(dt)` → PBD, collisions→Clash events, status ticks, buff spawn/pickup, win check; emits events + produces a snapshot.
- [ ] `＋ tests/` — PBD energy conservation, status timers, parser correctness.

### M4 — `.match` schema & parser — `sources/match/`

- [ ] `＋ match/match_parser.{h,cc}` — rapidjson → `MatchConfig` → initial `Simulation`; validation + errors.
- [ ] `＋ matches/*.match` — sample scenarios.
- [ ] `＋ tests/` — parser + round-trip.

### M5 — Snapshot & sim↔render bridge (lockstep now, ring-buffer-ready)

- [ ] `＋ sim/snapshot.h` — POD `SimSnapshot` { frame; global effects (shake, frost-wave, flash); fighters[] (transform, class, tint, status flags); particles[]; buffs[] }, trivially copyable.
- [ ] `＋ sim/state_ring_buffer.h` — SPSC lock-free ring (interface built now; lockstep recording consumes immediately).
- [ ] `＋ sources/event_log.{h,cc}` — collect `SimEvent`s → timestamped (frame + sim time) **JSONL** log; include position+magnitude for audio; SPSC-shaped for a future audio thread.

### M6 — Bouncers render node & pipeline

- [ ] `＋ sources/render_nodes/bouncers_node.{h,cc}` (RenderNode) — consume `SimSnapshot`, draw fighters/buffs/particles via `SpriteBatch` into the **scene color texture** + parallel **R8 material-type texture**, no lighting. Replaces Canvas+Fire as the scene source, then feeds existing UVColorspace → JFA → SDF → RC.
- [ ] Global effects: frost-wave (screen-space distortion — synergizes with refraction), scene shake (offset uniform), star bursts (particles), damage discoloration (per-sprite tint).
- [ ] `~ sources/renderer.{h,cc}` — add `Mode::kBouncers` + `bouncers_pipeline_`; construct Renderer in "bouncers" vs "interactive" config; give the node access to the snapshot source.

### M7 — RC optics upgrade (alpha → reflection → refraction)

- [ ] Carry the **R8 material-type texture** through UVColorspace → JFA → SDF the same way color is, so the RC march can sample type at a hit. Continuous params via uniforms / per-type lookup; **normals from the SDF gradient**.
- [ ] `~ shaders/radiance_cascade_sdf.fs`, `radiance_cascade.fs`, `global_illumination.fs` — at a hit, read the type id and branch.
  - [ ] **M7a alpha** — translucent hit ⇒ attenuate+tint accumulated radiance by transmittance and keep marching (accumulate emission) instead of terminating.
  - [ ] **M7b reflection** — reflective hit ⇒ reflected secondary ray; blend by reflectivity.
  - [ ] **M7c refraction (experimental)** — `＋ shaders/radiance_cascade_refractive.fs`: Snell bend using the SDF-gradient normal; document the cascade-merge break + the chosen approximation (thesis material).

### M8 — Recording, manifest & callback

- [ ] `＋ sources/recorder.{h,cc}` — record mode replacing the `IsMeasuring()`-gated save; **double-buffered PBO async readback**, **zero-padded** filenames (`frame_%06d.png`), optional encoder thread / ffmpeg-stdin pipe; output dir from CLI.
- [ ] `＋ sources/process.h` — cross-platform process launch (Windows `CreateProcess` / `std::system`).
- [ ] On match end: flush ring buffer → finalize event log → write **`manifest.json`** (rapidjson) → spawn `on_complete_cmd` with the manifest path → clean shutdown.

### M9 — Live mode & polish (later)

- [ ] Flip the bridge to **threaded**: sim thread `Step`s at fixed dt → pushes to `state_ring_buffer`; render thread consumes latest with **interpolation**; event queue feeds an optional live-audio thread.
- [ ] ImGui Bouncers controls (pause/restart/pick `.match`); additional fighter classes (data + ability strategy).

---

## Design notes (things to watch)

1. **Frame-indexed event stamps** — audio is built from the log, so events are stamped by frame index/sim tick, not wall-clock, so cues land on the right recorded frame. Full byte-exact determinism is a nice-to-have, not a requirement.
2. **Headless still needs a GL context** on Windows/MinGW — use a hidden GLFW window + FBO readback (true surfaceless is painful here). Guard ImGui and mouse-observer wiring behind the interactive flag.
3. **`glReadPixels` per frame stalls the GPU** and PNG encoding is CPU-bound — use double-buffered PBO readback + an encoder thread, or pipe raw frames to ffmpeg stdin; zero-pad filenames for globbing.
4. **Two "class" axes** — gameplay class (Fire/Ice = data + ability strategy, in the CPU sim) is separate from visual rendering (one `BouncersNode`). Don't make a fighter "inherit Renderer".
5. **No sprite primitive / no image loading today** — everything is fullscreen-quad passes; we add an instanced sprite batch + atlas and vendor `stb_image`. Handle premultiplied alpha + sRGB↔linear or compositing/lighting will be wrong.
6. **Snapshots are POD, double-buffered, no pointers**; plan for interpolation when sim rate ≠ video fps. Snapshots and events are separate streams (the audio thread is a second consumer of events).
7. **Resolution is a compile-time constant (1024)** used widely on the CPU — keep it fixed initially (shaders already take a `resolution` uniform).
8. **Hand off a `manifest.json`**, not just CLI args, to the downstream audio/ffmpeg tool; mind argument quoting on the callback.

---

## Engine docs

For everything about the underlying renderer — requirements, build presets, controls, the GI/RC/Cached-RC/Comparison modes, the pipeline stages, and the per-stage profiler — see **[RADIANCE_CASCADES.md](RADIANCE_CASCADES.md)**.
