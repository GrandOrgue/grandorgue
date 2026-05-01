# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GrandOrgue is a sample-based pipe organ simulator written in C++20 with wxWidgets GUI. It supports cross-platform audio (Linux/Windows/macOS) via multiple backends (PortAudio, RtAudio, JACK) and provides realistic polyphonic organ sound synthesis with multi-threaded audio processing.

## Building the Project

### Prerequisites Installation

**Linux (Debian/Ubuntu-based):**
```bash
./build-scripts/for-linux/prepare-debian-based.sh
```

**Linux (Fedora):**
```bash
./build-scripts/for-linux/prepare-fedora.sh
```

**Linux (OpenSuse):**
```bash
./build-scripts/for-linux/prepare-opensuse.sh
```

**macOS:**
```bash
./build-scripts/for-osx/prepare-osx.sh
```

### Building

**Quick build (Linux):**
```bash
./build-scripts/for-linux/build-on-linux.sh
```
Output: `build/linux/bin/` (executables), `build/linux/` (packages)

**Manual build:**
```bash
mkdir -p build/linux
cd build/linux
cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" ../..
make -j$(nproc)
```

**Debug build:**
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=-g -DCMAKE_C_FLAGS=-g -G "Unix Makefiles" ../..
make -j$(nproc)
```

**Create packages:**
```bash
make package
```

### Testing

**Enable testing:**
```bash
cmake -DGO_BUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug -G "Unix Makefiles" ../..
make -j$(nproc)
```

**Run all tests:**
```bash
make test
# or
ctest
```

**Run test executable directly:**
```bash
./bin/GOTestExe
```

**Code coverage (requires GO_BUILD_TESTING=ON):**
The build system adds `--coverage` flag automatically when testing is enabled.

## Architecture Overview

### High-Level System Structure

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│  (src/grandorgue/)                                           │
│  GOApp, GODocument, GOOrganController, wxWidgets GUI        │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────┴──────────────────────────────────────┐
│                    Core Engine Layer                         │
│  (src/core/)                                                 │
│  GOOrganModel, GOSoundEngine, GOSoundScheduler              │
│  GOSoundingPipe, GOWindchest, Memory/File Utilities         │
└──────────────────────┬──────────────────────────────────────┘
                       │
┌──────────────────────┴──────────────────────────────────────┐
│                   Audio Backend Layer                        │
│  (src/portaudio/, src/rt/)                                   │
│  GOSoundPort → GOSoundPortaudioPort, GOSoundRtPort,         │
│                GOSoundJackPort                               │
└─────────────────────────────────────────────────────────────┘
```

### Audio Processing Pipeline

The real-time audio flow:

```
MIDI/GUI Input
    ↓
GOOrganController → GOEventDistributor
    ↓
GOManual/GOStop → GOSoundingPipe (model objects)
    ↓
GOSoundEngine (master orchestrator)
    ├→ GOSoundScheduler (lock-free task dispatch)
    │   ├→ GOSoundTremulantTask
    │   ├→ GOSoundWindchestTask (per windchest DSP)
    │   ├→ GOSoundGroupTask
    │   ├→ GOSoundOutputTask
    │   └→ GOSoundReleaseTask
    │
    ├→ GOSoundSamplerPool (manages active samplers)
    │   └→ GOSoundSampler (per-voice state)
    │       └→ GOSoundStream (decoding, resampling)
    │
    └→ GOSound (facade, thread coordination)
        └→ GOSoundPort (abstract backend interface)
            ├→ GOSoundPortaudioPort
            ├→ GOSoundRtPort
            └→ GOSoundJackPort
                ↓
            Audio Hardware
```

### Key Abstractions

**Sound Buffer Classes** (`src/grandorgue/sound/buffer/`):
- `GOSoundBuffer`: Read-only wrapper (non-owning, interleaved float data)
- `GOSoundBufferMutable`: Mutable buffer interface
- `GOSoundBufferManaged`: RAII-based owned buffer with automatic cleanup

**Audio Callback Signature:**
```cpp
bool AudioCallback(GOSoundBufferMutable &outputBuffer)
```
This pattern flows through: `GOSoundPort` → `GOSound` → `GOSoundEngine`

**Provider Pattern:**
- `GOSoundProvider`: Abstract base for audio sources
- `GOSoundProviderWave`: Loads wave files (attack/release segments)
- `GOSoundProviderSynthedTrem`: Synthesized tremulant oscillations

**Task-Based Scheduling:**
- `GOSoundScheduler`: Lock-free multi-threaded task dispatch
- Task priorities: `TREMULANT(10) < WINDCHEST(20) < AUDIOGROUP(50) < AUDIOOUTPUT(100) < RELEASE(160) < TOUCH(700)`
- Thread pool: `GOSoundThread` instances execute tasks from scheduler queue

### Organ Model Architecture

**GOOrganModel**: Central organ structure
- Contains all organ components (manuals, stops, couplers, tremulants, enclosures)
- Manages windchests (1-indexed, each groups related pipes)
- Stores ranks and pipes, handles combination system

**GOSoundingPipe**: Individual pipe model
- Bridges organ model and sound engine
- Contains `GOSoundProviderWave` (audio data)
- Manages pipe configuration (audio group, windchest, tuning, volume)
- Handles tremulant switching callbacks

**GOWindchest**: Logical sound grouping
- Groups pipes for synchronized processing
- Applies enclosure attenuation
- Associates tremulants with pipes

### Threading Model

- **Main thread**: UI updates, configuration changes
- **Audio thread**: Real-time callback from audio backend (high priority)
- **Worker threads**: `GOSoundThread` pool for parallel DSP processing
- **Lock-free scheduler**: Minimizes latency in audio path

### Configuration & Caching

- **ODF files**: Organ Definition Format (organ structure)
- **CMB files**: Combination files (user settings/state)
- **Cache files**: Precomputed pipe audio (WavPack compressed)
- `GOCache`, `GOCacheWriter`: Handle serialization

### MIDI Integration

- `GOMidi`: Central MIDI interface
- `GOMidiListener`: Event listener interface
- `GOMidiRecorder`/`GOMidiPlayer`: Record/playback functionality
- Events dispatched via `GOEventDistributor`

## Code Organization

### Main Source Directories

- `src/grandorgue/`: Main application (wxWidgets GUI, controllers, config)
  - `combinations/`: Combination system (pistons, generals, divisionals)
  - `config/`: Configuration management
  - `control/`: Organ control elements (buttons, sliders, etc.)
  - `document-base/`: Document base classes
  - `gui/`: wxWidgets UI panels
  - `help/`: Help system
  - `loader/`: ODF parsing
  - `midi/`: MIDI handling
  - `model/`: Organ model (pipes, stops, windchests)
  - `modification/`: Organ modification tracking
  - `sound/`: Audio engine and sound processing
  - `updater/`: ODF/CMB version updater
  - `yaml/`: YAML serialization utilities

- `src/core/`: Core utilities (file I/O, memory management, abstractions)

- `src/portaudio/`: PortAudio wrapper and integration

- `src/rt/`: RtAudio and RtMidi library integration

- `src/tests/`: Test framework and test suites
  - `common/`: Test infrastructure (`GOTest`, `GOTestCollection`)
  - `testing/`: Actual test implementations

- `perftests/`: Performance test data (WAV files for benchmarking)

- `src/tools/`: Command-line tools (GrandOrgueTool, GrandOrguePerfTest)

- `src/build/`: Build-time code generation tools

### Build Configuration Options

Key CMake options (see `CMakeLists.txt` for full list):
- `RTAUDIO_USE_JACK`: Enable JACK support (default: ON on Linux)
- `RTAUDIO_USE_ALSA`: Enable ALSA support (Linux, default: ON)
- `RTAUDIO_USE_ASIO`: Enable ASIO support (Windows, default: ON)
- `GO_BUILD_TESTING`: Enable test suite (default: OFF, set to ON for testing)
- `INSTALL_DEMO`: Install demo sampleset (default: ON)
- `USE_INTERNAL_PORTAUDIO`: Use bundled PortAudio (default: ON)
- `CMAKE_BUILD_TYPE`: `Release` or `Debug`

## Coding Conventions

### Naming: Variables and Parameters

- **Style**: `camelCase` for local variables and parameters
  - ✅ `devIndex`, `nSamples`, `outputIndex`
  - ❌ `dev_index`, `n_frames`, `audio_output`

- **Count variables**: prefix `n` (not `Count` suffix)
  - ✅ `nAudioGroups`, `nSamples`, `nFrames`, `nSamplesPerBuffer`
  - ❌ `audioGroupCount`, `sampleCount`, `frameCount`

- **Loop index from 0**: suffix `I`
  - ✅ `for (unsigned sampleI = 0; sampleI < nSamples; ++sampleI)`
  - ✅ `channelI`, `deviceI`, `frameI`

- **Loop number from 1**: suffix `N`
  - ✅ `for (unsigned deviceN = 1; deviceN <= deviceCount; ++deviceN)`
  - ✅ `channelN`, `stopN`

### Naming: Member Variables

- `m_` prefix; simple names lowercase, compound names PascalCase
  - Simple: ✅ `m_buffer`, `m_data`
  - Compound: ✅ `m_AudioOutput`, `m_NChannels`, `m_InterleavedBuffer`
  - Rule: `audioOutput` → `m_AudioOutput`, `nChannels` → `m_NChannels`
  - ❌ No prefix: `interleavedBuffer`, `channels`, `sampleRate`

### Naming: Pointers

- **Local pointers**: `pCamelCase`
  - ✅ `pData`, `pBuffer`, `pDevice`, `pSrc`, `pDst`

- **Non-owning member pointers**: `p_` prefix
  - Simple names lowercase: `p_buffer`, `p_data`
  - Compound names PascalCase: `p_AudioOutput`, `p_AudioEngine`

- **Owning member pointers**: `mp_` prefix
  - Simple names lowercase: `mp_buffer`, `mp_device`
  - Compound names PascalCase: `mp_AudioOutput`, `mp_AudioEngine`

- **Non-owning references stored as members** (injected via constructor): `r_` prefix
  - ✅ `GOOrganModel &r_OrganModel`, `GOMemoryPool &r_MemoryPool`
  - Prefer reference over pointer when non-nullable

- **Never compare pointers explicitly with `nullptr`** — use implicit bool conversion
  - ✅ `if (pEngine)`, `return mp_Engine.load();`
  - ❌ `if (pEngine != nullptr)`, `return mp_Engine.load() != nullptr;`

### Naming: Output Parameters

- Place at **end** of parameter lists, prefix with `out`
  - ✅ `void GetAudioOutput(unsigned outputIndex, bool isLast, GOSoundBufferMutable &outOutputBuffer)`
  - ❌ `void GetAudioOutput(GOSoundBufferMutable &outputBuffer, unsigned outputIndex, bool isLast)`

### Naming: Boolean Variables

- **Must start with a question word**: `is`, `has`, `are`, `was`, `can`, `will`, etc.
  - ✅ `isEnabled`, `hasTask`, `isDirect`, `wasSetup`
  - ❌ `enabled`, `direct`, `downmix`, `taskBuilt`
- Applies to all contexts: local vars, parameters, struct fields, member variables (after prefix)
  - ✅ `m_IsWorking`, `m_HasTaskBuilt`, `m_IsDownmix`
  - ❌ `m_Downmix`, `m_Working`, `m_TaskBuilt`

### Naming: Boolean Accessors

- **Setter**: `Set` + name **without** question word
  - ✅ `SetDownmix(bool isDownmix)`, `SetScaleRelease(bool isScaleRelease)`
  - ❌ `SetIsDownmix(...)`, `SetIsScaleRelease(...)`
- **Getter**: question word directly, **no** `Get` prefix
  - ✅ `IsDownmix()`, `HasTaskBuilt()`, `IsWorking()`
  - ❌ `GetIsDownmix()`, `GetDownmix()`

### Naming: Types and Functions

- **Classes/structs**: `PascalCase` (e.g., `GOSoundEngine`, `GOSoundBuffer`)
- **Enum values**: `UPPER_CASE`, no prefix (e.g., `IDLE`, `WORKING`, `USED`)
- **File-scope static functions**: `snake_case` (e.g., `assert_item_equal`, `fill_with_sine_wave`)
- **Static class member functions**: `camelCase` (e.g., `fillWithSequential`, `computeOffset`)

### Naming: Idempotent Methods

- Complex idempotent methods (not setters) should start with `Ensure`
  - ✅ `EnsureInitialized()`, `EnsureAllocated()`, `EnsureBufferReady()`
  - ❌ `Initialize()`, `Allocate()` (for idempotent operations)

### Header vs Implementation Files

- **Single-line methods** only in `.h` (inline): `bool IsOrganStarted() const { return m_IsOrganStarted; }`
- **Two-line and multi-line methods** must be in `.cpp`
  - ✅ `void OnSoundClosing(...) override;` in `.h`, implementation in `.cpp`
  - ❌ Multi-line body in `.h`

### Code Formatting

- **Empty line** between variable declarations and surrounding code (both before and after the declaration block), but no empty lines between consecutive declarations:
  ```cpp
  // ✅
  const unsigned nChannels = 2;
  const unsigned nFrames = 4;
  const unsigned nItems = nChannels * nFrames;

  std::vector<float> data(nItems);
  ```

- **Ternary operator** for simple conditional returns:
  - ✅ `return condition ? trueValue : falseValue;`
  - ❌ Multi-line if/else for simple returns

- **Prefer references over pointers** where appropriate:
  - ✅ `GOSoundOutput &device = m_AudioOutputs[devIndex];`
  - ❌ `GOSoundOutput *device = &m_AudioOutputs[devIndex];`

### Control Flow

- **Single return point**: functions must have only one `return` statement, at the end
  - ✅ Use guard variables or restructure logic to avoid early returns
  - ❌ Early returns (`if (condition) return;`) are not allowed

- **Avoid `continue`** in loops — use `if` to wrap the body instead
  - ✅ `if (cond) { ... loop body ... }`
  - ❌ `if (!cond) continue;`

- **Avoid recomputing `size()` in loop condition** — compute once before the loop:
  - ✅ `for (unsigned n = v.size(), i = 0; i < n; i++)`
  - ❌ `for (unsigned i = 0; i < v.size(); i++)`

- **Loop variable naming** depends on whether the bound is already in scope:
  - Bound is a named variable already in scope → use named index, reference bound directly:
    - ✅ `for (unsigned itemI = 0; itemI < nItems; ++itemI)`
  - Bound is a method call or expression → both named (multi-line body) or both unnamed (single-line body):
    - ✅ multi-line: `for (unsigned nGroups = ..., groupI = 0; groupI < nGroups; groupI++)`
    - ✅ single-line: `for (unsigned n = v.size(), i = 0; i < n; i++)`
    - ❌ mixed: `for (unsigned n = v.size(), groupI = 0; groupI < n; groupI++)`

- **Use range-based `for`** when loop index is not used in body:
  - ✅ `for (GOSoundThread *pThread : m_Threads) pThread->Wakeup();`
  - ❌ `for (unsigned n = m_Threads.size(), i = 0; i < n; i++) m_Threads[i]->Wakeup();`

- **Pre-compute repeated subexpressions** (e.g. `v[i]`, `v[i].size()`) into named variables before use

### Includes and Forward Declarations

- **Sort alphabetically within a group**
  - Forward declarations: all `class GOSound*;` together, sorted
  - Includes: all includes in a group sorted by full path
- Avoid duplicate includes — if a header is included in `.h`, don't re-include in `.cpp`

### Memory Management and Assertions

- **Prefer RAII** and managed objects over manual memory management
  - ✅ `GOSoundBufferManaged m_Buffer;`
  - ❌ `float *m_Buffer = new float[size]; ... delete m_Buffer;`
- Use `assert()` to check invariants, especially in real-time audio code
- Add documentation for all new classes, functions, and methods in header files

## Important Patterns

### Buffer Handling
Always use buffer wrapper classes (`GOSoundBuffer`, `GOSoundBufferMutable`, `GOSoundBufferManaged`). Pass buffers by reference when possible. Use managed buffers for owned memory.

### Real-Time Audio Code
- Avoid allocations in audio callback path
- Use pool-based allocation for samplers
- Assert invariants (e.g., buffer size validation)
- Minimize locks (use lock-free scheduler)

### Callback Chain
When implementing audio callbacks, follow the signature:
```cpp
bool AudioCallback(GOSoundBufferMutable &outputBuffer)
```
Return `true` to continue, `false` to stop audio.

### Task-Based Processing
For multi-threaded audio processing, create tasks that inherit from appropriate base classes and register with `GOSoundScheduler`. Tasks execute on worker threads from the thread pool.

### Replacing Methods
When eliminating redundant methods, prefer using existing object methods rather than custom helpers:
- ✅ `outOutputBuffer.FillWithSilence()`
- ❌ `m_SoundEngine.GetEmptyAudioOutput(devIndex, nFrames, outputBuffer)`

## Common Development Workflows

### Adding a New Audio Backend
1. Create new class inheriting from `GOSoundPort`
2. Implement `AudioCallback()` method
3. Handle latency reporting
4. Integrate with `GOSound` for lifecycle management
5. Add CMake option to enable/disable backend

### Modifying Audio Processing
1. Locate relevant task class in `GOSoundScheduler`
2. Modify task's `Run()` method for DSP changes
3. Ensure thread-safety (tasks run in parallel)
4. Add assertions for buffer size validation
5. Test with various buffer sizes and sample rates

### Adding New Organ Components
1. Create model class in `src/grandorgue/model/`
2. Implement serialization (ODF loading, cache support)
3. Add to `GOOrganModel` ownership/lifecycle
4. Create controller logic if needed
5. Add GUI representation in `src/grandorgue/gui/`

## Repository Information

- **License**: GPL-2.0-or-later
- **Main branch**: `master`
- **C++ Standard**: C++20
- **Build system**: CMake 3.10+
- **GUI Framework**: wxWidgets
- **Primary platforms**: Linux, Windows, macOS
