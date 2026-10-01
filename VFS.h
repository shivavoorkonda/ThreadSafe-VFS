#pragma once
#include <string>
#include <shared_mutex>
#include <chrono>

struct VirtualFile {
    std::string name;
    char* data_ptr;       
    bool in_memory;       
    size_t size;
    long long last_accessed; 

    mutable std::shared_mutex rw_lock; 

    VirtualFile(std::string n) : name(n), data_ptr(nullptr), in_memory(true), size(0) {
        updateAccessTime();
    }

    void updateAccessTime() {
        last_accessed = std::chrono::system_clock::now().time_since_epoch().count();
    }
};
