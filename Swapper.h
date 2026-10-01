#pragma once
#include <fstream>
#include <vector>
#include <iostream>
#include <climits>
#include "VFS.h"

class Swapper {
private:
    std::string swap_file = "swap.sys";

public:
    void pageOut(std::vector<VirtualFile*>& all_files, MemoryAllocator& allocator) {
        VirtualFile* oldest_file = nullptr;
        long long oldest_time = LLONG_MAX;

        for (auto* file : all_files) {
            if (file->in_memory && file->last_accessed < oldest_time) {
                oldest_time = file->last_accessed;
                oldest_file = file;
            }
        }

        if (oldest_file) {
            std::ofstream out(swap_file, std::ios::app | std::ios::binary);
            if (oldest_file->data_ptr) {
                out.write(oldest_file->data_ptr, 4096);
            }
            out.close();

            allocator.freePage(oldest_file->data_ptr);
            oldest_file->in_memory = false;
            oldest_file->data_ptr = nullptr;
        }
    }

    void pageIn(VirtualFile* file, MemoryAllocator& allocator) {
        std::cout << "[PAGE FAULT] File '" << file->name << "' not in RAM. Fetching from disk...\n";
        file->data_ptr = allocator.allocatePage();
        file->in_memory = true;
    }
};
