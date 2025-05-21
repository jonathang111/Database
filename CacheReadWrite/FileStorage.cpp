#include "FileStorage.h"

bool FileStorage::open(const std::string& path, std::ios::openmode mode){
    file.open(path, std::ios::in | std::ios::out | std::ios::binary);
    filename = path;
    return file.is_open();
}

void FileStorage::close(){
    file.close();
}

size_t FileStorage::read(void* buffer, size_t size){
    file.read(reinterpret_cast<char*>(buffer), size);
    return static_cast<size_t>(file.gcount());
}

size_t FileStorage::write(const void* data, size_t size){
    file.write(reinterpret_cast<const char*>(data), size);
    return size;
}

void FileStorage::seek(size_t offset){
    file.seekg(offset);
    file.seekp(offset);
}
/*Stream pos is position marker, marks position in stream
It's opaque, not guaranteed to be plain integer
Streamoff is athe actual signed int represnting the number of bytes
Stream pos ins't castable to numeric, need to cast to streamoff first.
return static_cast<size_t>(static_cast<std::streamoff>(pos)); but
return static_cast<size_t>(pos); might work too*/
size_t FileStorage::tell() const{
    std::streampos pos = file.tellp();
    return static_cast<size_t>(pos);
}

bool FileStorage::isOpen() const{
    return file.is_open();
}

bool FileStorage::eof() const{
    return file.eof();
}

const std::string FileStorage::getLabel() const{
    return filename;
}

FileStorage::~FileStorage(){
    close();
}