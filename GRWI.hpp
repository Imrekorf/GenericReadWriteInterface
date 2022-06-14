#pragma once

#include <exception>
#include <string>
#include <utility>
#include <type_traits>
#include <memory>
#include <cstring> // for memcpy
#include <functional>

#include <iostream> // TEMP

class iGIO;

class iIOable {
friend class iGIO;
protected:
	virtual ~iIOable() {}
	virtual std::unique_ptr<const char[]> toBytes() = 0;
	virtual void toObject(const std::unique_ptr<char[]> data) = 0;
	/**
	 * @brief The size of the object in bytes, This does not equal sizeof(T)!
	 * It is equal to the amount of data in the object in bytes.
	 * @return std::size_t The amount of bytes in the object
	 */
	virtual std::size_t ObjectByteSize() = 0;
};

class iGIO {
public:
	class IOfailure : std::exception {
		std::string _message;
	public:
		explicit IOfailure(const std::string& message) : _message(message){}
		virtual ~IOfailure(){}
		const char* what() const noexcept override {return _message.c_str();}
	};
protected:
	/**
	 * @brief Reads length amount of bytes into the passed buffer.
	 * Buffer should be allocated before passing. Returns -1 on error.
	 * @param buffer The buffer to read bytes into.
	 * @param length The amount of bytes to read.
	 * @return int The amount of bytes read.
	 */
	virtual int iRead(char* buffer, const std::size_t length) = 0;
	/**
	 * @brief Writes length amount of bytes. Returns -1 on error.
	 * @param buffer The buffer containing the bytes to write
	 * @param length the amount of bytes to write
	 * @return int the amount of bytes written. 
	 */
	virtual int iWrite(const char* buffer, const std::size_t length) = 0;

	/**
	 * @brief Returns the interface implementation name, e.g. I2C, SPI, UART, TCP
	 * @return const char* The name of the implementation
	 */
	virtual inline const char* iName() const = 0;
private:
	int _read(char* buffer, const std::size_t length){
		int n = iRead(buffer, length); // if throwing will throw before generic IOfailure
		if(n == -1)
			throw IOfailure(std::string("IOFailure reading: ") + iName());
		return n;
	}
	int _write(const char* buffer, const std::size_t length) {
		int n = iWrite(buffer, length); // if throwing will throw before generic IOfailure
		if(n == -1)
			throw IOfailure(std::string("IOFailure writing: ") + iName());
		return n;
	}

	template<class T> using is_ioable = std::is_base_of<iIOable, T>;

	// ----------------------------------------------------------------
	// check if something is a container, based from: https://stackoverflow.com/a/9407521
	template<typename T> static std::true_type  has_const_iterator_test(typename T::const_iterator*); // if it does have a const_iterator, return true value
	template<typename T> static std::false_type has_const_iterator_test(...); // if not have a default function
	template<class T> using has_const_iterator = decltype(has_const_iterator_test<T>(0));

	template<typename T, typename = decltype(std::declval<T&>().begin())>
	static std::true_type  has_begin_iterator_test(const T&);
	static std::false_type has_begin_iterator_test(...);
	template<class T> using has_begin_iterator = decltype(has_begin_iterator_test(std::declval<T>()));

	template<typename T, typename = decltype(std::declval<T&>().end())>
	static std::true_type  has_end_iterator_test(const T&);
	static std::false_type has_end_iterator_test(...);
	template<class T> using has_end_iterator = decltype(has_end_iterator_test(std::declval<T>()));

	template<typename T> using is_container = std::integral_constant<bool, has_const_iterator<T>::value && has_begin_iterator<T>::value && has_end_iterator<T>::value>;
	
	template <class T, class = void>
	struct is_iterator : std::false_type { };
	template <class T>
	struct is_iterator<T, typename std::enable_if<!std::is_same<typename std::iterator_traits<T>::iterator_category, void>::value, void>::type> : std::true_type { };
	// ----------------------------------------------------------------

	

public:
	virtual ~iGIO(){}

	/**
	 * @brief Reads an array from the interface into the specified buffer.
	 * @tparam T the type of the array
	 * @tparam size the size of the array
	 * @return std::size_t the amount of T objects read
	 */
	template<typename T, std::size_t size> typename std::enable_if<
		!is_ioable<T>::value && !is_container<T>::value, std::size_t>::type
		read(T(&buffer)[size]) { return _read((char*)buffer, size * sizeof(T)) / sizeof(T); }
	template<typename T, std::size_t size> typename std::enable_if<
		 is_ioable<T>::value, std::size_t>::type
		read(T(&buffer)[size]) {
			// create buffer, and read into it
			auto data = std::make_unique<char[]>(size * ((iIOable&)buffer[0]).ObjectByteSize());
			auto bytesread = _read(data.get(), size * ((iIOable&)buffer[0]).ObjectByteSize());
			// copy buffered data into objects
			for(std::size_t i = 0; i < size; i++){
				auto data_temp = std::make_unique<char[]>(((iIOable&)buffer[0]).ObjectByteSize());
				std::memcpy(data_temp.get(), data.get() + i * ((iIOable&)buffer[0]).ObjectByteSize(), ((iIOable&)buffer[0]).ObjectByteSize());
				((iIOable&)buffer[i]).toObject(std::move(data_temp));
			}
			return bytesread / ((iIOable&)buffer[0]).ObjectByteSize();
		}
	/**
	 * @brief Reads length amount of objects from the interface into pointer buffer
	 * @tparam T The pointer type
	 * @param buffer the pointer buffer
	 * @param length the amount of objects in the pointer (array)
	 * @return std::size_t the amount of T objects written
	 */
	template<typename T> typename std::enable_if<
		!is_ioable<typename std::remove_pointer<T>::type>::value && std::is_pointer<T>::value && !is_container<T>::value && !is_iterator<T>::value, std::size_t>::type
		read(T buffer, const std::size_t length) { return _read((char*)buffer, length * sizeof(typename std::remove_pointer<T>::type)) / sizeof(typename std::remove_pointer<T>::type); }
	template<typename T> typename std::enable_if<
		 is_ioable<typename std::remove_pointer<T>::type>::value && std::is_pointer<T>::value, std::size_t>::type
		read(T buffer, const std::size_t length) {
			// create buffer, and read into it
			auto data = std::make_unique<char[]>(length * ((iIOable&)buffer[0]).ObjectByteSize());
			auto bytesread = _read(data.get(), length * ((iIOable&)buffer[0]).ObjectByteSize());
			// copy buffered data into objects
			for(std::size_t i = 0; i < length; i++){
				auto data_temp = std::make_unique<char[]>(((iIOable&)buffer[0]).ObjectByteSize());
				std::memcpy(data_temp.get(), data.get() + i * ((iIOable&)buffer[0]).ObjectByteSize(), ((iIOable&)buffer[0]).ObjectByteSize());
				((iIOable&)buffer[i]).toObject(std::move(data_temp));
			}
			return bytesread / ((iIOable&)buffer[0]).ObjectByteSize();
		}
	/**
	 * @brief Reads an lvalue from the interface
	 * @tparam T The type of the lvalue
	 * @param buffer the lvalue
	 * @return std::size_t the amount of lvalue written
	 */
	template<typename T> typename std::enable_if<
		!is_ioable<T>::value && !std::is_pointer<T>::value && !is_container<T>::value && !is_iterator<T>::value, std::size_t>::type 
		read(T& buffer) { return _read((char*)&buffer, sizeof(T)) / sizeof(T); }
	template<typename T> typename std::enable_if<
		 is_ioable<T>::value && !std::is_pointer<T>::value, std::size_t>::type 
		read(T& buffer) {
			auto data = std::make_unique<char[]>(((iIOable&)buffer).ObjectByteSize());
			auto bytesread = _read(data.get(),   ((iIOable&)buffer).ObjectByteSize());
			((iIOable&)buffer).toObject(std::move(data));
			return bytesread / ((iIOable&)buffer).ObjectByteSize();
		}

	/**
	 * @brief Reads from the interface into a container
	 * @tparam InputIt The iterator type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @return InputIt::iterator pointing to the last element send
	 */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<typename std::iterator_traits<InputIt>::value_type>::value && is_iterator<InputIt>::value, InputIt>::type
		read(InputIt first, InputIt last) {
			for(; first!=last; ++first)
				read(*first);
			return last;
		}
	
	/**
	 * @brief Writes an rvalue T to the interface
	 * @tparam T The type of the rvalue
	 * @param buffer the rvalue
	 * @return std::size_t the amount of rvalue written
	 */
	template<typename T> typename std::enable_if<
		!is_ioable<T>::value && !is_container<T>::value, std::size_t>::type
		write(const T&& buffer) { return _write((const char*)&buffer, sizeof(T)) / sizeof(T); }
	template<typename T> typename std::enable_if< 
		 is_ioable<T>::value, std::size_t>::type
		write(const T&& buffer) { return _write(((iIOable&)buffer).toBytes().get(), ((iIOable&)buffer).ObjectByteSize()) / ((iIOable&)buffer).ObjectByteSize(); }
	/**
	 * @brief Writes an array to the interface.
	 * @tparam T The type of the array
	 * @tparam size the size of the array
	 * @return std::size_t the amount of T objects written
	 */
	template<typename T, std::size_t size> typename std::enable_if<
		!is_ioable<T>::value && !is_container<T>::value && !is_iterator<T>::value, std::size_t>::type
		write(const T(&buffer)[size]) { return _write((const char*)buffer, size * sizeof(T)) / sizeof(T); }
	template<typename T, std::size_t size> typename std::enable_if< 
		 is_ioable<T>::value, std::size_t>::type
		write(const T(&buffer)[size]) {
			auto data = std::make_unique<char[]>(size * ((iIOable&)buffer[0]).ObjectByteSize());
			// copy pointer buffer into byte buffer
			for(std::size_t i = 0; i < size; i++)
				std::memcpy(data.get() + i * ((iIOable&)buffer[0]).ObjectByteSize(),
					((iIOable&)buffer[i]).toBytes().get(), ((iIOable&)buffer[0]).ObjectByteSize());
			// write all data at once
			return _write(data.get(), size * ((iIOable&)buffer[0]).ObjectByteSize()) / ((iIOable&)buffer[0]).ObjectByteSize();
		}
	/**
	 * @brief Writes a pointer buffer of size length to the interface
	 * @tparam T The pointer type
	 * @param buffer the pointer buffer
	 * @param length the amount of objects in the pointer (array)
	 * @return std::size_t the amount of T objects written
	 */
	template<typename T> typename std::enable_if<
		!is_ioable<typename std::remove_pointer<T>::type>::value && std::is_pointer<T>::value && !is_container<T>::value && !is_iterator<T>::value, std::size_t>::type
		write(const T  buffer, const std::size_t&& length) { return _write((const char*)buffer, length * sizeof(typename std::remove_pointer<T>::type)) / sizeof(typename std::remove_pointer<T>::type); }
	template<typename T> typename std::enable_if< 
		 is_ioable<typename std::remove_pointer<T>::type>::value && std::is_pointer<T>::value, std::size_t>::type
		write(const T  buffer, const std::size_t&& length) {
			auto data = std::make_unique<char[]>(length * ((iIOable&)buffer[0]).ObjectByteSize());
			// copy pointer buffer into byte buffer
			for(std::size_t i = 0; i < length; i++)
				std::memcpy(data.get() + i * ((iIOable&)buffer[0]).ObjectByteSize(),
					((iIOable&)buffer[i]).toBytes().get(), ((iIOable&)buffer[0]).ObjectByteSize());
			// write all data at once
			return _write(data.get(), length * ((iIOable&)buffer[0]).ObjectByteSize()) / ((iIOable&)buffer[0]).ObjectByteSize();
		}
	/**
	 * @brief Writes an lvalue to the interface
	 * @tparam T The type of the lvalue
	 * @param buffer the lvalue
	 * @return std::size_t the amount of lvalue written
	 */
	template<typename T> typename std::enable_if<
		!is_ioable<T>::value && !std::is_pointer<T>::value && !is_container<T>::value && !is_iterator<T>::value, std::size_t>::type 
		write(const T&  buffer) { return _write((const char*)&buffer, sizeof(T)) / sizeof(T); }
	template<typename T> typename std::enable_if< 
		 is_ioable<T>::value && !std::is_pointer<T>::value, std::size_t>::type 
		write(const T&  buffer) { 
			return _write(((iIOable&)buffer).toBytes().get(), ((iIOable&)buffer).ObjectByteSize()) / ((iIOable&)buffer).ObjectByteSize(); }

	/**
	 * @brief Writes a container to the interface, executing a predicate for every element
	 * @tparam InputIt The iterator type
	 * @tparam Predicate The predicate function type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @param p the predicate function to call for every element in the container before sending it. Should return the a value and accept an element of container element type
	 * @return InputIt::iterator pointing to the last element send
	 */
	template<typename InputIt, class Predicate> constexpr typename std::enable_if<
		!is_container<typename std::iterator_traits<InputIt>::value_type>::value && is_iterator<InputIt>::value, InputIt>::type
		write(InputIt first, InputIt last, Predicate p) {
			for(; first!=last; ++first)
				write(p(*first));
			return last;
		}
	/**
	 * @brief Writes a container to the interface
	 * @tparam InputIt The iterator type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @return InputIt::iterator pointing to the last element send
	 */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<typename std::iterator_traits<InputIt>::value_type>::value && is_iterator<InputIt>::value, InputIt>::type
		write(InputIt first, InputIt last) {
			for(; first!=last; ++first)
				write(*first);
			return last;
		}
	/**
	 * @brief Writes an N dimensional container to the interface, unwrapping every dimension
	 * @tparam InputIt The iterator type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @return InputIt::iterator pointing to the last element send
	 */
	template<typename InputIt> constexpr typename std::enable_if<is_container<
		typename std::iterator_traits<InputIt>::value_type>::value && is_iterator<InputIt>::value, InputIt>::type
		write(InputIt first, InputIt last) {
			std::cout << "expanding container" << std::endl;
			for(; first!=last; ++first)
				write(first->begin(), first->end()); // no need to check output as it will throw on error
			return last;
		}

	/**
	 * @brief Writes a string rvalue to the interface
	 * @tparam T Template to check if parameter is string
	 * @param _string the string object
	 * @return std::size_t the amount of characters in the string written
	 */
	template<typename T> typename std::enable_if<
		std::is_same<T, std::string>::value, std::size_t>::type
		write(const T&& _string) {
		return write(_string.c_str(), _string.length());
	}

	/**
	 * @brief Writes a string lvalue to the interface
	 * @tparam T Template to check if parameter is string
	 * @param _string the string object
	 * @return std::size_t the amount of characters in the string written
	 */
	template<typename T> typename std::enable_if<
		std::is_same<T, std::string>::value, std::size_t>::type
		write(const T& _string) {
		return (write(_string.c_str(), _string.length()) == _string.length());
	}
};