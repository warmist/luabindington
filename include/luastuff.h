#ifndef LUASTUFF_H
#define LUASTUFF_H


extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

//#include "luna.h"

#include "luaxx/luaxx.hpp"
#include <list>
#include <string>
using std::string;
typedef std::vector <string> vString;
namespace lua
{
    class glua
    {
    public:
        static state &Get();
    private:
        glua();
        static glua *ptr;
        state mystate;
    };
    void dofile(lua::state &L,string path);
    void cd(lua::state &L,string path);
    void RegBasics(lua::state &L);
    void RegisterWrapper(lua::state &L);

    void RegLogging(lua::state &L);
    vString Hint(string line);

    template <class T>
    void MakeTCODWrap(state s,T *ptr,string name)
    {
        s.newtable();
        int ntbl=s.gettop();
        s.pushlightuserdata<T*>(ptr);
        s.setfield(name);
        s.getglobal("tcod");
        s.getfield(name);

        s.pushvalue(-1);
        //s.push("__index");
        //s.settable(ntbl);
        s.setfield("__index",ntbl);
        lua_setmetatable(s,ntbl);
        s.pop();
    }

}
void RegFunctions(lua::state &L,luaL_reg const *arr);
void RegFunctionsLocal(lua::state &L,luaL_reg const *arr);




#endif // LUASTUFF_H
