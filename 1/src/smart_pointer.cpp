#include <iostream>
#include <utility>

template<typename T>
class SmartPointer {
private:
    T* ptr;
    unsigned* ref_count;

public:
    // Конструктор
    explicit SmartPointer(T* p = nullptr) : ptr(p), ref_count(new unsigned(1)) {}

    // Конструктор копирования
    SmartPointer(const SmartPointer<T>& sp) : ptr(sp.ptr), ref_count(sp.ref_count) {
        ++(*ref_count);
    }

    // Оператор присваивания
    SmartPointer<T>& operator=(const SmartPointer<T>& sp) {
        if (this != &sp) {
            if (--(*ref_count) == 0) {
                delete ptr;
                delete ref_count;
            }
            ptr = sp.ptr;
            ref_count = sp.ref_count;
            ++(*ref_count);
        }
        return *this;
    }

    // Деструктор
    ~SmartPointer() {
        if (--(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }
    }

    // Разыменование
    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
    const T* operator->() const { return ptr; }
    // Сброс указателя
    void reset(T* p = nullptr) {
        if (--(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }
        ptr = p;
        ref_count = new unsigned(1);
    }

    // Обмен указателей
    void swap(SmartPointer<T>& sp) {
        std::swap(ptr, sp.ptr);
        std::swap(ref_count, sp.ref_count);
    }

    // Получение сырого указателя
    T* get() const { return ptr; }

    // Операции сравнения
    bool operator==(const SmartPointer<T>& sp) const { return ptr == sp.ptr; }
    bool operator!=(const SmartPointer<T>& sp) const { return ptr != sp.ptr; }
};