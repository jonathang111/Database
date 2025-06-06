#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <optional>
#include <cstring>
#include "StorageInterface.h"
#include "FileStorage.h"

#define MAX_KEYSIZE 32 //add option for static keysize
#define BLOCK_SIZE 4096
#define GROWTH_FACTOR 1.5
#define MAX_CACHESIZE 32
#define MAP_TAG "MC3"
#define VECTOR_PAIR_TAG "VP1"
#define VERSION 0.05
#define DATE 00000

namespace CacheRW{
 //possible size increase of all offsets.
struct CacheHeader{
    char tag[4];
    int keyAmount;
    int entryAmount;
    float version;
    int date;
    int footerStart;
};

struct KeyIndexEntry{
    char* key;
    int keySize;
    int offset;
    int count;
};

//add member to signal static or dynamic key size
struct CacheMeta{
    char tag[4];
    char cacheName[MAX_CACHESIZE];
    int keyAmount;
    int entryAmount; 
    int date;
    float version;
    int footerStart;

    CacheMeta(CacheHeader meta, const char* fileName)
    : keyAmount(meta.keyAmount), entryAmount(meta.entryAmount), date(meta.date)
    , version(meta.version), footerStart(meta.footerStart){
        strcpy(tag, meta.tag);
        strcpy(cacheName, fileName);
        cacheName[MAX_CACHESIZE-1] = '\0';
    }
};


namespace Internal{
    //Trait declarations for use in StoreToCache - defintions of partial specialized in Internal
    template<typename T>
    struct is_map;
    
    template<typename T>
    struct is_vector;

    template<typename T>
    struct is_raw_pointer;

    //there is no need for defining a path for each structure, rather let's exploit an objects iterable nature.
    template <typename T, typename = void>
    struct is_iterable : std::false_type{};

    template<typename T>
    struct is_iterable<T, std::void_t<decltype(std::begin(std::declval<T>())), decltype(std::end(std::declval<T>()))>>
    : std::true_type{};

    template<typename T>
    constexpr bool is_iterable_v = is_iterable<T>::value;

    template<typename T, typename = void>
    struct has_size : std::false_type{};

    template <typename T>
    struct has_size<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type{};

    template<typename T>
    constexpr bool has_size_v = has_size<T>::value;

    template<typename K, typename V>
    void StoreVectorPair(std::vector<std::pair<K,V>>, const char*); //distinguish between vector pair vs lone vector for loadunorderdmap internal
    
    template<typename K, typename V>
    void StoreUnorderedMap(std::unordered_map<K, V>, StorageInterface&);

    template<typename Structure>
    void StoreIterable(Structure&, StorageInterface&);
}


auto GetKeyArray(CacheMeta cacheMetaData)
    -> std::optional<KeyIndexEntry*>;


// template<typename Structure>
// void StoreToCache(Structure loadingStruct, StorageInterface& storage){
//     if constexpr(CacheRW::Internal::is_map<Structure>::value)
//         CacheRW::Internal::StoreUnorderedMap(loadingStruct, storage);
//     else if constexpr(CacheRW::Internal::is_vector<Structure>::value)
//         CacheRW::Internal::StoreVectorPair(loadingStruct, storage.getLabel().c_str(), storage);
//     else
//         std::cout << "Unsupported loading structure\n";

// }

template<typename Structure>
void StoreToCache(Structure Object, StorageInterface& storage){
    if constexpr(CacheRW::Internal::is_iterable_v<Structure>)
        CacheRW::Internal::StoreIterable(Object, storage);
    else
        std::cout << "Unsupported input structure; currently only accept iterable paired structs.\n";
}


std::optional<CacheMeta> ReadCacheMetaData(StorageInterface&);

template<typename returnType> //ensure free values
auto ReadKeyValues(KeyIndexEntry keyIndex, StorageInterface& storage) //use constexpr to create option CacheDB input or const char* input
    -> std::optional<returnType*> {
    
    if(keyIndex.count <= 0){
        std::cout << "Invalid key index\n";
        return {};
    }
    returnType* values = new returnType[keyIndex.count];
    if(!storage.isOpen()){
        std::cout << "Could not open " << storage.getLabel() << std::endl;
        return {};
    }

    storage.seek(keyIndex.offset);
    if(storage.eof()){
        std::cout << "Invalid key offset\n";
        delete[] values;
        return {};
    }
    /*ONLY USE DEREF ON NON-POINTERS BECUASE IT NEEDS THE ADDRESS TO WRITE
    IF IT IS A POINTER, YOU'RE JUST SENDING IT A POINTER TO A POINTER WHICH WILL BE OVERIDDEN WITH BOGUS*/
    storage.read(values, sizeof(returnType) * keyIndex.count);
    

    return values;
} 

std::ostream& operator<<(std::ostream& os, const KeyIndexEntry& entry);
}
