#include <iostream>
#include <string>
#include <luabindington.hpp>
#include "function_test.h"
#include "struct_test.h"
#include "object_test.h"
#include "lightweigth_test.h"
using namespace std;
int main()
{

    lua::state state=lua::glua::Get();
    state.loadfile("util.lua");
    state.pcall();
    //do_func_test();
    //do_struct_test();
    //do_object_test();
    do_light_test();
    return 0;
}
