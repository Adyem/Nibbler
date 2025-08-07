#include "LibraryManager.hpp"
#include <dlfcn.h>
#include <iostream>

LibraryManager::LibraryManager() : _currentLibraryIndex(-1) {
    clearError();
}

LibraryManager::~LibraryManager() {
    // Clean up all loaded libraries
    for (auto& libInfo : _libraries) {
        cleanupLibrary(libInfo);
    }
    _libraries.clear();
}

int LibraryManager::loadLibrary(const std::string& libraryPath) {
    clearError();

    // Load the shared library
    void* handle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    if (!handle) {
        setError(std::string("Failed to load library: ") + dlerror());
        return 1;
    }

    LibraryInfo libInfo;
    libInfo.handle = handle;
    libInfo.path = libraryPath;
    libInfo.instance = nullptr;

    // Load the required symbols
    if (!loadLibrarySymbols(libInfo)) {
        dlclose(handle);
        return 1;
    }

    // Get library name
    if (libInfo.getNameFunc) {
        libInfo.name = libInfo.getNameFunc();
    } else {
        libInfo.name = "Unknown Library";
    }

    // Create an instance of the library
    if (libInfo.createFunc) {
        libInfo.instance = libInfo.createFunc();
        if (!libInfo.instance) {
            setError("Failed to create library instance");
            dlclose(handle);
            return 1;
        }
    } else {
        setError("Library does not provide createGraphicsLibrary function");
        dlclose(handle);
        return 1;
    }

    // Add to our list of libraries
    _libraries.push_back(libInfo);

    // If this is the first library, make it active
    if (_currentLibraryIndex == -1) {
        _currentLibraryIndex = static_cast<int>(_libraries.size() - 1);
    }

    return 0;
}

void LibraryManager::unloadCurrentLibrary() {
    if (_currentLibraryIndex >= 0 && _currentLibraryIndex < static_cast<int>(_libraries.size())) {
        cleanupLibrary(_libraries[_currentLibraryIndex]);
        _libraries.erase(_libraries.begin() + _currentLibraryIndex);

        // Adjust current index
        if (_libraries.empty()) {
            _currentLibraryIndex = -1;
        } else if (_currentLibraryIndex >= static_cast<int>(_libraries.size())) {
            _currentLibraryIndex = static_cast<int>(_libraries.size()) - 1;
        }
    }
}

int LibraryManager::switchToLibrary(int libraryIndex) {
    clearError();

    if (libraryIndex < 0 || libraryIndex >= static_cast<int>(_libraries.size())) {
        setError("Invalid library index");
        return 1;
    }

    _currentLibraryIndex = libraryIndex;
    return 0;
}

IGraphicsLibrary* LibraryManager::getCurrentLibrary() const {
    if (_currentLibraryIndex >= 0 && _currentLibraryIndex < static_cast<int>(_libraries.size())) {
        return _libraries[_currentLibraryIndex].instance;
    }
    return nullptr;
}

size_t LibraryManager::getLibraryCount() const {
    return _libraries.size();
}

const char* LibraryManager::getLibraryName(int index) const {
    if (index >= 0 && index < static_cast<int>(_libraries.size())) {
        return _libraries[index].name.c_str();
    }
    return nullptr;
}

const char* LibraryManager::getError() const {
    return _lastError.c_str();
}

bool LibraryManager::hasActiveLibrary() const {
    return _currentLibraryIndex >= 0 && _currentLibraryIndex < static_cast<int>(_libraries.size());
}

// Private helper methods
void LibraryManager::clearError() {
    _lastError.clear();
}

void LibraryManager::setError(const std::string& error) {
    _lastError = error;
}

bool LibraryManager::loadLibrarySymbols(LibraryInfo& libInfo) {
    // Clear any existing dlerror
    dlerror();

    // Load required function symbols
    libInfo.createFunc = reinterpret_cast<IGraphicsLibrary* (*)()>(
        dlsym(libInfo.handle, "createGraphicsLibrary"));

    libInfo.destroyFunc = reinterpret_cast<void (*)(IGraphicsLibrary*)>(
        dlsym(libInfo.handle, "destroyGraphicsLibrary"));

    libInfo.getNameFunc = reinterpret_cast<const char* (*)()>(
        dlsym(libInfo.handle, "getLibraryName"));

    libInfo.getVersionFunc = reinterpret_cast<const char* (*)()>(
        dlsym(libInfo.handle, "getLibraryVersion"));

    // Check for errors
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
        setError(std::string("Failed to load library symbols: ") + dlsym_error);
        return false;
    }

    // Verify required functions are present
    if (!libInfo.createFunc || !libInfo.destroyFunc) {
        setError("Library missing required functions (createGraphicsLibrary or destroyGraphicsLibrary)");
        return false;
    }

    return true;
}

void LibraryManager::cleanupLibrary(LibraryInfo& libInfo) {
    if (libInfo.instance && libInfo.destroyFunc) {
        libInfo.destroyFunc(libInfo.instance);
        libInfo.instance = nullptr;
    }

    if (libInfo.handle) {
        dlclose(libInfo.handle);
        libInfo.handle = nullptr;
    }
}