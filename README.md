# GameEngine

## To-Do list

#### Graphics Module
- Clean up and provide interface for switching skybox in Graphics Module.
- Implement cascaded shadowmaps, and look deeper into shadow acne problems.
  - https://docs.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps 
- Improve materials system (PBR)
  - Implement image based lighting (IBL)
  - Implement parallax mapping with height maps.
- Add light types (point lights, directional spotlights, etc.)
  - Improve light (and general) performance
#### UI Module
- Add scrolling + other immediate mode UI concepts.
  - Scrolling
  - Fields for more variable types
- Add deferred UI render commands
#### Physics Module
- Start implementing physics engine
#### Asset Manager
- Unified manager for loading all asset types (and managing asset lifetimes/preventing double loading)
#### Editor
- Add vertex edit tool to edit brushes/blocks.
- Improve terrain editing
  - Dynamic terrain tesselation
  - Tweak terrain sculpt parameters, add strength/brush type parameters
#### Scene
- Create a file type for named data (or use JSON) for scene loading/saving.
#### New features
- Add a network module.
