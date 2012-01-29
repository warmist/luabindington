#include "function_test.h"
#include <iostream>
#include <string>
using std::string;
void f_void(int a)
{
    std::cout<<"in f_void with:"<<a<<"\n";
}
int f_first(int a,double b,int c)
{
    std::cout<<"in f_first with:"<<a<<" "<<b<<" "<<c<<"\n";
    return a+b+c;
}
string f_second(int a,string b,int c)
{
    std::cout<<"in f_second with:"<<a<<" "<<b<<" "<<c<<"\n";
    return "Hello world.";
}
void f_fails(int a,int b)
{
    std::cout<<"should not be in f_fail!\n";
}
void do_func_test()
{
    lua::state state=lua::glua::Get();
    lua_function<void,int>(&f_void,"f_void",state);
    lua_function<int,int,double,int>(&f_first,"f_first",state);
    lua_function<string,int,string,int>(&f_second,"f_second",state);
    lua_function<void,int,int>(&f_fails,"f_fails",state);
    state.getglobal("Error");
    state.loadfile("function_test.lua");
    try{
        state.pcall(0,0,-2);
    }
    catch(lua::exception &e)
    {
        std::cout<<"IN C:"<<e.what()<<"\n";
    }
}
