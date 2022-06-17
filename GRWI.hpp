#pragma once

#include <exception>
#include <string>
#include <utility>
#include <type_traits>
#include <memory>
#include <cstring> // for memcpy
#include <functional>
#include <limits>
#include <iostream>

class custom {
public:
	int i = 4;

	void increase_int() {i++;}

	int j = 0;

	friend std::ostream& operator<<(std::ostream& os, const custom& dt){
		os << dt.i << " " << dt.j;
		return os;
	}
};


class iGIO;

class iIOable {
friend class iGIO;
protected:
	virtual ~iIOable() {}
	virtual std::unique_ptr<const char[]> toBytes() const = 0;
	virtual void toObject(const std::unique_ptr<char[]> data) = 0;
	/**
	 * @brief The size of the object in bytes, This does not equal sizeof(T)!
	 * It is equal to the amount of data in the object in bytes.
	 * @return std::size_t The amount of bytes in the object
	 */
	virtual std::size_t ObjectByteSize() const = 0;
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
	 * Function implementation requires that 0 byte return is a non-error return possibility.
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

	template<typename T, typename = decltype(std::declval<T&>().push_back(*std::declval<T&>().begin()))>
	static std::true_type  has_pushback_test(const T&);
	static std::false_type has_pushback_test(...);
	template<class T> using has_pushback = decltype(has_pushback_test(std::declval<T>()));

	template<typename T, typename IT, typename = decltype(std::declval<T&>() << std::declval<IT>())>
	static std::true_type  can_accept_stream_test(const T&, const IT&);
	static std::false_type can_accept_stream_test(...);
	template<class T, class IT> using can_accept_stream = decltype(can_accept_stream_test(std::declval<T>(), std::declval<IT>()));

	template<typename T> using is_container = std::integral_constant<bool, has_const_iterator<T>::value && has_begin_iterator<T>::value && has_end_iterator<T>::value>;
	
	// is_iterator only returns true for ::iterator types, not pointers
	template <class T, class = void>
	struct is_iterator : std::false_type { };
	template <class T>
	struct is_iterator<T, typename std::enable_if<!std::is_pointer<T>::value && !std::is_same<typename std::iterator_traits<T>::value_type, void>::value, void>::type> : std::true_type { };
	// ----------------------------------------------------------------

	// readability
	template<typename T> using remPtrType = typename std::remove_pointer<T>::type;
	template<typename InputIt> using itertype = typename std::iterator_traits<InputIt>::value_type;

public:
	virtual ~iGIO(){}

	//? ======== Base read and write wrappers ========>>==========================================================================================
	/** @brief Writes an rvalue T to the interface
	 * @tparam T The type of the rvalue
	 * @param buffer the rvalue
	 * @return std::size_t the amount of rvalue written */
	template<typename T> typename std::enable_if<
		!is_container<T>::value, 
	std::size_t>::type	write(const T&& buffer)		  { return _write((const char*)&buffer, sizeof(T)) / sizeof(T); }
	std::size_t			write(const iIOable&& buffer) { return _write(buffer.toBytes().get(), buffer.ObjectByteSize()) / buffer.ObjectByteSize(); }
	// note: no rvalue read as it doesn't make sense.
	

	/** @brief Reads until either length is reached or terminator is reached
	 * @tparam T The buffer type
	 * @param buffer the buffer to store into, preallocated
	 * @param terminator The to search for terminator character
	 * @param maxlength The maximum amount of bytes to read
	 * @return sts::size_t the amount of T read */
	template<typename T> typename std::enable_if<
		!is_container<T>::value,
	std::size_t>::type	read(T* buffer, const T& terminator, const std::size_t maxlength) {
		std::size_t T_read = 0;
		std::size_t _maxlength = maxlength ? maxlength : std::numeric_limits<std::size_t>::max();
		while(read(buffer, 1) && buffer++ != terminator && T_read++ < _maxlength){}
		return T_read;
	}

	/** @brief Reads until either length is reached or terminator is reached
	 * @tparam T The buffer type
	 * @param buffer the buffer to store into, preallocated
	 * @param terminator The to search for terminator character
	 * @param maxlength The maximum amount of bytes to read
	 * @return sts::size_t the amount of T read */
	template<typename T, std::size_t size> typename std::enable_if<
		!is_container<T>::value,
	std::size_t>::type	read(T(&buffer)[size], const T& terminator, const std::size_t maxlength = size) {
		return read(buffer, terminator, maxlength);
	}

	/** @brief Writes a pointer buffer of size length to the interface
	 * @tparam T The pointer type
	 * @param buffer the pointer buffer
	 * @param length the amount of objects in the pointer (array)
	 * @return std::size_t the amount of T objects written */
	template<typename T> typename std::enable_if<
		std::is_pointer<T>::value && !is_container<T>::value, 
	std::size_t>::type	write(const T  buffer, const std::size_t&& size) 	   { return _write((const char*)buffer, size * sizeof(remPtrType<T>)) / sizeof(remPtrType<T>); }
	std::size_t 	  	write(const iIOable* buffer, const std::size_t&& size) {
		auto data = std::make_unique<char[]>(size * buffer[0].ObjectByteSize());
		// copy pointer buffer into byte buffer
		for(std::size_t i = 0; i < size; i++)
			std::memcpy(data.get() + i * buffer[0].ObjectByteSize(),
				buffer[i].toBytes().get(), buffer[0].ObjectByteSize());
		// write all data at once
		return _write(data.get(), size * buffer[0].ObjectByteSize()) / buffer[0].ObjectByteSize();
	}
	/** @brief Reads length amount of objects from the interface into pointer buffer
	 * @tparam T The pointer type
	 * @param buffer the pointer buffer
	 * @param length the amount of objects in the pointer (array)
	 * @return std::size_t the amount of T objects written */
	template<typename T> typename std::enable_if<
		std::is_pointer<T>::value && !is_container<T>::value, 
	std::size_t>::type 	read(T buffer, const std::size_t size) 		  { return _read((char*)buffer, size * sizeof(remPtrType<T>)) / sizeof(remPtrType<T>); }
	std::size_t 		read(iIOable* buffer, const std::size_t size) {
		// create buffer, and read into it
		auto data = std::make_unique<char[]>(size * buffer[0].ObjectByteSize());
		auto bytesread = _read(data.get(), size * buffer[0].ObjectByteSize());
		// copy buffered data into objects
		for(std::size_t i = 0; i < size; i++){
			auto data_temp = std::make_unique<char[]>(buffer[0].ObjectByteSize());
			std::memcpy(data_temp.get(), data.get() + i * buffer[0].ObjectByteSize(), buffer[0].ObjectByteSize());
			buffer[i].toObject(std::move(data_temp));
		}
		return bytesread / buffer[0].ObjectByteSize();
	}
	

	/** @brief Writes an array to the interface.
	 * @tparam T The type of the array
	 * @tparam size the size of the array
	 * @return std::size_t the amount of T objects written */
	template<typename T, std::size_t size> typename std::enable_if<
		!std::is_pointer<T>::value && !is_container<T>::value && !is_iterator<T>::value, 
	std::size_t>::type 	write(const T(&buffer)[size]) 		{ return _write((const char*)buffer, size * sizeof(T)) / sizeof(T); }
	template<typename T, std::size_t size> typename std::enable_if<
		std::is_pointer<T>::value && std::is_same<T, const char*>::value && !is_container<T>::value && !is_iterator<T>::value, 
	std::size_t>::type 	write(const T(&buffer)[size]) 		{ 
		std::size_t bytes_written = 0;
		for(std::size_t i = 0; i < size; i++)
			bytes_written += write(buffer[i], std::strlen(buffer[i]));
		return bytes_written;
	 }
	template<typename T, std::size_t size>
	std::size_t 		write(const iIOable(&buffer)[size]) { return write(buffer, size); }
	/** @brief Reads an array from the interface into the specified buffer.
	 * @tparam T the type of the array
	 * @tparam size the size of the array
	 * @return std::size_t the amount of T objects read */
	template<typename T, std::size_t size> typename std::enable_if<
		!is_container<T>::value, 
	std::size_t>::type 	read(T(&buffer)[size]) 		 { return _read((char*)buffer, size * sizeof(T)) / sizeof(T); }
	template<typename T, std::size_t size>
	std::size_t 		read(iIOable(&buffer)[size]) { return read(buffer, size); }


	/** @brief Writes an lvalue to the interface
	 * @tparam T The type of the lvalue
	 * @param buffer the lvalue
	 * @return std::size_t the amount of lvalue written */
	template<typename T> typename std::enable_if<
		!std::is_pointer<T>::value && !is_container<T>::value && !is_iterator<T>::value, 
	std::size_t>::type 	write(const T&  buffer) 	  { return _write((const char*)&buffer, sizeof(T)) / sizeof(T); }
	std::size_t 		write(const iIOable&  buffer) { return _write(buffer.toBytes().get(), buffer.ObjectByteSize()) / buffer.ObjectByteSize(); }
	/** @brief Reads an lvalue from the interface
	 * @tparam T The type of the lvalue
	 * @param buffer the lvalue
	 * @return std::size_t the amount of lvalue written */
	template<typename T> typename std::enable_if<
		!std::is_pointer<T>::value && !is_container<T>::value && !is_iterator<T>::value && !std::is_base_of<std::ios_base, T>::value, 
	std::size_t>::type 	read(T& buffer)		  { return _read((char*)&buffer, sizeof(T)) / sizeof(T); }
	std::size_t			read(iIOable& buffer) {
		auto data = std::make_unique<char[]>(buffer.ObjectByteSize());
		auto bytesread = _read(data.get(),   buffer.ObjectByteSize());
		buffer.toObject(std::move(data));
		return bytesread / buffer.ObjectByteSize();
	}


	//? ======== Container R/W wrappers ========>>==========================================================================================


	/** @brief Writes a container to the interface, executing a predicate for every element
	 * @tparam InputIt The iterator type
	 * @tparam Predicate The predicate function type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @param p the predicate function to call for every element in the container before sending it. Should return the a value and accept an element of container element type
	 * @return InputIt::iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<itertype<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type	write(InputIt first, InputIt last, std::function<itertype<InputIt>(itertype<InputIt> val)> p) {
		for(; first!=last; ++first)
			write(p(*first));
		return last;
	}
	/** @brief Writes a container to the interface, executing a predicate for every element read
	 * Useful when mutating a string after reading, or decoding input before storing the element
	 * @tparam InputIt The iterator type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @param m the mutator function to call for every element read from interface. Should accept container element type. Use to mutate read element before writing to container. Should return container's element type
	 * @return InputIt::iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<itertype<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type 	read(InputIt first, InputIt last, std::function<InputIt(itertype<InputIt> val)> m) {
		itertype<InputIt> iter_buffer;
		for(; first!=last; ++first)
			read(iter_buffer), *first = p(iter_buffer);
		return last;
	}
	/** @brief Writes a container to the interface, executing a mutator for every element read, and passing a reference of the current iterator to the predicate
	 * Useful when reading into a map or special container which needs skips after every read. Mutator m modifies value in place.
	 * @tparam InputIt The iterator type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @param m the mutator function to call for every element read from interface. Should accept container element type reference and iterator reference. Should return void
	 * @return InputIt::iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<itertype<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type 	read(InputIt first, InputIt last, std::function<void(itertype<InputIt> val, InputIt& curr)> m) {
		for(; first!=last; ++first)
			read(*first), m(*first, first);
		return last;
	}


	/** @brief Writes a container to the interface
	 * @tparam InputIt The iterator type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @return InputIt::iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<itertype<InputIt>>::value && is_iterator<InputIt>::value, 
	InputIt>::type	write(InputIt first, InputIt last) {
		for(; first!=last; ++first)
			write(*first);
		return last;
	}
	/** @brief Reads from the interface into a container
	 * @tparam InputIt The iterator type
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @return InputIt::iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<itertype<InputIt>>::value && is_iterator<InputIt>::value, 
	InputIt>::type 	read(InputIt first, InputIt last) {
		for(; first!=last; ++first)
			read(*first);
		return last;
	}
	
	/** @brief Writes an N dimensional container to the interface, unwrapping every dimension
	 * @tparam InputIt The iterator type 
	 * @param first iterator pointing to the start of range
	 * @param last  iterator pointing to the end of range
	 * @return InputIt::iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		is_container<itertype<InputIt>>::value && is_iterator<InputIt>::value, 
	InputIt>::type		write(InputIt first, InputIt last) {
		for(; first!=last; ++first)
			write(first->begin(), first->end()); // no need to check output as it will throw on error
		return last;
	}

private:
	template<typename BT, class predicate>
	std::size_t read_into_T(predicate p, const std::size_t maxlength = 0){
		std::size_t _maxlength = maxlength ? maxlength : std::numeric_limits<std::size_t>::max();
		std::size_t i = 0;
		for(; i < _maxlength; i++){
			BT buffer; 
			if(read(buffer) <= 0)
				break;
			p(buffer);
		}
		return i;
	}

public:

	/** @brief Reads into an ostream until no data available or length is reached
	 * @tparam OsT The OutputStream Type
	 * @tparam IT The InputType
	 * @param Container Stream to write to
	 * @param maxlength maximum amount of IT type to read
	 * @return std::size_t, the amount of bytes read */
	template<typename IT = char, typename OsT> constexpr typename std::enable_if<
		std::is_base_of<std::ios_base, OsT>::value && can_accept_stream<OsT, IT>::value, 
	std::size_t>::type	read(OsT& stream, const std::size_t maxlength = 0) {
		auto opout = [&](IT& buffer){stream << buffer;};
		return read_into_T<IT>(opout, maxlength);
	}

	/** @brief Reads into a container until no data available or length is reached
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param maxlength maximum amount of CT's type to read
	 * @return std::size_t, the amount of bytes read */
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, const std::size_t maxlength = 0) {
		auto pushback = [&](typename CT::value_type& buffer){Container.push_back(buffer);};
		return read_into_T<typename CT::value_type>(pushback, maxlength);
	}

	/** @brief Mutates read buffer before writing to container until no data available or length is reached
	 * @tparam BT The element type to read, by default equal to container element's type, otherwise needs to be explicitly defined
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param m	Mutator function, should accept buffer value type and return  aswell.
	 * @param maxlength maximum amount of CT's type to read
	 * @return std::size_t, the amount of bytes read */
	template<typename BT, typename CT> constexpr typename std::enable_if<
		true, //is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::function<typename CT::value_type(BT buf)> m, const std::size_t maxlength = 0){
		auto mut = [&](BT& buffer){Container.push_back(m(buffer));};
		return read_into_T<BT>(mut, maxlength);
	}

	template<typename CT> constexpr typename std::enable_if<
		true, //is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::function<typename CT::value_type(typename CT::value_type buf)> m, const std::size_t maxlength = 0){
		return read<typename CT::value_type>(Container, m, maxlength);
	}

	//? ======== String R/W wrappers ========>>==========================================================================================
	
	/** @brief Writes a string rvalue to the interface, including ending 0
	 * @tparam T Template to check if parameter is string
	 * @param _string the string object
	 * @return std::size_t the amount of characters in the string written */
	template<typename T> typename std::enable_if<
		std::is_same<T, std::string>::value, 
	std::size_t>::type	write(const T&& _string) { return write(_string.c_str(), _string.length()); }

	/** @brief Writes a string lvalue to the interface, including ending 0
	 * @tparam T Template to check if parameter is string
	 * @param _string the string object
	 * @return std::size_t the amount of characters in the string written */
	template<typename T> typename std::enable_if<
		std::is_same<T, std::string>::value, 
	std::size_t>::type	write(const T& _string) { return write(_string.c_str(), _string.length()); }
};