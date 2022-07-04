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

#include <codecvt>
#include <locale>

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
	bool operator==(test_struct& other){
		return (a == other.a && b == other.b && c == other.c && d == other.d);
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

	bool operator==(test_iIOable& other){
		return (a == other.a && b == other.b && c == other.c && d == other.d);
	}
};

iFileIO file("test.txt");

#define w(_w) std::setw(_w)

template<typename T>
void print_arr(T* start, std::size_t length){
	for(std::size_t i = 0; i < length; i++)
		std::cout << start[i] << ", ";
	std::cout << std::endl;
}

void print_arr(test_struct* start, std::size_t length){
	for(std::size_t i = 0; i < length; i++)
		start[i].print();
}

void print_arr(test_iIOable* start, std::size_t length){
	for(std::size_t i = 0; i < length; i++)
		start[i].print();
}

template<typename T>
void print_container(T start, T end){
	for(; start != end; ++start)
		std::cout << *start << ", ";
	std::cout << std::endl;
}

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
	std::cout << "\n[rvalue test]" << std::endl;
	{
	int ret_test = 0;
	file.write(10); // implicit cast to int
	file.read(ret_test);
	std::string equal = 10 == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "rvalue int: ";
	std::cout << ret_test << std::endl;
	file.cleanFile();
	}
	{
	double ret_test = 0;
	file.write(10.5); // implicit cast to double
	file.read(ret_test);
	std::string equal = 10.5 == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "rvalue double: ";
	std::cout << ret_test << std::endl;
	file.cleanFile();
	}
	{
	test_struct ret_test;
	file.write(test_struct{25, 50, 75, 100});
	file.read(ret_test);
	std::string equal = test_struct{25, 50, 75, 100} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "rvalue test_struct: ";
	ret_test.print();
	file.cleanFile();
	}
	{
	test_iIOable ret_test;
	file.write(test_iIOable(25, 50, 75, 100));
	file.read(ret_test);
	std::string equal = test_iIOable(25, 50, 75, 100) == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "rvalue test_iIOable: ";
	ret_test.print();
	file.cleanFile();
	}
}

void LValue_test(){
	std::cout << "\n[lvalue test]" << std::endl;
	{
	int test = 10;
	int ret_test = 0;
	file.write(test);
	file.read(ret_test);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "lvalue int: ";
	std::cout << ret_test << std::endl;
	file.cleanFile();
	}
	{
	double test = 10.5;
	double ret_test = 0;
	file.write(test);
	file.read(ret_test);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "lvalue double: ";
	std::cout << ret_test << std::endl;
	file.cleanFile();
	}
	{
	test_struct test = test_struct{25, 50, 75, 100};
	test_struct ret_test;
	file.write(test);
	file.read(ret_test);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "lvalue test_struct: ";
	ret_test.print();
	file.cleanFile();
	}
	{
	test_iIOable test = test_iIOable(25, 50, 75, 100);
	test_iIOable ret_test;
	file.write(test);
	file.read(ret_test);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "lvalue test_iIOable: ";
	ret_test.print();
	file.cleanFile();
	}
}

void Pointer_test(){
	std::cout << "\n[Pointer test]" << std::endl;
	{
	int test = 10;
	int ret_test = 0;
	file.write(&test, 1);
	file.read(&ret_test, 1);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer int: ";
	std::cout << ret_test << std::endl;
	file.cleanFile();
	}
	{
	double test = 10.5;
	double ret_test = 0;
	file.write(&test, 1);
	file.read(&ret_test, 1);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer double: ";
	std::cout << ret_test << std::endl;
	file.cleanFile();
	}
	{
	test_struct test = test_struct{25, 50, 75, 100};
	test_struct ret_test;
	file.write(&test, 1);
	file.read(&ret_test, 1);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer test_struct: ";
	ret_test.print();
	file.cleanFile();
	}
	{
	test_iIOable test = test_iIOable(25, 50, 75, 100);
	test_iIOable ret_test;
	file.write(&test, 1);
	file.read(&ret_test, 1);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer test_iIOable: ";
	ret_test.print();
	file.cleanFile();
	}
}

void Pointer_arr_test(){
	std::cout << "\n[Ptr Array test]" << std::endl;
	{
	int test[4] = {10, 11, 12, 13};
	int ret_test[4];
	file.write(test, 4);
	file.read(ret_test, 4);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array int: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	double test[4] = {10.5, 11.5, 12.5, 13.5};
	double ret_test[4];
	file.write(test, 4);
	file.read(ret_test, 4);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array double: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_struct test[4] = {test_struct{25, 50, 75, 100}, test_struct{125, 150, 175, 200}, test_struct{225, 250, 275, 300}, test_struct{325, 350, 375, 400}};
	test_struct ret_test[4];
	file.write(test, 4);
	file.read(ret_test, 4);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array test_struct: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_iIOable test[4] = {test_iIOable(25, 50, 75, 100), test_iIOable(125, 150, 175, 200), test_iIOable(225, 250, 275, 300), test_iIOable(325, 350, 375, 400)};
	test_iIOable ret_test[4];
	file.write(test, 4);
	file.read(ret_test, 4);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array test_iIOable: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
}

void Array_test(){
	std::cout << "\n[Array test]" << std::endl;
	{
	int test[4] = {10, 11, 12, 13};
	int ret_test[4];
	file.write(test);
	file.read(ret_test);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array int: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	double test[4] = {10.5, 11.5, 12.5, 13.5};
	double ret_test[4];
	file.write(test);
	file.read(ret_test);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array double: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_struct test[4] = {test_struct{25, 50, 75, 100}, test_struct{125, 150, 175, 200}, test_struct{225, 250, 275, 300}, test_struct{325, 350, 375, 400}};
	test_struct ret_test[4];
	file.write(test);
	file.read(ret_test);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array test_struct: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_iIOable test[4] = {test_iIOable(25, 50, 75, 100), test_iIOable(125, 150, 175, 200), test_iIOable(225, 250, 275, 300), test_iIOable(325, 350, 375, 400)};
	test_iIOable ret_test[4];
	file.write(test);
	file.read(ret_test);
	std::string equal = std::equal(std::begin(test), std::end(test), std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array test_iIOable: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
}

void Range_test(){
	std::cout << "\n[Range test]" << std::endl;
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test(4, 0);
	
	file.write(test.begin(), test.end());
	file.read(ret_test.begin(), ret_test.end());
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::forward_list<int> test = {10, 11, 12, 13};
	std::forward_list<int> ret_test(4, 0);
	
	file.write(test.begin(), test.end());
	file.read(ret_test.begin(), ret_test.end());
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "forward_list<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test(2, 0);
	
	file.write(test.begin(), test.end());
	file.read(ret_test.begin(), ret_test.end());
	std::string equal = std::vector{10, 11} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>(2): ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test(5, 0);
	
	file.write(test.begin(), test.end());
	file.read(ret_test.begin(), ret_test.end());
	std::string equal = std::vector{10, 11, 12, 13, 0} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>(5): ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
}

void Container_test(){
	std::cout << "\n[Container test]" << std::endl;
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test;
	
	file.write(test);
	file.read(ret_test);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::forward_list<int> test = {10, 11, 12, 13};
	std::forward_list<int> ret_test;
	
	file.write(test);
	file.read(ret_test);
	std::string equal = std::forward_list{13, 12, 11, 10} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "forward_list<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
}

void String_test(){
	std::cout << "\n[String test]" << std::endl;
	{
	std::string test = "string!";
	std::string ret_test;
	
	file.write(test);
	file.read(ret_test);
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "string: " << ret_test << std::endl;
	file.cleanFile();
	}
	{
	std::wstring test = L"wstring!";
	std::wstring ret_test;
	
	file.write(test);
	file.read(ret_test);
	std::wstring equal = test == ret_test ? L"[success] : " : L"[failure] : ";
	std::wcout << equal << L"wstring: " << ret_test << std::endl;
	file.cleanFile();
	}
	{
	std::u16string test = u"u16string!";
	std::u16string ret_test;
	
	file.write(test);
	file.read(ret_test);
	std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cv;
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << cv.to_bytes(u"u16string: ") << cv.to_bytes(ret_test) << std::endl;
	file.cleanFile();
	}
	{
	std::u32string test = U"u32string!";
	std::u32string ret_test;
	
	file.write(test);
	file.read(ret_test);
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	std::string equal = test == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << cv.to_bytes(U"u32string: ") << cv.to_bytes(ret_test) << std::endl;
	file.cleanFile();
	}
}

void Stream_test(){
	std::cout << "\n[Stream test]" << std::endl;
	{
		std::stringstream out;
		std::stringstream in;
		out << "hello world!";

		file.write(out, " ");
		std::cout << "start read!" << std::endl;
		file.read(in);
		// std::cout << out.str() << std::endl;
		std::string equal = out.str() == in.str() ? "[success] : " : "[failure] : ";
		std::cout << equal << "stringstream: " << in.str() << std::endl;
		file.cleanFile();
	}
}

void Until_Pointer_arr_test(){
	std::cout << "\n[Ptr Array until test]" << std::endl;
	{
	int test[4] = {10, 11, 12, 13};
	int ret_test[4] = {0, 0, 0, 0};
	file.write(test, 4);
	file.read_until(ret_test, 12, 4);
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array int: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	double test[4] = {10.5, 11.5, 12.5, 13.5};
	double ret_test[4] = {0, 0, 0, 0};
	file.write(test, 4);
	file.read_until(ret_test, 12.5, 4);
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array double: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_struct test[4] = {test_struct{25, 50, 75, 100}, test_struct{125, 150, 175, 200}, test_struct{225, 250, 275, 300}, test_struct{325, 350, 375, 400}};
	test_struct ret_test[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
	file.write(test, 4);
	file.read_until(ret_test, test_struct{225, 250, 275, 300}, 4);
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array test_struct: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_iIOable test[4] = {test_iIOable(25, 50, 75, 100), test_iIOable(125, 150, 175, 200), test_iIOable(225, 250, 275, 300), test_iIOable(325, 350, 375, 400)};
	test_iIOable ret_test[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
	file.write(test, 4);
	file.read_until(ret_test, test_iIOable(225, 250, 275, 300), 4);
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "pointer array test_iIOable: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
}

void Until_Array_test(){
	std::cout << "\n[Array until test]" << std::endl;
	{
	int test[4] = {10, 11, 12, 13};
	int ret_test[4] = {0, 0, 0, 0}; 
	file.write(test);
	file.read_until(ret_test, 12);
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array int: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	double test[4] = {10.5, 11.5, 12.5, 13.5};
	double ret_test[4] = {0, 0, 0, 0};
	file.write(test);
	file.read_until(ret_test, 12.5);
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array double: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_struct test[4] = {test_struct{25, 50, 75, 100}, test_struct{125, 150, 175, 200}, test_struct{225, 250, 275, 300}, test_struct{325, 350, 375, 400}};
	test_struct ret_test[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
	file.write(test);
	file.read_until(ret_test, test_struct{225, 250, 275, 300});
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array test_struct: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
	{
	test_iIOable test[4] = {test_iIOable(25, 50, 75, 100), test_iIOable(125, 150, 175, 200), test_iIOable(225, 250, 275, 300), test_iIOable(325, 350, 375, 400)};
	test_iIOable ret_test[4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
	file.write(test);
	file.read_until(ret_test, test_iIOable{225, 250, 275, 300});
	std::string equal = std::equal(std::begin(test), std::end(test)-1, std::begin(ret_test)) ? "[success] : " : "[failure] : ";
	std::cout << equal << "array test_iIOable: ";
	print_arr(ret_test, 4);
	file.cleanFile();
	}
}

void Until_Range_test(){
	std::cout << "\n[Range until test]" << std::endl;
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test(4, 0);
	
	file.write(test.begin(), test.end());
	file.read_until(ret_test.begin(), ret_test.end(), 12);
	std::string equal = std::vector<int>{10, 11, 12, 0} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::forward_list<int> test = {10, 11, 12, 13};
	std::forward_list<int> ret_test(4, 0);
	
	file.write(test.begin(), test.end());
	file.read_until(ret_test.begin(), ret_test.end(), 12);
	std::string equal = std::forward_list<int>{10, 11, 12, 0} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "forward_list<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test(2, 0);
	
	file.write(test.begin(), test.end());
	file.read_until(ret_test.begin(), ret_test.end(), 12);
	std::string equal = std::vector{10, 11} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>(2): ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test(5, 0);
	
	file.write(test.begin(), test.end());
	file.read_until(ret_test.begin(), ret_test.end(), 12);
	std::string equal = std::vector{10, 11, 12, 0, 0} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>(5): ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
}

void Until_Container_test(){
	std::cout << "\n[Container until test]" << std::endl;
	{
	std::vector<int> test = {10, 11, 12, 13};
	std::vector<int> ret_test;
	
	file.write(test);
	file.read_until(ret_test, 12);
	std::string equal = std::vector<int>{10, 11, 12} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "vector<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
	{
	std::forward_list<int> test = {10, 11, 12, 13};
	std::forward_list<int> ret_test;
	
	file.write(test);
	file.read_until(ret_test, 12);
	std::string equal = std::forward_list{12, 11, 10} == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "forward_list<int>: ";
	print_container(ret_test.begin(), ret_test.end());
	file.cleanFile();
	}
}

void Until_String_test(){
	std::cout << "\n[String until test]" << std::endl;
	{
	std::string test = "string!";
	std::string ret_test;
	
	file.write(test);
	file.read_until(ret_test, 'r');
	std::string equal = "str" == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << "string: " << ret_test << std::endl;
	file.cleanFile();
	}
	{
	std::wstring test = L"wstring!";
	std::wstring ret_test;
	
	file.write(test);
	file.read_until(ret_test, 'r');
	std::wstring equal = L"wstr" == ret_test ? L"[success] : " : L"[failure] : ";
	std::wcout << equal << L"wstring: " << ret_test << std::endl;
	file.cleanFile();
	}
	{
	std::u16string test = u"u16string!";
	std::u16string ret_test;
	
	file.write(test);
	file.read_until(ret_test, 'r');
	std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cv;
	std::string equal = u"u16str" == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << cv.to_bytes(u"u16string: ") << cv.to_bytes(ret_test) << std::endl;
	file.cleanFile();
	}
	{
	std::u32string test = U"u32string!";
	std::u32string ret_test;
	
	file.write(test);
	file.read_until(ret_test, 'r');
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cv;
	std::string equal = U"u32str" == ret_test ? "[success] : " : "[failure] : ";
	std::cout << equal << cv.to_bytes(U"u32string: ") << cv.to_bytes(ret_test) << std::endl;
	file.cleanFile();
	}
}


int main(){
	SFINEA_test();

	RValue_test();
	LValue_test();
	Pointer_test();
	Pointer_arr_test();
	Array_test();
	Range_test();
	Container_test();
	String_test();
	Stream_test();

	Until_Pointer_arr_test();
	Until_Array_test();
	Until_Range_test();
	Until_Container_test();
	Until_String_test();

	return 0;
}