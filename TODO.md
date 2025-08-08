# Nibbler Project TODO List
*Generated: August 7, 2025*

## 🚀 **CRITICAL: 3rd Graphics Library Implementation**

### **MISSING: Third Graphics Library**
- [ ] **URGENT**: Implement 3rd graphics library (currently only have NCurses and SDL2)
- [ ] **Options for 3rd library:**
  - [ ] **SFML** (Recommended - modern, well-documented)
  - [ ] **OpenGL** (Raw OpenGL with GLFW/GLUT)
  - [ ] **Allegro** (Cross-platform alternative)
  - [ ] **FLTK** (Lightweight GUI toolkit)
- [ ] Create `SFMLGraphics.cpp` and `SFMLGraphics.hpp` files
- [ ] Implement all IGraphicsLibrary interface methods
- [ ] Add extern "C" factory functions
- [ ] Update Makefile to build 3rd library
- [ ] Test dynamic loading of 3rd library

---

## 📋 **Mandatory Requirements Compliance Check**

### **Core Architecture ✅/❌**
- [x] Main executable separate from graphics libraries
- [ ] 3 dynamic libraries requirement ❌ (Only 2 implemented)
- [x] Dynamic loading using dlopen/dlsym/dlclose
- [x] No graphics library references in main executable
- [x] Uniform interface for all libraries (IGraphicsLibrary)
- [x] extern "C" entry points for dynamic loading

### **Game Logic Requirements**
- [ ] **Verify**: Snake starts with size 4 in middle of area
- [ ] **Verify**: Constant speed movement
- [ ] **Verify**: Cannot move backwards
- [ ] **Verify**: Food spawning (at least 1 always present)
- [ ] **Verify**: Snake growth on food consumption
- [ ] **Verify**: Game over on wall/self collision
- [ ] **Verify**: Keyboard controls (arrows + 1/2/3 + ESC)

### **Input/Output Requirements**
- [ ] **Test**: Width/height command line parameters
- [ ] **Test**: Usage message for wrong arguments
- [ ] **Test**: Error handling for invalid dimensions
- [ ] **Test**: Error handling for missing/incompatible libraries
- [ ] **Test**: Library switching with keys 1, 2, 3
- [ ] **Test**: ESC key to quit game

### **Technical Requirements**
- [x] C++ compilation with `-Wall -Wextra -Werror`
- [x] Makefile with standard rules
- [x] No `using namespace` statements
- [x] Functions not implemented in headers (except templates)
- [x] Canonical form for classes with attributes
- [ ] **Verify**: No external libraries pushed to repo

---

## 🔧 **Implementation Tasks**

### **High Priority**
1. [ ] **Implement 3rd Graphics Library (SFML)**
   - [ ] Research SFML installation/setup
   - [ ] Create SFMLGraphics class
   - [ ] Implement rendering system
   - [ ] Implement input handling
   - [ ] Test integration with main executable

2. [ ] **Complete Testing**
   - [ ] Test all three libraries work independently
   - [ ] Test library switching (1/2/3 keys) during gameplay
   - [ ] Test edge cases (invalid args, missing libs)
   - [ ] Test game rules compliance

### **Medium Priority**
3. [ ] **Code Quality & Compliance**
   - [ ] Review all code for PDF requirement compliance
   - [ ] Ensure no graphics lib references in main
   - [ ] Verify canonical form for all classes
   - [ ] Check error handling completeness

4. [ ] **Documentation & Setup**
   - [ ] Create setup script for dependencies
   - [ ] Update README with build instructions
   - [ ] Document library switching mechanism
   - [ ] Document supported platforms

### **Low Priority**
5. [ ] **Optional Improvements**
   - [ ] Add library auto-detection
   - [ ] Improve error messages
   - [ ] Add debug mode
   - [ ] Performance optimizations

---

## 🎯 **Bonus Features (After Mandatory Complete)**

### **Game Enhancements**
- [ ] Multiple game modes
  - [ ] Speed variations
  - [ ] Disappearing food
  - [ ] Score system
  - [ ] Obstacles
- [ ] Sound system (as dynamic library)
- [ ] Multiplayer support
- [ ] Network multiplayer

### **Technical Enhancements**
- [ ] CMake build system
- [ ] Cross-platform compatibility
- [ ] Better asset management
- [ ] Configuration files

---

## 🚨 **Critical Issues to Address**

1. **BLOCKING**: Missing 3rd graphics library
2. **HIGH**: Verify game rules implementation
3. **MEDIUM**: Test library switching functionality
4. **MEDIUM**: Validate error handling

---

## 📝 **Notes**

- **PDF Compliance**: Must have exactly 3 graphics libraries
- **Architecture**: Main executable only handles game logic
- **Dynamic Loading**: All graphics through dlopen at runtime
- **No External Libs**: Don't push SDL2/SFML/etc sources to repo
- **Setup Required**: Must provide dependency installation method

---

## ✅ **Progress Tracking**

**Completion Status:**
- Main Executable: ✅ Done
- Graphics Library 1 (NCurses): ✅ Done  
- Graphics Library 2 (SDL2): ✅ Done
- Graphics Library 3: ❌ **MISSING - CRITICAL**
- Game Logic: 🔄 Needs verification
- Testing: ❌ Not complete
- Error Handling: 🔄 Needs verification

**Next Actions:**
1. Implement SFML graphics library
2. Test all three libraries
3. Verify PDF compliance
4. Complete error handling