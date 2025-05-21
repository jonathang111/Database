#include <stdlib.h>
#include <memory>
#include <iostream>

template<typename T>
class MemWrapper {
private:
    std::unique_ptr<T> ptr;

public: // = nullptr is just a default for the parameter. so if no input, default to nullptr
    explicit MemWrapper(T* p = nullptr) : ptr(p) {}

    explicit MemWrapper(int x) : ptr(std::make_unique<T>(x)) {}

    MemWrapper(MemWrapper&& other) noexcept : ptr(std::move(other.ptr)) {}
    //private variables are accessible to instances of the same class.
    MemWrapper& operator=(MemWrapper&& other) noexcept { //only accepts r value, so other function explicitly calls std::move
        if (this != &other) {
            ptr = std::move(other.ptr);
        }
        return *this;
    }

    MemWrapper(const MemWrapper&) = delete;
    MemWrapper& operator=(const MemWrapper&) = delete;

    T& get() { return *ptr; }
    const T& get() const { return *ptr; }

    T& operator*() const { return *ptr; }

    void set(T t) { *ptr = t; }

    void set(std::unique_ptr<T> uptr) { ptr = std::move(uptr); }

    std::unique_ptr<T> release() { return std::move(ptr); }
    //cool thing about this one is that since it return std::move(ptr) to unique_ptr<T>,
    // then the new unique_ptr<T> return actually usees std::move(ptr) as the contrsuctor, thus settings
    // ptr as nullptr in the process.
    bool valid() const { return ptr != nullptr; }
};
