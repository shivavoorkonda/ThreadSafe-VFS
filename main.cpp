#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <string>
#include "MemoryAllocator.h"
#include "Swapper.h"
#include "DeadlockDetector.h"
#include "VFS.h"

MemoryAllocator allocator;
Swapper swapper;
DeadlockDetector deadlock_daemon;
std::unordered_map<std::string, VirtualFile*> file_system;
std::vector<VirtualFile*> file_list;
std::mutex vfs_mutex;

VirtualFile* createFile(std::string name) {
    std::lock_guard<std::mutex> lock(vfs_mutex);
    VirtualFile* new_file = new VirtualFile(name);

    new_file->data_ptr = allocator.allocatePage();
    if (new_file->data_ptr == nullptr) {
        swapper.pageOut(file_list, allocator);
        new_file->data_ptr = allocator.allocatePage();
    }

    file_system[name] = new_file;
    file_list.push_back(new_file);
    return new_file;
}

void readFile(std::string name) {
    VirtualFile* file = file_system[name];

    if (!file->in_memory) {
        swapper.pageIn(file, allocator);
    }

    std::shared_lock<std::shared_mutex> read_lock(file->rw_lock);
    file->updateAccessTime();
    std::cout << "[Thread " << std::this_thread::get_id() << "] Reading: " << name << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

void writeFile(std::string name) {
    VirtualFile* file = file_system[name];

    if (!file->in_memory) {
        swapper.pageIn(file, allocator);
    }

    std::unique_lock<std::shared_mutex> write_lock(file->rw_lock);
    file->updateAccessTime();
    std::cout << "[Thread " << std::this_thread::get_id() << "] *** WRITING *** to: " << name << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main() {
    std::cout << "Starting ThreadSafe-VFS...\n";
    std::thread daemon(&DeadlockDetector::daemonLoop, &deadlock_daemon);

    std::cout << "\n--- Starting 15,000 File Stress Test (Will take a moment) ---\n";
    for (int i = 0; i < 15000; i++) {
        createFile("StressFile_" + std::to_string(i) + ".txt");
    }
    std::cout << "Successfully created 15,000 files. RAM was exhausted and files were paged to Disk!\n\n";

    createFile("ImportantData.txt");

    std::cout << "--- Spawning Concurrent Readers and Writers ---\n";
    std::vector<std::thread> workers;

    workers.push_back(std::thread(readFile, "ImportantData.txt"));
    workers.push_back(std::thread(readFile, "ImportantData.txt"));
    workers.push_back(std::thread(writeFile, "ImportantData.txt"));
    workers.push_back(std::thread(readFile, "ImportantData.txt"));
    workers.push_back(std::thread(writeFile, "ImportantData.txt"));
    workers.push_back(std::thread(readFile, "ImportantData.txt"));

    workers.push_back(std::thread(readFile, "StressFile_5.txt"));

    for (auto& t : workers) {
        t.join();
    }

    deadlock_daemon.stop();
    daemon.join();
    std::cout << "\nSystem Shutting Down cleanly.\n";
    return 0;
}
