/*

Lua script language implementation into C++
Just in case someone wants to make a Lua => C++ language translator

features:
*) one type for everything: nil, number, string, boolean, table, function
*) objects are passed-by-value.  refcounted smart-pointer hides details.
*) conversion to objects with constructors and operator=
*) conversion from objects with cast operators
*) implicit algebra operators
*) metatables, metamethods:
 *) arithmetic
 *) __index & __newindex
 *) __call
 *) __gc metamethod on data dtor
*) table type
 *) initializer_list support
 *) operator[] for separate read- and write- access (requires Access object)
 *) hashing of any object into table (TODO use better hashing function)
*) function type
 *) implicit call() via operator()
 *) automatic assignment of static function
 *) automatic assignment of lambdas / objects with operator()
  *) separate specialization for VarArg->* and *->VarArg functions to dodge the casting back and forth
  *) automatic casting to/from std::functions using function wrappers
 *) function() variadic macro for lambda creation
  *) implicit return nil (or implicit return type altogether)
*) vararg creation with comma operator
*) swizzle assignment with comma operator (required separate VarArg and VarArgRef structures)
*) correctly thrown exceptions / correctly thrown messages

*/


#if 0	//if you want to monitor gc...
#include <stdlib.h>
#include <iostream>
size_t totalSize = 0;
void* operator new(size_t size) {
	totalSize += size;
	size_t* ptr = (size_t*)malloc(size+sizeof(size_t));
	*ptr = size;
	std::cout << "allocating... total is " << totalSize << std::endl;
	return ptr+1;
}
void operator delete(void* ptr) noexcept (true) {
	size_t* sptr = (size_t*)ptr;
	--sptr;
	totalSize -= sptr[0];
	std::cout << "freeing... total is " << totalSize << std::endl;
	free(sptr);
}
#endif

#include "CxxAsLua/Object.h"
#include <typeinfo>

using namespace CxxAsLua;

#define ECHO(stmts) { local o; stmts; print(#stmts, o, type(o)); }

//TODO account for all conversions so we don't have to explicitly cast here?
#define ASSERT_EQUALS(a,b) { if ((a)!=(b)) throw std::runtime_error( std::string("expected ") + (std::string)tostring(Object(a)) + std::string(" but got ") + (std::string)tostring(Object(b))); }

#define O_ASSERT_EQUALS(stmts,value) { local o; stmts; print(#stmts, o, type(o)); ASSERT_EQUALS(o,Object(value)); }
#define ASSERT_FAIL(stmts) { bool failed=false; try { local o; stmts; failed=true; } catch(std::exception& e) {} if (failed) throw std::runtime_error("expected failure instead passed"); }

//looks like the C++ parser not only neglects comma counting within <> template arguments -- since they could be mistaken for operator< and operator>
// -- but also neglects to count them between {}'s in initializer_list's ... because ???
#define COMMA ,

static void bar_vv() { print("bar_vv"); }
static void bar_vi(int i) { print("bar_vi", i); ASSERT_EQUALS(i, 20); }
static int bar_iv() { print("bar_iv"); return 10; }
static int bar_ii(int i) { print("bar_ii", i); ASSERT_EQUALS(i, 20); return 10; }

void test_main() {

#if 1
	O_ASSERT_EQUALS(,Object())
	//operator= tests
	O_ASSERT_EQUALS(o=false,false)
	O_ASSERT_EQUALS(o=true,true)
	O_ASSERT_EQUALS(o='c',"c")
	O_ASSERT_EQUALS(o=L'c',"c")
	//O_ASSERT_EQUALS(o=char16_t('c'),"c")
	//O_ASSERT_EQUALS(o=char32_t('c'),"c")
	O_ASSERT_EQUALS(o=(short)-32768,-32768)
	O_ASSERT_EQUALS(o=(short)32767,32767)
	O_ASSERT_EQUALS(o=(unsigned short)65535,65535)
	O_ASSERT_EQUALS(o=(int)4,4)
	//O_ASSERT_EQUALS(o=(int)-2147483647,-2147483647)
	//O_ASSERT_EQUALS(o=(int)2147483648,2147483648)
	//O_ASSERT_EQUALS(o=(unsigned int)4294967295,4294967295)
	//O_ASSERT_EQUALS(o=(long)-9223372036854775807,-9223372036854775807)
	//O_ASSERT_EQUALS(o=(long)9223372036854775808,9223372036854775808)
	//O_ASSERT_EQUALS(o=(unsigned long)18446744073709551615,18446744073709551615)
	O_ASSERT_EQUALS(o=3.f,3)
	O_ASSERT_EQUALS(o=2.0,2)
	O_ASSERT_EQUALS(o="foo","foo")
	O_ASSERT_EQUALS(o=std::string("foo"),"foo")
	O_ASSERT_EQUALS(o=std::wstring{L'f' COMMA L'o' COMMA L'o'},"foo")
	//O_ASSERT_EQUALS(o=std::u16string{L'f' COMMA L'o' COMMA L'o'},"foo")
	//O_ASSERT_EQUALS(o=std::u32string{L'f' COMMA L'o' COMMA L'o'},"foo")
	/*
	Works:
		Object o = {{}};
		Object o({});
		Object o{{}};
		Object o; o={};
		Object o = Object({});
		Object o = Object::Map();
	Doesn't work:
		Object o{}; 
		Object o = {}; 
		Object o = Object{};
	...because C++ has single empty {}'s go back to the default ctor.  not an empty list ctor.  why?  because that would just be common sense if it did.
	*/
	ECHO(o=Object::Map())
	ECHO(o=Object::Map(); o["foo"]="bar";)
	ECHO(o={Object::Map::value_type("a",1)})
	ECHO(o={Object::Map::value_type("a",1) COMMA Object::Map::value_type("b",2)})
	ECHO(o={std::make_pair("a",1)})
	ECHO(o={std::make_pair("a",1) COMMA std::make_pair("b",2)})
	ECHO(o={})
	ECHO(o={{"a" COMMA 1}})
	ECHO(o={{"a" COMMA 1} COMMA {"b" COMMA 2} COMMA {"c" COMMA 3}})
	ECHO(o={}; print(o==o));
	ECHO(o={}; print(o==Object(Object::Map())));
	O_ASSERT_EQUALS(o=1; o=o+2, 3)
	O_ASSERT_EQUALS(o=1; o=o+2.5, 3.5)
	O_ASSERT_EQUALS(o=1; o=o-2, -1)
	O_ASSERT_EQUALS(o=1; o=o-2.5, -1.5)
	O_ASSERT_EQUALS(o=1; o=o*2, 2)
	O_ASSERT_EQUALS(o=1; o=o*2.5, 2.5)
	O_ASSERT_EQUALS(o=1; o=o/2, .5)
	O_ASSERT_EQUALS(o=1; o=o/2.5, .4)
	O_ASSERT_EQUALS(o=-2.5; o=o%1, .5)
	O_ASSERT_EQUALS(o=-2.5; o=o%1.5, .5)
	O_ASSERT_EQUALS(o=1; o+=2, 3)
	O_ASSERT_EQUALS(o=1; o+=2.5, 3.5)
	O_ASSERT_EQUALS(o=1; o-=2, -1)
	O_ASSERT_EQUALS(o=1; o-=2.5, -1.5)
	O_ASSERT_EQUALS(o=1; o*=2, 2)
	O_ASSERT_EQUALS(o=1; o*=2.5, 2.5)
	O_ASSERT_EQUALS(o=1; o/=2, .5)
	O_ASSERT_EQUALS(o=1; o/=2.5, .4)
	O_ASSERT_EQUALS(o=1; o%=2, 1)
	O_ASSERT_EQUALS(o=1; o%=2.5, 1)
	O_ASSERT_EQUALS(o=1; o++, 2)
	O_ASSERT_EQUALS(o=1; ++o, 2)
	O_ASSERT_EQUALS(o=1; o--, 0)
	O_ASSERT_EQUALS(o=1; --o, 0)
	O_ASSERT_EQUALS(o=1; o+="2.5", 3.5)
	ASSERT_FAIL(o=1; o+="threeve")
	ASSERT_FAIL(o=1; o+=nil)
	ASSERT_FAIL(o=1; o+=Object{})
	O_ASSERT_EQUALS(o=1; std::cout<<(double)o<<std::endl, 1)
	O_ASSERT_EQUALS(o="foo"; std::cout<<(std::string)o<<std::endl, "foo")
	O_ASSERT_EQUALS(o="2.5"; std::cout<<(double)o<<std::endl, "2.5")
	O_ASSERT_EQUALS(o=true; std::cout<<(bool)o<<std::endl, true)
	O_ASSERT_EQUALS(o=Object(1)==Object(1), true)
	O_ASSERT_EQUALS(o=Object(1)==Object(2), false)
	O_ASSERT_EQUALS(o=Object(1)!=Object(1), false)
	O_ASSERT_EQUALS(o=Object(1)!=Object(2), true)
	O_ASSERT_EQUALS(o=Object(1)<Object(1), false)
	O_ASSERT_EQUALS(o=Object(1)<Object(2), true)
	O_ASSERT_EQUALS(o=Object(1)<=Object(1), true)
	O_ASSERT_EQUALS(o=Object(1)<=Object(2), true)
	O_ASSERT_EQUALS(o=Object(2)>Object(1), true)
	O_ASSERT_EQUALS(o=Object(2)>Object(2), false)
	O_ASSERT_EQUALS(o=Object(2)>=Object(1), true)
	O_ASSERT_EQUALS(o=Object(2)>=Object(2), true)
	O_ASSERT_EQUALS(o="a";o=o.concat("b"), "ab")
	O_ASSERT_EQUALS(o=2;o=o.concat(2), "22")
	O_ASSERT_EQUALS(o=2;o=o.concat("a"), "2a")
	O_ASSERT_EQUALS(o="a";o=o.concat(2), "a2")
	ASSERT_FAIL(o=true;o=o.concat(1))
	ASSERT_FAIL(o=Object{};o=o.concat(1))

	//proof that assignment is always by pointer
	O_ASSERT_EQUALS(o=Object::Map();local p=o;p["foo"]="bar";o=o["foo"], "bar")

	ECHO(o=function(){print("this gets correctly matched now that I've added ::operator()-detect SFINAE ctors."); return nil; }; o())

	{
		Object o = function(x){
			std::cout << "i = " << x << std::endl;
			return 40;
		};

		print(o, type(o));

		std::cout << "o(20) = " << o(20) << std::endl;
	}

	//comma-operator-combined values:
	//if assigning to VarArgs of non-Objects then the non-Object-type's operator, will kick in and a VarArg will not be created
	//so either (1) explicitly construct each Object(),
	//or (2) wrap the whole in a VarArg()
	O_ASSERT_EQUALS(o=(Object(1), Object(2)), 1)
	O_ASSERT_EQUALS(local p; (o, p)=(Object(2), Object(1)), 2)
	O_ASSERT_EQUALS(local p; (o, p)=(Object(2), Object(1)); o=p, 1)
	//assign to refs from values
	O_ASSERT_EQUALS(local p; (o,p)=VarArg(2,1), 2)
	O_ASSERT_EQUALS(local p; (o,p)=VarArg(2,1); o=p, 1)
	//assign to refs from refs
	O_ASSERT_EQUALS(o=1; local p=2; (o,p)=(p,o), 2)
	O_ASSERT_EQUALS(o=1; local p=2; (o,p)=(p,o); o=p, 1)
#endif

#if 1
	{
		//it would be nice if an operator= would...
		//(1) implicit create a "assignment" object
		//(2) collect via operator, into stack objects...
		//(3a) if placed in a initializer_list, be used to construct a table
		//(3b) if not, evaluate to variable assignments 

		bool destroyed = false;
		{
std::cout << "begin constructing o" << std::endl;
			local o = setmetatable({
				{"foo", 10}	
			}, {
				{"__add", function(self,x){
					return self["foo"] * x;
				}},
				{"__call", function(self,a,b){
					print(self,a,b);
					ASSERT_EQUALS(self, o);
					ASSERT_EQUALS(a, Object("foo"));
					ASSERT_EQUALS(b, Object(2));
					return "bar";
				}},
				{"__gc", function(){
					print("dtor!");
					destroyed = true;
					return nil;
				}},
			});
std::cout << "done constructing o" << std::endl;

			print(o);
			ASSERT_EQUALS(Object(o + 2), Object(20));	//test __add
			ASSERT_EQUALS((Object)o("foo", 2), Object("bar"));	//test __call
		}
		ASSERT_EQUALS(destroyed, true);
	}
#endif

	//automatic conversion of various function wrappers
#if 1
	// static functions:
	std::cout << "using ctor" << std::endl; //using ctor
	{ Object o = bar_vv; o(); }
	{ Object o = bar_vi; o(20); }
	{ Object o = bar_iv; ASSERT_EQUALS((Object)o(), 10); }
	{ Object o = bar_ii; ASSERT_EQUALS((Object)o(20), 10); }
	std::cout << "using operator=" << std::endl; //using operator=
	{ Object o; o = bar_vv; o(); }
	{ Object o; o = bar_vi; o(20); }
	{ Object o; o = bar_iv; ASSERT_EQUALS((Object)o(), 10); }
	{ Object o; o = bar_ii; ASSERT_EQUALS((Object)o(20), 10); }

	// callable objects:
	std::cout << "using ctor" << std::endl; //using ctor
	{ Object o = [](){ print("lambda vv"); }; o(); }
	{ Object o = [](int i){ print("lambda vi", i); ASSERT_EQUALS(i, 20); }; o(20); }
	{ Object o = []()->int{ print("lambda iv"); return 10; }; ASSERT_EQUALS((Object)o(), 10); }
	{ Object o = [](int i)->int{ print("lambda vi", i); ASSERT_EQUALS(i, 20); return 10; }; ASSERT_EQUALS((Object)o(20), 10); }
	std::cout << "using operator=" << std::endl; //using operator=
	{ Object o; o = [](){ print("lambda vv"); }; o(); }
	{ Object o; o = [](int i){ print("lambda vi", i); ASSERT_EQUALS(i, 20); }; o(20); }
	{ Object o; o = []()->int{ print("lambda iv"); return 10; }; ASSERT_EQUALS((Object)o(), 10); }
	{ Object o; o = [](int i)->int{ print("lambda vi", i); ASSERT_EQUALS(i, 20); return 10; }; ASSERT_EQUALS((Object)o(20), 10); }

	//function argument casting to and from Object
	{
		auto f = [](double x)->double{ return x+1; };
		print("f", f(2));

		Object o = f;
		print("o", o(2));
	
		std::function<double(double)> g = o;
		print("g", g(2));
	}
#endif

#if 1
	{
		//how to get an implicit cast from any primitive to Object to VarArg
		//should I even try for something like that?
		local fact = function(o){
			if (o < 1) return 1;
			return o*fact(o-1);
		};
		print(fact);
		print(fact(0));
		print(fact(5));
	}
#endif

#if 1	
	{
		local curry = function(f,x){
			//curry by reference fails
			return [=](Object y){
				return f(x,y);
			};
		};

		local add = function(a,b) { return a + b; };
		print(add(1,1));
		local addTwo = curry(add, 2);
		print(addTwo(1));
	}
#endif

#if 1
	{
		local f = [=](VarArg args)->VarArg {
			print(args[1], args[2], args[3]);
			return VarArg(4,5,6);
		};
		print(f(1,2,3));
	}
#endif

#if 1
	{
		//can't use [&] or we'll be returning a reference of the function, and that crashes
		//but if using [=] then we can't assign to external references
		local f = [=](){
			return [=](VarArg args){
				print("hi", args.len());
				print(args[1], args[2], args[3]);
				return VarArg(1, 2, 3);
			};
		};
		//passing functions around works fine as long as no other objects are passed/referenced with them
		// because their types are at the main() function scope, so we can pass-by-equality and be safe
		local g = f();
		print(g(4,5,6));
	}
#endif

#if 1 //translated from the live demo site
	
	// bisect.lua
	// bisection method for solving non-linear equations

	local delta = 1e-6;	// tolerance
	local n;

	local bisect = function(f,a,b,fa,fb) {
		local c = (a + b) / 2;
		io.write(n," c=",c," a=",a," b=",b,"\n");
		if (c==a or c==b or (Object)math.abs(a-b) < delta) { return VarArg(c,b-a); }
		n=n+1;
		local fc=f(c);
		if (fa*fc<0) {
			return bisect(f,a,c,fa,fc);
		}
		return bisect(f,c,b,fc,fb);
	};

	// find root of f in the inverval [a,b]. needs f(a)*f(b)<0
	local solve = function(f,a,b) {
		n=0;
		local z, e;
		(z, e) = bisect(f,a,b,f(a),f(b));
		io.write("after ",n," steps, root is ",z," with error ",e,", f=",f(z),"\n");
		return nil;
	};

	// our function
	local f = function(x) {
		return x*x*x-x-1;
	};

	// find zero in [1,2]
	solve(f,1,2);

#endif
}
