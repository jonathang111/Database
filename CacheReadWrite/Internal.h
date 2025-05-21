#pragma once
#include "CacheRW.h"

namespace CacheRW::Internal{

    template<typename T>
    struct is_map : std::false_type {};

    template<typename K, typename V>
    struct is_map<std::unordered_map<K,V>> : std::true_type{};
    
    template<typename T>
    struct is_vector : std::false_type{};

    template<typename V>
    struct is_vector<std::vector<V>> : std::true_type{};

    template<typename T>
    struct is_raw_pointer : std::false_type{};

    template<typename V>
    struct is_raw_pointer<V*> : std::true_type{};

    size_t ComputeOffset(size_t EntryEnd);

    template<typename V>
    void StoreEntries(V entry, StorageInterface& storage){
        using ValueType = typename V::value_type;

        if constexpr(is_vector<V>::value)
            storage.write(entry.data(), sizeof(ValueType) * entry.size());
        else if constexpr(is_raw_pointer<V>::value)
            storage.write(entry, sizeof(ValueType));   
    }
    
    void StoreKeyIndex(int keyAmount, KeyIndexEntry*& keyArray ,StorageInterface& storage);

    template <typename K>
    void UpdateKeyEntries(KeyIndexEntry& keyIndex, const K& key, size_t offset, int count){
        if constexpr (std::is_same<K, char>::value){
            keyIndex.key = new char[1];
            keyIndex.keySize = 1;
            keyIndex.key[0] = key;
            keyIndex.key[1] = '\0';
        }
        else if constexpr (std::is_same<K, std::string>::value){
            keyIndex.keySize = strlen(key);
            keyIndex.key = new char[keyIndex.keySize];
            strncpy(keyIndex.key, key, strlen(key));
            keyIndex.key[MAX_KEYSIZE-1] = '\0';
        }
        else{
            std::cout << "Cannot handle key types\n";
            return;
        }
        keyIndex.offset = offset;
        keyIndex.count = count;
    }

    template<typename K, typename V>
    void storeVectorPair(std::vector<K, V> vector, const char* inputFile){
        std::ofstream cacheFile(inputFile, std::ios::binary);
        int keycount = vector.size();
        CacheRW::CacheHeader header = {VECTOR_PAIR_TAG, keycount, 0, VERSION, DATE};
        size_t currentoffset = sizeof(CacheHeader);
    }

    template<typename K, typename V>
    void StoreUnorderedMap(std::unordered_map<K, V> map, StorageInterface& storage){ //must be map with vector value
        if(!storage.isOpen()){
            std::cerr << "File " << storage.getLabel() << " could not be opened\n";
            return;
        }
        
        int keycount = map.size();
        if(keycount == 0)
            std::cerr << "Warning: storing empty map\n";

        CacheRW::CacheHeader header = {MAP_TAG, keycount, 0, VERSION, DATE};
        storage.seek(sizeof(CacheHeader));
        KeyIndexEntry* keyentries = new KeyIndexEntry[keycount];
        using ValueType = typename V::value_type; //may not need

        int p = 0;
        for(auto it = map.begin(); it != map.end(); it++, p++){
            UpdateKeyEntries(keyentries[p], it->first, storage.tell(), it->second.size());
            header.entryAmount += it->second.size();
            StoreEntries(it->second, storage);
        }
        
        header.footerStart = ComputeOffset(storage.tell());
        storage.seek(0);
        storage.write(&header, sizeof(CacheHeader));
        storage.seek(header.footerStart);
        StoreKeyIndex(keycount, keyentries, storage);
        //storage.write(keyentries, keycount * sizeof(KeyIndexEntry));
            
        delete[] keyentries;
    }

}