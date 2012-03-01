#include "lightweigth_test.h"
struct somestruct
{
    int a,b,c;
    string name;
}test;
LIGHT_WRAP_DECL(somestruct)
struct layered
{
    somestruct a;
    int b;
}test2;
LIGHT_WRAP_DECL(layered)
somestruct* getStruct()
{
    return &test;
}
layered* getLayered()
{
    return &test2;
}
template<>
int pushtolua(std::vector<int> vec, lua::state&s)
{
    s.newtable();
    for(size_t k=0;k<vec.size();k++)
    {
        s.push(k+1);
        s.push(vec[k]);
        s.settable();
    }
    return 1;
}
std::vector<int> Fibo(unsigned num)
{
    std::vector<int> ret;
    ret.push_back(1);
    ret.push_back(1);
    for(size_t i=2;i<num;i++)
        ret.push_back(ret[i-2]+ret[i-1]);
    return ret;
}
void do_light_test()
{
    test.a=10;
    test.b=20;
    test.c=-10;
    test.name="hello world";
    lua::state s=lua::glua::Get();
    LIGHT_WRAP_IMPL(somestruct,"somestruct",s);
    LIGHT_WRAP_MEMBER(somestruct,a);
    LIGHT_WRAP_MEMBER(somestruct,b);
    LIGHT_WRAP_MEMBER(somestruct,c);
    LIGHT_WRAP_MEMBER(somestruct,name);

    LIGHT_WRAP_IMPL(layered,"layered",s);
    LIGHT_WRAP_MEMBER(layered,a);
    LIGHT_WRAP_MEMBER(layered,b);

    lua_function<somestruct*>(&getStruct,"getStruct",s);
    lua_function<std::vector<int>,unsigned>(&Fibo,"Fibo",s);
    s.getglobal("Error");
    s.loadfile("light_test.lua");
    try{
        s.pcall(0,0,-2);
    }
    catch(std::exception &e)
    {
        std::cout<<"IN C:"<<e.what()<<"\n";
    }
}
