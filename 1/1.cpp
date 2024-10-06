#include <iostream>
#include <functional>
class A {
    private:
        int i = 0;

    public:
        void Func(){
            auto f = [this] {++i;};
            auto g = [&] {++i;};
            auto h = [=] {++i;};
            f(); g(); h();
        }
        int GetI() const {return i;}
};
int f(int x, int y, int z){
    return x +y*y-z;
}
int main(){
    A x;
    x.Func();
    auto bf = std::bind(f, std::placeholders::_2, std::placeholders::_2, 7);
    std::cout << x.GetI() << " ";
    std::cout<< bf(0,4);
}