#pragma once
#include "StorageInterface.h"
#include <fstream>
#include <string>
#include <cstring>

class FileStorage : public StorageInterface{
    private:
        mutable std::fstream file;
        std::string filename;
    public:
        bool open(const std::string& path, std::ios::openmode mode) override;
        void close() override;
        size_t read(void* buffer, size_t size) override;
        size_t write(const void* data, size_t size) override;
        void seek(size_t offset) override;
        size_t tell() const override;
        bool isOpen() const override;
        bool eof() const override;
        const std::string getLabel() const override;
        ~FileStorage() override;
};