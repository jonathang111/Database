#pragma once
#include <string>
#include <ios>

class StorageInterface {
    public:
        virtual ~StorageInterface() = default;
        virtual bool open(const std::string& path, std::ios::openmode mode) = 0;
        virtual void close() = 0;
        virtual size_t read(void* buffer, size_t size) = 0;
        virtual size_t write(const void* data, size_t size) = 0;
        virtual void seek(size_t offset) = 0;
        virtual size_t tell() const = 0;
        virtual bool isOpen() const = 0;
        virtual bool eof() const = 0;
        virtual const std::string getLabel() const = 0;
};