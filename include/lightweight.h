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
    static std::string _name;
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
    static int PushUData(lua::state &s,lua_udata *p) //table(metable info), udata
    {
        int udata_pos=s.gettop();
        s.pushvalue(udata_pos-1);
        lua_setmetatable(s,udata_pos);
        //s.pop();
        return 1;
    }
    static int NewObject(lua_State *L,T* ptr=nullptr)
    {
        lua::state s(L);
        s.getglobal("types");
        if(s.is<lua::nil>())
        {
            throw std::runtime_error("types table does not exist!");
        }
        s.getfield(_name);
        if(s.is<lua::nil>())
        {
            throw std::runtime_error("object metatable does not exist!");
        }

        lua_udata *p=(lua_udata*)s.newuserdata(sizeof(lua_udata));
        if(ptr==nullptr)
            p->ptr=new T;
        else
            p->ptr=ptr;
        return PushUData(s,p);
    }
    static int lua_Index(lua_State *L)// 1: userdata,2:key
    {
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
        _name=name;
        s.newtable(); //this will be metatable for this ptrwrapper
        int id_metatable=s.gettop();

        s.push(name);
        s.setfield("typename");

        //lua_pushcclosure(s,&NewObject,0);
        //s.setfield("new");

        lua_pushcfunction(s,&lua_Index);
        s.setfield("__index");

        lua_pushcfunction(s,&lua_NewIndex);
        s.setfield("__newindex");


        //s.newtable();
        //s.push("kv");
        //s.setfield("__mode");
        //lua_setmetatable(s,-2);
        //lua::StackDump(s);
        s.getglobal("types");
        if(s.is<lua::nil>())
        {
            s.newtable();
            s.pushvalue(-1);
            s.setglobal("types");
        }
        s.pushvalue(id_metatable);
        s.setfield(name);
        s.pop();
    }
	template <class T_parent>
    static void AddParent()
    {
        getters.insert(T_parent::getters.begin(),T_parent::getters.end());
		setters.insert(T_parent::setters.begin(),T_parent::setters.end());
    }
};

template <typename T>
typename ptr_wrap<T>::manip_t ptr_wrap<T>::getters;
template <typename T>
typename ptr_wrap<T>::manip_t ptr_wrap<T>::setters;
template <typename T>
 string ptr_wrap<T>::_name;
#define LIGHT_WRAP_DECL(type) typedef lua::ptr_wrap<type> _##type##wrap;\
template<>int pushtolua<type*>(type* ptr,lua::state &s){ return lua::ptr_wrap<type>::NewObject(s,ptr);}\
template<>int pushtolua<type&>(type& ptr,lua::state &s){ return lua::ptr_wrap<type>::NewObject(s,&ptr);}
#define LIGHT_WRAP_IMPL(type,name,state) _##type##wrap::Register(state,name);
#define LIGHT_WRAP_MEMBER_SET(type,member,name)  _##type##wrap::getters[name]=[](type *t,lua::state &s){ return convert_to_lua(t->member,s); }
#define LIGHT_WRAP_MEMBER_GET(type,member,name)  _##type##wrap::setters[name]=[](type *t,lua::state &s){ int dum=3;t->member=convert_from_lua<decltype(t->member)&>(s,dum);return 0;}
#define LIGHT_WRAP_MEMBER(type,member)  LIGHT_WRAP_MEMBER_SET(type,member,#member);LIGHT_WRAP_MEMBER_GET(type,member,#member);
}
#endif // LIGHTWEIGHT_H_INCLUDED
