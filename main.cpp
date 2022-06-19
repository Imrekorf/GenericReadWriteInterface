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
#include <sstream>


int main(){
	iFileIO file("test.txt");
	std::vector<int> vec = {25, 50, 75, 100};
	std::vector<int> vec2(5, 0);


	std::vector<int>::iterator read_end;
	try{
	file.write(vec.begin(), vec.end(), [](int i){
		return i;
	});
	read_end = file.read(vec2.begin(), vec2.end());
	} catch(const iGIO::IOfailure& e){
		if(read_end != vec2.end())
			std::cout << "not enough data" << std::endl;
	}
	std::cout << "vec2: ";
	for(int i : vec2)
		std::cout << i << " ";
	std::cout << std::endl;

	file.cleanFile();
	std::cout << "finished predicate write test and vector range reading" << std::endl;

	file.write(vec.begin(), vec.end(), [](int i){
		return i;
	});
	std::cout << "write finished" << std::endl;
	vec2.clear();
	file.read(vec2, 5);
	std::cout << "vec2: ";
	for(int i : vec2)
		std::cout << i << " ";
	std::cout << std::endl;

	file.cleanFile();
	std::cout << "finished predicate write test and vector maxlength reading" << std::endl;

	file.write(custom{5});
	std::stringstream strbuff;
	file.read<custom>(std::cout);
	std::cout << strbuff.str() << std::endl;
	std::cout << std::endl;
	
	file.cleanFile();
	std::cout << "finished writing to cout stream" << std::endl;

	std::string input_str = "hello world!";
	std::string output_str;
	file.write(input_str);
	file.read(output_str);
	std::cout << "read output_str: " << output_str << std::endl;
	file.cleanFile();

	std::cout << "finished string to r/w" << std::endl;

	std::vector<int> buffer;
	file.write({"5 ", "6 ", "7 ", "8 "});
	file.read<std::string>(buffer, [](std::string text){
		return std::stoi(text);
	});
	std::cout << "buffer: ";
	for(int i : buffer)
		std::cout << i << " ";
	std::cout << std::endl;

	std::cout << "finished array based predicate reading to r/w" << std::endl;
	file.cleanFile();
	
	std::stringstream temp;
	// std::ofstream os("temp.txt");
	temp << "hello world!";
	// os << temp.str();
	// os.close();
	// std::ifstream is("temp.txt");
	file.write(std::cin);
	// is.close();

	file.cleanFile();
	std::cout << "Finished write from stream" << std::endl;

	// read from file to object
	// file >> object;
	// object << file;
	// write from object to file
	// object >> file;

	// file.write(p);

	// DONE: string reading
	//? DONE?: container reading
	// TODO: operator << and >> overloading
	// TODO: terminator character reading variants, of new types
	// TODO: terminator arrays? 
	// TODO: delimiters? e.g. read string splitting? also implement for writing?
	// TODO: input parsers???

	return 0;
}