#ifndef LIGHTWEIGHT_H_INCLUDED
#define LIGHTWEIGHT_H_INCLUDED

#include "luastuff.h"
#include "unpack.h"
#include <boost/any.hpp>
#include <stdexcept>
#include <sstream>
#include <type_traits>
#include <functional>
#include <boost/mem_fn.hpp>
#include <map>
#include "luabindington/util.h"
#include "luabindington/function.h"

namespace lua{
template <class T>
struct ptr_wrap
{
    typedef T ptrType;
    typedef std::map<string,std::function<int(T*,lua::state&)> > manip_t;
    static manip_t getters;
    static manip_t setters;
    struct lua_udata
    {
        T* ptr;
    };
    static T* GetPointer(lua::state &s,int index)
    {
        lua_udata *udata;
        s.touserdata(udata,index);
        return udata->ptr;
    }
    static int lua_Index(lua_State *L)// 1: userdata,2:key
    {
        //check ancestors table first, don't check casted table (nothing there)
        //std::cout<<"INDEX\n";
        lua::state s(L);
        T* ptr=GetPointer(s,1);
        if(s.is<string>(2))
        {
            string key=s.as<string>(2);
            if(getters.find(key)!=getters.end())
            {
                return getters[key](ptr,s);
            }
        }
        return 0;
    }
    static int lua_NewIndex(lua_State *L)
    {
        //if is casted create new index in ancestors table not here.
       // std::cout<<"NEWINDEX\n";
        //1-userdata,2-key,3-value
        lua::state s(L);
        T* ptr=GetPointer(s,1);
        if(s.is<string>(2))
        {
            string key=s.as<string>(2);
            if(setters.find(key)!=setters.end())
            {
                return setters[key](ptr,s);
            }
        }
        return 0;
    }
    static void Register(lua::state &s,string name)
    {
        s.newtable(); //this will be metatable for this ptrwrapper
        int id_metatable=s.gettop();



        s.push(name);
        s.setfield("typename");

        lua_pushcclosure(s,&NewObject,0);
        s.setfield("new");

        lua_pushcfunction(s,&lua_Index);
        s.setfield("__index");

        lua_pushcfunction(s,&lua_NewIndex);
        s.setfield("__newindex");

        s.pushvalue(id_metatable);

        //s.newtable();
        //s.push("kv");
        //s.setfield("__mode");
        //lua_setmetatable(s,-2);
        //lua::StackDump(s);
        s.getglobal("types");
        if(s.is<lua::nil>())
        {
            s.newtable();
            s.setglobal("types");
        }
        s.setfield(name);
        s.pop();
    }

};

template <typename T>
typename lua_object<T>::manip_t lua_object<T>::getters;
template <typename T>
typename lua_object<T>::manip_t lua_object<T>::setters;
#define LIGHT_WRAP(type,state){ ptr_wrap<type> wrp; wrp.register(state,name,#type);
#define
#define LIGHT_WRAP_END  }
}
#endif // LIGHTWEIGHT_H_INCLUDED
