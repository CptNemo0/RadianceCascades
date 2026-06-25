# The Bouncers

> A deterministic 2D physics battle simulation that renders itself into mass-producible videos, built on the project's Radiance Cascades global-illumination engine.

**The Bouncers** are circle "fighters" that bounce around a square arena with full energy conservation. Each fighter has a **class** (starting with **Fire** and **Ice**) and equal starting health. Every few seconds a **buff** spawns; the first fighter to grab it arms their class ability, which triggers on the next **clash** with an enemy:

- **Fire** — on pickup the fighter is engulfed in flame. The next clash makes the enemy **burn**: damage over time, ticking on a fixed interval.
- **Ice** — on pickup the enemy is **frozen** for ~2 s. If the ice fighter clashes into them during that window, the enemy is knocked flying and takes burst damage.

More classes will follow once Fire and Ice are solid.

The point isn't only to _watch_ a match — it's to **mass-produce videos of them**. A match runs headless, writes **every frame** to disk, and emits a **timestamped event log** (wall hits, buff pickups, clashes, ability uses, damage ticks, deaths). A downstream tool turns the log into an audio track; `ffmpeg` then muxes frames + audio into a finished video.

Matches are authored as small **`.match` JSON scenarios** that an LLM can generate. The app runs like a **server**: a dedicated reader thread consumes **newline-delimited JSON** from **stdin** — one match per line, either the inline config or a `{ "match_file": "path/to.match" }` reference — validates each, and **queues** it. The simulation renders the queued matches one after another, so a producer can stream an unbounded backlog of fights into a single long-lived process.

The simulation renders **unlit** sprites into the scene; the existing **Radiance Cascades** pipeline then lights it. See **[RADIANCE_CASCADES.md](RADIANCE_CASCADES.md)** for the engine internals (render-graph nodes, JFA/SDF, the RC/Cached-RC/GI lighting methods, build presets, and the profiler).

> **Status: roadmap / in development.** This document is the implementation plan. The sections below describe the target design and the milestone-by-milestone changes; not all of it exists yet.

---

## How it fits on the engine

The engine is a node-based render graph. Today the lit-scene front-end is **Canvas → Fire → UVColorspace → JFA → SDF → {GI | RC | Cached RC | Comparison}**, where the "scene" is a single hand-drawn RGBA texture.

The Bouncers replace the hand-drawn front-end with a simulation-driven one:

```
stdin (NDJSON) ─reader thread─► match queue ─► Simulation (CPU) ─┬─snapshot─► BouncersNode ─► UVColorspace ─► JFA ─► SDF ─► RC ─► frame
                                                                  │             (sprites, unlit)
                                                                  └─events────► event log (JSONL) ─► audio tool ─► ffmpeg mux
```

- **Input server (reader thread).** A `std::jthread` blocks on stdin, reads one NDJSON line at a time, resolves `match_file` references, validates each against the `.match` schema, and pushes valid `MatchConfig`s onto a bounded thread-safe **match queue** (the existing `RingBuffer`). Invalid lines are reported to stderr and skipped; a full queue back-pressures the producer.
- **Simulation** is pure CPU (no OpenGL): it pops the next match, runs Position-Based Dynamics, collisions, class abilities, status effects, buff spawning, and win conditions to completion, then pops the next. Unit-testable in isolation. (Already on its own `jthread` in `bouncers/simulation.h`.)
- Each step it produces a **POD snapshot** (fighter transforms, status visuals, particles, global effects) and emits **events**.
- **`BouncersNode`** draws the snapshot's sprites into the **scene color texture** plus a parallel **R8 material-type texture**, with no lighting. The rest of the RC pipeline lights the result.
- **Run modes:** the streaming **server** above is the primary feed; a one-shot `--match file` path remains for single renders. Sim and render stay decoupled by the snapshot ring buffer so the producer never blocks rendering.

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

### Input protocol (streaming stdin)

In server mode the app reads **one JSON value per line** (NDJSON) from stdin. Each line is one of:

- an **inline** match object (the schema above), or
- a **reference**: `{ "match_file": "matches/duel.match" }` — the reader loads and parses that file.

Each valid match is appended to the queue and rendered in arrival order; an invalid line is logged to stderr and skipped without stopping the stream. Reaching EOF on stdin drains the queue, finalizes outputs, then exits.

---

## Planned usage

```sh
# one-shot: headless render of a single scenario
RadianceCascades.exe --headless --match matches/duel.match --out out/ --fps 60 \
    --on-complete "python tools/make_video.py"

# server: stream a backlog of matches in as newline-delimited JSON
cat backlog.ndjson | RadianceCascades.exe --headless --server --out out/ --fps 60
#   backlog.ndjson, one match (or {"match_file": ...}) per line:
#   {"version":1,"seed":1,"fighters":[ ... ]}
#   {"match_file":"matches/duel.match"}

# an LLM producer can pipe directly and keep the process alive
my_llm_match_generator | RadianceCascades.exe --headless --server --out out/
```

Per match the program writes its frames to a per-match subfolder of `--out`, finalizes that match's JSONL event log, writes a **`manifest.json`** (frames dir, fps, resolution, log path, audio cues), then invokes `--on-complete` with the manifest path so the downstream audio + `ffmpeg` stage can run. The same binary still launches as the interactive engine when no match/server/headless flags are given.

---

## Roadmap

Built **MVP-first**: stand up a hardcoded vertical slice — balls bounce → lit by RC → recorded to a video — end to end, then layer gameplay, then data-driven authoring (`.match` / stdin), and do the rendering upgrade last. Most items are meant to land rough and get **iterated while later stages are built**, not perfected in isolation. `＋` = new file, `~` = modified file.

### Stage 0 — Prerequisites (done)

- [x] `~ CMakeLists.txt` / `main.cc` — FetchContent **rapidjson**, vendor **`stb_image.h`** (+ impl define), `tests/` via CTest, threads. Threading/recording primitives already in the tree: `RingBuffer`, `PoolRingBuffer`, `FramePool`, `SaveFrameJob`, `AsyncImageWriter`; `Simulation` already runs on its own `jthread`.

### Stage 1 — MVP vertical slice (hardcoded match; running in a window is fine)

Goal: a hardcoded set of circles bounces, gets lit by RC, and is written to frames. No classes, effects, buffs, `.match`, or stdin yet.

- [x] **CLI parser w/ validation** `＋ sources/cli.{h,cc}` — argv → a small parsed-options struct, **validation embedded** (`--out`, `--fps`, frame/duration cap; `--match` / `--server` added later). **Do not** design a global config up front — grow the struct as real flags appear.
- [ ] **Basic bouncing sim** `~ sources/bouncers/{simulation.h,entity.h}` — a basic **bouncing fighter** (circle: pos, vel, radius, mass) + a minimal **PBD** step: predict → circle-circle & circle-wall constraints → derive velocities, restitution = 1 (energy conserved). Hardcoded initial fighters. No health/effects/buffs. (Replace the empty spin loop; fix `!stop_token.stop_possible()` → `!stop_requested()`.)
- [ ] **Snapshot + bridge** `＋ sources/bouncers/snapshot.h` — POD `SimSnapshot` { frame; fighters[] (transform) } (grows later); hand sim→render over the existing `RingBuffer` / `PoolRingBuffer` (borrow/return `Node`s avoid per-frame reallocation).
- [ ] **Bouncers render node** `＋ sources/render_nodes/bouncers_node.{h,cc}` + `~ renderer.{h,cc}` — consume a snapshot, draw circles **procedurally** (simple shader, no textures yet) into the scene color texture (+ an R8 type texture, all opaque/emissive for now), then feed the existing UVColorspace → JFA → SDF → RC chain. Add `Mode::kBouncers` + `bouncers_pipeline_`.
- [ ] **Recording** — replace the `IsMeasuring()`-gated save, reusing `AsyncImageWriter` + `FramePool` + `SaveFrameJob`; add **double-buffered PBO readback** (kill the `glReadPixels` stall) + **zero-padded** names (`frame_%06d.png`) → `--out`. Mux the folder with ffmpeg by hand to confirm the slice. ✅ **MVP milestone.**

### Stage 2 — Gameplay (effects & classes)

Layer onto the working slice; iterate visuals as you go.

- [ ] **Health, status effects, classes** — extend the fighter with health; generic duration/tick **status effects** (Burn DoT, Freeze, Knockback); per-class **ability strategy** (Fire: pickup arms flame → clash applies Burn; Ice: pickup freezes enemy ~2 s → clash ⇒ knockback + damage).
- [ ] **Buffs & win conditions** — buff entity + spawn cadence + pickup detection; victory (last alive / health at time cap) + time cap.
- [ ] **Events + event log** `＋ sources/bouncers/events.h`, `＋ sources/event_log.{h,cc}` — `SimEvent` { type, frame, fighter ids, position (pan), magnitude (intensity) }; WallHit/BuffSpawn/BuffPickup/Clash/AbilityUse/DamageTick/Freeze/Knockback/Death/MatchEnd → timestamped **JSONL** log for the audio stage.
- [ ] **Textured sprites** `~ texture.{h,cc}` (`FromFile` via stb_image), `＋ asset_manager`, `＋ sprite_batch` + `shaders/sprite.{vs,fs}` — swap procedural circles for textured sprites/atlas; on-fighter effect tints, particles (star bursts), global effects (scene shake, frost-wave).
- [ ] `＋ tests/` — PBD energy conservation, status-effect timers.

### Stage 3 — Authoring (replace hardcoded with data)

- [ ] **`.match` parser** `＋ sources/match/match_parser.{h,cc}` — rapidjson → `MatchConfig` → initial sim; validation + errors. `＋ matches/*.match` samples. Plug in where Stage 1 hardcoded the fighters.
- [ ] **Input server** `＋ sources/bouncers/match_server.{h,cc}` — `std::jthread` reading **NDJSON** from stdin; resolve `{ "match_file": "…" }` refs; validate via the parser (bad line → stderr + skip); `Push` valid matches onto a bounded `RingBuffer<MatchConfig,N>`; sim `WaitPop`s and renders each to completion. stdin EOF → drain → exit. Gate behind `--server`.
- [ ] **Manifest + callback** `＋ sources/process.h` — per-match output subfolder; on match end write **`manifest.json`** (frames dir, fps, resolution, log path, cues) → spawn `--on-complete` with the manifest path.
- [ ] `＋ tests/` — parser + round-trip.

### Stage 4 — Optional / later

- [ ] **Headless + fixed-dt clock + seeded RNG** — `GLFW_VISIBLE=false`, skip Ui/mouse observers; fixed-dt clock; seedable `Rng`. **Optional** — only needed for true background rendering / repeatable runs; the slice works in a window without it. Determinism is explicitly not a goal.
- [ ] **Live preview & polish** — threaded sim with snapshot **interpolation**; ImGui Bouncers controls (pause/restart/pick `.match`); more fighter classes.

### Stage 5 — Rendering upgrade (the very end)

- [ ] Carry the **R8 material-type texture** through UVColorspace → JFA → SDF; branch in `radiance_cascade_sdf.fs` / `radiance_cascade.fs` / `global_illumination.fs`; **normals from the SDF gradient**.
  - [ ] **alpha** — translucent hit ⇒ attenuate + tint, keep marching.
  - [ ] **reflection** — reflective hit ⇒ reflected secondary ray, blend by reflectivity.
  - [ ] **refraction (experimental)** — `＋ shaders/radiance_cascade_refractive.fs`: Snell bend; document the cascade-merge break (thesis material).

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
