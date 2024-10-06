#include <iostream>
#include <utility>

template<typename T>
class SmartPointer {
private:
    T* ptr; // указатель на управляемый объект типа T. Именно этот объект будет находиться в динамической памяти и управляться умным указателем.
    unsigned* ref_count; //указатель на переменную, которая хранит количество ссылок на объект. Это используется для подсчёта, сколько копий данного указателя ссылаются на один и тот же объект

public:
    // Конструктор
    explicit SmartPointer(T* p = nullptr) : ptr(p), ref_count(new unsigned(1)) {}

    //При копировании указателя копируются адрес управляемого объекта (ptr) и указатель на счётчик ссылок (ref_count).
    //После этого увеличивается значение счётчика ссылок (++(*ref_count)),  появился ещё один указатель на этот же объект.
    SmartPointer(const SmartPointer<T>& sp) : ptr(sp.ptr), ref_count(sp.ref_count) {
        ++(*ref_count);
    }

    // Оператор присваивания
    SmartPointer<T>& operator=(const SmartPointer<T>& sp) {
        //Проверяется, что текущий объект не является тем же самым, что и правый операнд (проверка this != &sp).
        //Если это не один и тот же объект, уменьшается счётчик ссылок текущего объекта. Если после уменьшения счётчик становится равным 0, объект и память под счётчик освобождаются (delete ptr и delete ref_count).
        //После этого происходит копирование указателя и счётчика ссылок от правого операнда, и счётчик ссылок увеличивается.
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
    //Когда объект умного указателя уничтожается, уменьшается счётчик ссылок. Если счётчик становится равным 0, объект и память под счётчик освобождаются.
    ~SmartPointer() {
        if (--(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }
    }

    // Разыменование
    //operator* — возвращает ссылку на объект, управляемый указателем. Позволяет работать с объектом так, как если бы мы имели обычный указатель.
    //operator-> — позволяет доступаться к членам класса через умный указатель так же, как через обычный указатель.
    T& operator*() { return *ptr; }
    T* operator->() { return ptr; }
    const T* operator->() const { return ptr; }
    // Сброс указателя
    //Этот метод позволяет сбросить текущий указатель и установить новый. Если на старый объект больше никто не ссылается, он удаляется.
    void reset(T* p = nullptr) {
        if (--(*ref_count) == 0) {
            delete ptr;
            delete ref_count;
        }
        ptr = p;
        ref_count = new unsigned(1);
    }

    // Обмен указателей
    //Этот метод меняет местами содержимое текущего указателя и другого объекта типа SmartPointer
    void swap(SmartPointer<T>& sp) {
        std::swap(ptr, sp.ptr);
        std::swap(ref_count, sp.ref_count);
    }

    // Получение сырого указателя
    //Возвращает указатель на управляемый объект.
    T* get() const { return ptr; }

    // Операции сравнения
    //Эти операторы сравнивают два указателя по тому, указывают ли они на один и тот же объект.
    bool operator==(const SmartPointer<T>& sp) const { return ptr == sp.ptr; }
    bool operator!=(const SmartPointer<T>& sp) const { return ptr != sp.ptr; }
    int getCount(){
        return *ref_count;
    }
};


// class Test {
// public:
//     Test() { std::cout << "Test объект создан" << std::endl; }
//     ~Test() { std::cout << "Test объект уничтожен" << std::endl; }

//     void show() { std::cout << "Test::show()" << std::endl; }
// };

// int main() {
//     {
//         // Тест на создание и копирование указателей
//         SmartPointer<Test> sp1(new Test());
//         sp1->show();  // Вызов метода show через умный указатель

//         SmartPointer<Test> sp2 = sp1;  // Копирование умного указателя
//         std::cout << "sp1 == sp2: " << (sp1 == sp2) << std::endl;  // Сравнение указателей
//         std::cout << sp2.getCount() << std::endl;
//         sp2.reset(new Test());  // Сброс указателя sp2
//         sp2->show();
//         std::cout << sp2.getCount() << std::endl;
//         std::cout << "sp1 == sp2: " << (sp1 == sp2) << std::endl;

//         // Тест на swap
//         std::cout << "Тест swap:" << std::endl;
//         SmartPointer<Test> sp3(new Test());
//         SmartPointer<Test> sp4(new Test());

//         std::cout << "Перед swap:" << std::endl;
//         std::cout << "sp3: "; sp3->show();
//         std::cout << "sp4: "; sp4->show();


//         // Обмен указателями sp3 и sp4
//         sp3.swap(sp4);

//         std::cout << "После swap:" << std::endl;
//         std::cout << "sp3: "; sp3->show();
//         std::cout << "sp4: "; sp4->show();
//     } // Выход из области видимости, вызов деструкторов
//     std::cout << "Конец main()" << std::endl;

//     return 0;
// }
