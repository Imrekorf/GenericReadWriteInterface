#include <iostream>
#include "iFileIO.hpp"


// #include <array>
#include <vector>
#include <map>
// #include <tuple>
// #include <deque>
// #include <list>
// #include <forward_list>
#include <algorithm>

int main(){

	iFileIO file("test.txt");
	// std::array<int, 4> pure_array = {25, 50, 75, 100};
	std::vector<int> vec = {25, 50, 75, 100};
	std::vector<int> vec2(5, 0);
	std::map<int, int> m = {
		{1, 1},
		{2, 2},
		{3, 3},
		{4, 4},
		{5, 5}
	};
	std::pair<int, int> p = {
		5, 5
	};
	// file.write(pure_array);
	std::vector<int>::iterator read_end;


	try{
	file.write(vec.begin(), vec.end());
	read_end = file.read(vec2.begin(), vec2.end());
	} catch(const iGIO::IOfailure& e){
		if(read_end != vec2.end())
			std::cout << "not enough data" << std::endl;
	}
	std::cout << "vec2: ";
	for(int i : vec2)
		std::cout << i << " ";
	std::cout << std::endl;
	// file.write(p);

	// TODO: container reading
	// TODO: special pair overloading?

	return 0;
}