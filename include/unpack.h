#ifndef UNPACK_H_INCLUDED
#define UNPACK_H_INCLUDED

#include <tuple>

// Recursive case, unpack Nth argument
template<typename ret,unsigned int N>
struct Apply_aux
{
template<typename... ArgsF, typename... ArgsT, typename... Args>
static ret apply(ret (*f)(ArgsF...), std::tuple<ArgsT...> const& t,
Args... args)
{
return Apply_aux<ret,N-1>::apply(f, t, std::get<N-1>(t), args...);
}
};

// Terminal case, call the function with unpacked arguments
template<typename ret>
struct Apply_aux<ret,0>
{
template<typename... ArgsF, typename... ArgsT, typename... Args>
static ret apply(ret (*f)(ArgsF...), std::tuple<ArgsT...> const&,
Args... args)
{
return f(args...);
}
};

// Actual apply function
template<typename ret,typename... ArgsF>
ret apply(ret (*f)(ArgsF...), std::tuple<ArgsF...> const& t)
{
return Apply_aux<ret,sizeof...(ArgsF)>::apply(f, t);
}


//////////////////////////////////////////////////MEMB_FUN
/**
 * Object Function Tuple Argument Unpacking
 *
 * This recursive template unpacks the tuple parameters into
 * variadic template arguments until we reach the count of 0 where the function
 * is called with the correct parameters
 *
 * @tparam N Number of tuple arguments to unroll
 *
 * @ingroup g_util_tuple
 */
template < unsigned int N >
struct apply_obj_func
{
  template < typename T,typename R, typename... ArgsF, typename... ArgsT, typename... Args >
  static R applyTuple( T* pObj,
                          R (T::*f)( ArgsF... ),
                          const std::tuple<ArgsT...>& t,
                          Args... args )
  {
    return apply_obj_func<N-1>::applyTuple( pObj, f, t, std::get<N-1>( t ), args... );
  }
};

//-----------------------------------------------------------------------------

/**
 * Object Function Tuple Argument Unpacking End Point
 *
 * This recursive template unpacks the tuple parameters into
 * variadic template arguments until we reach the count of 0 where the function
 * is called with the correct parameters
 *
 * @ingroup g_util_tuple
 */
template <>
struct apply_obj_func<0>
{
  template < typename T,typename R, typename... ArgsF, typename... ArgsT, typename... Args >
  static R applyTuple( T* pObj,
                          R (T::*f)( ArgsF... ),
                          const std::tuple<ArgsT...>& /* t */,
                          Args... args )
  {
    return (pObj->*f)( args... );
  }
};

//-----------------------------------------------------------------------------

/**
 * Object Function Call Forwarding Using Tuple Pack Parameters
 */
// Actual apply function
template < typename T,typename R, typename... ArgsF, typename... ArgsT >
R applyTuple( T* pObj,
                 R (T::*f)( ArgsF... ),
                 std::tuple<ArgsT...> const& t )
{
   return apply_obj_func<sizeof...(ArgsT)>::applyTuple( pObj, f, t );
}

#endif // UNPACK_H_INCLUDED
