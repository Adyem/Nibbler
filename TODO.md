# Nibbler Project TODO List
---

## 📋 **Mandatory Requirements Compliance Check**

### **Core Architecture ✅/❌**
- [x] Main executable separate from graphics libraries
- [x] 3 dynamic libraries requirement
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

1. [ ] **Complete Testing**
   - [ ] Test all three libraries work independently
   - [ ] Test library switching (1/2/3 keys) during gameplay
   - [ ] Test edge cases (invalid args, missing libs)
   - [ ] Test game rules compliance

### **Medium Priority**
2. [ ] **Code Quality & Compliance**
   - [ ] Review all code for PDF requirement compliance
   - [ ] Ensure no graphics lib references in main
   - [ ] Check error handling completeness
---