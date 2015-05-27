#include <typeinfo>
#include <type_traits>
#include <functional>
#include <iostream>

//http://stackoverflow.com/a/18603716
template<typename F, typename = decltype(&F::operator())>
std::true_type supports_call_test(const F&);
std::false_type supports_call_test(...);
template<typename F> using supports_call = decltype(supports_call_test(std::declval<F>()));

struct foo {

	foo() { std::cout << "empty ctor" << std::endl; }
	foo(const foo& t) { std::cout << typeid(t).name() << " is copy ctor" << std::endl; }
	foo(const foo&& t) { std::cout << typeid(t).name() << " is move ctor" << std::endl; }
	foo(short t) { std::cout << typeid(t).name() << " is short" << std::endl; }
	foo(int t) { std::cout << typeid(t).name() << " is int" << std::endl; }
	foo(float t) { std::cout << typeid(t).name() << " is float" << std::endl; }
	foo(double t) { std::cout << typeid(t).name() << " is double" << std::endl; }
	foo(const char* t) { std::cout << typeid(t).name() << " is const char*" << std::endl; }
	foo(const std::string& t) { std::cout << typeid(t).name() << " is std::string" << std::endl; }
	foo(bool t) { std::cout << typeid(t).name() << " is bool" << std::endl; }

	template<
		typename F, typename... X
	>
	foo(F f(X...)) {
		std::cout << typeid(f).name() << " is a static function" << std::endl;
	}

	template<
		typename T,
		typename std::enable_if<supports_call<T>::value>::type...
	>
	foo(const T& t) {
		std::cout << typeid(t).name() << " has operator()" << std::endl;
	}
	
	template<
		typename T,
		typename std::enable_if<!supports_call<T>::value>::type...
	>
	foo(const T& t) {
		std::cout << typeid(t).name() << " is not a function" << std::endl;
	}

};

void bar_vv() {}
void bar_vi(int) {}
int bar_iv() { return 0; }
int bar_ii(int) { return 0; }

int main() {
	{ foo f(false); }
	{ foo f('\0'); }
	{ foo f((short)0); }
	{ foo f(0); }
	{ foo f(0.f); }
	{ foo f(0.0); }
	{ foo f(std::function<void()>([](){})); }						//std::function
	{ foo f(std::function<void(int)>([](int){})); }					//std::function
	{ foo f(std::function<int()>([]()->int{ return 0; })); }		//std::function
	{ foo f(std::function<int(int)>([](int)->int{ return 0; })); }	//std::function
	{ foo f(bar_vv); }												//static function
	{ foo f(bar_vi); }												//static function
	{ foo f(bar_iv); }												//static function
	{ foo f(bar_ii); }												//static function
	{ foo f([](){}); }												//lambda
	{ foo f([](int){}); }											//lambda
	{ foo f([]()->int{ return 0; }); }								//lambda
	{ foo f([](int)->int{ return 0; }); }							//lambda
	{ struct { void operator()(){} } b; foo f(b); }
	{ struct {} b; foo f(b); }
}

