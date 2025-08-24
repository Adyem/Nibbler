#pragma once

#include "IGraphicsLibrary.hpp"
#include <string>
#include <vector>

class LibraryManager {
  public:
    LibraryManager();
    ~LibraryManager();

    int loadLibrary(const std::string& libraryPath);

    void unloadCurrentLibrary();

    int switchToLibrary(int libraryIndex);

    IGraphicsLibrary* getCurrentLibrary() const;

    int getCurrentLibraryIndex() const;

    size_t getLibraryCount() const;

    const char* getLibraryName(int index) const;

    const char* getError() const;

    bool hasActiveLibrary() const;

  private:
    struct LibraryInfo {
        void* handle;
        IGraphicsLibrary* instance;
        std::string path;
        std::string name;

        IGraphicsLibrary* (*createFunc)();
        void (*destroyFunc)(IGraphicsLibrary*);
        const char* (*getNameFunc)();
        const char* (*getVersionFunc)();
    };

    std::vector<LibraryInfo> _libraries;
    int _currentLibraryIndex;
    std::string _lastError;

    void clearError();
    void setError(const std::string& error);
    bool loadLibrarySymbols(LibraryInfo& libInfo);
    void cleanupLibrary(LibraryInfo& libInfo);
};