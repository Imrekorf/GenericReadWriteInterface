#include "iFileIO.hpp"
#include <iostream>

#define RVAL_TEST

#define LVAL_TEST

#define ARR_TEST

#define PTR_TEST

#define PTR_ARR_TEST

#define CONTAINER_TEST

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
	ctest(int a) : a(a) {}
	ctest(int a, int b, int c, int d) : a(a), b(b), c(c), d(d) {}

	void print(){
		std::cout << a << " " << b << " " << c << " " << d << " " << std::endl;
	}
};


int main(){
	iFileIO file("test.txt");

// Lvalue IO tests
#ifdef LVAL_TEST
	// default lvalue test
	std::cout << "\nLvalue test" << std::endl;
	int lvalbuffer = 25;
	std::cout << "writing lvalue, size: " << file.write(lvalbuffer) << std::endl;
	int lvalbuffer2 = 0;
	std::cout << "reading lvalue, size: " << file.read(lvalbuffer2) << std::endl;
	std::cout << "lvalue: " << lvalbuffer2 << std::endl;
	file.cleanFile();

	// class lvalue test
	std::cout << "\nClass lvalue test" << std::endl;
	ctest ct = {25, 50, 75, 100};
	std::cout << "writing lvalue, size: " << file.write(ct) << std::endl;
	ctest ct2 = {0, 0, 0, 0};
	std::cout << "reading lvalue, size: " << file.read(ct2) << std::endl;
	std::cout << "lvalue: "; ct2.print();
	file.cleanFile();

	// struct lvalue test
	std::cout << "\nStruct lvalue test" << std::endl;
	test t = {25, 50, 75, 100};
	std::cout << "writing lvalue, size: " << file.write(t) << std::endl;
	test t2 = {0};
	std::cout << "reading lvalue, size: " << file.read(t2) << std::endl;
	std::cout << "lvalue: " << t2.a << " " << t2.b << " " << t2.c << " " << t2.d << std::endl;
	file.cleanFile();
#endif

// Rval IO tests
#ifdef RVAL_TEST
	// default rvalue test
	std::cout << "\nRvalue test" << std::endl;
	std::cout << "writing rvalue, size: " << file.write(25) << std::endl;
	int rval;
	std::cout << "reading rvalue, size: " << file.read(rval) << std::endl;
	std::cout << "rvalue: " << rval << std::endl;
	file.cleanFile();

	// class rvalue test
	std::cout << "\nClass rvalue test" << std::endl;
	std::cout << "writing rvalue, size: " << file.write<ctest>(25) << std::endl;
	ctest ctestbuff{0, 0, 0, 0};
	std::cout << "reading rvalue, size: " << file.read(ctestbuff) << std::endl;
	std::cout << "rvalue: "; ctestbuff.print();
	file.cleanFile();

	// struct rvalue test
	std::cout << "\nStruct rvalue test" << std::endl;
	std::cout << "writing rvalue, size: " << file.write<test>(25) << std::endl;
	test testbuff{0, 0, 0, 0};
	std::cout << "reading ctestbuff, size: " << file.read(testbuff) << std::endl;
	std::cout << "rvalue: " << testbuff.a << " " << testbuff.b << " " << testbuff.c << " " << testbuff.d << std::endl;
	file.cleanFile();
#endif

// Array IO tests
#ifdef ARR_TEST
	// default array test
	std::cout << "\nArray test" << std::endl;
	int arr[] = {1, 2, 3, 4};
	std::cout << "writing array, size: " << file.write(arr) << std::endl;
	int arr2[4];
	std::cout << "reading array, size: " << file.read(arr2) << std::endl;
	std::cout << "array: " << arr2[0] << " " << arr2[1] << " " << arr2[2] << " " << arr[3] << std::endl;
	file.cleanFile();

	// class array test
	std::cout << "\nClass array test" << std::endl;
	ctest ctarr[] = {{1,2,3,4}, {5, 6, 7, 8}};
	std::cout << "writing array, size: " << file.write(ctarr) << std::endl;
	ctest ct2arr[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
	std::cout << "reading array, size: " << file.read(ct2arr) << std::endl;
	std::cout << "ct2arr[0]: "; ct2arr[0].print();
	std::cout << "ct2arr[1]: "; ct2arr[1].print();
	file.cleanFile();

	// Struct array test
	std::cout << "\nStruct array test" << std::endl;
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
	// default pointer 
	std::cout << "\nPointer test" << std::endl;
	int* i = new int[1]{127};
	std::cout << "writing pointer, size: " << file.write(i, 1) << std::endl;
	int* i2 = new int[1];
	std::cout << "reading pointer, size: " << file.read(i2, 1) << std::endl;
	std::cout << "pointer: " << *i2 << std::endl;
	delete i;
	delete i2;
	file.cleanFile();

	// class pointer
	std::cout << "\nClass pointer test" << std::endl;
	ctest* ctptr = new ctest(25, 50, 75, 100);
	std::cout << "writing pointer, size: " << file.write(ctptr, 1) << std::endl;
	ctest* ctptr2 = new ctest(0, 0, 0, 0);
	std::cout << "reading pointer, size: " << file.read(ctptr2, 1) << std::endl;
	std::cout << "pointer: "; ctptr2->print();
	delete ctptr;
	delete ctptr2;
	file.cleanFile();

	// struct pointer
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
	// default pointer array
	std::cout << "\nPointer array test" << std::endl;
	int* ptrarri = new int[4]{25, 50, 75, 100};
	std::cout << "writing pointer array, size: " << file.write(ptrarri, 4) << std::endl;
	int* ptriarr2 = new int[4];
	std::cout << "reading pointer array, size: " << file.read(ptriarr2, 4) << std::endl;
	std::cout << "pointer array: " << ptriarr2[0] << " "  << ptriarr2[1] << " " << ptriarr2[2] << " " << ptriarr2[3] << std::endl;
	delete[] ptrarri;
	delete[] ptriarr2;
	file.cleanFile();

	// class pointer array
	std::cout << "\nClass pointer array test" << std::endl;
	ctest* ctptrarr = new ctest[2]{{25, 50, 75, 100}, {125, 150, 175, 200}};
	std::cout << "writing pointer array, size: " << file.write(ctptrarr, 2) << std::endl;
	ctest* ctptrarr2 = new ctest[2]{{0, 0, 0, 0}, {0, 0, 0, 0}};
	std::cout << "reading pointer array, size: " << file.read(ctptrarr2, 2) << std::endl;
	std::cout << "pointer[0]: "; ctptrarr2[0].print();
	std::cout << "pointer[1]: "; ctptrarr2[1].print();
	delete[] ctptrarr;
	delete[] ctptrarr2;
	file.cleanFile();

	// struct pointer array
	std::cout << "\nStruct pointer array test" << std::endl;
	test* tptrarr = new test[2]{{25, 50, 75, 100}, {125, 150, 175, 200}};
	std::cout << "writing pointer array, size: " << file.write(tptrarr, 2) << std::endl;
	test* tptrarr2 = new test[2]{{0, 0, 0, 0}, {0, 0, 0, 0}};
	std::cout << "reading pointer array, size: " << file.read(tptrarr, 2) << std::endl;
	std::cout << "pointer[0]: " << tptrarr2[0].a << " " << tptrarr2[0].b << " " << tptrarr2[0].c << " " << tptrarr2[0].d << std::endl;
	std::cout << "pointer[1]: " << tptrarr2[1].a << " " << tptrarr2[1].b << " " << tptrarr2[1].c << " " << tptrarr2[1].d << std::endl;
	delete[] tptrarr;
	delete[] tptrarr2;
	file.cleanFile();
#endif

// Container tests
#ifdef CONTAINER_TEST

#endif


return 0;
}