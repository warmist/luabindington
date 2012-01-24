#include "luastuff.h"
#include <vector>
#include <sstream>
#include <iostream>


lua::glua* lua::glua::ptr=0;
lua::glua::glua()
{
    RegBasics(mystate);
    RegLogging(mystate);
    RegisterWrapper(mystate);
    mystate.push("data/");
    mystate.setglobal("_CD");
    //lua::dofile(mystate,"/data/libtcod_defines.lua");
    //lua::dofile(mystate,"/data/init.lua");
}
lua::state &lua::glua::Get()
{
    if(!glua::ptr)
        glua::ptr=new glua();
    return glua::ptr->mystate;
}
std::list<string> &Ui_Split(string path,std::list<string> &elems)
{
    std::list<string> ret;
    std::stringstream ss(path);
    std::string item;
    while(std::getline(ss, item, '/')) {
        elems.push_back(item);
    }
    return elems;
}
void lua::cd(lua::state &L,string path)
{
    //LOG<<"CD with:"<<path<<"\n";
    if(path=="")
        return;
    L.getglobal("_CD");
    string cd=L.vpop<std::string>();
    if(path[0]=='/')
    {
        //absolute
        cd="";
        path=path.substr(1);
    }
    std::list<string> fullpath,epath;
    Ui_Split(cd,fullpath);
    Ui_Split(path,fullpath);
    for(auto it=fullpath.begin();it!=fullpath.end();it++)
    {
        //LOG<<"\t"<<*it<<"\n";
        //auto pit=it;
        if(*it=="..")
        {
            epath.pop_back();

        }
        else
        {
            epath.push_back(*it);
        }
    }
    //LOG<<"After reiter\n";
    string newpath;
    for(auto it=epath.begin();it!=epath.end();it++)
    {
            newpath+=*it;
            newpath+="/";
    }
    //LOG<<"newpath:"<<newpath<<"\n";
    L.push(newpath);
    L.setglobal("_CD");
}
void lua::dofile(lua::state &L,string path)
{
    L.getglobal("_CD");
    string cd=L.vpop<std::string>();

    size_t p=path.rfind("/");
    if(p!=path.npos)
    {
    L.getglobal("cd");
    L.push(path.substr(0,p));
    L.pcall(1);
    }
    L.getglobal("_CD");
    string ncd=L.vpop<std::string>();


    //LOG<<"Cur dir:"<<ncd<<"\n";
    //LOG<<"Dofile called with:"<<path.substr(p+1)<<"\n";

    //L.loadfile(path);
    //LOG<<"Full path:"<<ncd+path.substr(p+1)<<"\n";
    L.getglobal("OnError");
    int errfunc=L.gettop();
    L.loadfile(ncd+path.substr(p+1));
    L.pcall(0,0,errfunc);

    L.push(cd);//restore cur dir
    L.setglobal("_CD");
}
int lua_StackDump(lua_State *L)
{
    lua::StackDump(L);
    return 0;
}
int lua_Ver_Lua(lua_State *L)
{
    lua::state st(L);
    st.push(LUA_RELEASE);
    return 1;
}
int lua_Ver_Game(lua_State *L)
{
    lua::state st(L);
    //st.push(str(boost::format(" V%1% rev%2%")%0%REV));
    return 1;
}
int lua_dofile(lua_State *L)
{
    lua::state st(L);
    string path=st.vpop<std::string>();
    lua::dofile(st,path);
    return 0;
}
int lua_cd(lua_State *L)
{
    lua::state st(L);
    string path=st.vpop<std::string>();
    lua::cd(st,path);
    return 0;
}
static const struct luaL_reg lua_basic_lib [] =
{
    {"getluaver", lua_Ver_Lua},
    {"getgamever", lua_Ver_Game},
    {"StackDump", lua_StackDump},
    {"dofile", lua_dofile},
    {"cd", lua_cd},
    {NULL, NULL}  /* sentinel */
};
void lua::RegBasics(lua::state &L)
{
    luaL_openlibs(L);
    RegFunctions(L,lua_basic_lib);
}
void lua::RegisterWrapper(lua::state &L)
{

}
void RegFunctions(lua::state &L,luaL_reg const*arr)
{
    luaL_reg const *cur=arr;
    while(cur->name!=NULL)
    {
        lua_pushcfunction(L, cur->func);
        lua_setglobal(L, cur->name);
        //Logger::Get().Log(str(boost::format("lua: function reg:%1%\n")%cur->name));
        cur++;
    }
}
void RegFunctionsLocal(lua::state &L,luaL_reg const*arr)
{
    luaL_reg const *cur=arr;
    while(cur->name!=NULL)
    {
        lua_pushcfunction(L, cur->func);
        //lua_setglobal(L, cur->name);
        L.setfield(cur->name);
        //Logger::Get().Log(str(boost::format("lua: function reg:%1%\n")%cur->name));
        cur++;
    }
}
void TestLua()
{

    //lua_State *L = lua_open();
    lua::state L;

    RegLogging(L);
    luaL_openlibs(L);
    luaL_dofile(L, "test.lua");
    //lua_close(L);
}






static int lua_Log (lua_State *L)
{
    //string d = lua_Lcheckstring(L, 1);
    lua::state t(L);
    string d=t.as<string>();
    //Logger::Get().Log(d);
    return 0;
}
static int lua_Warn (lua_State *L)
{
    //string d = lua_Lcheckstring(L, 1);
    lua::state t(L);
    string d=t.as<string>();
    //Logger::Get().Warn(d);
    return 0;
}
static int lua_Fail (lua_State *L)
{
    //string d = lua_Lcheckstring(L, 1);
    lua::state t(L);
    string d=t.as<string>();
    //Logger::Get().Fail(d);
    return 0;
}
static const struct luaL_reg lua_Logger_lib [] =
{
    {"log", lua_Log},
    {"warn", lua_Warn},
    {"fail", lua_Fail},
    {NULL, NULL}  /* sentinel */
};

void lua::RegLogging(lua::state &L)
{
    RegFunctions(L,lua_Logger_lib);
}
vString lua::Hint(string line)
{
    vString ret;
    lua::state gl=lua::glua::Get();
    gl.getglobal("Hint");
    if(!gl.is<lua::function>())
    {
        return ret;
    }
    gl.push(line);
    gl.call(1,1);
    if(gl.is<lua::table>())
    {
        gl.push();
        while(gl.next())
        {
            string p=gl.as<string>(-1);
            //LOG<<"Got string:"<<p<<"\n";
            ret.push_back(p);
            gl.pop();
        }
    }
    gl.pop();
    return ret;
}
