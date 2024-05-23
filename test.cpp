
/*#if !MBED_TEST_MODE
#ifdef NO_DEBUG
#include "resource.h"
#include "node_object.h"
#include <vector>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    std::vector<Resource> list;
    int retCode;
    list.push_back(Resource(1, ResourceOp::RES_RDWR, std::string("Coucou"), Units::AMPER));
    list.push_back(3.14);
    list.push_back(std::string("hello world"));
    list.push_back(Resource());


    std::cout << "objet 0 rights is " << int(list[0].GetOp()) << std::endl;

    if(!list[0].BindOnWrite<int>([](int a){
        std::cout << "Listener 1 value write is : " << a << std::endl;
    })){
        std::cout << "bind fail error code is " << list[0].GetErrorCode() << std::endl;
    }

    if(!list[0].BindOnWrite<int>([](int a){
        std::cout << "Listener 2 value write is : " << a << std::endl;
    })){
        std::cout << "bind fail error code is " << list[0].GetErrorCode() << std::endl;
    }

    if(list[0].Write<int>(2)){
        std::cout << "write list[0] fail err code is " << int(list[0].GetErrorCode()) << std::endl;
    }

    for (auto &a : list)
    {
        std::cout << "type = " << a.Type().name()
                  << " value = ";
        int b;
        double c;
        std::string d;
        float e;

        if (a.Type() == typeid(int))
        {
            std::cout << *a.Read<int>();
        }
        else if (a.Type() == typeid(double))
        {
            std::cout << (*a.Read<double>() == 3.14 ? "3.14" : "nope");
        }
        else if (a.Type() == typeid(std::string))
        {
            std::cout << *a.Read<std::string>();
        }
        else if (a.Type() == typeid(float))
        {
            std::cout << *a.Read<float>();
        }
        std::cout << std::endl;
    }
    return 0;
}
#endif
#endif*/