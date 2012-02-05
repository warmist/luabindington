#include "object_test.h"
#include <iostream>
class first_class
{
    int _a;
public:
    virtual ~first_class(){};
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
    void GC()
    {
        std::cout<<"IN first_class gc:"<<this<<"\n";
        delete this;
    }
    LUA_WRAP(first_class)
    LUA_FUNC<void,first_class,int>(s,&first_class::void_funct,"void_funct");
    LUA_FUNC<int,first_class,int>(s,&first_class::int_funct,"int_funct");
    LUA_GET(_a,"_a");
    LUA_SET(_a,"_a_set");
    //LUA_FUNC<void>(&first_class::GC,"__gc");
    LUA_GC(&first_class::GC);
    //LUA_STATIC<void,first_class*>(&first_class::GC,"__gc");
    //lua_function<void,first_class*>(&first_class::GC,"__gc");
    LUA_END_WRAP()
};
class second_class:public first_class
{
    int b;
    string somestring;
public:
    virtual ~second_class(){std::cout<<"Second_class destroyed.\n";}
    void PrintData(string text)
    {
        somestring=text;
        std::cout<<"b="<<b<<" somestring="<<somestring<<"\n";
        b++;
    }
    /*first_class* cast()
    {
        return this;
    }*/
    LUA_WRAP(second_class)
    LUA_ADD_BASE2(first_class,"first_class");
    //LUA_FUNC<int,first_class,int>(s,&first_class::int_funct,"int_funct");
    LUA_FUNC<void,second_class,string>(s,&second_class::PrintData,"PrintData");
    //LUA_FUNC<first_class*>(s,&second_class::cast,"cast");

    LUA_END_WRAP()
    LUA_CAST(first_class)
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
    second_class::mywrap::Register(state,"second_class");
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
