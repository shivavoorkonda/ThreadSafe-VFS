#pragma once
#include <vector>
#include <mutex>
#include <iostream>

class MemoryAllocator {
private:
    static constexpr size_t POOL_SIZE = 50 * 1024 * 1024;
    static constexpr size_t PAGE_SIZE = 4096;
    static constexpr size_t NUM_PAGES = POOL_SIZE / PAGE_SIZE;

    char* memory_pool;
    std::vector<bool> free_pages;
    std::mutex alloc_mutex;

public:
    MemoryAllocator() {
        memory_pool = new char[POOL_SIZE];
        free_pages.assign(NUM_PAGES, true);
        std::cout << "[SYSTEM] Initialized 50MB Custom Memory Pool.\n";
    }

    ~MemoryAllocator() {
        delete[] memory_pool;
    }

    char* allocatePage() {
        std::lock_guard<std::mutex> lock(alloc_mutex);
        for (size_t i = 0; i < NUM_PAGES; ++i) {
            if (free_pages[i]) {
                free_pages[i] = false;
                return memory_pool + (i * PAGE_SIZE);
            }
        }
        return nullptr;
    }

    void freePage(char* ptr) {
        std::lock_guard<std::mutex> lock(alloc_mutex);
        size_t index = (ptr - memory_pool) / PAGE_SIZE;
        if (index < NUM_PAGES) {
            free_pages[index] = true;
        }
    }
};
