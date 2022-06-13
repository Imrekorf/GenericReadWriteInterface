#include "GRWI.hpp"
#include <iostream>

// #define TESTING
#define RVAL_TEST
#define CLASS_RVAL_TEST
#define STRUCT_RVAL_TEST

#define LVAL_TEST
#define CLASS_LVAL_TEST
#define STRUCT_LVAL_TEST

#define ARR_TEST
#define CLASS_ARR_TEST
#define STRUCT_ARR_TEST

#define PTR_TEST
#define CLASS_PTR_TEST
#define STRUCT_PTR_TEST

#define PTR_ARR_TEST
#define CLASS_PTR_ARR_TEST
#define STRUCT_PTR_ARR_TEST

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

struct test {
	int a = 3;
	int b = 7;
	int c = 9;
	int d = 24;
	void writetest(iFileIO& file){
		file.write(*this);
	}
	test(int a) : a(a) {}
	test(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}
};

class ctest : public iIOable {
	int a = 25;
	int b = 50;
	int c = 75;
	int d = 100;

	std::unique_ptr<const char[]> toBytes() override{
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

	size_t ObjectByteSize() override {
		return sizeof(int) * 4;
	}
public:
	ctest(int a) : a(a) {}
	ctest(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}

	void print(){
		std::cout << a << " " << b << " " << c << " " << d << " " << std::endl;
	}
};


int main(){
	iFileIO file("test.txt");

#ifdef TESTING
// Rval IO tests
#ifdef RVAL_TEST
	std::cout << "\nRvalue test" << std::endl;
	std::cout << "writing rvalue, size: " << file.write(25) << std::endl;
	int rval;
	std::cout << "reading rvalue, size: " << file.read(rval) << std::endl;
	std::cout << "rvalue: " << rval << std::endl;
	file.cleanFile();
#endif
#ifdef CLASS_RVAL_TEST
	std::cout << "\nClass rvalue test" << std::endl;
	std::cout << "writing rvalue, size: " << file.write<ctest>(25) << std::endl;
	ctest ctestbuff{0, 0, 0, 0};
	std::cout << "reading rvalue, size: " << file.read(ctestbuff) << std::endl;
	std::cout << "rvalue: "; ctestbuff.print();
	file.cleanFile();
#endif
#ifdef STRUCT_RVAL_TEST
	std::cout << "\nStruct rvalue test" << std::endl;
	std::cout << "writing rvalue, size: " << file.write<test>(25) << std::endl;
	test testbuff{0, 0, 0, 0};
	std::cout << "reading ctestbuff, size: " << file.read(testbuff) << std::endl;
	std::cout << "rvalue: " << testbuff.a << " " << testbuff.b << " " << testbuff.c << " " << testbuff.d << std::endl;
	file.cleanFile();
#endif

// Lvalue IO tests
#ifdef LVAL_TEST
	std::cout << "\nLvalue test" << std::endl;
	int lvalbuffer = 25;
	std::cout << "writing lvalue, size: " << file.write(lvalbuffer) << std::endl;
	int lvalbuffer2 = 0;
	std::cout << "reading lvalue, size: " << file.read(lvalbuffer2) << std::endl;
	std::cout << "lvalue: " << lvalbuffer2 << std::endl;
	file.cleanFile();
#endif
#ifdef CLASS_LVAL_TEST
	std::cout << "\nClass lvalue test" << std::endl;
	ctest ct = {25, 50, 75, 100};
	std::cout << "writing lvalue, size: " << file.write(ct) << std::endl;
	ctest ct2 = {0, 0, 0, 0};
	std::cout << "reading lvalue, size: " << file.read(ct2) << std::endl;
	std::cout << "lvalue: "; ct2.print();
	file.cleanFile();
#endif
#ifdef STRUCT_LVAL_TEST
	std::cout << "\nStruct lvalue test" << std::endl;
	test t = {25, 50, 75, 100};
	std::cout << "writing lvalue, size: " << file.write(t) << std::endl;
	test t2 = {0};
	std::cout << "reading lvalue, size: " << file.read(t2) << std::endl;
	std::cout << "lvalue: " << t2.a << " " << t2.b << " " << t2.c << " " << t2.d << std::endl;
	file.cleanFile();
#endif

// Array IO tests
#ifdef ARR_TEST
	std::cout << "\nArray test" << std::endl;
	int arr[] = {1, 2, 3, 4};
	std::cout << "writing array, size: " << file.write(arr) << std::endl;
	int arr2[4];
	std::cout << "reading array, size: " << file.read(arr2) << std::endl;
	std::cout << "array: " << arr2[0] << " " << arr2[1] << " " << arr2[2] << " " << arr[3] << std::endl;
	file.cleanFile();
#endif
#ifdef CLASS_ARR_TEST
	std::cout << "\nClass array test" << std::endl;
	ctest ctarr[] = {{1,2,3,4}, {5, 6, 7, 8}};
	std::cout << "writing array, size: " << file.write(ctarr) << std::endl;
	ctest ct2arr[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
	std::cout << "reading array, size: " << file.read(ct2arr) << std::endl;
	std::cout << "ct2arr[0]: "; ct2arr[0].print();
	std::cout << "ct2arr[1]: "; ct2arr[1].print();
	file.cleanFile();
#endif
#ifdef STRUCT_ARR_TEST
	std::cout << "\nClass array test" << std::endl;
	test tarr[] = {{1,2,3,4}, {5, 6, 7, 8}};
	std::cout << "writing array, size: " << file.write(tarr) << std::endl;
	test t2arr[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
	std::cout << "reading array, size: " << file.read(t2arr) << std::endl;
	std::cout << "t2arr[0]: " << t2arr[0].a << " " << t2arr[0].b << " " << t2arr[0].c << " " << t2arr[0].d << std::endl;
	std::cout << "t2arr[1]: " << t2arr[1].a << " " << t2arr[1].b << " " << t2arr[1].c << " " << t2arr[1].d << std::endl;
	file.cleanFile();
#endif

// pointer IO tests
#ifdef PTR_TEST
	std::cout << "\nPointer test" << std::endl;
	int* i = new int[1]{127};
	std::cout << "writing pointer, size: " << file.write(i, 1) << std::endl;
	int* i2 = new int[1];
	std::cout << "reading pointer, size: " << file.read(i2, 1) << std::endl;
	std::cout << "pointer: " << *i2 << std::endl;
	delete i;
	delete i2;
	file.cleanFile();
#endif
#ifdef CLASS_PTR_TEST
	std::cout << "\nClass pointer test" << std::endl;
	ctest* ctptr = new ctest(25, 50, 75, 100);
	std::cout << "writing pointer, size: " << file.write(ctptr, 1) << std::endl;
	ctest* ctptr2 = new ctest(0, 0, 0, 0);
	std::cout << "reading pointer, size: " << file.read(ctptr2, 1) << std::endl;
	std::cout << "pointer: "; ctptr2->print();
	delete ctptr;
	delete ctptr2;
	file.cleanFile();
#endif
#ifdef STRUCT_PTR_TEST
	std::cout << "\nStruct pointer test" << std::endl;
	test* tptr = new test(25, 50, 75, 100);
	std::cout << "writing pointer, size: " << file.write(tptr, 1) << std::endl;
	test* tptr2 = new test(0, 0, 0, 0);
	std::cout << "reading pointer, size: " << file.read(tptr2, 1) << std::endl;
	std::cout << "pointer: " << tptr2->a << " " << tptr2->b << " " << tptr2->c << " " << tptr2->d << std::endl;
	delete tptr;
	delete tptr2;
	file.cleanFile();
#endif

// pointer array IO tests
#ifdef PTR_ARR_TEST
	std::cout << "\nPointer array test" << std::endl;
	int* i = new int[4]{25, 50, 75, 100};
	std::cout << "writing pointer array, size: " << file.write(i, 4) << std::endl;
	int* i2 = new int[4];
	std::cout << "reading pointer array, size: " << file.read(i2, 4) << std::endl;
	std::cout << "pointer array: " << i2[0] << " "  << i2[1] << " " << i2[2] << " " << i2[3] << std::endl;
	delete[] i;
	delete[] i2;
	file.cleanFile();
#endif
#ifdef CLASS_PTR_ARR_TEST
	std::cout << "\nClass pointer array test" << std::endl;
	ctest* ctptr = new ctest[2]{{25, 50, 75, 100}, {125, 150, 175, 200}};
	std::cout << "writing pointer array, size: " << file.write(ctptr, 2) << std::endl;
	ctest* ctptr2 = new ctest[2]{{0, 0, 0, 0}, {0, 0, 0, 0}};
	std::cout << "reading pointer array, size: " << file.read(ctptr2, 2) << std::endl;
	std::cout << "pointer[0]: "; ctptr2[0].print();
	std::cout << "pointer[1]: "; ctptr2[1].print();
	delete[] ctptr;
	delete[] ctptr2;
	file.cleanFile();
#endif
#ifdef STRUCT_PTR_ARR_TEST
	std::cout << "\nStruct pointer array test" << std::endl;
	test* tptr = new test[2]{{25, 50, 75, 100}, {125, 150, 175, 200}};
	std::cout << "writing pointer array, size: " << file.write(tptr, 2) << std::endl;
	test* tptr2 = new test[2]{{0, 0, 0, 0}, {0, 0, 0, 0}};
	std::cout << "reading pointer array, size: " << file.read(tptr2, 2) << std::endl;
	std::cout << "pointer[0]: " << tptr2[0].a << " " << tptr2[0].b << " " << tptr2[0].c << " " << tptr2[0].d << std::endl;
	std::cout << "pointer[1]: " << tptr2[1].a << " " << tptr2[1].b << " " << tptr2[1].c << " " << tptr2[1].d << std::endl;
	delete[] tptr;
	delete[] tptr2;
	file.cleanFile();
#endif
#endif

return 0;
}