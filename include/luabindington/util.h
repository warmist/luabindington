#ifndef LUABINDINGTON_UTIL_H_INCLUDED
#define LUABINDINGTON_UTIL_H_INCLUDED

template<typename T,bool is_class_type=(std::is_class<typename std::remove_pointer<T>::type>::value  and std::is_class<T>::value) >
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

template<typename T,bool is_class_type=(std::is_class<typename std::remove_pointer<T>::type>::value  and !std::is_class<T>::value)>
struct class_has_pullfromlua_ptr_function:
    std::false_type
{};

template<typename T>
struct class_has_pullfromlua_ptr_function<T,true>
{
    template<T (*)(lua::state &,int &)> struct tester;

    template<typename U>
    static char has_matching_member(tester<&std::remove_pointer<U>::type::pullfromlua>*); // A
    template<typename U>
    static long has_matching_member(...); // B

    static const bool value=sizeof(has_matching_member<T>(0))==sizeof(char);
};
template <class T>
struct has_pullfromlua:
    std::integral_constant<bool,

        (class_has_pullfromlua_ptr_function<T>::value ||
        class_has_pullfromlua_function<T>::value)>
{};

template <class T>
typename std::enable_if<class_has_pullfromlua_ptr_function<T>::value,T>::type
convert_from_lua_impl(lua::state &s,int &stacknum)
{

    if(!lua_getmetatable(s,stacknum))//type check
    {
        throw lua::bad_conversion("Argument does not have metatable.");
    }
    else
    {
        s.getfield("typename");
        if(s.is<lua::nil>())
        {
            s.pop(2); //pop nil and metatable
            throw lua::bad_conversion("Argument does not have correct metatable.");
        }
        std::string tname=s.as<std::string>();
        s.pop(1); //pop typename
        if(tname!=std::remove_pointer<T>::type::mywrap::_name)
        {
            s.getfield("cast_"+std::remove_pointer<T>::type::mywrap::_name);
            if(s.is<lua::nil>())
            {
                s.pop(1);//pop nil
                s.pop(1);//pop metatable
                throw lua::bad_conversion("Argument is not correct type and does not have a correct cast.");
            }
            else //exist casts, replace value in stack
            {
                s.remove(-2);//remove metatable
                s.pushvalue(stacknum);
                s.call(1,1); //call function.
                s.replace(stacknum);//replace argument
            }

        }
    }
    T v=std::remove_pointer<T>::type::pullfromlua(s,stacknum);
    return v;
}
template <class T>
typename std::enable_if<class_has_pullfromlua_function<T>::value,T>::type
convert_from_lua_impl(lua::state &s,int &stacknum)
{
    T v=T::pullfromlua(s,stacknum);
    return v;
}

template <class T>
typename std::enable_if<!has_pullfromlua<T>::value && !std::is_pointer<T>::value,T>::type
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

template <typename t_type,int num> //terminal
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
        //lua_Lerror (s,"%s at:%d argument",e.what(),num);
        std::stringstream err;
        err<<e.what();
        err<<" at:"<<num<<" argument.";
        //s.push(err.str());
        //lua_error(s); //TODO is this safe because it never returns, using longjmp?
        //s.traceback();
        throw lua::bad_conversion(err.str().c_str());
    }
    //stacknum++;
    lua_to_tuple<t_type,num+1,Tail...>(mtuple,s,stacknum); //continue converting
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
typename std::enable_if<class_has_pushtolua_function<typename std::remove_pointer<T>::type>::value && std::is_pointer<T>::value,int>::type
convert_to_lua_impl(T val,lua::state &s)
{
    return val->pushtolua(s);
}
template <class T>
typename std::enable_if<class_has_pushtolua_function<T>::value && !std::is_pointer<T>::value,int>::type
convert_to_lua_impl(T val,lua::state &s)
{
    return val.pushtolua(s);
}

template <class T>
typename std::enable_if<!class_has_pushtolua_function<T>::value && !std::is_pointer<T>::value,int>::type
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


#endif
