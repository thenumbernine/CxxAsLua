#pragma once

#include <string>
#include <map>
#include <memory>
#include <initializer_list>
#include <vector>
#include <sstream>
#include <functional>
#include <iostream>
#include <cmath>

namespace CxxAsLua {

//detect whether a class has operator()
//http://stackoverflow.com/a/18603716
template<typename F, typename = decltype(&F::operator())>
std::true_type supports_call_test(const F&);
std::false_type supports_call_test(...);
template<typename F> using supports_call = decltype(supports_call_test(std::declval<F>()));


struct Object;
struct Object_Details;

extern const Object nil;

struct MapCompare {
	bool operator()(const Object&, const Object&);
};

//collection of Object's is used for argument lists passing to and from functions
template<typename ObjectType> struct VarArgType;
typedef VarArgType<Object> VarArg;
typedef VarArgType<std::reference_wrapper<Object>> VarArgRef;


template<typename T>
struct ObjectIsType;

struct Access;

struct Object {

	typedef intptr_t Int;
	typedef std::map<Object, Object, MapCompare> Map;
	typedef std::function<VarArg(VarArg)> Function;

public:	//protected:

	std::shared_ptr<Object_Details> details;

public:
	
	virtual ~Object();

	Object();
	Object(const Object& x);
	Object(const Object&& x);
	Object(const std::shared_ptr<Object_Details>& details_);
	Object(bool x);
	Object(char x);
	Object(unsigned char x);
	Object(signed char x);
	Object(wchar_t x);
	Object(char16_t x);
	Object(char32_t x);
	Object(short x);
	Object(unsigned short x);
	Object(int x);
	Object(unsigned int x);
	Object(long x);
	Object(unsigned long x);
	Object(long long x);
	Object(unsigned long long x);
	Object(float x);
	Object(double x);
	Object(long double x);
	Object(const char* x);
	Object(const signed char* x);
	Object(const unsigned char* x);
	Object(const wchar_t* x);
	Object(const char16_t* x);
	Object(const char32_t* x);
	Object(const std::string& x);
	Object(const std::wstring& x);
	Object(const std::u16string& x);
	Object(const std::u32string& x);
	Object(const Map& x);
#if 0
	template<typename T> Object(const std::initializer_list<T>& x);
#endif
#if 1
	Object(const std::initializer_list<Map::value_type>& x);
#endif
	template<typename ReturnType, typename... Args> Object(ReturnType (*func)(Args...));
	template<typename T, typename std::enable_if<supports_call<T>::value>::type...> Object(const T& t);

	/* non-function, non-primitive ... userdata?
	template<
		typename T,
		typename std::enable_if<!supports_call<T>::value>::type...
	>
	foo(const T& t) {
		std::cout << typeid(t).name() << " is not a function" << std::endl;
	}
	*/

	Object& operator=(const Object& x);
	Object& operator=(bool x);
	Object& operator=(char x);
	Object& operator=(signed char x);
	Object& operator=(unsigned char x);
	Object& operator=(wchar_t x);
	Object& operator=(char16_t x);
	Object& operator=(char32_t x);
	Object& operator=(short x);
	Object& operator=(unsigned short x);
	Object& operator=(int x);
	Object& operator=(unsigned int x);
	Object& operator=(long x);
	Object& operator=(unsigned long x);
	Object& operator=(long long x);
	Object& operator=(unsigned long long x);
	Object& operator=(float x);
	Object& operator=(double x);
	Object& operator=(long double x);
	Object& operator=(const char* x);
	Object& operator=(const signed char* x);
	Object& operator=(const unsigned char* x);
	Object& operator=(const std::string& x);
	Object& operator=(const std::wstring& x);
	Object& operator=(const std::u16string& x);
	Object& operator=(const std::u32string& x);
	Object& operator=(const Map& x);

#if 0
	template<typename T>
	Object& operator=(const std::initializer_list<T>& x);
#endif
#if 1
	Object& operator=(const std::initializer_list<Map::value_type>& x);
#endif
	template<typename ReturnType, typename... Args> 
	Object& operator=(ReturnType (*func)(Args...));
	
	template<typename T, typename std::enable_if<supports_call<T>::value>::type...>
	Object& operator=(const T& t);

	//using VarArg's cast operator instead
	// for some reason, enabling this makes things fail more often
	//Object& operator=(const VarArg& x);

	operator bool() const;
	operator char() const;
	//I think I'll handle char-casts as the first element of strings...
	operator signed char() const;
	operator unsigned char() const;
	operator wchar_t() const;
	operator char16_t() const;
	operator char32_t() const;
	operator short() const;
	operator unsigned short() const;
	operator int() const;
	operator unsigned int() const;
	operator long() const;
	operator unsigned long() const;
	operator long long() const;
	operator unsigned long long() const;
	operator float() const;
	operator double() const;
	operator long double() const;
	operator std::string() const;
	operator std::wstring() const;
	operator std::u16string() const;
	operator std::u32string() const;
	operator Map() const;
	
	template<typename ReturnType, typename... Args>
	operator std::function<ReturnType(Args...)>();
	
	template<typename... Args>
	operator std::function<void(Args...)>();

	bool is_boolean() const;
	bool is_number() const;
	bool is_string() const;
	bool is_table() const;
	bool is_function() const;
	bool is_nil() const;

	template<typename T> bool is_type() const;

	std::string type() const;

	//explicit to-number conversion.
	//returns 'true' if valid and output is placed in 'out'
	//returns 'false' otherwise
	bool tonumber(double& out) const;

	//explicit to-string conversion, bypasses conversion errors
	std::string tostring() const;

	//notice: if key is nil for reading, return nil
	//if key is nil for writing, throw error
	//to do this you must return an accessor object, then overload its read (cast) and write (ctor, operator=) functionality 
	Access operator[](Object key);
	Access operator[](const char* key);

	template<typename... Args>
	VarArg operator()(Args... args) const;
	
	VarArg call(VarArg args);
	
	VarArgRef operator,(Object& o);
	VarArg operator,(const Object& o) const;

	enum Type_t {
		TYPE_NUMBER,
		TYPE_STRING,
		TYPE_TABLE,
		TYPE_BOOLEAN,
		TYPE_FUNCTION,
		TYPE_NIL,
		NUM_TYPES
	};

	Type_t getTypeIndex() const;

	//helper function
	static double lmod(double a, double b);

	Object getMetaHandler(const std::string& event) const;
	
	static Object getBinHandler(
		const Object& op1,
		const Object& op2,
		const std::string& event
	);
	
	static Object invokeNumberMetaBinary(
		Object a,
		Object b,
		const std::string& event,
		std::function<double(double,double)> func
	);

	static Object invokeStringMetaBinary(
		Object op1,
		Object op2,
		const std::string& event,
		std::function<bool(const Object&)> testType,
		std::function<std::string(std::string,std::string)> func
	);

	//TODO these technically do their test by attempting conversion...
	template<typename T> Object operator+(const T& o) const;
	template<typename T> Object operator-(const T& o) const;
	template<typename T> Object operator*(const T& o) const;
	template<typename T> Object operator/(const T& o) const;
	template<typename T> Object operator%(const T& o) const;
	template<typename T> Object pow(const T& o) const;
	
	Object operator-() const;
		
	Object concat(const Object& o) const;

	Object len() const;

	static Object getCompareHandler(
		Object op1,
		Object op2,
		const std::string& event
	);

	template<typename T> Object operator==(const T& o) const;
	template<typename T> Object operator!=(const T& o) const;
	template<typename T> Object operator<(const T& o) const;
	template<typename T> Object operator>(const T& o) const;
	template<typename T> Object operator<=(const T& o) const;
	template<typename T> Object operator>=(const T& o) const;

	//logical
	template<typename T> const Object& operator&&(const T& o) const;
	template<typename T> const Object& operator||(const T& o) const;
	template<typename T> Object operator!() const;

	//bitwise
	template<typename T> Object operator&(const T& o) const;
	template<typename T> Object operator|(const T& o) const;
	template<typename T> Object operator~() const;
	
	//extras
	Object operator~() const;
	template<typename T> Object operator^(const T& o) const;
	template<typename T> Object operator<<(const T& o) const;
	template<typename T> Object operator>>(const T& o) const;
	template<typename T> Object& operator+=(const T& o);
	template<typename T> Object& operator-=(const T& o);
	template<typename T> Object& operator*=(const T& o);
	template<typename T> Object& operator/=(const T& o);
	template<typename T> Object& operator%=(const T& o);
	template<typename T> Object& operator>>=(const T& o);
	template<typename T> Object& operator<<=(const T& o);
	template<typename T> Object& operator&=(const T& o);
	template<typename T> Object& operator|=(const T& o);
	template<typename T> Object& operator^=(const T& o);
	Object& operator++();
	Object operator++(int);
	Object& operator--();
	Object operator--(int);
};

struct Object_Details {
public:

	//This can safely be Object_Details_Table, which is a nested class
	//...but you can't forward declare nested classes
	//...and moving them all outside Object means they can't have access to Object's typedefs
	//   ...unless I forward-declare all these, and move all Object's function bodies to after this... 
	std::shared_ptr<Object_Details> metatable;
	
	virtual ~Object_Details();

	virtual std::string type() const;

	//implicit casting (which is conservative)
	virtual double to_number() const;
	virtual std::string to_string() const;
	virtual Object::Map to_table() const;
	virtual Object::Function to_function() const;
	virtual bool to_boolean() const;

	//explicit casting
	virtual std::string explicit_to_string() const;

	virtual bool compare(const Object& o) const;
};

template<typename T>
struct Object_Details_Type : public Object_Details {
	typedef Object_Details Super;
public:
	T value;

public:
	Object_Details_Type() : Super(), value(T()) {}
	Object_Details_Type(const T& value_) : Super(), value(value_) {}
};

struct Object_Details_Number : public Object_Details_Type<double> {
	typedef Object_Details_Type<double> Super;
public:
	using Super::Super;
	
	virtual std::string type() const;
	virtual double to_number() const;
	virtual std::string to_string() const;
	virtual bool to_boolean() const;
	virtual bool compare(const Object& o) const;
};

struct Object_Details_String : public Object_Details_Type<std::string> {
	typedef Object_Details_Type<std::string> Super;
public:
	using Super::Super;
	
	virtual std::string type() const;
	
	virtual double to_number() const;
	virtual std::string to_string() const;
	virtual bool to_boolean() const;

	virtual bool compare(const Object& o) const;
};

struct Object_Details_Table : public Object_Details_Type<Object::Map> {
	typedef Object_Details_Type<Object::Map> Super;
public:
	using Super::Super;
	
	virtual std::string type() const;
	
	virtual Object::Map to_table() const;
	virtual bool to_boolean() const;

	virtual std::string explicit_to_string() const;

	virtual bool compare(const Object& o) const;
};

struct Object_Details_Boolean : public Object_Details_Type<bool> {
	typedef Object_Details_Type<bool> Super;
public:
	using Super::Super;
	
	virtual std::string type() const;
	
	virtual bool to_boolean() const;
	
	virtual std::string explicit_to_string() const;

	virtual bool compare(const Object& o) const;
};

struct Object_Details_Function : public Object_Details_Type<Object::Function> {
	typedef Object_Details_Type<Object::Function> Super;
public:
	using Super::Super;

	virtual std::string type() const;

	virtual Object::Function to_function() const;
	virtual bool to_boolean() const;

	virtual std::string explicit_to_string() const;

	virtual bool compare(const Object& o) const;
};

struct Object_Details_Nil : public Object_Details {
	typedef Object_Details Super;
public:
	using Super::Super;
	
	virtual std::string type() const;
	
	virtual std::string explicit_to_string() const;

	virtual bool compare(const Object& o) const;
};

std::ostream& operator<<(std::ostream& o, const Object& x);


template<typename ObjectType> struct InObjectTypeMap;
template<> struct InObjectTypeMap<Object> { typedef Object Type; };
template<> struct InObjectTypeMap<std::reference_wrapper<Object>> { typedef Object& Type; };

template<typename ObjectType> struct ReturnEmptyMap;

template<> struct ReturnEmptyMap<Object> {
	typedef Object ObjectType;
	typedef typename InObjectTypeMap<ObjectType>::Type InObjectType;
	static InObjectType exec(const VarArgType<ObjectType>& vargs) {
		return nil;
	}
};

template<> struct ReturnEmptyMap<std::reference_wrapper<Object>> {
	typedef std::reference_wrapper<Object> ObjectType;
	typedef typename InObjectTypeMap<ObjectType>::Type InObjectType;
	static InObjectType exec(const VarArgType<ObjectType>& vargs) {
		throw std::bad_cast();	//or out-of-bounds or something ...
	}
};

template<typename ObjectType> struct InitializerListOptionMap;

template<> struct InitializerListOptionMap<Object> {
	typedef Object ObjectType;
	typedef typename InObjectTypeMap<ObjectType>::Type InObjectType;
	typedef std::initializer_list<Object> Type;
	static void exec(VarArgType<ObjectType>& owner, const Type& x);
};

template<> struct InitializerListOptionMap<std::reference_wrapper<Object>> {
	typedef std::reference_wrapper<Object> ObjectType;
	typedef typename InObjectTypeMap<ObjectType>::Type InObjectType;
	struct Type {};	//use a type that won't match to anything.  TODO if you want, enable_if
	static void exec(VarArgType<ObjectType>& owner, const Type& x);
};

template<typename ObjectType>
struct ObjectGetRef;

template<> struct ObjectGetRef<Object> {
	static Object& get(Object& dst) { return dst; }
};
template<> struct ObjectGetRef<std::reference_wrapper<Object>> {
	static Object& get(std::reference_wrapper<Object>& dst) { return dst.get(); }
	static Object& get(const std::reference_wrapper<Object>& dst) {  return dst.get(); }
};

template<typename ObjectType, typename OtherObjectType>
struct VarArgAssignOperator {
	static void exec(
		VarArgType<ObjectType>& dst,
		const VarArgType<OtherObjectType>& src);
};

template<typename ObjectType>
struct VarArgType {
	typedef typename InObjectTypeMap<ObjectType>::Type InObjectType;
	typedef ReturnEmptyMap<ObjectType> ReturnEmpty;
	typedef InitializerListOptionMap<ObjectType> InitializerListOption;

public:
	std::vector<ObjectType> objects;

public:

	template<typename... Args>
	VarArgType(Args... args);

	VarArgType(const VarArg& o);
	VarArgType(const VarArgRef& o);
	
	//gives the "use of deleted function" bug(?) in gcc
	//VarArgType(const VarArgType&& x);

	VarArgType operator,(InObjectType o);

	template<typename... Args>
	std::tuple<Args...> toTuple() const;

	InObjectType get() const;
	InObjectType get(int offset) const;
	
	operator InObjectType() const;
	InObjectType operator[](int offset) const;

	size_t len() const { return objects.size(); }

	template<typename OtherObjectType>
	VarArgType& operator=(const VarArgType<OtherObjectType>& x) {
		VarArgAssignOperator<ObjectType, OtherObjectType>::exec(*this, x);
		return *this;
	}

	//even if you implement template<typename T> operator=(T& t)
	//you still need to override operator= for the implementing class
	VarArgType& operator=(const VarArgType& x) {
		VarArgAssignOperator<ObjectType, ObjectType>::exec(*this, x);
		return *this;
	}
/*
	operator VarArg() const {
std::cout << "cast operation" << std::endl;
		VarArg results;
		for (const ObjectType& o : objects) {
			results.objects.push_back(ObjectGetRef<ObjectType>::get(o));
		}
		return results;
	}
*/
};

template<typename ObjectType>
struct VarArgBufferSource;

template<> struct VarArgBufferSource<Object> {
	const VarArg& src;
	VarArgBufferSource(const VarArg& src_);
};

template<> struct VarArgBufferSource<std::reference_wrapper<Object>> {
	VarArg src;
	VarArgBufferSource(const VarArgRef& src_);
};

template<typename ObjectType, typename OtherObjectType>
void VarArgAssignOperator<ObjectType, OtherObjectType>::exec(
	VarArgType<ObjectType>& dstVarArg,
	const VarArgType<OtherObjectType>& srcVarArg)
{
	//intermediately buffer results so ref swizzles don't just snowball values (and instead swizzle properly)
	//...only if necessary		
	VarArgBufferSource<OtherObjectType> buffer(srcVarArg);

	typename std::vector<Object>::const_iterator src = buffer.src.objects.begin();
	typename std::vector<ObjectType>::iterator dst = dstVarArg.objects.begin();
	for (; src != buffer.src.objects.end() && dst != dstVarArg.objects.end(); ++src, ++dst) {
		ObjectGetRef<ObjectType>::get(*dst) = *src;			
	}
}

template<int offset, int size, typename... Args>
struct RecursiveTupleToVarArg {
	static void exec(const std::tuple<Args...>& t, VarArg& args) {
		Object o = std::get<offset>(t);
		args.objects.push_back(o);
		RecursiveTupleToVarArg<offset+1, size, Args...>::exec(t, args);
	}
};

template<int size, typename... Args>
struct RecursiveTupleToVarArg<size, size, Args...> {
	static void exec(const std::tuple<Args...>&, VarArg&) {}
};

template<typename ObjectType, typename... Args>
struct VarArgTypeConstructor {
	static void exec(VarArgType<ObjectType>& dest, Args... args) {
		std::tuple<Args...> t(args...);
		RecursiveTupleToVarArg<
			0,
			std::tuple_size<std::tuple<Args...>>::value,
			Args...
		>::exec(t, dest);
	}
};

template<typename ObjectType>
struct VarArgTypeConstructor<ObjectType> {
	static void exec(VarArgType<ObjectType>& dest) {}
};

/*
template<typename ObjectType>
struct VarArgTypeConstructor<ObjectType, VarArgType<ObjectType>> {
	static void exec(VarArgType<ObjectType>& dest, const VarArgType<ObjectType>& src) {
		dest.objects = src.objects;
	}
};
*/

template<typename ObjectType>
struct VarArgTypeConstructor<ObjectType, std::initializer_list<typename VarArgType<ObjectType>::InObjectType>> {
	static void exec(VarArgType<ObjectType>& dest, const std::initializer_list<typename VarArgType<ObjectType>::InObjectType>& src) {
		VarArgType<ObjectType>::InitializerListOption::exec(dest, src);
	}
};

template<typename ObjectType>
template<typename... Args>
VarArgType<ObjectType>::VarArgType(Args... args) {
	VarArgTypeConstructor<ObjectType, Args...>::exec(*this, args...);	
}

template<>
inline VarArgType<Object>::VarArgType(const VarArgType<Object>& x)
: objects(x.objects) {}

template<>
inline VarArgType<Object>::VarArgType(const VarArgType<std::reference_wrapper<Object>>& x)
{
	for (const std::reference_wrapper<Object>& o : x.objects) objects.push_back(o.get());
}

/*
template<>
inline VarArgType<std::reference_wrapper<Object>>::VarArgType(const VarArgType<Object>& x) {
	static_assert(false, "can't make references from objects ... without knowing they'll fizzle after the statement");
}
*/

template<>
inline VarArgType<std::reference_wrapper<Object>>::VarArgType(const VarArgType<std::reference_wrapper<Object>>& x)
: objects(x.objects) {}

/*
template<typename ObjectType>
VarArgType<ObjectType>::VarArgType(const VarArgType&& x) 
: objects(x.objects)
{
	std::cout << "VarArgType move constructor" << std::endl;
}
*/

template<typename ObjectType>
VarArgType<ObjectType> VarArgType<ObjectType>::operator,(InObjectType o) {
	VarArgType result(*this);
	result.objects.push_back(o);
	return result;
}

template<typename ObjectType>
typename VarArgType<ObjectType>::InObjectType VarArgType<ObjectType>::get() const {
	
	//VarArg can (kindly) return nil
	//VarArgRef needs to return a non-const reference that will survive past this function end ... 
	//...or throw
	if (objects.empty()) {
		return ReturnEmpty::exec(*this); 
	}	
	return objects[0];
}

template<typename ObjectType>
typename VarArgType<ObjectType>::InObjectType VarArgType<ObjectType>::get(int offset) const {
	--offset;
	if (offset < 0 || offset >= (int)len()) return nil;
	return objects[offset];
}

template<typename ObjectType>
VarArgType<ObjectType>::operator InObjectType() const {
	return get();
}

template<typename ObjectType>
typename VarArgType<ObjectType>::InObjectType VarArgType<ObjectType>::operator[](int offset) const {
	return get(offset);
}


template<int offset, typename... Args>
struct RecursiveVarArgToTuple;

template<int offset>
struct RecursiveVarArgToTuple<offset> {
	static std::tuple<> exec(const VarArg& args) {
		return std::make_tuple<>();
	}
};

template<int offset, typename T, typename... Args>
struct RecursiveVarArgToTuple<offset, T, Args...> {
	static std::tuple<T, Args...> exec(const VarArg& args) {
		return std::tuple_cat(
			std::make_tuple<T>((T)args.get(offset+1)),	//+1 because args (and tables) are 1-based
			RecursiveVarArgToTuple<offset+1, Args...>::exec(args)
		);
	}
};

//specialization for converting VarArg->*
template<>
struct RecursiveVarArgToTuple<0, VarArg> {
	static std::tuple<VarArg> exec(const VarArg& args) {
		return std::tuple<VarArg>(args);
	}
};

template<typename ObjectType>
template<typename... Args>
std::tuple<Args...> VarArgType<ObjectType>::toTuple() const {
	return RecursiveVarArgToTuple<0, Args...>::exec(*this);
}


//currently only defined for VarArg, which is VarArgType<Object>
template<typename... Args>
VarArg tupleToVarArg(const std::tuple<Args...>& t) {
	VarArg args;	
	RecursiveTupleToVarArg<
		0, 
		std::tuple_size<std::tuple<Args...>>::value,
		Args...
	>::exec(t, args);
	return args;
}

template<typename T>
std::ostream& operator<<(std::ostream& o, const VarArgType<T>& y) {
	std::string sep = "";
	for (const T& x : y.objects) {
		o << sep << x;
		sep = "\t";
	}
	return o;
}

template<>
struct ObjectIsType<bool> {
	bool value;
	ObjectIsType(const Object& o) : value(o.is_boolean()) {}
	operator bool() const { return value; }
};

template<>
struct ObjectIsType<double> {
	bool value;
	ObjectIsType(const Object& o) : value(o.is_number()) {}
	operator bool() const { return value; }
};

template<>
struct ObjectIsType<std::string> {
	bool value;
	ObjectIsType(const Object& o) : value(o.is_string()) {}
	operator bool() const { return value; }
};

template<typename T> bool Object::is_type() const { return ObjectIsType<T>(*this); }


//initializer-list for key/value pairs

//this option is for Map::value_type (std::pair<Object,Object>) constructing key/value tables,
// and all else constructing integer-indexed tables
#if 0
template<typename T>
Object::Object(const std::initializer_list<T>& x) {
	std::shared_ptr<Object_Details_Table> tptr = std::make_shared<Object_Details_Table>();
	details = tptr;
	int i = 1;
	for (const T& o : x) {
		tptr->value[i++] = o;
	}
}
	
template<>
inline Object::Object(const std::initializer_list<Map::value_type>& x)
: details(std::make_shared<Object_Details_Table>(Map(x))) {}

template<typename T>
Object& Object::operator=(const std::initializer_list<T>& x) {
	std::shared_ptr<Object_Details_Table> tptr = std::make_shared<Object_Details_Table>(); 
	details = tptr;
	int i = 1;
	for (const T& o : x) {
		tptr->value[i++] = o;
	}
	return *this;
}
template<>
inline Object& Object::operator=(const std::initializer_list<Map::value_type>& x) { 
	details = std::make_shared<Object_Details_Table>(Map(x)); 
	return *this; 
}
#endif
//this option is for key/value tables only
#if 1
inline Object::Object(const std::initializer_list<Map::value_type>& x) : details(std::make_shared<Object_Details_Table>(Map(x))) {}
inline Object& Object::operator=(const std::initializer_list<Map::value_type>& x) { details = std::make_shared<Object_Details_Table>(Map(x)); return *this; }
#endif

//http://stackoverflow.com/a/9288547
template<int ...> struct seq {};
template<int N, int ...S> struct gens : gens<N-1, N-1, S...> {};
template<int ...S> struct gens<0, S...>{ typedef seq<S...> type; };

template<typename Callable, typename ReturnType, typename... Args>
struct DelayDispatch {
public://protected:
	Callable func;
	std::tuple<Args...> params;

public:
	DelayDispatch(
		Callable func_,
		const std::tuple<Args...>&& params_)
	: func(func_), params(params_) {}

	ReturnType delayed_dispatch() const {
		return callFunc(typename gens<sizeof...(Args)>::type());
	}

protected:
	template<int... S>
	ReturnType callFunc(seq<S...>) const {
		return func(std::get<S>(params)...);
	}
};

template<typename Callable, typename ReturnType>
struct VarArgForReturnOfDelayedDispatch {
	template<typename... Args>
	static VarArg exec(const DelayDispatch<Callable, ReturnType, Args...>& save) {
		return VarArg(save.delayed_dispatch()); 
	}
};

//specialization for converting *->VarArg
//if the return type is VarArg then don't wrap it in another VarArg
template<typename Callable>
struct VarArgForReturnOfDelayedDispatch<Callable, VarArg> {
	template<typename... Args>
	static VarArg exec(const DelayDispatch<Callable, VarArg, Args...>& save) {
		return save.delayed_dispatch();
	}
};

//if the return type is void then return an empty VarArg
template<typename Callable>
struct VarArgForReturnOfDelayedDispatch<Callable, void> {
	template<typename... Args>
	static VarArg exec(const DelayDispatch<Callable, void, Args...>& save) {
		save.delayed_dispatch(); 
		return VarArg();
	}
};

template<typename T, typename ReturnType, typename... Args>
struct AssignCallable {
	static void exec(Object& o, const T& t) {
		o.details = std::make_shared<Object_Details_Function>(
			[=](VarArg args)->VarArg{
				DelayDispatch<T, ReturnType, Args...> save(t, args.toTuple<Args...>());
				return VarArgForReturnOfDelayedDispatch<T, ReturnType>::template exec<Args...>(save);
			}
		);
	}
};

//ctor-based function assignment from static function
template<typename ReturnType, typename... Args>
Object::Object(ReturnType (*func)(Args...)) {
	AssignCallable<ReturnType(*)(Args...), ReturnType, Args...>::exec(*this, func);
}

//operator= based function assignment from static function
template<typename ReturnType, typename... Args>
Object& Object::operator=(ReturnType (*func)(Args...)) {
	AssignCallable<ReturnType(*)(Args...), ReturnType, Args...>::exec(*this, func);
	return *this;
}

template<typename ReturnType, typename... Args>
Object::operator std::function<ReturnType(Args...)>() {
	return [=](Args... args)->ReturnType{
		VarArg results = this->template operator()<Args...>(args...);
		return (ReturnType)results[1];
	};
}

template<typename... Args>
Object::operator std::function<void(Args...)>() {
	return [=](Args... args)->void{
		this->template operator()<Args...>(args...);
	};
}

template<typename T, typename F>
struct AssignCallableToObject;

//notice that,
//because this entire process is deferred,
//it is making a copy of T (as it is passed to DelayDispatch)
template<typename T, typename ReturnType, typename... Args>
struct AssignCallableToObject<T, ReturnType (T::*)(Args...) const> {
	static void exec(Object& o, const T& t) {
		AssignCallable<T, ReturnType, Args...>::exec(o, t);
	}
};

//ctor-based function assignment from object with operator()
template<
	typename T,
	typename std::enable_if<supports_call<T>::value>::type...
>
Object::Object(const T& t) {
	AssignCallableToObject<T, decltype(&T::operator())>::exec(*this, t);
}

//operator= based function assignment from object with operator()
template<
	typename T,
	typename std::enable_if<supports_call<T>::value>::type...
>
Object& Object::operator=(const T& t) {
	AssignCallableToObject<T, decltype(&T::operator())>::exec(*this, t);
	return *this;
}

struct Access {
	Object* owner;
	Object key;
	
	Access(Object* owner_, Object key_);

	Object get() const;
	void set(Object value);
	
	Access& operator=(Object value);
	operator Object() const;

	//help C++ along with its casting...
	template<typename T> Object operator+(const T& t) const;
	template<typename T> Object operator-(const T& t) const;
	template<typename T> Object operator*(const T& t) const;
	template<typename T> Object operator/(const T& t) const;
	template<typename T> Object operator%(const T& t) const;

	//this is a helper function so table["key-of-func"]() works as-is without any extra casts...
	//...however this (for better or worse) allows Object o = table["key-of-func"] to re-wrap the original object as a function call (rather than its details) ...
	//...hmm...
	template<typename... Args>
	VarArg operator()(Args... args) {
		std::tuple<Args...> t(args...);
		VarArg vargs = tupleToVarArg<Args...>(t);
		return get().call(vargs);
	}
};


template<typename T> Object Access::operator+(const T& t) const { return get() + t; }
template<typename T> Object Access::operator-(const T& t) const { return get() - t; }
template<typename T> Object Access::operator*(const T& t) const { return get() * t; }
template<typename T> Object Access::operator/(const T& t) const { return get() / t; }
template<typename T> Object Access::operator%(const T& t) const { return get() % t; }

//the const-cast stuff gets around the whole 'mutable' / const-ness of operator() and lambdas and what not
//technically not correct, I know

template<typename... Args>
VarArg Object::operator()(Args... args) const {
	return const_cast<Object*>(this)->call(
		tupleToVarArg<Args...>(
			std::tuple<Args...>(args...)
		)
	);
}

template<>
inline VarArg Object::operator()(VarArg args) const {
	return const_cast<Object*>(this)->call(args);
}

template<typename T> Object Object::operator+(const T& o) const { return invokeNumberMetaBinary(*this, o, "__add", std::plus<double>()); }
template<typename T> Object Object::operator-(const T& o) const { return invokeNumberMetaBinary(*this, o, "__sub", std::minus<double>()); }
template<typename T> Object Object::operator*(const T& o) const { return invokeNumberMetaBinary(*this, o, "__mul", std::multiplies<double>()); }
template<typename T> Object Object::operator/(const T& o) const { return invokeNumberMetaBinary(*this, o, "__div", std::divides<double>()); }
template<typename T> Object Object::operator%(const T& o) const { return invokeNumberMetaBinary(*this, o, "__mod", lmod); }
template<typename T> Object Object::pow(const T& o) const { return invokeNumberMetaBinary(*this, o, "__pow", ::pow); }

template<typename T>
Object Object::operator==(const T& o) const {
	const Object& op1 = *this;
	Object op2 = Object(o);
	if (op1.getTypeIndex() != op2.getTypeIndex()) return false;
	if (op1.details.get() == op2.details.get()) return true;	//compare pointers, used for tables and functions ... and any other primitive
	if (!(is_table() || is_function())) {	//otherwise use built-in primitive compare
		return Object(details->compare(op2));
	}
	//by here it's a table (or function) and isn't identical, so fall back on metamethods
	Object h = getCompareHandler(op1, op2, "__eq");
	if (h) {
		return h(op1, op2);
	} else {
		return false;
	}
}

template<typename T>
Object Object::operator!=(const T& o) const {
	return Object(!operator==(o));
}

template<typename T>
Object Object::operator<(const T& o) const { 
	const Object& op1 = *this;
	Object op2(o);

	int ai = op1.getTypeIndex();
	int bi = op2.getTypeIndex();

	if (ai == bi) {
		switch (ai) {
		case TYPE_NUMBER:
			return Object(op1.details->to_number() < op2.details->to_number());
		case TYPE_STRING:
			return Object(op1.details->to_string() < op2.details->to_string());
		}
	}

	Object h = getCompareHandler(op1, op2, "__lt");
	if (h) {
		return h(op1, op2);
	} else {
		throw std::bad_cast();
	}
}

template<typename T>
Object Object::operator>(const T& o) const {
	return Object(o) < *this;
}

template<typename T>
Object Object::operator<=(const T& o) const {
	const Object& op1 = *this;
	Object op2(o);

	int ai = op1.getTypeIndex();
	int bi = op2.getTypeIndex();

	if (ai == bi) {
		switch (ai) {
		case TYPE_NUMBER:
			return Object(op1.details->to_number() <= op2.details->to_number());
		case TYPE_STRING:
			return Object(op1.details->to_string() <= op2.details->to_string());
		}
	}

	Object h = getCompareHandler(op1, op2, "__le");
	if (h) {
		return h(op1, op2);
	} else {
		h = getCompareHandler(op1, op2, "__lt");
		if (h) {
			return !(Object)h(op2, op1);
		} else {
			throw std::bad_cast();
		}
	}
}

template<typename T>
Object Object::operator>=(const T& o) const {
	return Object(o) <= *this;
}

//logical
template<typename T> const Object& Object::operator&&(const T& o) const { if (!details->to_boolean()) return *this; return o; }
template<typename T> const Object& Object::operator||(const T& o) const { if (details->to_boolean()) return *this; return o; }
template<typename T> Object Object::operator!() const { return Object(!details->to_boolean()); } 

//bitwise
template<typename T> Object Object::operator&(const T& o) const { return Object((Int)details->to_number() & (Int)Object(o).details->to_number()); } 
template<typename T> Object Object::operator|(const T& o) const { return Object((Int)details->to_number() | (Int)Object(o).details->to_number()); } 
template<typename T> Object Object::operator~() const { return Object(~(Int)details->to_number()); } 

//extras
template<typename T> Object Object::operator^(const T& o) const { return Object((Int)details->to_number() ^ (Int)Object(o).details->to_number()); }
template<typename T> Object Object::operator<<(const T& o) const { return Object((Int)details->to_number() << (Int)Object(o).details->to_number()); }
template<typename T> Object Object::operator>>(const T& o) const { return Object((Int)details->to_number() >> (Int)Object(o).details->to_number()); }
template<typename T> Object& Object::operator+=(const T& o) { details = std::make_shared<Object_Details_Number>(details->to_number() + Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator-=(const T& o) { details = std::make_shared<Object_Details_Number>(details->to_number() - Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator*=(const T& o) { details = std::make_shared<Object_Details_Number>(details->to_number() * Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator/=(const T& o) { details = std::make_shared<Object_Details_Number>(details->to_number() / Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator%=(const T& o) { details = std::make_shared<Object_Details_Number>(lmod(details->to_number(), Object(o).details->to_number())); return *this; }
template<typename T> Object& Object::operator>>=(const T& o) { details = std::make_shared<Object_Details_Number>((Int)details->to_number() >> (Int)Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator<<=(const T& o) { details = std::make_shared<Object_Details_Number>((Int)details->to_number() << (Int)Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator&=(const T& o) { details = std::make_shared<Object_Details_Number>((Int)details->to_number() & (Int)Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator|=(const T& o) { details = std::make_shared<Object_Details_Number>((Int)details->to_number() | (Int)Object(o).details->to_number()); return *this; }
template<typename T> Object& Object::operator^=(const T& o) { details = std::make_shared<Object_Details_Number>((Int)details->to_number() ^ (Int)Object(o).details->to_number()); return *this; }

Object tostring(Object o);

Object type(Object o);

//hmm... it'd be nice if there was a convenient vector wrapper (operator, => VarArg)
// that would also cast correctly between C++ primitives and Objects ...
#if 1
template<typename... Args>
void printnext(Args...);

template<>
inline void printnext() {}

template<typename T, typename ...Args>
void printnext(const T& o, Args... args) {
	std::cout << "\t" << o;
	printnext(args...);
}

template<typename... Args>
void print(Args... args);

template<>
inline void print() {
	std::cout << std::endl;
}

template<typename T>
void print(const T& o) {
	std::cout << o;
	std::cout << std::endl;
}

template<typename T, typename... Args>
void print(const T& o, Args&&... args) {
	std::cout << o;
	printnext(std::forward<Args>(args)...);
	std::cout << std::endl;
}

template<>
#endif
inline void print(const VarArg& args) {
	std::string sep = "";
	for (const Object& o : args.objects) {
		std::cout << sep;
		std::cout << o;
		sep = "\t";
	}
	std::cout << std::endl;
}

Object getmetatable(Object x);
Object setmetatable(Object x, Object m);

typedef Object local;

/*
vararg macro to map function(a,b,c, ...) to [&](Object a, Object b, Object c, ...)->VarArg
*/

#define CONCATENATE(arg1, arg2)   CONCATENATE1(arg1, arg2)
#define CONCATENATE1(arg1, arg2)  CONCATENATE2(arg1, arg2)
#define CONCATENATE2(arg1, arg2)  arg1##arg2

#define function_args_0(x)	
#define function_args_1(x) Object x
#define function_args_2(x, ...) Object x, function_args_1(__VA_ARGS__)
#define function_args_3(x, ...) Object x, function_args_2(__VA_ARGS__)
#define function_args_4(x, ...) Object x, function_args_3(__VA_ARGS__)
#define function_args_5(x, ...) Object x, function_args_4(__VA_ARGS__)
#define function_args_6(x, ...) Object x, function_args_5(__VA_ARGS__)
#define function_args_7(x, ...) Object x, function_args_6(__VA_ARGS__)
#define function_args_8(x, ...) Object x, function_args_7(__VA_ARGS__)
#define function_args_9(x, ...) Object x, function_args_8(__VA_ARGS__)
#define function_args_10(x, ...) Object x, function_args_9(__VA_ARGS__)
#define function_args_11(x, ...) Object x, function_args_10(__VA_ARGS__)
#define function_args_12(x, ...) Object x, function_args_11(__VA_ARGS__)
#define function_args_13(x, ...) Object x, function_args_12(__VA_ARGS__)
#define function_args_14(x, ...) Object x, function_args_13(__VA_ARGS__)
#define function_args_15(x, ...) Object x, function_args_14(__VA_ARGS__)
#define function_args_16(x, ...) Object x, function_args_15(__VA_ARGS__)

#define function_args_(N, ...) CONCATENATE(function_args_, N)(__VA_ARGS__)
#define function_args(...) function_args_(NARG(__VA_ARGS__), __VA_ARGS__)

#define REVERSE 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define ARGN(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define NARG_(dummy, ...) ARGN(__VA_ARGS__)
#define NARG(...) NARG_(dummy, ##__VA_ARGS__, REVERSE)

/*
considerations:

pass-by-reference crashes if an inner function references an object -- because the function fizzles
pass-by-equality might work, but it messes with any previous object scope assignment

returning implicit (no ->) causes the compiler to struggle when matching
returning Object casts all things correctly, but prevents multiple return
*/
#define function(args...) [&](function_args(args))->VarArg 


//custom classes for exposing table members as C++ members...
struct Math : public Object {
	Access abs, acos, asin, atan, atan2, ceil, cos, cosh, deg, exp, floor, fmod,
		frexp, huge, ldexp, log, log10, max, min, mod, modf, pi, pow, rad,
		random, randomseed, sin, sinh, sqrt, tan, tanh;
	Math();
};
extern Math math;

struct IO : public Object {
	Access write;
	IO();
};
extern IO io;


} //namespace CxxAsLua

