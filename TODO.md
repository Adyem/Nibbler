# Nibbler Project TODO List
---

## 📋 **Mandatory Requirements Compliance Check**
- [x] extern "C" entry points for dynamic loading

### **Game Logic Requirements** bastian
- [ ] **Verify**: Snake starts with size 4 in middle of area
- [ ] **Verify**: Constant speed movement
- [ ] **Verify**: Cannot move backwards
- [ ] **Verify**: Food spawning (at least 1 always present)
- [ ] **Verify**: Snake growth on food consumption
- [ ] **Verify**: Game over on wall/self collision
- [ ] **Verify**: Keyboard controls (arrows + 1/2/3 + ESC)

### **Input/Output Requirements** rodolfo
- [ ] **Test**: Width/height command line parameters
- [x] **Test**: Usage message for wrong arguments
- [x] **Test**: Error handling for invalid dimensions
- [ ] **Test**: Error handling for missing/incompatible libraries
- [ ] **Test**: Library switching with keys 1, 2, 3
- [ ] **Test**: ESC key to quit game
