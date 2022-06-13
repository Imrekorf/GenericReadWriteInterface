#include <iostream>
#include "GRWI.hpp"
#include <fstream>

class iFileIO : public iGIO {
private:
	std::string _filename;
protected:
	virtual int iRead(char* buffer, std::size_t length) override{
		std::ifstream file(_filename, std::ifstream::in | std::ifstream::app);
		if(!file.is_open())
			throw std::runtime_error("failed to open file " + _filename);
		file.read(buffer, length);
		int n = file.gcount(); // check read success
		if(!file)
			n = -1;
		file.close();
		return n;
	}

	virtual int iWrite(const char* buffer, std::size_t length) const override{
		std::ofstream file(_filename, std::ofstream::out | std::ofstream::app);
		if(!file.is_open())
			throw std::runtime_error("failed to open file " + _filename);
		file.write(buffer, length);
		int n = length;
		if(file.fail() || file.bad()){ // check write success
			std::cout << "failbit: " << file.fail() << " badbit: " << file.bad() << std::endl;
			n = -1;
		}
		file.close();
		return n;
	}

	virtual inline const char* iName() const override{
		return "FileIO";
	}
public:
	iFileIO(std::string filename) : _filename(filename) {
		std::ofstream file(_filename, std::ofstream::out);
		if(!file.is_open())
			throw std::runtime_error("failed to open file " + _filename);
		file << "" << std::flush;
		file.close();
	}
	virtual ~iFileIO(){}

	void cleanFile() {
		std::ofstream file(_filename, std::ofstream::out);
		if(!file.is_open())
			throw std::runtime_error("failed to open file " + _filename);
		file << "" << std::flush;
		file.close();
	}
};

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
	// std::vector<std::vector<int>> vec = {{25, 50, 75, 100}, {125, 150, 175, 200}};
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
	file.write(m.begin(), m.end(), [](auto p){
		return p.second;
	});
	// file.write(p);

	// TODO: container reading
	// TODO: special pair overloading?

	return 0;
}