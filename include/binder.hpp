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

//TODO: typecasting, from lua as global func (e.g tuples/pairs etc), split this file
//BUGS: registry as weak values
//typecasting:
//1.add parent to registry table (multiple parents ok?)
//2.add auto index casting e.g.
/*
    struct widget{int x,int y};
    struct window:public widget{int z};

    a. window.x must be possible (if it has setter/getter, or function)
    --b. not needed (not downcasting needed?)
*/

template<typename T,bool is_class_type=std::is_class<T>::value >
struct class_has_pullfromlua_function:
    std::false_type
{};

template<typename T>
struct class_has_pullfromlua_function<T,true>
{
    template<T (*)(lua::state &,int &)> struct tester;

    template<typename U>
    static char has_matching_member(tester<&U::pullfromlua>*); // A
    template<typename U>
    static long has_matching_member(...); // B

    static const bool value=sizeof(has_matching_member<T>(0))==sizeof(char);
};

template <class T>
typename std::enable_if<class_has_pullfromlua_function<T>::value,T>::type
convert_from_lua_impl(lua::state &s,int &stacknum)
{
    T v=T::pullfromlua(s,stacknum);
    return v;
}

template <class T>
typename std::enable_if<!class_has_pullfromlua_function<T>::value,T>::type
convert_from_lua_impl(lua::state &s,int &stacknum)
{
    T v=s.as<T>(stacknum);
    stacknum++;
    return v;
}
template <class T>
T convert_from_lua(lua::state &s,int &stacknum)
{
    return convert_from_lua_impl<T>(s,stacknum);
}

template <typename t_type,int num>
void lua_to_tuple(t_type &mtuple,lua::state &s,int &stacknum)
{

}
template <typename t_type,int num,typename T,typename ...Tail>
void lua_to_tuple(t_type &mtuple,lua::state &s,int &stacknum)
{
    try
    {
        std::get<num>(mtuple)=convert_from_lua<T>(s,stacknum);//s.as<T>(stacknum);
    }
    catch(lua::bad_conversion &e)
    {
        std::stringstream err;
        err<<e.what();
        err<<" at:"<<num<<" argument.";
        throw lua::bad_conversion(err.str().c_str());
    }
    //stacknum++;
    lua_to_tuple<t_type,num+1,Tail...>(mtuple,s,stacknum);
}


template<typename T,bool is_class_type=std::is_class<T>::value>
struct class_has_pushtolua_function:
    std::false_type
{};

template<typename T>
struct class_has_pushtolua_function<T,true>
{
    template<int (T::*)(lua::state&)> struct tester;

    template<typename U>
    static char has_matching_member(tester<&U::pushtolua>*); // A
    template<typename U>
    static long has_matching_member(...); // B

    static const bool value=sizeof(has_matching_member<T>(0))==sizeof(char);
};





template <class T>
typename std::enable_if<class_has_pushtolua_function<T>::value,int>::type
convert_to_lua_impl(T val,lua::state &s)
{
    return val.pushtolua(s);
}

template <class T>
typename std::enable_if<!class_has_pushtolua_function<T>::value,int>::type
convert_to_lua_impl(T val,lua::state &s)
{
    s.push(val);
    return 1;
}
template <class T>
int convert_to_lua(T val,lua::state &s)
{
    return convert_to_lua_impl(val,s);
}
class lua_function_base
{
public:
    virtual void Register(lua_State *L)=0;
};
template <typename rettype,typename ...Args>
class lua_function:public lua_function_base
{
    typedef std::tuple<Args...> myargs;
    typedef rettype myret;
    typedef myret(*myfunc)(Args...) ;
    myfunc f;
    std::string name;
public:
    lua_function(myfunc f,std::string name):f(f),name(name)
    {

    }
    void Register(lua_State *L)
    {
        lua::state s(L);
        s.pushlightuserdata(f);
        lua_pushcclosure(L,&call,1);
        s.setglobal(name);
    }
    static int call(lua_State *L)
    {
        lua::state s(L);
        /*if(s.gettop()!=sizeof...(Args)) //not so simple anymore...
        {
            std::stringstream err;
            err<<"Incorrect argument count. Got:";
            err<<s.gettop();
            err<<" Must be:";
            err<<sizeof...(Args);
            throw lua::runtime_error(err.str().c_str());
        }*/

        myargs args;
        myfunc f2=(myfunc)lua_touserdata(L,lua_upvalueindex(1));
        int stacknum=1;
        lua_to_tuple<myargs,0,Args...>(args,s,stacknum);
        myret r=apply<myret,Args...>(f2,args);
        return convert_to_lua(r,s);
        //f(args...);
    }
};
template < typename ...Args>
class lua_function<void,Args...>:public lua_function_base
{
    typedef std::tuple<Args...> myargs;
    typedef void myret;
    typedef myret(*myfunc)(Args...) ;
    myfunc f;
    std::string name;
public:
    lua_function(myfunc f,std::string name):f(f),name(name)
    {

    }
    void Register(lua_State *L)
    {
        lua::state s(L);
        s.pushlightuserdata(f);
        lua_pushcclosure(L,&call,1);
        s.setglobal(name);
    }
    static int call(lua_State *L)
    {
        lua::state s(L);
        /*if(s.gettop()!=sizeof...(Args)) //not so simple anymore...
        {
            std::stringstream err;
            err<<"Incorrect argument count. Got:";
            err<<s.gettop();
            err<<" Must be:";
            err<<sizeof...(Args);
            throw lua::runtime_error(err.str().c_str());
        }*/

        myargs args;
        myfunc f2=(myfunc)lua_touserdata(L,lua_upvalueindex(1));
        int stacknum=1;
        lua_to_tuple<myargs,0,Args...>(args,s,stacknum);
        apply<myret,Args...>(f2,args);
        return 0;
        //f(args...);
    }
};

template <class T,typename rettype,typename ...Args>
class lua_function_member:public lua_function_base
{
    typedef std::tuple<Args...> myargs;
    typedef rettype myret;
    typedef myret(T::*myfunc)(Args...) ;

    //typedef boost::mem_fun_t<myret,T,Args...> myfunc;
    myfunc f;
    std::string name;

    struct lua_hold
    {
        myfunc fptr;
    };
public:
    lua_function_member(myfunc f,std::string name):f(f),name(name)
    {

    }
    void Register(lua_State *L)
    {
        lua::state s(L);
        lua_hold *p=(lua_hold*)s.newuserdata(sizeof(lua_hold));
        p->fptr=f;
        lua_pushcclosure(L,&call,1);
        s.setfield(name);
    }
    static int call(lua_State *L)
    {
        lua::state s(L);
        /*if(s.gettop()!=sizeof...(Args)) //not so simple anymore...
        {
            std::stringstream err;
            err<<"Incorrect argument count. Got:";
            err<<s.gettop();
            err<<" Must be:";
            err<<sizeof...(Args);
            throw lua::runtime_error(err.str().c_str());
        }*/

        myargs args;
        lua_hold *t_hold;
        s.touserdata(t_hold,lua_upvalueindex(1));
        int stacknum=2;
        T* obj_ptr;

        s.touserdata(obj_ptr,1);
        lua_to_tuple<myargs,0,Args...>(args,s,stacknum);
        myret r=applyTuple<T,myret,Args...>(obj_ptr,t_hold->fptr,args);
        return convert_to_lua(r,s);
        //f(args...);
    }
};
template <typename T,typename ...Args>
class lua_function_member<T,void,Args...>:public lua_function_base
{
    typedef std::tuple<Args...> myargs;
    typedef void myret;
    typedef myret(T::*myfunc)(Args...) ;

    //typedef boost::mem_fun_t<myret,T,Args...> myfunc;
    myfunc f;
    std::string name;

    struct lua_hold
    {
        myfunc fptr;
    };
public:
    lua_function_member(myfunc f,std::string name):f(f),name(name)
    {

    }
    void Register(lua_State *L)
    {
        lua::state s(L);
        lua_hold *p=(lua_hold*)s.newuserdata(sizeof(lua_hold));
        p->fptr=f;
        lua_pushcclosure(L,&call,1);
        s.setfield(name);
    }
    static int call(lua_State *L)
    {
        lua::state s(L);
        /*if(s.gettop()!=sizeof...(Args)) //not so simple anymore...
        {
            std::stringstream err;
            err<<"Incorrect argument count. Got:";
            err<<s.gettop();
            err<<" Must be:";
            err<<sizeof...(Args);
            throw lua::runtime_error(err.str().c_str());
        }*/

        myargs args;
        lua_hold *t_hold;
        s.touserdata(t_hold,lua_upvalueindex(1));
        int stacknum=2;
        T* obj_ptr;

        s.touserdata(obj_ptr,1);
        lua_to_tuple<myargs,0,Args...>(args,s,stacknum);
        applyTuple<T,myret,Args...>(obj_ptr,t_hold->fptr,args);
        return 0;
        //f(args...);
    }
};
void LogLua(lua::state &s)
{
    std::cout<<"Top:"<<s.gettop()<<"\n";
}
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
    static int PushUData(lua::state &s,lua_udata *p) //table(metable info), udata
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
        s.newtable();
        s.pushvalue(udata_pos);
        s.setfield("__udata");
        s.settable();

        s.pop();
        //lua::StackDump(s);
        return 1;
    }
    static int GetTable(lua_State *L,T* p)
    {
        std::cout<<"GETTABLE\n";
        lua::StackDump(L);
        lua::state s(L);
        s.getglobal(_name);
        s.gettable(LUA_REGISTRYINDEX);
        s.push<unsigned int>(reinterpret_cast<unsigned int>(p));
        //s.pushlightuserdata(p);
        s.gettable();
        if(s.is<lua::table>())
        {
            s.remove(-2);
            lua::StackDump(L);
            return 1;
        }
        else //table does not exist, not created in lua..., create userdata, and push it to registry, then return it.
        {
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
            lua::StackDump(L);
            return 1;
        }
    }
    static int NewObject(lua_State *L)
    {
        std::cout<<"New object\n";
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
        std::cout<<"INDEX\n";
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
            lua::StackDump(L);
            s.gettable();
            return 1;
        }
        return 1;
    }
    static int lua_NewIndex(lua_State *L)
    {
        std::cout<<"NEWINDEX\n";
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
        s.newtable();
        int myself=s.gettop();
        if(functions.size()==0)
        {
            T::addfunctions();
        }
        s.push(name);
        s.setfield("name");
        lua_pushcclosure(s,&NewObject,0);
        s.setfield("new");
        for(auto* i:functions)
        {
            i->Register(s);
        }
        lua_pushcfunction(s,&lua_Index);
        s.setfield("__index");

        lua_pushcfunction(s,&lua_NewIndex);
        s.setfield("__newindex");

        s.pushvalue(myself);
        s.newtable();
        s.settable(LUA_REGISTRYINDEX);
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
    int pushtolua(lua::state &s){ type::mywrap::GetTable(s,this);s.getfield("__udata");s.remove(-2);return 1;}\
    static void addfunctions(){\

#define LUA_FUNC mywrap::AddFunction
#define LUA_GET(var,name) mywrap::getters[name]=[](mywrap::mytype *t,lua::state &s){ return convert_to_lua(t->var,s); }
#define LUA_SET(var,name) mywrap::setters[name]=[](mywrap::mytype *t,lua::state &s){ int dum=3;t->var=convert_from_lua<decltype(var)>(s,dum);return 0;}
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
