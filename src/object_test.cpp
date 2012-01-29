#include "object_test.h"
#include <iostream>
class first_class
{
    int _a;
public:
    first_class()
    {
        std::cout<<"Constructed (default)\n";
        _a=0;
    }
    void void_funct(int a)
    {
        std::cout<<"In object, with void function:"<<a<<" local var:"<<_a<<"\n";
        _a=a+1;
    }
    int int_funct(int a)
    {
        std::cout<<"In object, with int function:"<<a<<" local var:"<<_a<<"\n";
        _a=a*2;
        return a+_a/2;
    }
    LUA_WRAP(first_class)
    LUA_FUNC<void,int>(&first_class::void_funct,"void_funct");
    LUA_FUNC<int,int>(&first_class::int_funct,"int_funct");
    LUA_GET(_a,"_a");
    LUA_SET(_a,"_a_set");
    LUA_END_WRAP()
};
void funct_that_takes_obj(first_class *ptr)
{
    std::cout<<"In void_f, that takes object ptr:"<<ptr<<"\n";
    ptr->void_funct(100);
}
first_class* funct_that_rets_obj()
{
    std::cout<<"In function that returns obj ptr.\n";
    return new first_class;
}
void do_object_test()
{
    lua::state state=lua::glua::Get();
    first_class::mywrap::Register(state,"first_class");
    lua_function<void,first_class*>(&funct_that_takes_obj,"funct_that_takes_obj",state);
    lua_function<first_class*>(&funct_that_rets_obj,"funct_that_rets_obj",state);
    state.getglobal("Error");
    state.loadfile("object_test.lua");
    try
    {
        state.pcall(0,0,-2);
    }
    catch(lua::exception &e)
    {
        std::cout<<"IN C:"<<e.what()<<"\n";
    }
}
