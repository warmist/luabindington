#ifndef BINDER_HPP_INCLUDED
#define BINDER_HPP_INCLUDED
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
//TODO: typecasting, from lua as global func (e.g tuples/pairs etc), split this file,type checking (possibly casting)
//TODO: fix includes.
//typecasting:
//1.add parent to registry table (multiple parents ok?)
//2.add auto index casting e.g.
/*
    struct widget{int x,int y};
    struct window:public widget{int z};

    a. window.x must be possible (if it has setter/getter, or function)
    --b. not needed (not downcasting needed?)
*/



template <typename T,typename F>
T unioncast(F v)
{
    union zz
    {
        F input;
        T output;
    }g;
    g.input=v;
    return g.output;
}
template <typename T>
struct lua_object
{
    typedef T mytype;
    typedef std::vector<lua_function_base*> lua_f_vec;
    typedef std::map<string,std::function<int(T*,lua::state&)> > manip_t;

    static manip_t getters;
    static manip_t setters;
    static lua_f_vec functions;
    static string _name;
    struct lua_udata
    {
        T* ptr;
    };
    template<typename RetType,typename ...args>
    static void AddFunction(RetType (T::*fptr)(args...) ,string name)
    {
        functions.push_back(new lua_function_member<T,RetType,args...>(fptr,name));
    }
    static int PushUData(lua::state &s,lua_udata *p) //table(metable info), udata TODO __gc does not work for some reason.
    {
        //LogLua(s);
        std::cout<<"PUSHUDATA\n";
        //lua::StackDump(s);
        int udata_pos=s.gettop();
        s.pushvalue(udata_pos-1);
        //LogLua(s);
        lua_setmetatable(s,udata_pos);
        //LogLua(s);
        s.pushvalue(udata_pos-1);
        s.gettable(LUA_REGISTRYINDEX); //registry[T][ptr]=newtable
        //s.pushlightuserdata(p->ptr);
        s.push<unsigned int>(reinterpret_cast<unsigned int>(p->ptr));
        s.newtable(); //table which will have all lua data

        s.newtable(); //table to udata

        s.newtable();//metatable
        s.push("kv");
        s.setfield("__mode");
        lua_setmetatable(s,-2);

        s.push(1);
        s.pushvalue(udata_pos);
        s.settable(); //t[1]=udata

        s.setfield("__udata"); //put it into __udata[1]=udata
        s.settable(); //reg[class_name][ptr]=that table.

        s.pop();
        //lua::StackDump(s);
        return 1;
    }
    static int GetTable(lua_State *L,T* p) //BUG HERE
    {
        std::cout<<"GETTABLE w name="<<_name<<" and p="<<p<<"\n";
        lua::StackDump(L);
        lua::state s(L);
        s.getglobal(_name);
        s.gettable(LUA_REGISTRYINDEX);
        s.push<unsigned int>(reinterpret_cast<unsigned int>(p));
        lua::StackDump(L);
        //s.pushlightuserdata(p);
        s.gettable();
        if(s.is<lua::table>())
        {
            s.remove(-2);
            //lua::StackDump(L);
            return 1;
        }
        else //table does not exist, not created in lua..., create userdata, and push it to registry, then return it.
        {
            std::cout<<"Table does not exist, creating new...\n";
            s.pop(2);
            s.getglobal(_name);
            lua_udata *udata=(lua_udata*)s.newuserdata(sizeof(lua_udata));
            udata->ptr=p;
            lua::StackDump(L);
            PushUData(s,udata);//TODO return userdata, not table, could return table...
            s.pop(2);

            s.getglobal(_name);
            s.gettable(LUA_REGISTRYINDEX);
            s.push<unsigned int>(reinterpret_cast<unsigned int>(p));
            s.gettable();
            s.remove(-2);
            //lua::StackDump(L);
            return 1;
        }
    }
    static int NewObject(lua_State *L)
    {
        //std::cout<<"New object\n";
        lua::state s(L);
        //LogLua(s);
        lua_udata *p=(lua_udata*)s.newuserdata(sizeof(lua_udata));
        p->ptr=new T;
        //std::cout<<"Pointer:"<<p->ptr<<"\n";
        return PushUData(s,p);
    }
    static T* GetPointer(lua::state &s,int index)
    {
        lua_udata *udata;

        s.touserdata(udata,index);
        return udata->ptr;
    }
    static int lua_Index(lua_State *L)// 1: userdata,2:key
    {
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
        s.getmetatable(1);
        s.pushvalue(2);
        s.gettable();

        if(s.is<lua::nil>())
        {
            s.pop(2);
            GetTable(L,ptr);
            s.pushvalue(2);
            //lua::StackDump(L);
            s.gettable();
            return 1;
        }
        return 1;
    }
    static int lua_NewIndex(lua_State *L)
    {
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
        GetTable(L,ptr);
        s.insert(1); //1 table 2 userdata 3 key 4 value
        s.remove(2); //1 table 2 key 3 value
        s.settable();
        return 0;
    }
    static void Register(lua::state &s,string name)
    {
        _name=name;
        s.newtable(); //metatable.

        int myself=s.gettop();
        if(functions.size()==0)
        {
            T::addfunctions();
        }
        s.push(name);
        s.setfield("name");

        lua_pushcclosure(s,&NewObject,0);
        s.setfield("new");

        for(lua_f_vec::iterator i=functions.begin();i!=functions.end();i++)
        {
            (*i)->Register(s);
        }

        lua_pushcfunction(s,&lua_Index);
        s.setfield("__index");

        lua_pushcfunction(s,&lua_NewIndex);
        s.setfield("__newindex");

        s.pushvalue(myself);
        s.newtable();
        s.newtable();
        s.push("kv");
        s.setfield("__mode");
        lua_setmetatable(s,-2);
        lua::StackDump(s);
        s.settable(LUA_REGISTRYINDEX);
        lua::StackDump(s);
        s.setglobal(name);
    }
};
template <typename T>
typename lua_object<T>::manip_t lua_object<T>::getters;
template <typename T>
typename lua_object<T>::manip_t lua_object<T>::setters;
template <typename T>
typename lua_object<T>::lua_f_vec lua_object<T>::functions;
template <typename T>
 string lua_object<T>::_name;
#define LUA_WRAP(type)typedef lua_object<type> mywrap;\
    static type* pullfromlua(lua::state &s,int &start){ start++;return type::mywrap::GetPointer(s,start-1);}\
    int pushtolua(lua::state &s){type::mywrap::GetTable(s,this);s.getfield("__udata");s.push(1);s.gettable();s.remove(-2);return 1;}\
    static void addfunctions(){\

#define LUA_FUNC mywrap::AddFunction
#define LUA_GC(func) mywrap::AddFunction<void>(func,"__gc")
#define LUA_GET(var,name) mywrap::getters[name]=[](mywrap::mytype *t,lua::state &s){ return convert_to_lua(t->var,s); }
#define LUA_SET(var,name) mywrap::setters[name]=[](mywrap::mytype *t,lua::state &s){ int dum=3;t->var=convert_from_lua<decltype(t->var)>(s,dum);return 0;}
#define LUA_END_WRAP() }
struct lua_state_dummy
{

    lua::state s;
    static lua_state_dummy pullfromlua(lua::state &s,int &start)
    {
        lua_state_dummy p;
        p.s=s;
        return p;
    }
    int pushtolua(lua::state &s)
    {
        return 1; //TODO think about this, that is programmer must do GetTable(this)... Must be a way to automate this
    }
};
template <typename T,typename membertype,bool hasget,bool hasset>
struct lua_property
{
    typedef T classtype;
    typedef membertype T::* memptr;
    memptr myptr;
    lua_property(memptr tptr):myptr(tptr){};

};

#endif // BINDER_HPP_INCLUDED
