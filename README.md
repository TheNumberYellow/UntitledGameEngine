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
- Add billboards
#### UI Module
- Optimize + clean up UI Module functions.
- Add scrolling + other immediate mode UI concepts.
  - Scrolling
  - Fields for more variable types
- Add frame dynamic resizing, minimizing, etc.
- Add concept of overlapping UI elements (so clicks are only routed to the topmost element)
#### Collision Module
- Add triangle octree for complex meshes
- Add octree for broad pruning of meshes 
#### Physics Module
- idk do physics things lol
#### Asset Manager
- Unified manager for loading all asset types (and managing asset lifetimes/preventing double loading)
#### Editor
- Improve editor experience.
  - Add inspector window changes.
- Add vertex edit tool to edit brushes/blocks.
- Improve terrain editing
  - Dynamic terrain tesselation
  - Tweak terrain sculpt parameters, add strength/brush type parameters
- Add non-Model editor constructs (for things like point lights, or anything which doesn't have a runtime graphical representation)
#### Scene
- Create a file type for named data (or use JSON) for scene loading/saving.
#### New features
- Implement states and state switching.
- Add a network module.
