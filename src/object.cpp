#include "CxxAsLua/Object.h"
#include <limits>
#include <cmath>
#include <cstdlib>

namespace CxxAsLua {

//from http://www.linuxquestions.org/questions/programming-9/wstring-utf8-conversion-in-pure-c-701084/
std::string u32strToUtf8(const std::u32string& src){
	std::string dest = "";
	for (char32_t w : src) {
		if (w <= 0x7f) {
			dest.push_back((char)w);
		} else if (w <= 0x7ff) {
			dest.push_back(0xc0 | ((w >> 6)& 0x1f));
			dest.push_back(0x80 | (w & 0x3f));
		} else if (w <= 0xffff) {
			dest.push_back(0xe0 | ((w >> 12)& 0x0f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		} else if (w <= 0x10ffff) {
			dest.push_back(0xf0 | ((w >> 18)& 0x07));
			dest.push_back(0x80 | ((w >> 12) & 0x3f));
			dest.push_back(0x80 | ((w >> 6) & 0x3f));
			dest.push_back(0x80 | (w & 0x3f));
		} else {
			dest.push_back('?');
		}
	}
	return dest;
}

std::string u16strToUtf8(const std::u16string& src) {
	std::u32string tmp;
	std::string dest;
	char16_t c = 0;
	bool pushed = false;
	for (size_t i = 0; i < src.size() || pushed;) {
		if (!pushed) c = src[i++];
		pushed = false;
		if (c >= 0xd800 && c <= 0xdbff) {
			char16_t c2 = src[i++];
			if (c2 >= 0xdc00 && c2 <= 0xdfff) {
				tmp.push_back((c << 10) + c2 - 0x35fdc00);
			} else {
				c = c2;
				pushed = true;
			}
		} else {
			tmp.push_back(c);
		}
	}
	return u32strToUtf8(tmp);
}

std::string wstrToUtf8(const std::wstring& src) {
	if (sizeof(wchar_t) == 2) {
		std::u16string tmp;
		for (char16_t c : src) {
			tmp.push_back(c);
		}
		return u16strToUtf8(tmp);
	} else if (sizeof(wchar_t) == 4) {
		std::u32string tmp;
		for (char32_t c : src) {
			tmp.push_back(c);
		}
		return u32strToUtf8(tmp);
	}
}

std::u16string utfToU16str(const std::string&) {
	throw std::runtime_error("TODO");
}

std::u32string utfToU32str(const std::string&) {
	throw std::runtime_error("TODO");
}

std::wstring utfToWstr(const std::string& s) {
	if (sizeof(wchar_t) == 2) {
		std::wstring result;
		for (char16_t c : utfToU16str(s)) {
			result.push_back(c);
		}
		return result;
	} else if (sizeof(wchar_t) == 4) {
		std::wstring result;
		for (char32_t c : utfToU32str(s)) {
			result.push_back(c);
		}
		return result;
	}
}

//TODO base on hash
bool MapCompare::operator()(const Object& a, const Object& b) {
	//first sort by type
	//then sort by type's comparison 
	int ai = a.getTypeIndex();
	int bi = b.getTypeIndex();
	if (ai < bi) return true;
	if (ai > bi) return false;
	//same type
	switch (ai) {
	case Object::TYPE_NIL:
		return false;	//always equals
	case Object::TYPE_BOOLEAN:
		return a.details->to_boolean() < b.details->to_boolean();
	case Object::TYPE_NUMBER:
		return a.details->to_number() < b.details->to_number();
	case Object::TYPE_STRING:
		return a.details->to_string() < b.details->to_string();
	//by-pointer
	case Object::TYPE_TABLE:
	case Object::TYPE_FUNCTION:
		return a.details.get() < b.details.get();
	}
	//unknown type?
	throw std::runtime_error("tried to compare objects of unknown types");
}

Object::~Object() {
	if (!details.unique()) return;
	//last one!
	std::shared_ptr<Object_Details_Table> mt = std::dynamic_pointer_cast<Object_Details_Table>(details->metatable);
	if (!mt) return;
		
	Object::Map::iterator v = mt->value.find("__gc");
	if (v == mt->value.end()) return;

	//don't throw exceptions in the dtor (right?)
	v->second(*this);
}
	
Object::Object() : details(std::make_shared<Object_Details_Nil>()) {}
Object::Object(const Object& x) : details(x.details) {}
Object::Object(const Object&& x) : details(x.details) {}
Object::Object(const std::shared_ptr<Object_Details>& details_) : details(details_) {}
Object::Object(bool x) : details(std::make_shared<Object_Details_Boolean>(x)) {}
Object::Object(char x) : details(std::make_shared<Object_Details_String>(std::string{x})) {}
Object::Object(unsigned char x) : details(std::make_shared<Object_Details_String>(std::string{(char)x})) {}
Object::Object(signed char x) : details(std::make_shared<Object_Details_String>(std::string{(char)x})) {}
Object::Object(wchar_t x) : details(std::make_shared<Object_Details_String>(wstrToUtf8(std::wstring{x}))) {}
Object::Object(char16_t x) : details(std::make_shared<Object_Details_String>(u16strToUtf8(std::u16string{x}))) {}
Object::Object(char32_t x) : details(std::make_shared<Object_Details_String>(u32strToUtf8(std::u32string{x}))) {}
Object::Object(short x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(unsigned short x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(int x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(unsigned int x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(long x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(unsigned long x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(long long x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(unsigned long long x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(float x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(double x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(long double x) : details(std::make_shared<Object_Details_Number>(x)) {}
Object::Object(const char* x) : details(std::make_shared<Object_Details_String>(std::string(x))) {}
Object::Object(const signed char* x) : details(std::make_shared<Object_Details_String>(std::string((const char*)x))) {}
Object::Object(const unsigned char* x) : details(std::make_shared<Object_Details_String>(std::string((const char*)x))) {}
Object::Object(const wchar_t* x) : details(std::make_shared<Object_Details_String>(wstrToUtf8(std::wstring(x)))) {}
Object::Object(const char16_t* x) : details(std::make_shared<Object_Details_String>(u16strToUtf8(std::u16string(x)))) {}
Object::Object(const char32_t* x) : details(std::make_shared<Object_Details_String>(u32strToUtf8(std::u32string(x)))) {}
Object::Object(const std::string& x) : details(std::make_shared<Object_Details_String>(x)) {}
Object::Object(const std::wstring& x) : details(std::make_shared<Object_Details_String>(wstrToUtf8(x))) {}
Object::Object(const std::u16string& x) : details(std::make_shared<Object_Details_String>(u16strToUtf8(x))) {}
Object::Object(const std::u32string& x) : details(std::make_shared<Object_Details_String>(u32strToUtf8(x))) {}
Object::Object(const Map& x) : details(std::make_shared<Object_Details_Table>(x)) {}

Object& Object::operator=(const Object& x) { details = x.details; return *this; }
Object& Object::operator=(bool x) { details = std::make_shared<Object_Details_Boolean>(x); return *this; }
Object& Object::operator=(char x) { details = std::make_shared<Object_Details_String>(std::string{x}); return *this; }
Object& Object::operator=(signed char x) { details = std::make_shared<Object_Details_String>(std::string{(char)x}); return *this; }
Object& Object::operator=(unsigned char x) { details = std::make_shared<Object_Details_String>(std::string{(char)x}); return *this; }
Object& Object::operator=(wchar_t x) { details = std::make_shared<Object_Details_String>(wstrToUtf8(std::wstring{x})); return *this; }
Object& Object::operator=(char16_t x) { details = std::make_shared<Object_Details_String>(u16strToUtf8(std::u16string{x})); return *this; }
Object& Object::operator=(char32_t x) { details = std::make_shared<Object_Details_String>(u32strToUtf8(std::u32string{x})); return *this; }
Object& Object::operator=(short x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(unsigned short x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(int x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(unsigned int x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(long x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(unsigned long x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(long long x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(unsigned long long x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(float x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(double x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(long double x) { details = std::make_shared<Object_Details_Number>(x); return *this; }
Object& Object::operator=(const char* x) { details = std::make_shared<Object_Details_String>(std::string(x)); return *this; }
Object& Object::operator=(const signed char* x) { details = std::make_shared<Object_Details_String>(std::string((const char*)x)); return *this; }
Object& Object::operator=(const unsigned char* x) { details = std::make_shared<Object_Details_String>(std::string((const char*)x)); return *this; }
Object& Object::operator=(const std::string& x) { details = std::make_shared<Object_Details_String>(x); return *this; }
Object& Object::operator=(const std::wstring& x) { details = std::make_shared<Object_Details_String>(wstrToUtf8(x)); return *this; }
Object& Object::operator=(const std::u16string& x) { details = std::make_shared<Object_Details_String>(u16strToUtf8(x)); return *this; }
Object& Object::operator=(const std::u32string& x) { details = std::make_shared<Object_Details_String>(u32strToUtf8(x)); return *this; }
Object& Object::operator=(const Map& x) { details = std::make_shared<Object_Details_Table>(x); return *this; }

Object::operator bool() const { return details->to_boolean(); }
Object::operator char() const { return details->to_string()[0]; }
Object::operator signed char() const { return (signed char)details->to_string()[0]; }
Object::operator unsigned char() const { return (unsigned char)details->to_string()[0]; }
Object::operator wchar_t() const { return utfToWstr(details->to_string())[0]; }
Object::operator char16_t() const { return utfToU16str(details->to_string())[0]; }
Object::operator char32_t() const { return utfToU32str(details->to_string())[0]; }
Object::operator short() const { return (short)details->to_number(); }
Object::operator unsigned short() const { return (unsigned short)details->to_number(); }
Object::operator int() const { return (int)details->to_number(); }
Object::operator unsigned int() const { return (unsigned int)details->to_number(); }
Object::operator long() const { return (long)details->to_number(); }
Object::operator unsigned long() const { return (unsigned long)details->to_number(); }
Object::operator long long() const { return (long long)details->to_number(); }
Object::operator unsigned long long() const { return (unsigned long long)details->to_number(); }
Object::operator float() const { return (float)details->to_number(); }
Object::operator double() const { return details->to_number(); }
Object::operator long double() const { return details->to_number(); }
Object::operator std::string() const { return details->to_string(); }
Object::operator std::wstring() const { return utfToWstr(details->to_string()); }
Object::operator std::u16string() const { return utfToU16str(details->to_string()); }
Object::operator std::u32string() const { return utfToU32str(details->to_string()); }
Object::operator Map() const { return details->to_table(); }

bool Object::is_boolean() const { return !!std::dynamic_pointer_cast<Object_Details_Boolean>(details); }
bool Object::is_number() const { return !!std::dynamic_pointer_cast<Object_Details_Number>(details); }
bool Object::is_string() const { return !!std::dynamic_pointer_cast<Object_Details_String>(details); }
bool Object::is_table() const { return !!std::dynamic_pointer_cast<Object_Details_Table>(details); }
bool Object::is_function() const { return !!std::dynamic_pointer_cast<Object_Details_Function>(details); }
bool Object::is_nil() const { return !!std::dynamic_pointer_cast<Object_Details_Nil>(details); }

std::string Object::type() const { return details->type(); }

bool Object::tonumber(double& out) const {
	std::shared_ptr<Object_Details_Number> nptr = std::dynamic_pointer_cast<Object_Details_Number>(details);
	if (nptr) {
		out = nptr->value;
		return true;
	}

	std::shared_ptr<Object_Details_String> sptr = std::dynamic_pointer_cast<Object_Details_String>(details);
	if (sptr) {
		std::istringstream ss(sptr->value);
		return !!(ss >> out);
	}

	return false;
}

std::string Object::tostring() const {
	return details->explicit_to_string();
}

std::ostream& operator<<(std::ostream& o, const Object& x) { 
	return o << x.tostring();
}

Access Object::operator[](Object key) { return Access(this, key); }
Access Object::operator[](const char* key) { return Access(this, Object(key)); }	//...or else C++ chokes with literal string dereferences: "ambiguous overloaded operator"

//using VarArg's cast operator instead.  go back to this if that becomes a problem.
//Object& Object::operator=(const VarArg& x) { details = x.objects[0].get().details; return *this; }


VarArg Object::call(VarArg args) {
	std::shared_ptr<Object_Details_Function> fptr = std::dynamic_pointer_cast<Object_Details_Function>(details);
	if (fptr) {
		return fptr->value(args);
	} else {
		Object h = getMetaHandler("__call");
		if (h) {
			args.objects.insert(args.objects.begin(), *this);
			return h(args);
		} else {
			throw std::runtime_error(
				std::string("attempted to call a ")
				+ details->type() +
				std::string(" value"));
		}
	}
}

Object::Type_t Object::getTypeIndex() const {
	if (is_number()) return TYPE_NUMBER;
	if (is_string()) return TYPE_STRING;
	if (is_table()) return TYPE_TABLE;
	if (is_boolean()) return TYPE_BOOLEAN;
	if (is_function()) return TYPE_FUNCTION;
	if (is_nil()) return TYPE_NIL;
	throw std::runtime_error("unable to determine what type an object is");
}

//helper function
double Object::lmod(double a, double b) {
	double v = fmod(a,b);
	return v < 0 ? v + b : v;
}

VarArgRef Object::operator,(Object& o) {
	VarArgRef result;
	result.objects = {*this, o};
	return result;
}

VarArg Object::operator,(const Object& o) const {
	return VarArg(*this, o);
}

Object Object::getMetaHandler(const std::string& event) const {
	std::shared_ptr<Object_Details_Table> mt = std::dynamic_pointer_cast<Object_Details_Table>(details->metatable);
	if (mt) {
		Object me = mt->value[Object(event)];
		if (!me.is_nil()) return me;
	}
	return nil;
}

Object Object::getBinHandler(const Object& op1, const Object& op2, const std::string& event) {
	return op1.getMetaHandler(event) || op2.getMetaHandler(event);
}

Object Object::invokeNumberMetaBinary(Object op1, Object op2, const std::string& event, std::function<double(double,double)> f) {
	double o1, o2;
	bool op1_isnumber = op1.tonumber(o1);
	bool op2_isnumber = op2.tonumber(o2);
	if (op1_isnumber && op2_isnumber) {
		return Object(f(o1,o2));
	} else {
		Object h = getBinHandler(op1, op2, event);
		if (h) {
			return h(op1, op2);
		} else {
			throw std::runtime_error(
				std::string("attempt to perform arithmetic on a ")
				+ (op1_isnumber ? op2 : op1).details->type() +
				std::string(" value"));	//no handler available
		}
	}
}

Object Object::invokeStringMetaBinary(
	Object op1,
	Object op2,
	const std::string& event,
	std::function<bool(const Object&)> testType,
	std::function<std::string(std::string,std::string)> func
) {
	bool op1_istype = testType(op1);
	bool op2_istype = testType(op2);
	if (op1_istype && op2_istype) {
		return Object(func((std::string)op1, (std::string)op2));
	} else {
		Object h = getBinHandler(op1, op2, event);
		if (h) {
			return h(op1, op2);
		} else {
			throw std::runtime_error(
				std::string("attempt to concatenate a ")
				+ (op1_istype ? op2 : op1).details->type() +
				std::string(" value"));	//no handler available
		}
	}
}

Object Object::operator-() const { 
	if (is_number()) return Object(-((double)*this));
	Object m = getMetaHandler("__unm");
	if (m) {
		return m(*this);
	} else {
		throw std::runtime_error(
			std::string("attempt to perform arithmetic on a ")
			+ details->type() +
			std::string(" value"));
	}
}

Object Object::concat(const Object& o) const {
	return invokeStringMetaBinary(
		*this, o, "__concat",
		std::function<bool(const Object&)>([=](const Object& o)->bool{
			return o.is_number() || o.is_string();
		}),
		std::plus<std::string>());
}


Object Object::len() const {
	//string check
	std::shared_ptr<const Object_Details_String> sptr = std::dynamic_pointer_cast<const Object_Details_String>(details);
	if (sptr) {
		return Object(double(sptr->value.length()));
	}

	//table gets precedence over meta
	std::shared_ptr<const Object_Details_Table> tptr = std::dynamic_pointer_cast<const Object_Details_Table>(details);
	if (tptr) {
		double max = 0.0;
		for (const Map::value_type& pair : tptr->value) {
			if (pair.first.is_number()) {
				double v = (double)pair.first;
				if (v == floor(v)) {	//only ints allowed
					max = std::max(max, v);
				}
			}
		}
		return Object(max);
	}

	//meta:
	Object h = getMetaHandler("__len");
	if (h) {
		return h(Object(*this));
	} else {
		throw std::runtime_error(
			std::string("attempt to get length of a ")
			+ details->type() + 
			std::string(" value"));
	}
}

Object Object::getCompareHandler(Object op1, Object op2, const std::string& event) {
	if (op1.getTypeIndex() != op2.getTypeIndex()) return nil;
	Object mm1 = op1.getMetaHandler(event);
	Object mm2 = op2.getMetaHandler(event);
	if (mm1 == mm2) return mm1;
	return nil;
}


//extras
Object Object::operator~() const { return Object(~(Int)details->to_number()); }
Object& Object::operator++() { details = std::make_shared<Object_Details_Number>(details->to_number() + 1); return *this; }
Object Object::operator++(int) { Object copy(*this); details = std::make_shared<Object_Details_Number>(details->to_number() + 1); return copy; }
Object& Object::operator--() { details = std::make_shared<Object_Details_Number>(details->to_number() - 1); return *this; }
Object Object::operator--(int) { Object copy(*this); details = std::make_shared<Object_Details_Number>(details->to_number() - 1); return copy; }

Object type(Object o) {
	return o.type();
}

Object tostring(Object o) {
	return o.tostring();
}

Object getmetatable(Object x) {
	return Object(x.details->metatable);
}

Object setmetatable(Object x, Object m) {
	if (m.is_nil()) {
		x.details->metatable.reset();
	} else {
		std::shared_ptr<Object_Details_Table> tptr = std::dynamic_pointer_cast<Object_Details_Table>(m.details);
		if (!tptr) throw std::runtime_error("bad argument #2 to 'setmetatable' (nil or table expected)");
		x.details->metatable = tptr;
	}
	return x;
}

const Object nil;


Object_Details::~Object_Details() {}

std::string Object_Details::type() const { return "none"; }

double Object_Details::to_number() const { throw std::bad_cast(); }
std::string Object_Details::to_string() const { throw std::bad_cast(); }
Object::Map Object_Details::to_table() const { return Object::Map(); }
Object::Function Object_Details::to_function() const { return Object::Function(); }
bool Object_Details::to_boolean() const { return false; }

std::string Object_Details::explicit_to_string() const { return to_string(); }

bool Object_Details::compare(const Object& o) const { return false; }


std::string Object_Details_Number::type() const { return "number"; }

double Object_Details_Number::to_number() const { return value; }

std::string Object_Details_Number::to_string() const { 
	std::ostringstream ss;
	ss << value;
	return ss.str();
}

bool Object_Details_Number::to_boolean() const { return true; }

bool Object_Details_Number::compare(const Object& o) const {
	std::shared_ptr<const Object_Details_Number> optr = std::dynamic_pointer_cast<const Object_Details_Number>(o.details);
	if (!optr) return false;
	return value == optr->value;
}


std::string Object_Details_String::type() const { return "string"; }

double Object_Details_String::to_number() const { 
	std::istringstream ss(value);
	double d = 0;
	if (!(ss >> d)) throw std::bad_cast();
	return d;
}
std::string Object_Details_String::to_string() const { return value; }
bool Object_Details_String::to_boolean() const { return true; }

bool Object_Details_String::compare(const Object& o) const {
	std::shared_ptr<const Object_Details_String> optr = std::dynamic_pointer_cast<const Object_Details_String>(o.details);
	if (!optr) return false;
	return value == optr->value;
}

std::string Object_Details_Table::type() const { return "table"; }

Object::Map Object_Details_Table::to_table() const { return value; }
bool Object_Details_Table::to_boolean() const { return true; }

std::string Object_Details_Table::explicit_to_string() const {
	std::ostringstream ss;
	ss << "table: 0x" << std::hex << &value;
	return ss.str();
}

bool Object_Details_Table::compare(const Object& o) const {
	std::shared_ptr<const Object_Details_Table> optr = std::dynamic_pointer_cast<const Object_Details_Table>(o.details);
	if (!optr) return false;
	return &value == &optr->value;
}


std::string Object_Details_Boolean::type() const { return "boolean"; }

bool Object_Details_Boolean::to_boolean() const { return value; }

std::string Object_Details_Boolean::explicit_to_string() const { return value ? "true" : "false"; }

bool Object_Details_Boolean::compare(const Object& o) const {
	std::shared_ptr<const Object_Details_Boolean> optr = std::dynamic_pointer_cast<const Object_Details_Boolean>(o.details);
	if (!optr) return false;
	return value == optr->value;
}


std::string Object_Details_Function::type() const { return "function"; }

Object::Function Object_Details_Function::to_function() const { return value; }
bool Object_Details_Function::to_boolean() const { return true; }

std::string Object_Details_Function::explicit_to_string() const {
	std::ostringstream ss;
	ss << "function: 0x" << std::hex << &value;
	return ss.str();
}

bool Object_Details_Function::compare(const Object& o) const {
	std::shared_ptr<const Object_Details_Function> fptr = std::dynamic_pointer_cast<const Object_Details_Function>(o.details);
	if (!fptr) return false;
	return &value == &fptr->value;
}


std::string Object_Details_Nil::type() const { return "nil"; }

std::string Object_Details_Nil::explicit_to_string() const { return "nil"; }

bool Object_Details_Nil::compare(const Object& o) const {
	std::shared_ptr<const Object_Details_Nil> optr = std::dynamic_pointer_cast<const Object_Details_Nil>(o.details);
	if (!optr) return false;
	return true;
}

void InitializerListOptionMap<Object>::exec(
	VarArgType<ObjectType>& owner, const Type& x)
{
	owner.objects = std::vector<ObjectType>(x);
}
	
void InitializerListOptionMap<std::reference_wrapper<Object>>::exec(
	VarArgType<ObjectType>& owner, const Type& x) 
{
	throw std::runtime_error("not implemented");	//function not implemented.  shouldn't ever call this code
}

VarArgBufferSource<Object>::VarArgBufferSource(const VarArg& src_) : src(src_) {}
VarArgBufferSource<std::reference_wrapper<Object>>::VarArgBufferSource(const VarArgRef& src_) : src(src_) {}

Access::Access(Object* owner_, Object key_)
: owner(owner_), key(key_) {}

Object Access::get() const {
	Object h;
	std::shared_ptr<Object_Details_Table> tptr = std::dynamic_pointer_cast<Object_Details_Table>(owner->details);
	if (tptr) {
		Object::Map::const_iterator v = tptr->value.find(key);
		if (v != tptr->value.end()) return v->second;
		h = owner->getMetaHandler("__index");
		if (h.is_nil()) return nil;
	} else {
		h = owner->getMetaHandler("__index");
		if (h.is_nil()) throw std::runtime_error(
			std::string("attempt to index a ")
			+ owner->details->type() +
			std::string(" value"));
	}
	if (h.is_function()) {
		return h(owner, key);
	} else {
		return h[key];
	}
}
	
void Access::set(Object value) {
	Object h;
	std::shared_ptr<Object_Details_Table> tptr = std::dynamic_pointer_cast<Object_Details_Table>(owner->details);
	if (tptr) {
		Object::Map::iterator v = tptr->value.find(key);
		if (v != tptr->value.end()) {
			v->second = value;
			return;
		}
		h = owner->getMetaHandler("__newindex");
		if (!h) {
			tptr->value[key] = value;
			return;
		}
	} else {
		h = owner->getMetaHandler("__newindex");
		if (!h) throw std::runtime_error(
			std::string("attempt to index a ")
			+ owner->details->type() +
			std::string(" value"));
	}
	if (h.is_function()) {
		h(owner, key, value);
	} else {
		h[key] = value;
	}
}


Access& Access::operator=(Object value) { set(value); return *this; }
Access::operator Object() const { return get(); }


static double pi = 4 * std::atan(1);

Math::Math() : Object({
	{"abs", ::fabs},
	{"acos", ::acos},
	{"asin", ::asin},
	{"atan", ::atan},
	{"atan2", ::atan2},
	{"ceil", ::ceil},
	{"cos", ::cos},
	{"cosh", ::cosh},
	{"deg", function(x) { return x*180/::CxxAsLua::pi; }},
	{"exp", ::exp},
	{"floor", ::floor},
	{"fmod", Object::lmod},
	{"frexp", function(x) {
		int exp = std::numeric_limits<int>::lowest();
		double a = ::frexp(x, &exp);
		return VarArg(a, exp);
	}},
	{"huge", ::INFINITY},
	{"ldexp", ::ldexp},
	{"log", ::log},
	{"log10", ::log10},
	{"max", function(a,b) { return a>b?a:b; }},
	{"min", function(a,b) { return a<b?a:b; }},
	{"mod", Object::lmod},	//alias for fmod
	{"modf", function(x) {
		double intpart = std::numeric_limits<double>::quiet_NaN();
		double fracpart = ::modf(x, &intpart);
		return VarArg(intpart, fracpart);
	}},
	{"pi", ::CxxAsLua::pi},
	{"pow", ::pow},
	{"rad", function(x) { return x*::CxxAsLua::pi/180; }},
	{"random", [=](VarArg args)->VarArg {
		if (args.len() == 0) {
			return (double)::rand()/(double)RAND_MAX;
		} else if (args.len() == 1) {
			int max = args[1];
			if (max < 1) throw std::runtime_error("bad argument #1 to 'random' (interval is empty)");
			return (::rand() % max) + 1;
		} else if (args.len() == 2) {
			int min = args[1];
			int max = args[2];
			int span = max - min + 1;
			if (span <= 0) throw std::runtime_error("bad argument #2 to 'random' (interval is empty)");
			return (::rand() % span) + min;
		}
		throw std::runtime_error("wrong number of arguments");
	}},
	{"randomseed", ::srand}, 
	{"sin", ::sin},
	{"sinh", ::sinh},
	{"sqrt", ::sqrt},
	{"tan", ::tan},
	{"tanh", ::tanh}
})
, abs(this, "abs")
, acos(this, "acos")
, asin(this, "asin")
, atan(this, "atan")
, atan2(this, "atan2")
, ceil(this, "ceil")
, cos(this, "cos")
, cosh(this, "cosh")
, deg(this, "deg")
, exp(this, "exp")
, floor(this, "floor")
, fmod(this, "fmod,")
, frexp(this, "frexp")
, huge(this, "huge")
, ldexp(this, "ldexp")
, log(this, "log")
, log10(this, "log10")
, max(this, "max")
, min(this, "min")
, mod(this, "mod")
, modf(this, "modf")
, pi(this, "pi")
, pow(this, "pow")
, rad(this, "rad,")
, random(this, "random")
, randomseed(this, "randomseed")
, sin(this, "sin")
, sinh(this, "sinh")
, sqrt(this, "sqrt")
, tan(this, "tan")
, tanh(this, "tanh")
{}

Math math;

IO::IO() : Object({
	{"write", [=](VarArg args)->VarArg{
		for (const Object& o : args.objects) {
			std::cout << o.tostring();
		}
		return nil;
	}}
})
, write(this, "write")
{}

IO io;

}
