# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C++ OpenGL assignment (student ID: 1652026) for a Computer Graphics course (Đồ họa máy tính / DHMT), academic year 2025-2026 HK II. The task is to build an interactive 3D model of a simple mechanical gyroscope device.

**Deadline:** 23:00 on 16/5/2026. Submit 2 files (uncompressed) to dhmt.bku@gmail.com:
1. `assignment-1652026.cpp` (all source merged into one file, filename lowercase)
2. `Phieuchamdiem-1652026.xls`

The window title must display student name and ID (penalty: -1.0 point if missing).

## Build System

The project uses Code::Blocks 13.12 with MinGW GCC on Windows (grader uses this exact version).

**Open and build in CodeBlocks:**
```
Open: assignment-1652026/assignment-1652026.cbp
Build > Build (or F9)
Output: assignment-1652026/bin/Debug/assignment-1652026.exe
```

**Manual GCC build (from `assignment-1652026/` directory):**
```
g++ main.cpp -o output.exe -IF:/CodeBlocks/MinGW/include -LF:/CodeBlocks/MinGW/lib -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
```

The `glut32.dll` runtime must be in the same directory as the executable (already present at project root).

## Architecture

Single-file project: all logic is in [assignment-1652026/main.cpp](assignment-1652026/main.cpp).

**GLUT callback structure:**
- `display()` — clears the screen, pushes/pops matrix state to position and rotate 6 shapes (3 solid + 3 wireframe in two rows), calls `glutSwapBuffers()`
- `resize(w, h)` — rebuilds the perspective projection matrix on window resize
- `key(k, x, y)` — handles ESC/q (exit), `+`/`-` (adjust `slices`/`stacks` geometry detail, minimum 3)
- `idle()` — calls `glutPostRedisplay()` to drive continuous animation
- `main()` — GLUT init, OpenGL state setup (depth test, lighting, face culling), material/light properties, enters `glutMainLoop()`

**Global state:** only `slices` and `stacks` (int, default 16) control tessellation detail for all shapes.

## Assignment Requirements

### Model to Build — Gyroscope Device

Six parts with a strict parent-child hierarchy (child inherits parent's rotation):

| Part | Vietnamese | Shape | Color | Rotates when parent rotates |
|------|-----------|-------|-------|----------------------------|
| Base (Đế) | — | 2 cylinders | red | — (root) |
| Frame (Khung) | — | 1 torus + 2 cylinders | purple | yes, follows base |
| Gimbal 1 (Khớp vạn năng 1) | — | 1 torus + 2 cylinders | green | yes, follows frame |
| Gimbal 2 (Khớp vạn năng 2) | — | 1 torus | red | yes, follows gimbal 1 |
| Axis (Trục) | — | 1 cylinder | red | fixed to gimbal 2 |
| Rotor (Đĩa quay) | logo BK | BK university logo | — | spins around axis |

- Base rotates freely around its own vertical axis; all other parts rotate with it.
- Gimbal 1 rotates around the axis defined by frame's two cylinders.
- Gimbal 2 rotates around the axis defined by gimbal 1's two cylinders.
- Rotor spins around the axis (trục).

**Floor:** Draw (not texture) a 20×10 tile grid totaling 100 tiles, each with patterned design as in the demo.

### Technical Constraints

- Only OpenGL/GLUT/GLU libraries allowed — no external libraries.
- **Must NOT use** `glutSolidCube`, `glutSolidSphere`, `glutSolidCylinder`, or any other `glutSolid*`/`glutWire*` primitives. All geometry must be implemented via a custom `Mesh` class.
- Shading must create a convincing 3D appearance (lighting + material properties).

### Camera System

Three global variables control the camera position; `gluLookAt` is recalculated on every change (only the eye position changes — look-at target and up vector stay fixed):

```cpp
float camera_angle;   // rotation around Oy axis
float camera_height;  // height above xOz plane
float camera_dis;     // distance from Oy axis
```

Eye position: `(camera_dis * sin(camera_angle), camera_height, camera_dis * cos(camera_angle))`

### Keyboard Controls

| Key | Action |
|-----|--------|
| 1 | Rotate base counter-clockwise |
| 2 | Rotate base clockwise |
| 3 | Rotate gimbal 1 counter-clockwise |
| 4 | Rotate gimbal 1 clockwise |
| 5 | Rotate gimbal 2 counter-clockwise |
| 6 | Rotate gimbal 2 clockwise |
| 7 | Rotate rotor (BK logo) counter-clockwise |
| 8 | Rotate rotor (BK logo) clockwise |
| R / r | Reset all rotations to initial state |
| S / s | Switch to smooth shading |
| F / f | Switch to flat shading |
| + | Increase camera distance to Oy (`camera_dis`) |
| - | Decrease camera distance to Oy (`camera_dis`) |
| ↑ | Increase camera height (`camera_height`) |
| ↓ | Decrease camera height (`camera_height`) |
| → | Rotate camera clockwise (`camera_angle`) |
| ← | Rotate camera counter-clockwise (`camera_angle`) |

### Scoring Summary (10 points total)

| Category | Item | Points |
|----------|------|--------|
| Model (4.0 pts) | Base | 0.5 |
| | Frame | 0.5 |
| | Gimbal 1 | 0.5 |
| | Gimbal 2 | 0.5 |
| | Axis | 0.5 |
| | Rotor (BK logo) | 0.75 |
| | Floor (drawn tiles with pattern) | 0.75 |
| Device controls (3.75 pts) | Keys 1,2 (base) | 0.5 |
| | Keys 3,4 (gimbal 1) | 0.5 |
| | Keys 5,6 (gimbal 2) | 0.5 |
| | Keys 7,8 (rotor) | 0.5 |
| | R/r (reset) | 0.25 |
| | S/s (smooth shading toggle) | 1.5 |
| Camera (0.75 pts) | +/- (distance) | 0.25 |
| | ↑/↓ (height) | 0.25 |
| | →/← (rotation) | 0.25 |
| Shading (1.5 pts) | 3D coloring per demo | 1.5 |
