#include <iostream>
#include <string>
#include <binder.hpp>
using namespace std;
/*class someclassname:public otherclass
{

    LUA_OBJECT(someclass,"someclass");
    LUA_FUNCTION(dosth,int,int,string);
    LUA_FUNCTION(dosth1,int,OPTIONAL(int),OPTIONAL(otherclass));
    LUA_PARENT(otherclass);
    LUA_END_OBJECT();
public:
    void Register(void *state)
    {

    }
};*/
/*class myclass
{
public:
    int a,b,c;
    std::string word;
    double c,d;
    void dosth()
    {
        std::cout<<a<<"\n";
    }
    double dosth2(int f)
    {
        a=f;
        std::cout<<"a="<<f<<'\n';
        return a*a;
    }

    LUA_OBJECT(myclass,"myclass")
    LUA_FUNCTION(dosth,void)
    LUA_FUNCTION(dosth2,double,int)
    //LUA_PARENT(otherclass)
    LUA_END_OBJECT();
};*/
struct mypair
{
    int a;
    double b;
    static mypair pullfromlua(lua::state &s,int &start)
    {
        //end=start+2;
        mypair p;
        p.a=s.as<int>(start);
        start++;
        p.b=s.as<double>(start);
        start++;
        return p;
    }
    int pushtolua(lua::state &s)
    {
        s.push(a);
        s.push(b);
        return 2;
    }
};
mypair sum(int a,double x)
{
    std::cout<<"At sum with:"<<a<<" "<<x<<'\n';
    mypair r;
    r.a=a;
    r.b=x;
    return r;
}
double exp(string a)
{
    std::cout<<"At exp with:"<<a<<'\n';
    return 1.5;
}
double sinsin(mypair x)
{
    std::cout<<"SINSIN with:"<<x.a<<" "<<x.b<<'\n';
    return x.a+x.b;
}
void s_void(mypair x)
{
    std::cout<<"svoid with:"<<x.a<<" "<<x.b<<'\n';
}
class otherthing
{
    int value;
    int varvar;
public:
    int DoSth(int a)
    {
        std::cout<<"In class with:"<<a<<'\n';
        value++;
        return 133;
    }
    void VoidTest(int b)
    {
        std::cout<<"In the void:"<<b<<'\n';
    }
    void DoLuaTest(lua_state_dummy s)
    {
        s.s.loadstring("print('out of lua in lua')");
        s.s.pcall();
    }
//// Wrapper
    LUA_WRAP(otherthing)
        //mywrap::AddFunction<int,int>(&otherthing::DoSth,"DoSth");
        LUA_FUNC<int,int>(&otherthing::DoSth,"DoSth");
        LUA_FUNC<void,int>(&otherthing::VoidTest,"VoidTest");
        LUA_FUNC<void,lua_state_dummy>(&otherthing::DoLuaTest,"DoLuaTest");
        LUA_GET(varvar,"count");
        LUA_SET(varvar,"miss");
    LUA_END_WRAP();
};
/* TODO special case for pointers
void insane_test(otherthing* p)
{
    std::cout<<"insane start.\n";
    p->DoSth(100);
    std::cout<<"insane end.\n";
}
*/
int main()
{
    lua_property<otherthing,int,true,true> prop();
    lua::state state=lua::glua::Get();
    lua_function<mypair,int,double> s(sum,"sum");
    s.Register(state);

    lua_function<double,string> e(exp,"exp");
    e.Register(state);

    lua_function<double,mypair> g(sinsin,"sinsin");
    g.Register(state);

    lua_function<void,mypair> ge(s_void,"testvoid");
    ge.Register(state);

    otherthing::mywrap::Register(state,"thingy");

    //lua_function<void,otherthing*> ge1(insane_test,"testinsane");
    //ge1.Register(state);

    //cout<<"Int:"<<class_has_pullfromlua_function<int>::value<<" pair:"<<class_has_pullfromlua_function<mypair>::value<<"\n";
    state.loadfile("test.lua");
    try{
        state.pcall();
    }
    catch(lua::exception &ex)
    {
        std::cout<<ex.what()<<std::endl;
    }
    state.getglobal("printValue");
    otherthing tt;
    tt.pushtolua(state);
    state.pcall(1);
    //state.loadstring("testinsane(kk)");
    //state.pcall();
    cout << "Hello world!" << endl;
    return 0;
}
