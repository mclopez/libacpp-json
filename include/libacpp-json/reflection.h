#pragma once

namespace libacpp::json::reflection {


template <typename OBJ, typename T>
class getter {
public:
getter(T OBJ::*prop): prop_(prop) {}
    T get(OBJ& obj) {
        return obj.*prop_;
    } 

    void set(OBJ& obj, const T& value) {
        obj.*prop_ = value;
    } 
private:
    T OBJ::*prop_; 
};

namespace v2 {

    

template <typename OBJ, typename T, T OBJ::*Member>    
class getter {
public:
static T get(OBJ& obj){
    return obj.*Member;
}

};

}



class type_info {

};



template <typename MemberFunction>
struct traits;

// Specialization for member function pointers
template <typename Class, typename ReturnType, typename... Args>
struct traits<ReturnType(Class::*)(Args...)> {
    using return_type = ReturnType;
    using class_type = Class;
};

template <typename Class, typename ReturnType>
struct traits<ReturnType Class::*> {
    using return_type = ReturnType;
    using class_type = Class;
};


template <size_t N>
struct fixed_string {
    char value[N];

    constexpr fixed_string(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }

    constexpr operator std::string_view() const {
        return { value, N - 1 }; // exclude null terminator??
    }

};



template <fixed_string Name, auto GETTER>
class ro_property {
public:
    
using class_type = typename traits<decltype(GETTER)>::class_type;
using value_type = typename traits<decltype(GETTER)>::return_type;

static constexpr auto name = Name;

static value_type get(class_type& c) {
    if constexpr (std::is_member_function_pointer<decltype(GETTER)>::value) {
        return (c.*GETTER)();
    }
    else if (std::is_member_pointer<decltype(GETTER)>::value){
        return c.*GETTER;
    } else {
        static_assert(std::is_member_function_pointer<decltype(GETTER)>::value || std::is_member_pointer<decltype(GETTER)>::value, "Invalid getter type");
    }
}

};

template <fixed_string S, auto GETTER, auto SETTER> 
class property: public ro_property<S, GETTER> {
public:
using class_type = typename ro_property<S, GETTER>::class_type;
using value_type = typename ro_property<S, GETTER>::value_type;
    
static  void set(class_type& c, const value_type& v) {
    if constexpr (std::is_member_function_pointer<decltype(SETTER)>::value) {
        (c.*SETTER)(v);
    }
    else if (std::is_member_pointer<decltype(SETTER)>::value){
        c.*SETTER = v;
    } else {
        static_assert(std::is_member_function_pointer<decltype(SETTER)>::value || std::is_member_pointer<decltype(SETTER)>::value, "Invalid setter type");
    }
}

};


template <typename Class> class editor; 




template <typename Tuple, typename Func, std::size_t... Is>
bool for_each_impl(const Tuple& t, Func f, std::index_sequence<Is...>) {
    // Call the function f on each element of the tuple
    return (... && (f(std::get<Is>(t)))); // Use logical AND to continue or break
}

// Public interface for the forEach function
template <typename... Args, typename Func>
void for_each(const std::tuple<Args...>& t, Func f) {
    for_each_impl(t, f, std::index_sequence_for<Args...>{});
}

template <typename Class, typename T>
T get_prop_value(Class& c, std::string_view prop_name) {
    typename editor<Class >::properties prps;
    T result; 
    for_each(prps, [prop_name, &c, &result](const auto& p){
        using prop_type = std::remove_reference<decltype(p)>::type;
        if constexpr (std::is_same_v<typename prop_type::value_type, T> == true ) {
            if (prop_type::name == prop_name)   {
                result = prop_type::get(c);
                return false;
            }
        }
        return true;
    });
    return result;
}

template <typename Class, typename T>
void set_prop_value(Class& c, std::string_view prop_name, T v) {
    typename editor<Class>::properties prps;
    for_each(prps, [prop_name, &c, &v](const auto& p){
        using prop_type = std::remove_reference<decltype(p)>::type;
        if constexpr (std::is_same_v<typename prop_type::value_type, T> == true ) {
            if (prop_type::name == prop_name)   {
                prop_type::set(c, v);
                return false;
            }
        }
        return true;
    });
}




} // libacpp::json::reflection
