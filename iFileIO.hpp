#pragma once
#include "GRWI.hpp"

#include <fstream>

class iFileIO : public iGIO {
private:
	std::string _filename;
	std::size_t read_offset = 0;
protected:
	virtual int iRead(char* buffer, const std::size_t length) override{
		std::ifstream file(_filename, std::ios_base::in);
		file.seekg(read_offset, std::ios_base::beg);
		file.read(buffer, length);
		int n = file.gcount(); // check read success
		read_offset += n;
		if(!file)
			n = -1;
		file.close();
		return n;
	}

	virtual int iWrite(const char* buffer, const std::size_t length) override{
		std::ofstream file(_filename, std::ios_base::out | std::ios_base::app);
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
	iFileIO(std::string filename) 
		: _filename(filename) {
		std::ofstream file(_filename, std::ios_base::out);
		if(!file.is_open())
			throw std::runtime_error("failed to open file " + filename);
		cleanFile();
	}
	virtual ~iFileIO(){}

	void cleanFile() {
		std::ofstream file(_filename, std::ios_base::out);
		if(!file.is_open())
			throw std::runtime_error("failed to open file " + _filename);
		file << "" << std::flush;
		file.close();
	}
};