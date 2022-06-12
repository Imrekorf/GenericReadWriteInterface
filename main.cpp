#include <iostream>

#include <exception>
#include <string>
#include <utility>

class iGIO {
public:
	class IOfailure : std::exception {
		std::string _message;
	public:
		explicit IOfailure(const std::string& message) : _message(message){}
		virtual ~IOfailure(){}
	};
protected:
	/**
	 * @brief Reads length amount of bytes into the passed buffer.
	 * Buffer should be allocated before passing. Returns -1 on error.
	 * @param buffer The buffer to read bytes into.
	 * @param length The amount of bytes to read.
	 * @return int The amount of bytes read.
	 */
	virtual int iRead(char* buffer, std::size_t length) = 0;
	/**
	 * @brief Writes length amount of bytes. Returns -1 on error.
	 * @param buffer The buffer containing the bytes to write
	 * @param length the amount of bytes to write
	 * @return int the amount of bytes written. 
	 */
	virtual int iWrite(const char* buffer, std::size_t length) const = 0;

	/**
	 * @brief Returns the interface implementation name, e.g. I2C, SPI, UART, TCP
	 * @return const char* The name of the implementation
	 */
	virtual inline const char* iName() const = 0;
private:
	int _read(char* buffer, std::size_t length){
		int n = iRead(buffer, length); // if throwing will throw before generic IOfailure
		if(n == -1)
			throw IOfailure(std::string("IOFailure reading: ") + iName());
		return n;
	}
	int _write(const char* buffer, std::size_t length) const{
		int n = iWrite(buffer, length); // if throwing will throw before generic IOfailure
		if(n == -1)
			throw IOfailure(std::string("IOFailure writing: ") + iName());
		return n;
	}
public:
	virtual ~iGIO(){}

	/**
	 * @brief Reads length amount of T objects into T* buffer.
	 * @tparam T The buffer type.
	 * @param buffer the buffer to read T objects into.
	 * @param length the amount of T objects to read.
	 * @return int the amount of T objects read.
	 */
	template<typename T> int read(T* buffer, std::size_t length){ return _read(buffer, length * sizeof(T)) / sizeof(T); }
	/**
	 * @brief Reads a T object into buffer
	 * @tparam T the buffer type.
	 * @param buffer the buffer to read into
	 * @return int the amount of T objects read
	 */
	template<typename T> int read(T& buffer){ return _read(&buffer, sizeof(T)) / sizeof(T); }

	template<typename T, std::size_t size> int write(const T(&buffer)[size]) const { return _write((const char*)buffer, size * sizeof(T)); }
	template<typename T> int write(const T&& buffer) const { return _write((const char*)&buffer, sizeof(T)); }
	template<typename T> typename std::enable_if<!std::is_pointer<T>::value, int>::type 
		write(const T&  buffer) const { return _write((const char*)&buffer, sizeof(T)); }
	template<typename T> typename std::enable_if< std::is_pointer<T>::value, int>::type
		write(const T  buffer, const int length) const { return _write((const char*)buffer, length * sizeof(decltype(*std::declval<T>()))); }
};

#include <fstream>

class iFileIO : public iGIO {
private:
	std::string _filename;
protected:
	virtual int iRead(char* buffer, std::size_t length) override{
		std::ifstream file(_filename, std::ifstream::in);
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
		std::ofstream file(_filename, std::ofstream::out);
		if(!file.is_open())
			throw std::runtime_error("failed to open file " + _filename);
		std::cout << "writing " << length << " bytes to " << _filename << std::endl;
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
	iFileIO(std::string filename) : _filename(filename) {}
	virtual ~iFileIO(){}
};

int main(){

	iFileIO file("test.txt");
	const char text[] = "hello world!\n";
	int arr2[4] = {1, 2, 3, 4};
	int* arr = new int[4];
	arr[0] = 1; arr[1] = 2; arr[2] = 3; arr[3] = 4;
	file.write(text);
	file.write("Hello to you!\n");

	return 0;
}