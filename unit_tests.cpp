#include "iFileIO.hpp"
#include <iostream>
#include <iomanip>

#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>

#include <ios>
#include <ostream>
#include <istream>
#include <fstream>
#include <sstream>

namespace details {
	template<template<class...>class Z, class, class...>
	struct can_apply:std::false_type{};
	template<template<class...>class Z, class...Ts>
	struct can_apply<Z, std::void_t<Z<Ts...>>, Ts...>:
	std::true_type{};
}

template<template<class...>class Z, class...Ts>
using can_apply=details::can_apply<Z, void, Ts...>;

template<class F, class...Ts>
using write_r = decltype(std::declval<F>().write( std::declval<Ts>()... ));

template<class F, class...Ts>
using read_r = decltype(std::declval<F>().read( std::declval<Ts>()... ));

template<class F, class...Ts>
using can_write = can_apply<write_r, F, Ts...>;

template<class F, class...Ts>
using can_read = can_apply<read_r, F, Ts...>;

struct test_struct {
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	void writetest(iFileIO& file){
		file.write(*this);
	}
	test_struct() {}
	test_struct(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}
	void print(){
		std::cout << a << " " << b << " " << c << " " << d << " " << std::endl;
	}
};

// here for SFINAE
class test_class {};

class test_iIOable : public iIOable {
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;

	std::unique_ptr<const char[]> toBytes() const override{
		auto bytebuffer = std::make_unique<char[]>(sizeof(int) * 4);
		((int*)(bytebuffer.get()))[0] = a;
		((int*)(bytebuffer.get()))[1] = b;
		((int*)(bytebuffer.get()))[2] = c;
		((int*)(bytebuffer.get()))[3] = d;
		return bytebuffer;
	}

	void toObject(const std::unique_ptr<char[]> data) override {
		a = ((int*)(data.get()))[0];
		b = ((int*)(data.get()))[1];
		c = ((int*)(data.get()))[2];
		d = ((int*)(data.get()))[3];
	}

	size_t ObjectByteSize() const override {
		return sizeof(int) * 4;
	}
public:
	test_iIOable() {}
	test_iIOable(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}

	void print(){
		std::cout << a << " " << b << " " << c << " " << d << " " << std::endl;
	}
};

iFileIO file("test.txt");

#define w(_w) std::setw(_w)

void SFINEA_test(){
	std::cout << std::left << "Rvalue SFINAE: " << std::endl;
	std::cout << w(10) << "type: " << w(5) << "int" << w(8) << "double" << w(8) << "struct" << w(7)  << "class" << w(9) << "iIOable" << w(9) << "pointer" << w(7) << "array" << w(11) << "container" << w(10) << "iterator" << w(8) << "stream" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(5) << can_write<iGIO&, int&&>::value;
	std::cout << w(8) << can_write<iGIO&, double&&>::value;
	std::cout << w(8) << can_write<iGIO&, test_struct&&>::value;
	std::cout << w(7) << can_write<iGIO&, test_class&&>::value;
	std::cout << w(9) << can_write<iGIO&, test_iIOable&&>::value;
	std::cout << w(9) << can_write<iGIO&, int*&&>::value;
	std::cout << w(7) << 0; // array of reference is illegal
	std::cout << w(11) << can_write<iGIO&, std::vector<int>&&>::value;
	std::cout << w(10) << can_write<iGIO&, std::vector<int>::iterator&&>::value;
	std::cout << w(8) << can_write<iGIO&, std::ostream&&>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(5) << can_read<iGIO&, int&&>::value;
	std::cout << w(8) << can_read<iGIO&, double&&>::value;
	std::cout << w(8) << can_read<iGIO&, test_struct&&>::value;
	std::cout << w(7) << can_read<iGIO&, test_class&&>::value;
	std::cout << w(9) << can_read<iGIO&, test_iIOable&&>::value;
	std::cout << w(9) << can_read<iGIO&, int*&&>::value;
	std::cout << w(7) << 0; // array of reference is illegal
	std::cout << w(11) << can_read<iGIO&, std::vector<int>&&>::value;
	std::cout << w(10) << can_read<iGIO&, std::vector<int>::iterator&&>::value;
	std::cout << w(8) << can_read<iGIO&, std::ostream&&>::value;
	std::cout << std::endl;

	std::cout << "Lvalue SFINAE: " << std::endl;
	std::cout << w(10) << "type: " << w(5) << "int" << w(8) << "double" << w(8) << "struct" << w(7)  << "class" << w(9) << "iIOable" << w(9) << "pointer" << w(7) << "array" << w(11) << "container" << w(10) << "iterator" << w(8) << "stream" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(5) << can_write<iGIO&, int&>::value;
	std::cout << w(8) << can_write<iGIO&, double&>::value;
	std::cout << w(8) << can_write<iGIO&, test_struct&>::value;
	std::cout << w(7) << can_write<iGIO&, test_class&>::value;
	std::cout << w(9) << can_write<iGIO&, test_iIOable&>::value;
	std::cout << w(9) << can_write<iGIO&, int*&>::value;
	std::cout << w(7) << 0; // array of reference is illegal
	std::cout << w(11) << can_write<iGIO&, std::vector<int>&>::value; // supported through container overloads
	std::cout << w(10) << can_write<iGIO&, std::vector<int>::iterator&>::value;
	std::cout << w(8) << can_write<iGIO&, std::iostream&>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(5) << can_read<iGIO&, int&>::value;
	std::cout << w(8) << can_read<iGIO&, double&>::value;
	std::cout << w(8) << can_read<iGIO&, test_struct&>::value;
	std::cout << w(7) << can_read<iGIO&, test_class&>::value;
	std::cout << w(9) << can_read<iGIO&, test_iIOable&>::value;
	std::cout << w(9) << can_read<iGIO&, int*&>::value;
	std::cout << w(7) << 0; // array of reference is illegal
	std::cout << w(11) << can_read<iGIO&, std::vector<int>&>::value; // supported through container overloads
	std::cout << w(10) << can_read<iGIO&, std::vector<int>::iterator&>::value;
	std::cout << w(8) << can_read<iGIO&, std::iostream&>::value;
	std::cout << std::endl;

	std::cout << "Pointer SFINAE: " << std::endl;
	std::cout << w(10) << "type: " << w(5) << "int" << w(8) << "double" << w(8) << "struct" << w(7)  << "class" << w(9) << "iIOable" << w(9) << "pointer" << w(7) << "array" << w(11) << "container" << w(10) << "iterator" << w(8) << "stream" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(5) << can_write<iGIO&, int*, std::size_t>::value;
	std::cout << w(8) << can_write<iGIO&, double*, std::size_t>::value;
	std::cout << w(8) << can_write<iGIO&, test_struct*, std::size_t>::value;
	std::cout << w(7) << can_write<iGIO&, test_class*, std::size_t>::value;
	std::cout << w(9) << can_write<iGIO&, test_iIOable*, std::size_t>::value;
	std::cout << w(9) << can_write<iGIO&, int**, std::size_t>::value;
	std::cout << w(7) << 0; // int[4]* gets casted to int**
	std::cout << w(11) << can_write<iGIO&, std::vector<int>*>::value;
	std::cout << w(10) << can_write<iGIO&, std::vector<int>::iterator*>::value;
	std::cout << w(8) << can_write<iGIO&, std::ostream*>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(5) << can_read<iGIO&, int*, std::size_t>::value;
	std::cout << w(8) << can_read<iGIO&, double*, std::size_t>::value;
	std::cout << w(8) << can_read<iGIO&, test_struct*, std::size_t>::value;
	std::cout << w(7) << can_read<iGIO&, test_class*, std::size_t>::value;
	std::cout << w(9) << can_read<iGIO&, test_iIOable*, std::size_t>::value;
	std::cout << w(9) << can_read<iGIO&, int**, std::size_t>::value;
	std::cout << w(7) << 0; // int[4]* gets casted to int**
	std::cout << w(11) << can_read<iGIO&, std::vector<int>*>::value;
	std::cout << w(10) << can_read<iGIO&, std::vector<int>::iterator*>::value;
	std::cout << w(8) << can_read<iGIO&, std::ostream*>::value;
	std::cout << std::endl;

	std::cout << "Array SFINAE: " << std::endl;
	std::cout << w(10) << "type: " << w(5) << "int" << w(8) << "double" << w(8) << "struct" << w(7)  << "class" << w(9) << "iIOable" << w(9) << "pointer" << w(7) << "array" << w(11) << "container" << w(10) << "iterator" << w(8) << "stream" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(5) << can_write<iGIO&, int[5]>::value;
	std::cout << w(8) << can_write<iGIO&, double[5]>::value;
	std::cout << w(8) << can_write<iGIO&, test_struct[5]>::value;
	std::cout << w(7) << can_write<iGIO&, test_class[5]>::value;
	std::cout << w(9) << can_write<iGIO&, test_iIOable[5]>::value;
	std::cout << w(9) << can_write<iGIO&, int[5][5]>::value;
	std::cout << w(7) << can_write<iGIO&, int*[5]>::value;
	std::cout << w(11) << can_write<iGIO&, std::vector<int>*>::value;
	std::cout << w(10) << can_write<iGIO&, std::vector<int>::iterator*>::value;
	std::cout << w(8) << can_write<iGIO&, std::ostream*>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(5) << can_read<iGIO&, int[5]>::value;
	std::cout << w(8) << can_read<iGIO&, double[5]>::value;
	std::cout << w(8) << can_read<iGIO&, test_struct[5]>::value;
	std::cout << w(7) << can_read<iGIO&, test_class[5]>::value;
	std::cout << w(9) << can_read<iGIO&, test_iIOable[5]>::value;
	std::cout << w(9) << can_read<iGIO&, int[5][5]>::value;
	std::cout << w(7) << can_read<iGIO&, int*[5]>::value;
	std::cout << w(11) << can_read<iGIO&, std::vector<int>*>::value;
	std::cout << w(10) << can_read<iGIO&, std::vector<int>::iterator*>::value;
	std::cout << w(8) << can_read<iGIO&, std::ostream*>::value;
	std::cout << std::endl;

	std::cout << "Container range SFINAE: " << std::endl;
	std::cout << w(10) << "type: " << w(7) << "array" << w(8) << "vector" << w(7) << "deque" << w(14)  << "forward_list" << w(6) << "list" << w(5) << "set" << w(5) << "map" << w(10) << "multiset" << w(10) << "multimap" << w(14) << "unordered_set" << w(15) << "unordered_map" << w(20) << "unordered_multiset" << w(15) << "unordered_map" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(7) << can_write<iGIO&, std::array<int, 5>::iterator, std::array<int, 5>::iterator>::value;
	std::cout << w(8) << can_write<iGIO&, std::vector<int>::iterator, std::vector<int>::iterator>::value;
	std::cout << w(7) << can_write<iGIO&, std::deque<int>::iterator, std::deque<int>::iterator>::value;
	std::cout << w(14) << can_write<iGIO&, std::forward_list<int>::iterator, std::forward_list<int>::iterator>::value;
	std::cout << w(6) << can_write<iGIO&, std::list<int>::iterator, std::list<int>::iterator>::value;
	std::cout << w(5) << can_write<iGIO&, std::set<int>::iterator, std::set<int>::iterator>::value;
	std::cout << w(5) << can_write<iGIO&, std::map<int, int>::iterator, std::map<int, int>::iterator>::value;
	std::cout << w(10) << can_write<iGIO&, std::multiset<int>::iterator, std::multiset<int>::iterator>::value;
	std::cout << w(10) << can_write<iGIO&, std::multimap<int, int>::iterator, std::multimap<int, int>::iterator>::value;
	std::cout << w(14) << can_write<iGIO&, std::unordered_set<int>::iterator, std::unordered_set<int>::iterator>::value;
	std::cout << w(15) << can_write<iGIO&, std::unordered_map<int, int>::iterator, std::unordered_map<int, int>::iterator>::value;
	std::cout << w(20) << can_write<iGIO&, std::unordered_multiset<int>::iterator, std::unordered_multiset<int>::iterator>::value;
	std::cout << w(15) << can_write<iGIO&, std::unordered_multimap<int, int>::iterator, std::unordered_multimap<int, int>::iterator>::value;
	// // std::cout << w(10) << can_write<iGIO&, std::stack<int>::iterator, std::stack<int>::iterator>::value;
	// // std::cout << w(10) << can_write<iGIO&, std::queue<int>::iterator, std::queue<int>::iterator>::value;
	// // std::cout << w(10) << can_write<iGIO&, std::priority_queue<int>::iterator, std::priority_queue<int>::iterator>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(7) << can_read<iGIO&, std::array<int, 5>::iterator, std::array<int, 5>::iterator>::value;
	std::cout << w(8) << can_read<iGIO&, std::vector<int>::iterator, std::vector<int>::iterator>::value;
	std::cout << w(7) << can_read<iGIO&, std::deque<int>::iterator, std::deque<int>::iterator>::value;
	std::cout << w(14) << can_read<iGIO&, std::forward_list<int>::iterator, std::forward_list<int>::iterator>::value;
	std::cout << w(6) << can_read<iGIO&, std::list<int>::iterator, std::list<int>::iterator>::value;
	std::cout << w(5) << can_read<iGIO&, std::set<int>::iterator, std::set<int>::iterator>::value;
	std::cout << w(5) << can_read<iGIO&, std::map<int, int>::iterator, std::map<int, int>::iterator>::value;
	std::cout << w(10) << can_read<iGIO&, std::multiset<int>::iterator, std::multiset<int>::iterator>::value;
	std::cout << w(10) << can_read<iGIO&, std::multimap<int, int>::iterator, std::multimap<int, int>::iterator>::value;
	std::cout << w(14) << can_read<iGIO&, std::unordered_set<int>::iterator, std::unordered_set<int>::iterator>::value;
	std::cout << w(15) << can_read<iGIO&, std::unordered_map<int, int>::iterator, std::unordered_map<int, int>::iterator>::value;
	std::cout << w(20) << can_read<iGIO&, std::unordered_multiset<int>::iterator, std::unordered_multiset<int>::iterator>::value;
	std::cout << w(15) << can_read<iGIO&, std::unordered_multimap<int, int>::iterator, std::unordered_multimap<int, int>::iterator>::value;
	// std::cout << w(10) << can_write<iGIO&, std::stack<int>::iterator, std::stack<int>::iterator>::value;
	// std::cout << w(10) << can_write<iGIO&, std::queue<int>::iterator, std::queue<int>::iterator>::value;
	// std::cout << w(10) << can_write<iGIO&, std::priority_queue<int>::iterator, std::priority_queue<int>::iterator>::value;
	std::cout << std::endl;

	std::cout << "Container reference SFINAE: " << std::endl;
	std::cout << w(10) << "type: " << w(7) << "array" << w(8) << "vector" << w(7) << "deque" << w(14)  << "forward_list" << w(6) << "list" << w(5) << "set" << w(5) << "map" << w(10) << "multiset" << w(10) << "multimap" << w(14) << "unordered_set" << w(15) << "unordered_map" << w(20) << "unordered_multiset" << w(15) << "unordered_map" << w(7) << "stack" << w(7) << "queue" << w(16) << "priority_queue" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(7) << can_write<iGIO&, std::array<int, 5>&>::value;
	std::cout << w(8) << can_write<iGIO&, std::vector<int>&>::value;
	std::cout << w(7) << can_write<iGIO&, std::deque<int>&>::value;
	std::cout << w(14) << can_write<iGIO&, std::forward_list<int>&>::value;
	std::cout << w(6) << can_write<iGIO&, std::list<int>&>::value;
	std::cout << w(5) << can_write<iGIO&, std::set<int>&>::value;
	std::cout << w(5) << can_write<iGIO&, std::map<int, int>&>::value;
	std::cout << w(10) << can_write<iGIO&, std::multiset<int>&>::value;
	std::cout << w(10) << can_write<iGIO&, std::multimap<int, int>&>::value;
	std::cout << w(14) << can_write<iGIO&, std::unordered_set<int>&>::value;
	std::cout << w(15) << can_write<iGIO&, std::unordered_map<int, int>&>::value;
	std::cout << w(20) << can_write<iGIO&, std::unordered_multiset<int>&>::value;
	std::cout << w(15) << can_write<iGIO&, std::unordered_multimap<int, int>&>::value;
	std::cout << w(7) << can_write<iGIO&, std::stack<int>&>::value;
	std::cout << w(7) << can_write<iGIO&, std::queue<int>&>::value;
	std::cout << w(16) << can_write<iGIO&, std::priority_queue<int>&>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(7) << can_read<iGIO&, std::array<int, 5>&>::value;
	std::cout << w(8) << can_read<iGIO&, std::vector<int>&>::value;
	std::cout << w(7) << can_read<iGIO&, std::deque<int>&>::value;
	std::cout << w(14) << can_read<iGIO&, std::forward_list<int>&>::value;
	std::cout << w(6) << can_read<iGIO&, std::list<int>&>::value;
	std::cout << w(5) << can_read<iGIO&, std::set<int>&>::value;
	std::cout << w(5) << can_read<iGIO&, std::map<int, int>&>::value;
	std::cout << w(10) << can_read<iGIO&, std::multiset<int>&>::value;
	std::cout << w(10) << can_read<iGIO&, std::multimap<int, int>&>::value;
	std::cout << w(14) << can_read<iGIO&, std::unordered_set<int>&>::value;
	std::cout << w(15) << can_read<iGIO&, std::unordered_map<int, int>&>::value;
	std::cout << w(20) << can_read<iGIO&, std::unordered_multiset<int>&>::value;
	std::cout << w(15) << can_read<iGIO&, std::unordered_multimap<int, int>&>::value;
	std::cout << w(7) << can_read<iGIO&, std::stack<int>&>::value;
	std::cout << w(7) << can_read<iGIO&, std::queue<int>&>::value;
	std::cout << w(16) << can_read<iGIO&, std::priority_queue<int>&>::value;
	std::cout << std::endl;

	std::cout << "String SFINAE" << std::endl;
	std::cout << w(10) << "type: " << w(13) << "std::string" << w(14) << "std::wstring" << w(16) << "std::u16string" << w(16) << "std::u32string" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(13) << can_write<iGIO&, std::string>::value;
	std::cout << w(14) << can_write<iGIO&, std::wstring>::value;
	std::cout << w(16) << can_write<iGIO&, std::u16string>::value;
	std::cout << w(16) << can_write<iGIO&, std::u32string>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(13) << can_read<iGIO&, std::string&>::value;
	std::cout << w(14) << can_read<iGIO&, std::wstring&>::value;
	std::cout << w(16) << can_read<iGIO&, std::u16string&>::value;
	std::cout << w(16) << can_read<iGIO&, std::u32string&>::value;
	std::cout << std::endl;

	std::cout << "Stream SFINAE" << std::endl;
	std::cout << w(10) << "type: " << w(14) << "std::ostream" << w(20) << "std::ostringstream" << w(15) << "std::ofstream" << w(15) << "std::iostream" << w(19) << "std::stringstream" << w(14) << "std::fstream" << w(14) << "std::istream" << w(20) << "std::istringstream" << w(15) << "std::ifstream" << std::endl;
	std::cout << w(10) << "write: ";
	std::cout << w(14) << can_write<iGIO&, std::basic_ostream<char>&>::value;
	std::cout << w(20) << can_write<iGIO&, std::basic_ostringstream<char>&>::value;
	std::cout << w(15) << can_write<iGIO&, std::basic_ofstream<char>&>::value;
	std::cout << w(15) << can_write<iGIO&, std::basic_iostream<char>&>::value;
	std::cout << w(19) << can_write<iGIO&, std::basic_stringstream<char>&>::value;
	std::cout << w(14) << can_write<iGIO&, std::basic_fstream<char>&>::value;
	std::cout << w(14) << can_write<iGIO&, std::basic_istream<char>&>::value;
	std::cout << w(20) << can_write<iGIO&, std::basic_istringstream<char>&>::value;
	std::cout << w(15) << can_write<iGIO&, std::basic_ifstream<char>&>::value;
	std::cout << std::endl;
	std::cout << w(10) << "read: ";
	std::cout << w(14) << can_read<iGIO&, std::basic_ostream<char>&>::value;
	std::cout << w(20) << can_read<iGIO&, std::basic_ostringstream<char>&>::value;
	std::cout << w(15) << can_read<iGIO&, std::basic_ofstream<char>&>::value;
	std::cout << w(15) << can_read<iGIO&, std::basic_iostream<char>&>::value;
	std::cout << w(19) << can_read<iGIO&, std::basic_stringstream<char>&>::value;
	std::cout << w(14) << can_read<iGIO&, std::basic_fstream<char>&>::value;
	std::cout << w(14) << can_read<iGIO&, std::basic_istream<char>&>::value;
	std::cout << w(20) << can_read<iGIO&, std::basic_istringstream<char>&>::value;
	std::cout << w(15) << can_read<iGIO&, std::basic_ifstream<char>&>::value;
	std::cout << std::endl;

}

void RValue_test(){
	std::cout << "rvalue int: ";
	int ivalue = 0;
	file.write(10); // implicit cast to int
	file.read(ivalue);
	std::cout << ivalue << std::endl;
	file.cleanFile();

	std::cout << "rvalue double: ";
	double dvalue = 0;
	file.write(10.5); // implicit cast to double
	file.read(dvalue);
	std::cout << dvalue << std::endl;
	file.cleanFile();

	std::cout << "rvalue test_struct: ";
	test_struct _test;
	file.write(test_struct{25, 50, 75, 100});
	file.read(_test);
	_test.print();
	file.cleanFile();

	std::cout << "rvalue ctest: ";
	test_iIOable _ctest;
	file.write(test_iIOable(25, 50, 75, 100));
	file.read(_ctest);
	_ctest.print();
	file.cleanFile();
}

void LValue_test(){
	std::cout << "lvalue int: ";
	int ivalue = 10;
	int ivalue_ret = 0;
	file.write(ivalue);
	file.read(ivalue_ret);
	std::cout << ivalue_ret << std::endl;
	file.cleanFile();

	std::cout << "lvalue double: ";
	double dvalue = 10.5;
	double dvalue_ret = 0;
	file.write(dvalue);
	file.read(dvalue_ret);
	std::cout << dvalue_ret << std::endl;
	file.cleanFile();

	std::cout << "lvalue test_struct: ";
	test_struct _test = test_struct{25, 50, 75, 100};
	test_struct _test_ret;
	file.write(_test);
	file.read(_test_ret);
	_test_ret.print();
	file.cleanFile();

	std::cout << "lvalue ctest: ";
	test_iIOable _ctest = test_iIOable(25, 50, 75, 100);
	test_iIOable _ctest_ret;
	file.write(_ctest);
	file.read(_ctest_ret);
	_ctest_ret.print();
	file.cleanFile();
}

void Pointer_test(){
	std::cout << "lvalue int: ";
	int ivalue = 10;
	int ivalue_ret = 0;
	file.write(&ivalue, 1);
	file.read(&ivalue_ret, 1);
	std::cout << ivalue_ret << std::endl;
	file.cleanFile();

	std::cout << "lvalue double: ";
	double dvalue = 10.5;
	double dvalue_ret = 0;
	file.write(&dvalue, 1);
	file.read(&dvalue_ret, 1);
	std::cout << dvalue_ret << std::endl;
	file.cleanFile();

	std::cout << "lvalue test_struct: ";
	test_struct _test = test_struct{25, 50, 75, 100};
	test_struct _test_ret;
	file.write(&_test, 1);
	file.read(&_test_ret, 1);
	_test_ret.print();
	file.cleanFile();

	std::cout << "lvalue test_iIOable: ";
	test_iIOable _ctest = test_iIOable(25, 50, 75, 100);
	test_iIOable _ctest_ret;
	file.write(_ctest);
	file.read(_ctest_ret);
	_ctest_ret.print();
	file.cleanFile();
}

int main(){

	SFINEA_test();


	return 0;
}