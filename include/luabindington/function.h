#ifndef LUABINDINGTON_FUNCTION_H_INCLUDED
#define LUABINDINGTON_FUNCTION_H_INCLUDED

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
    lua_function(myfunc f,std::string name,lua_State *L):f(f),name(name)
    {
        Register(L);
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
    lua_function(myfunc f,std::string name,lua_State *L):f(f),name(name)
    {
        Register(L);
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
    void Register(lua_State *L) //ok
    {
        //std::cout<<"MemberFunct register\n";
        lua::state s(L);
        //lua::StackDump(L);
        lua_hold *p=(lua_hold*)s.newuserdata(sizeof(lua_hold));
        p->fptr=f;
        lua_pushcclosure(L,&call,1);
        s.setfield(name);
        //lua::StackDump(L);
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
        typename T::mywrap::lua_udata* obj_ptr;

        s.touserdata(obj_ptr,1);
        lua_to_tuple<myargs,0,Args...>(args,s,stacknum);
        myret r=applyTuple<T,myret,Args...>(obj_ptr->ptr,t_hold->fptr,args);
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
        typename T::mywrap::lua_udata* obj_ptr;

        s.touserdata(obj_ptr,1);
        lua_to_tuple<myargs,0,Args...>(args,s,stacknum);
        applyTuple<T,myret,Args...>(obj_ptr->ptr,t_hold->fptr,args);
        return 0;
        //f(args...);
    }
};
#endif // LUABINDINGTON_FUNCTION_H_INCLUDED
