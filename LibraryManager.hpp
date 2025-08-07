#ifndef LIBRARYMANAGER_HPP
#define LIBRARYMANAGER_HPP

#include "IGraphicsLibrary.hpp"
#include <string>
#include <vector>

class LibraryManager {
public:
    LibraryManager();
    ~LibraryManager();

    // Load a graphics library from a shared object file
    // Returns 0 on success, non-zero on error
    int loadLibrary(const std::string& libraryPath);

    // Unload the currently loaded library
    void unloadCurrentLibrary();

    // Switch to a different library by index (0, 1, 2 for libraries 1, 2, 3)
    // Returns 0 on success, non-zero on error
    int switchToLibrary(int libraryIndex);

    // Get the currently active graphics library
    IGraphicsLibrary* getCurrentLibrary() const;

    // Get the number of loaded libraries
    size_t getLibraryCount() const;

    // Get the name of a library by index
    const char* getLibraryName(int index) const;

    // Get the last error message
    const char* getError() const;

    // Check if any library is currently loaded
    bool hasActiveLibrary() const;

private:
    struct LibraryInfo {
        void* handle;                           // dlopen handle
        IGraphicsLibrary* instance;             // Library instance
        std::string path;                       // Path to the .so file
        std::string name;                       // Library name

        // Function pointers from the dynamic library
        IGraphicsLibrary* (*createFunc)();
        void (*destroyFunc)(IGraphicsLibrary*);
        const char* (*getNameFunc)();
        const char* (*getVersionFunc)();
    };

    std::vector<LibraryInfo> _libraries;        // All loaded libraries
    int _currentLibraryIndex;                   // Index of currently active library
    std::string _lastError;                     // Last error message

    // Helper methods
    void clearError();
    void setError(const std::string& error);
    bool loadLibrarySymbols(LibraryInfo& libInfo);
    void cleanupLibrary(LibraryInfo& libInfo);
};

#endif // LIBRARYMANAGER_HPP