#include "struct_test.h"

struct simple_struct
{
    int a,b;
    string c;

    static simple_struct pullfromlua(lua::state &s,int &start)
    {
        simple_struct t;
        if(!s.is<lua::table>(start))
            throw lua::runtime_error("Incorrect argument, needs table");
        s.getfield("a");
        t.a=s.as<int>(-1);
        s.pop();
        s.getfield("b");
        t.b=s.as<int>(-1);
        s.pop();
        s.getfield("c");
        t.c=s.as<string>(-1);
        s.pop();
        start++;
        return t;
    }
    int pushtolua(lua::state &s)
    {
        s.newtable();
        s.push(a);
        s.setfield("a");
        s.push(b);
        s.setfield("b");
        s.push(c);
        s.setfield("c");
        return 1;
    }
};
struct complex_struct
{
    int a;
    simple_struct b;
    static complex_struct pullfromlua(lua::state &s,int &start)
    {
        complex_struct t;
        if(!s.is<lua::table>(start))
            throw lua::runtime_error("Incorrect argument, needs table");
        s.getfield("a");
        t.a=s.as<int>(-1);
        s.pop();
        s.getfield("b");
        int t_pos=s.gettop();
        t.b=convert_from_lua<simple_struct>(s,t_pos);
        s.pop();
        start++;
        return t;
    }
    /*int pushtolua(lua::state &s)
    {
        s.newtable();
        s.push(a);
        s.setfield("a");
        b.pushtolua(s);
        s.setfield("b");
        return 1;
    }*/
};
template<>
int pushtolua<complex_struct>(complex_struct inp,lua::state &s)
{
    s.newtable();
    s.push(inp.a);
    s.setfield("a");
    convert_to_lua(inp.b,s);
    s.setfield("b");
    return 1;
}
void arg_is_struct(simple_struct a)
{
    std::cout<<"in arg is struct a="<<a.a<<" b="<<a.b<<" c="<<a.c<<"\n";
}
simple_struct ret_is_struct()
{
    std::cout<<"Pushing struct to lua.\n";
    simple_struct aa;
    aa.a=-122;
    aa.b=334;
    aa.c="yay two";
    return aa;
}
void arg_is_complex(complex_struct a)
{
    std::cout<<"arg is complex_struct a="<<a.a<<"\n";
    std::cout<<"\ta="<<a.b.a<<"\t"<<a.b.b<<"\t"<<a.b.c<<" \t";
}
complex_struct make_complex()
{
    complex_struct a;
    a.a=0;
    return a;
}
void do_struct_test()
{
    lua::state state=lua::glua::Get();
    lua_function<void,simple_struct>(&arg_is_struct,"arg_is_struct",state);
    lua_function<simple_struct>(&ret_is_struct,"ret_is_struct",state);
    lua_function<void,complex_struct>(&arg_is_complex,"arg_is_complex",state);
    lua_function<complex_struct>(&make_complex,"make_complex",state);
    state.getglobal("Error");
    state.loadfile("struct_test.lua");
    try{
        state.pcall(0,0,-2);
    }
    catch(std::exception &e)
    {
        std::cout<<"IN C:"<<e.what()<<"\n";
    }
}
