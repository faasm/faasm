#include <stdio.h>
#include <string>

class MyObjB
{

  public:
    MyObjB()
      : name("object B")
    {}

    void sayHiB() { printf("Hello from %s!\n", name.c_str()); }

  private:
    std::string name;
};

class MyObjA
{
  public:
    void sayHi()
    {
        printf("Hello from object A!\n");

        objB.sayHiB();
    }

  private:
    MyObjB objB;
};

int main(int argc, char* argv[])
{

    auto objA = new MyObjA();

    objA->sayHi();

    delete objA;

    return 0;
}
