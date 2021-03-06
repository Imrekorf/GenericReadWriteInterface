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
#include <sstream>
#include <forward_list> // for specialization

#include <cxxabi.h>

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
	/**
	 * @brief Converts derived type to a byte array
	 * Byte array contains the data to be sent over the interface
	 * @return std::unique_ptr<const char[]> The bytes to be sent over the IO interface
	 */
	virtual std::unique_ptr<const char[]> toBytes() const = 0;

	/**
	 * @brief Converts byte array back into derived type
	 * Byte array contains the data that was sent over the IO interface
	 * Order of bytes is determined by toBytes() method
	 * @param data The byte array to convert
	 */
	virtual void toObject(const std::unique_ptr<char[]> data) = 0;

	/**
	 * @brief The size of the object in bytes, 
	 * IMPORTANT: This does not always equal sizeof(Type)!
	 * It is equal to the size of the data to be send in bytes.
	 * @return std::size_t The amount of bytes in the object
	 */
	virtual std::size_t ObjectByteSize() const = 0;
};

class iGIO {
public:
	class IOfailure : public std::exception{
		std::string _message;
	public:
		explicit IOfailure(const std::string& message) : _message(message){}
		virtual ~IOfailure(){}
		const char* what() const noexcept override {return _message.c_str();}
	};
protected:
	/** 
	 * @brief Reads length amount of bytes into the passed buffer
	 * Function has the following implementation requiredments
	 * IMPLEMENTATION: Reads a maximum of length bytes into buffer if length bytes are available
	 * IMPLEMENTATION: Returns the amount of bytes read into buffer
	 * IMPLEMENTATION: If no data is available to read the method should return 0.
	 * IMPLEMENTATION: Buffer is allocated before passing.
	 * IMPLEMENTATION: If an error occurs, use throw
	 * @param buffer The byte buffer to read into
	 * @param length The amount of bytes to read
	 * @return std::size_t The amount of bytes read 
	 */
	virtual std::size_t iRead(char* buffer, const std::size_t length) = 0;	
	
	/**
	 * @brief Writes length amount of bytes. Should throw on error
	 * IMPLEMENTATION: Writes a maximum of length bytes from buffer
	 * IMPLEMENTATION: Returns the amount of bytes written to interface
	 * IMPLEMENTATION: Buffer is allocated before passing.
	 * IMPLEMENTATION: If an error occurs, use throw
	 * @param buffer The buffer containing the bytes to write
	 * @param length The amount of bytes to write
	 * @return std::size_t The amount of bytes written
	 */
	virtual std::size_t iWrite(const char* buffer, const std::size_t length) = 0;

	/**
	 * @brief Amount of bytes read from iRead() method between terminator comparison checks
	 */
	const std::size_t TermBytesRead = 1;

	/**
	 * @brief Reads length amount of bytes into the passed buffer, 
	 * stopping when the buffer contains a terminator character or has reached length.
	 * Should be overwritten for optimized performance;
	 * If the only optimisation that can be made is to read more than 1 byte between each
	 * terminator check. Then set the TermBytesRead member value correctly
	 * REQUIREMENT: Has the same implemenation requirements as the default iRead method
	 * @param buffer The buffer to read bytes into.
	 * @param terminator  The terminator byte array to end on.
	 * @param term_length The length of the terminator byte array.
	 * @param max_length The amount of bytes to read if terminator is not found.
	 * @return std::size_t The amount of bytes read.
	 */
	virtual std::size_t iRead_until(char* buffer, const char* terminator, const std::size_t term_length, const std::size_t max_length = 0){
		std::size_t _max_length = max_length ? max_length : std::numeric_limits<std::size_t>::max();
		std::size_t i = iRead(buffer, term_length);
		for(std::size_t read_bytes = i, j = 0; read_bytes && i < _max_length; i+=read_bytes){
			if(i >= term_length && term_length){ // we have enough data to check for a comparison
				if(buffer[i-1] == terminator[j]){
					if(j++ == term_length-1){
						break;
					}
				}else
					j = 0; // reset comparison check
			}
			
			read_bytes = iRead(&buffer[i], TermBytesRead); // Set TermBytesRead to the desired number of bytes, default 1
		}
		return i;
	}

	/**
	 * @brief The byte sequence used in line endings
	 */
	const char* LineEnder = "\n";

	/**
	 * @brief Custom function for flushing the interface
	 * By default implemented as doing nothing, gets called by endl()
	 */
	static iGIO& flush(iGIO& ref){
		return ref;
	}

	/**
	 * @brief Custom function for endl handling
	 * Writes the LineEnder to the interface and flushes afterwards
	 */
	static iGIO& endl(iGIO& ref){
		ref.write(ref.LineEnder);
		return flush(ref);
	}

private:
	int _read(char* buffer, const std::size_t length){
		return iRead(buffer, length);
	}
	int _read_until(char* buffer, const char* terminator, const std::size_t term_length, std::size_t max_length){
		return iRead_until(buffer, terminator, term_length, max_length);
	}
	int _write(const char* buffer, const std::size_t length) {
		return iWrite(buffer, length);
	}

	// ----------------------------------------------------------------
	// check if something is a container, based from: https://stackoverflow.com/a/9407521
	template<typename Type> static std::true_type  has_const_iterator_test(typename Type::const_iterator*); // if it does have a const_iterator, return true value
	template<typename Type> static std::false_type has_const_iterator_test(...); // if not have a default function
	template<class Type> using has_const_iterator = decltype(has_const_iterator_test<Type>(0));

	template<typename Type, typename = decltype(std::declval<Type&>().begin())>
	static std::true_type  has_begin_iterator_test(const Type&);
	static std::false_type has_begin_iterator_test(...);
	template<class Type> using has_begin_iterator = decltype(has_begin_iterator_test(std::declval<Type>()));

	template<typename Type, typename = decltype(std::declval<Type&>().end())>
	static std::true_type  has_end_iterator_test(const Type&);
	static std::false_type has_end_iterator_test(...);
	template<class Type> using has_end_iterator = decltype(has_end_iterator_test(std::declval<Type>()));

	template<typename Type, typename = decltype(std::declval<Type&>().push_back(*std::declval<Type&>().begin()))>
	static std::true_type  has_pushback_test(const Type&);
	static std::false_type has_pushback_test(...);
	template<class Type> using has_pushback = decltype(has_pushback_test(std::declval<Type>()));

	template<typename Type, typename = decltype(std::declval<Type&>().push_front(*std::declval<Type&>().begin()))>
	static std::true_type  has_pushfront_test(const Type&);
	static std::false_type has_pushfront_test(...);
	template<class Type> using has_pushfront = decltype(has_pushfront_test(std::declval<Type>()));

	template<typename Type, typename IT, typename = decltype(std::declval<Type&>() << std::declval<IT>())>
	static std::true_type  can_accept_stream_test(const Type&, const IT&);
	static std::false_type can_accept_stream_test(...);
	template<class Type, class IT> using can_accept_stream = decltype(can_accept_stream_test(std::declval<Type>(), std::declval<IT&>()));

	template<typename Type, typename IT, typename = decltype(std::declval<Type&>() >> std::declval<IT&>())>
	static std::true_type  can_extract_to_test(const Type&, const IT&);
	static std::false_type can_extract_to_test(...);
	template<class Type, class IT> using can_extract_to = decltype(can_extract_to_test(std::declval<Type>(), std::declval<IT>()));
	
	template<typename Type, typename = decltype(std::declval<typename Type::container_type&>())>
	static std::true_type  is_cont_adapter_test(const Type&); // if it does have a container_type, return true value
	static std::false_type is_cont_adapter_test(...); // if not have a default function
	template<class Type> using is_container_adapter = decltype(is_cont_adapter_test(std::declval<Type>()));

	template<typename Type, typename = decltype(Type::operator())>
	static std::true_type  is_callable_test(const Type&); // if it does have a container_type, return true value
	static std::false_type is_callable_test(...); // if not have a default function
	template<class Type> using is_callable = decltype(is_callable_test(std::declval<Type>()));

	template<typename Type> using is_container = std::integral_constant<bool, has_const_iterator<Type>::value && has_begin_iterator<Type>::value && has_end_iterator<Type>::value || is_container_adapter<Type>::value>;
	template<typename Type> using is_string = std::is_same<std::basic_string<typename Type::value_type, typename Type::traits_type, typename Type::allocator_type>, Type>;
	template<typename Type> using is_stream = std::is_base_of<std::ios_base, Type>;
	
	// is_iterator only returns true for ::iterator types, not pointers
	template <class Type, class = void>
	struct is_iterator : std::false_type { };
	template <class Type>
	struct is_iterator<Type, typename std::enable_if<!std::is_pointer<Type>::value && !std::is_same<typename std::iterator_traits<Type>::value_type, void>::value, void>::type> : std::true_type { };
	// ----------------------------------------------------------------

	// readability
	template<typename Type> 	using remPtrType = typename std::remove_pointer<Type>::type;
	template<typename Type> 	using remArrType = typename std::remove_extent<Type>::type;
	template<typename InputIt>	using iterType   = typename std::iterator_traits<InputIt>::value_type;
	template<typename CT> 		using CElemType  = typename CT::value_type;

public:
	virtual ~iGIO(){}

	//? ======== Base read and write wrappers ========>>==========================================================================================
	//**** RValue
	/** @brief Writes an rvalue Type to the interface
	 * SUPPORTS: Any rvalue excluding pointers and arrays, containers, iterators and streams
	 * @tparam Type The type of the rvalue
	 * @param buffer The rvalue
	 * @return std::size_t The amount of rvalue written, 1 or 0 */
	template<typename Type> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value && !is_stream<Type>::value && !std::is_array<Type>::value, 
	std::size_t>::type	write(const Type&& buffer)		  { return _write((const char*)&buffer, sizeof(Type)) / sizeof(Type); }
	std::size_t			write(const iIOable&& buffer) { return _write(buffer.toBytes().get(), buffer.ObjectByteSize()) / buffer.ObjectByteSize(); }
	// note: no rvalue read as it doesn't make sense.

	//**** Pointer
	/** @brief Writes a pointer buffer of size length to the interface
	 * SUPPORTS: Any pointer excluding pointer pointers, pointers to arrays, containers, pointers to containers and iterators
	 * @tparam Type The pointer type
	 * @param buffer The pointer buffer
	 * @param length The amount of objects in the pointer (array)
	 * @return std::size_t The amount of Type objects written */
	template<typename Type> typename std::enable_if<
		std::is_pointer<Type>::value && !std::is_pointer<remPtrType<Type>>::value && !is_container<Type>::value && !is_container<remPtrType<Type>>::value && !std::is_array<remPtrType<Type>>::value && !is_iterator<Type>::value && !is_stream<remPtrType<Type>>::value,
	std::size_t>::type	write(const Type  buffer, const std::size_t&& size) 	   { return _write((const char*)buffer, size * sizeof(remPtrType<Type>)) / sizeof(remPtrType<Type>); }
	std::size_t 	  	write(const iIOable* buffer, const std::size_t&& size) {
		auto data = std::make_unique<char[]>(size * buffer[0].ObjectByteSize());
		// copy pointer buffer into byte buffer
		for(std::size_t i = 0; i < size; i++)
			std::memcpy(data.get() + i * buffer[0].ObjectByteSize(),
				buffer[i].toBytes().get(), buffer[0].ObjectByteSize());
		// write all data at once
		return _write(data.get(), size * buffer[0].ObjectByteSize()) / buffer[0].ObjectByteSize();
	}
	std::size_t	 	  	write(const char* string) { return _write(string, std::strlen(string)); }


	/** @brief Reads length amount of objects from the interface into pointer buffer
	 * SUPPORTS: Any pointer excluding pointer pointers, pointers to arrays, containers, pointers to containers and iterators
	 * @tparam Type The pointer type
	 * @param buffer The pointer buffer
	 * @param length The amount of objects in the pointer (array)
	 * @return std::size_t The amount of Type objects written */
	template<typename Type> typename std::enable_if<
		std::is_pointer<Type>::value && !std::is_pointer<remPtrType<Type>>::value && !is_container<Type>::value && !is_container<remPtrType<Type>>::value && !std::is_array<remPtrType<Type>>::value && !is_iterator<Type>::value && !is_stream<remPtrType<Type>>::value, 
	std::size_t>::type 	read(Type buffer, const std::size_t size) 		  { return _read((char*)buffer, size * sizeof(remPtrType<Type>)) / sizeof(remPtrType<Type>); }
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
	
	/** @brief Reads until either length is reached or terminator is reached
	 * SUPPORTS: buffer: Any pointer excluding pointer pointers, pointers to arrays, containers, pointers to containers and iterators
	 * SUPPORTS: terminator: any lvalue excluding pointers, containers, arrays and iterators
	 * @tparam Type The buffer type
	 * @tparam Type2 The terminator type
	 * @param buffer The buffer to store into, preallocated
	 * @param terminator The to search for terminator
	 * @param maxlength The maximum amount of Type elements to read
	 * @return sts::size_t The amount of Type elements read */
	template<typename Type> typename std::enable_if<
		std::is_pointer<Type>::value && !std::is_pointer<remPtrType<Type>>::value && !is_container<Type>::value && !is_container<remPtrType<Type>>::value && !std::is_array<remPtrType<Type>>::value && !is_iterator<Type>::value && !is_stream<remPtrType<Type>>::value, 
	std::size_t>::type	read_until(Type buffer, const remPtrType<Type>& terminator, const std::size_t maxlength) {
		return _read_until((char*)buffer, (char*)&terminator, sizeof(remPtrType<Type>), maxlength * sizeof(remPtrType<Type>)) / sizeof(Type);
	}
	template<typename Type, typename Type2> typename std::enable_if<
		std::is_pointer<Type>::value && !std::is_pointer<remPtrType<Type>>::value && !is_container<Type>::value && !is_container<remPtrType<Type>>::value && !std::is_array<remPtrType<Type>>::value && !is_iterator<Type>::value && !is_stream<remPtrType<Type>>::value &&
		std::is_pointer<Type2>::value && !is_container<Type2>::value && !std::is_array<Type2>::value && !is_iterator<Type2>::value && !is_stream<Type2>::value, 
	std::size_t>::type	read_until(Type buffer, const Type2& terminator, const std::size_t maxlength) {
		return _read_until((char*)buffer, (char*)&terminator, sizeof(Type2), maxlength * sizeof(remPtrType<Type>)) / sizeof(Type);
	}
	template<typename Type> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !std::is_array<remPtrType<Type>>::value && !is_iterator<Type>::value && !is_stream<remPtrType<Type>>::value, 
	std::size_t>::type	read_until(iIOable* buffer, const Type& terminator, const std::size_t maxlength) {
		// create buffer, and read into it
		auto data = std::make_unique<char[]>(maxlength * buffer[0].ObjectByteSize());
		auto bytesread = _read_until(data.get(), (char*)&terminator, sizeof(terminator), maxlength * buffer[0].ObjectByteSize());
		// copy buffered data into objects
		for(std::size_t i = 0; i < maxlength; i++){
			auto data_temp = std::make_unique<char[]>(buffer[0].ObjectByteSize());
			std::memcpy(data_temp.get(), data.get() + i * buffer[0].ObjectByteSize(), buffer[0].ObjectByteSize());
			buffer[i].toObject(std::move(data_temp));
		}
		return bytesread / buffer[0].ObjectByteSize();
	}
	std::size_t 		read_until(iIOable* buffer, const iIOable& terminator, const std::size_t maxlength) {
		// create buffer, and read into it
		auto data = std::make_unique<char[]>(maxlength * buffer[0].ObjectByteSize());
		auto bytesread = _read_until(data.get(), terminator.toBytes().get(), terminator.ObjectByteSize(), maxlength * buffer[0].ObjectByteSize());
		// copy buffered data into objects
		for(std::size_t i = 0; i < maxlength; i++){
			auto data_temp = std::make_unique<char[]>(buffer[0].ObjectByteSize());
			std::memcpy(data_temp.get(), data.get() + i * buffer[0].ObjectByteSize(), buffer[0].ObjectByteSize());
			buffer[i].toObject(std::move(data_temp));
		}
		return bytesread / buffer[0].ObjectByteSize();
	}	

	//**** Array
	/** @brief Writes an array to the interface
	 * if type Type is a signed character then it is treated as a c-style string 
	 * where ending 0 is omitted, if it is desired that the ending 0 is written then set write_ending_0 to 1
	 * SUPPORTS: Any array excluding pointer arrays, multidimensional arrays, containers and iterators
	 * @tparam Type The type of the array
	 * @tparam size The size of the array
	 * @param buffer The buffer to write
	 * @param write_ending_0 boolean indicating if ending 0 should be written, only implemented for c-style strings
	 * @return std::size_t The amount of Type objects written */
	template<typename Type, std::size_t size> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value  && !std::is_array<Type>::value && !is_stream<Type>::value, 
	std::size_t>::type 	write(const Type(&buffer)[size]) 		{ return _write((const char*)buffer, size * sizeof(Type)) / sizeof(Type); }
	template<std::size_t size>
	std::size_t		 	write(const char(&buffer)[size], const bool write_ending_0 = 0) { return _write((const char*)buffer, size-(!write_ending_0)); }
	template<std::size_t size>
	std::size_t 		write(const iIOable(&buffer)[size]) { return write(buffer, size); }

	/** @brief Writes an array of arrays to the interface
	 * SUPPORTS: Any pointer array excluding multidimensional arrays, containers and iterators
	 * @tparam Type The type of the array
	 * @tparam size The size of the array
	 * @param buffer The buffer to write
	 * @param elementsize the size of the arrays inside the array
	 * @param write_ending_0 boolean indicating if ending 0 should be written, only implemented for c-style strings
	 * @return std::size_t The amount of Type objects written */
	template<typename Type, std::size_t size> typename std::enable_if<
		std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value  && !std::is_array<Type>::value && !is_stream<Type>::value,
	std::size_t>::type 	write(const Type(&buffer)[size], const std::size_t elementsize) { 
		// writes array of pointer arrays with same size to the interface
		std::size_t types_written = 0;
		for(std::size_t i = 0; i < size; i++)
			types_written += write(buffer[i], elementsize) / sizeof(Type);
		return types_written;
	}
	
	/** @brief Reads an array from the interface into the specified buffer
	 * SUPPORTS: Any array excluding pointer arrays, multidimensional arrays, containers and iterators
	 * @tparam Type The type of the array
	 * @tparam size The size of the array
	 * @param buffer The buffer to read into
	 * @return std::size_t The amount of Type objects read */
	template<typename Type, std::size_t size> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value  && !std::is_array<Type>::value && !is_stream<Type>::value, 
	std::size_t>::type 	read(Type(&buffer)[size]) 	 { return _read((char*)buffer, size * sizeof(Type)) / sizeof(Type); }
	template<std::size_t size>
	std::size_t 		read(iIOable(&buffer)[size]) { return read(buffer, size); }
	
	/** @brief Reads arrays of elementsize from the interface into the specified array
	 * SUPPORTS: Any pointer array excluding multidimensional arrays, containers and iterators
	 * @tparam Type The type of the array
	 * @tparam size The size of the array
	 * @param buffer The buffer to read into
	 * @param elementsize the size of the arrays inside the array
	 * @return std::size_t The amount of Type objects read */
	template<typename Type, std::size_t size> typename std::enable_if<
		std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value  && !std::is_array<Type>::value && !is_stream<Type>::value, 
	std::size_t>::type 	read(Type(&buffer)[size], const std::size_t elementsize) { 
		// reads array of pointer arrays
		std::size_t types_read = 0;
		for(std::size_t i = 0; i < size; i++){
			types_read = read(buffer[i], elementsize);
		}
		return types_read; 
	}

	/** @brief Reads until either array size is reached or terminator is reached
	 * SUPPORTS: Any array excluding pointer arrays, multidimensional arrays, containers and iterators
	 * @tparam Type The buffer type
	 * @param buffer The buffer to store into, preallocated
	 * @param terminator The terminator to search for terminator
	 * @return sts::size_t The amount of Type read */
	template<typename Type, std::size_t size> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value && !std::is_array<Type>::value && !is_stream<Type>::value,
	std::size_t>::type	read_until(Type(&buffer)[size], const Type& terminator) { return read_until(&buffer[0], terminator, size * sizeof(Type)); }
	template<typename Type, typename Type2, std::size_t size> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value && !std::is_array<Type>::value && !is_stream<Type>::value &&
		!std::is_pointer<Type2>::value && !is_container<Type2>::value && !is_iterator<Type2>::value && !std::is_array<Type2>::value && !is_stream<Type2>::value,
	std::size_t>::type	read_until(Type(&buffer)[size], const Type2& terminator) { return read_until(&buffer[0], terminator, size * sizeof(Type)); }
	template<std::size_t size, typename TT> typename std::enable_if<
		!std::is_pointer<TT>::value && !is_container<TT>::value && !is_iterator<TT>::value && !std::is_array<TT>::value && !is_stream<TT>::value,
	std::size_t>::type	read_until(iIOable(&buffer)[size], const TT& terminator) { return read_until(&buffer[0], terminator, size * buffer[0].ObjectByteSize()); }
	template<std::size_t size>
	std::size_t 		read_until(iIOable(&buffer)[size], const iIOable& terminator) { return read_until(&buffer[0], terminator, size * buffer[0].ObjectByteSize());	}	

	//**** lvalue
	/** @brief Writes an lvalue to the interface
	 * SUPPORTS: Every lvalue excluding pointers, arrays, containers, iterators and streams
	 * @tparam Type The type of the lvalue
	 * @param buffer The lvalue
	 * @return std::size_t The amount of lvalue written, 1 or 0 */
	template<typename Type> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value && !is_stream<Type>::value && !std::is_array<Type>::value, 
	std::size_t>::type 	write(const Type& buffer) 	  	 { return _write((const char*)&buffer, sizeof(Type)) / sizeof(Type); }
	std::size_t 		write(const iIOable& buffer) { return _write(buffer.toBytes().get(), buffer.ObjectByteSize()) / buffer.ObjectByteSize(); }
	
	/** @brief Reads an lvalue from the interface
	 * SUPPORTS: Every lvalue excluding pointers, arrays, containers, iterators and streams
	 * @tparam Type The type of the lvalue
	 * @param buffer The lvalue
	 * @return std::size_t The amount of lvalue written, 1 or 0 */
	template<typename Type> typename std::enable_if<
		!std::is_pointer<Type>::value && !is_container<Type>::value && !is_iterator<Type>::value && !is_stream<Type>::value && !std::is_array<Type>::value, 
	std::size_t>::type 	read(Type& buffer)		  { return _read((char*)&buffer, sizeof(Type)) / sizeof(Type); }
	std::size_t			read(iIOable& buffer) {
		auto data = std::make_unique<char[]>(buffer.ObjectByteSize());
		auto bytesread = _read(data.get(),   buffer.ObjectByteSize());
		buffer.toObject(std::move(data));
		return bytesread / buffer.ObjectByteSize();
	}


	//? ======== Container range R/W wrappers ========>>==========================================================================================

	/** @brief Writes a container to the interface, executing a predicate for every element
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @tparam Predicate The predicate function type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @param p The predicate function to call for every element in the container before sending it. Should return the a value and accept an element of container element type
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type	write(InputIt first, InputIt last, std::function<iterType<InputIt>(iterType<InputIt> val)> p) {
		for(; first!=last; ++first)
			write(p(*first));
		return last;
	}
	/** @brief Reads from an interface into a container, executing a mutator for every element read
	 * Useful when mutating a string after reading, or decoding input before storing the element
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @param mutator The mutator function to call for every element read from interface. 
	 * 			Should accept container element type. Use to mutate read element before writing to container. 
	 * 			Should return container's element type
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type 	read(InputIt first, InputIt last, std::function<iterType<InputIt>(iterType<InputIt> val)> mutator) {
		iterType<InputIt> iter_buffer;
		for(; first!=last; ++first){
			if(!read(iter_buffer))
				break;
			*first = mutator(iter_buffer);
		}
		return last;
	}
	/** @brief Reads from an interface into a container, until either terminator is found or 
	 * end of range was reached, executing a mutator for every element read
	 * Useful when mutating a string after reading, or decoding input before storing the element
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @param terminator The terminator to read until
	 * @param mutator The mutator function to call for every element read from interface. Should accept container element type. Use to mutate read element before writing to container. Should return container's element type
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type 	read_until(InputIt first, InputIt last, const iterType<InputIt>& terminator, std::function<InputIt(iterType<InputIt> val)> mutator) {
		iterType<InputIt> iter_buffer;
		for(; first!=last; ++first){
			if(!read(iter_buffer))
				break;
			*first = mutator(iter_buffer);
			if(iter_buffer == terminator)
				break;
		}
		return last;
	}
	
	/** @brief Reads from an interface into a container, executing a mutator for every element read, and passing a reference of the current iterator to the mutator
	 * Useful when reading into a map or special container which needs skips after every read. The mutator modifies value in place
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @param mutator The mutator function to call for every element read from interface. Should accept container element type reference and iterator reference. Should return void
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type 	read(InputIt first, InputIt last, std::function<void(iterType<InputIt> val, InputIt& curr)> mutator) {
		for(; first!=last; ++first){
			if(!read(*first))
				break;
			mutator(*first, first);
		}
		return last;
	}
	/** @brief Reads from an interface into a container, until either terminator is found or end of range was reached,
	 * executing a mutator for every element read, and passing a reference of the current iterator to the mutator
	 * Useful when reading into a map or special container which needs skips after every read. The mutator modifies value in place
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @param terminator The terminator to read until
	 * @param mutator The mutator function to call for every element read from interface. Should accept container element type reference and iterator reference. Should return void
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value,
	InputIt>::type 	read_until(InputIt first, InputIt last, const iterType<InputIt>& terminator, std::function<void(iterType<InputIt> val, InputIt& curr)> mutator) {
		for(; first!=last; ++first){
			if(!read(*first))
				break;
			iterType<InputIt>& iter_buffer = *first;
			mutator(*first, first);
			if(iter_buffer == terminator)
				break;
		}
		return last;
	}


	/** @brief Writes a container to the interface
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value, 
	InputIt>::type	write(InputIt first, InputIt last) {
		for(; first!=last; ++first)
			write(*first);
		return last;
	}
	/** @brief Reads from the interface into a container
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value, 
	InputIt>::type 	read(InputIt first, InputIt last) {
		for(; first!=last; ++first)
			if(!read(*first))
				break;
		return last;
	}
	/** @brief Reads from the interface into a container until the terminator is reached or end of range is reached
	 * SUPPORTS: Any iterator type excluding N-dimensional container iterators
	 * @tparam InputIt The iterator type
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @param terminator The terminator to read until
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		!is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value, 
	InputIt>::type 	read_until(InputIt first, InputIt last, const iterType<InputIt>& terminator) {
		for(; first!=last; ++first){
			if(!read(*first))
				break;
			if(*first == terminator)
				break;
		}
		return last;
	}
	
	/** @brief Writes an N dimensional container to the interface, unwrapping every dimension
	 * SUPPORTS: Any iterator type
	 * @tparam InputIt The iterator type 
	 * @param first Iterator pointing to the start of range
	 * @param last  Iterator pointing to the end of range
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename InputIt> constexpr typename std::enable_if<
		is_container<iterType<InputIt>>::value && is_iterator<InputIt>::value, 
	InputIt>::type	write(InputIt first, InputIt last) {
		for(; first!=last; ++first)
			write(first->begin(), first->end()); // no need to check output as it will throw on error
		return last;
	}

private:
	template<typename BT, class predicate>
	std::size_t read_into_T(predicate p, std::size_t maxlength = 0){
		maxlength = maxlength ? maxlength : std::numeric_limits<std::size_t>::max();
		std::size_t i = 0;
		for(; i < maxlength; i++){
			BT buffer; 
			if(!read(buffer)) // acts recursively if BT is std::string
				break;
			if(!p(buffer)){
				break;
			}
		}
		return i;
	}

	template<typename BT, class predicate>
	std::size_t read_into_T_until(predicate p, const BT& terminator, std::size_t maxlength = 0){
		maxlength = maxlength ? maxlength : std::numeric_limits<std::size_t>::max();
		std::size_t i = 0;
		for(; i < maxlength; i++){
			BT buffer; 
			if(!read(buffer))
				break;
			if(!p(buffer)){
				break;
			}
			if(buffer == terminator)
				break;
		}
		return i;
	}
public:
	//? ======== Container Reference R/W wrappers ========>>==========================================================================================

	/** @brief Writes a container to the interface
	 * SUPPORTS: Any container except containers containing pointers or container adapters
	 * @tparam InputIt The iterator type
	 * @param Container The container to write
	 * @return InputIt::iterator Iterator pointing to the last element send */
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && !std::is_pointer<CElemType<CT>>::value && !is_container_adapter<CT>::value, 
	std::size_t>::type	write(CT& Container) {
		auto iterlast = write(Container.begin(), Container.end());
		if(iterlast == Container.end())
			return Container.size();
		return std::distance(Container.begin(), iterlast);
	}
	template<typename T>
	std::size_t write(std::forward_list<T>& Container){
		auto iterlast = write(Container.begin(), Container.end());
		return std::distance(Container.begin(), iterlast);
	}

	//* Push_back based

	/** @brief Reads into a container until no data available or length is reached
	 * SUPPORTS: Any container that supports the .push_back() method
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of CT's type read */
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::size_t maxlength = 0) {
		auto pushback = [&](CElemType<CT>& buffer){Container.push_back(buffer); return true;};
		return read_into_T<CElemType<CT>>(pushback, maxlength);
	}
	/** @brief Reads into a container until no data available, terminator is found or length is reached
	 * SUPPORTS: Any container that supports the .push_back() method
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param terminator The terminator to search for
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of CT's type read */
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read_until(CT& Container, const CElemType<CT>& terminator, std::size_t maxlength = 0) {
		auto pushback = [&](CElemType<CT>& buffer){Container.push_back(buffer); return true;};
		return read_into_T_until<CElemType<CT>>(pushback, terminator, maxlength);
	}

	//* push_front based

	/** @brief Reads into a container until no data available or length is reached
	 * SUPPORTS: Any container that supports the .push_front() method but not the .push_back() method
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of CT's type read */
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushfront<CT>::value && !has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::size_t maxlength = 0) {
		auto pushback = [&](CElemType<CT>& buffer){Container.push_front(buffer); return true;};
		return read_into_T<CElemType<CT>>(pushback, maxlength);
	}
	/** @brief Reads into a container until no data available, terminator is found or length is reached
	 * SUPPORTS: Any container that supports the .push_front() method but not the .push_back() method
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param terminator The terminator to search for
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of CT's type read */
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushfront<CT>::value && !has_pushback<CT>::value, 
	std::size_t>::type	read_until(CT& Container, const CElemType<CT>& terminator, std::size_t maxlength = 0) {
		auto pushback = [&](CElemType<CT>& buffer){Container.push_front(buffer); return true;};
		return read_into_T_until<CElemType<CT>>(pushback, terminator, maxlength);
	}

	//* push_back mutator based

	/** @brief Mutates read buffer before writing to container until no data available or length is reached
	 * SUPPORTS: Any container that supports the .push_back() method
	 * @tparam BT The element type to read, by default equal to container element's type, otherwise needs to be explicitly defined
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param mutator	Mutator function, should accept buffer value type and return  aswell
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of BT elements read */
	template<typename BT, typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::function<CElemType<CT>(BT& buf)> mutator, std::size_t maxlength = 0){
		auto mut = [&](BT& buffer){Container.push_back(mutator(buffer)); return true;};
		return read_into_T<BT>(mut, maxlength);
	}
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::function<CElemType<CT>(CElemType<CT>& buf)> mutator, std::size_t maxlength = 0){
		return read<CElemType<CT>>(Container, mutator, maxlength);
	}
	
	/** @brief Mutates read buffer before writing to container until no data available, terminator is found or length is reached
	 * SUPPORTS: Any container that supports the .push_back() method
	 * @tparam BT The element type to read
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param terminator The terminator to search for
	 * @param mutator	Mutator function, should accept buffer value type and return  aswell
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of BT elements read */
	template<typename BT, typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read_until(CT& Container, const BT& terminator, std::function<CElemType<CT>(BT& buf)> mutator, std::size_t maxlength = 0){
		auto mut = [&](BT& buffer){Container.push_back(mutator(buffer)); return true;};
		return read_into_T_until<BT>(mut, terminator, maxlength);
	}
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushback<CT>::value, 
	std::size_t>::type	read_until(CT& Container, const CElemType<CT>& terminator, std::function<CElemType<CT>(CElemType<CT>& buf)> mutator, std::size_t maxlength = 0){
		return read_until<CElemType<CT>>(Container, terminator, mutator, maxlength);
	}

	//* push_front mutator based

	/** @brief Mutates read buffer before writing to container until no data available or length is reached
	 * SUPPORTS: Any container that supports the .push_front() method but not the .push_back() method
	 * @tparam BT The element type to read, by default equal to container element's type, otherwise needs to be explicitly defined
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param mutator	Mutator function, should accept buffer value type and return  aswell
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of BT elements read */
	template<typename BT, typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushfront<CT>::value && !has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::function<CElemType<CT>(BT& buf)> mutator, std::size_t maxlength = 0){
		auto mut = [&](BT& buffer){Container.push_front(mutator(buffer)); return true;};
		return read_into_T<BT>(mut, maxlength);
	}
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushfront<CT>::value && !has_pushback<CT>::value, 
	std::size_t>::type	read(CT& Container, std::function<CElemType<CT>(CElemType<CT>& buf)> mutator, std::size_t maxlength = 0){
		return read<CElemType<CT>>(Container, mutator, maxlength);
	}

	/** @brief Mutates read buffer before writing to container until no data available, terminator is found or length is reached, using the insert function to add data to the container.
	 * SUPPORTS: Any container that supports the .push_front() method but not the .push_back() method
	 * @tparam BT The element type to read
	 * @tparam CT The container type
	 * @param Container Container to read into
	 * @param terminator The terminator to search for
	 * @param mutator	Mutator function, should accept buffer value type and return  aswell
	 * @param maxlength Maximum amount of CT's type to read
	 * @return std::size_t The amount of BT elements read */
	template<typename BT, typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushfront<CT>::value && !has_pushback<CT>::value, 
	std::size_t>::type	read_until(CT& Container, const BT& terminator, std::function<CElemType<CT>(BT& buf)> mutator, std::size_t maxlength = 0){
		auto mut = [&](BT& buffer){Container.push_front(mutator(buffer)); return true;};
		return read_into_T_until<BT>(mut, terminator, maxlength);
	}
	template<typename CT> constexpr typename std::enable_if<
		is_container<CT>::value && has_pushfront<CT>::value && !has_pushback<CT>::value, 
	std::size_t>::type	read_until(CT& Container, const CElemType<CT>& terminator, std::function<CElemType<CT>(CElemType<CT>& buf)> mutator, std::size_t maxlength = 0){
		return read_until<CElemType<CT>>(Container, terminator, mutator, maxlength);
	}

	//? ======== String R/W wrappers ========>>==========================================================================================
	
	/** @brief Writes a string rvalue to the interface, excluding ending 0
	 * SUPPORTS: any string type
	 * @tparam Type Template to check if parameter is string
	 * @param _string The string object
	 * @return std::size_t The amount of characters in the string written */
	template<typename Type> typename std::enable_if<
		is_string<Type>::value, 
	std::size_t>::type	write(const Type&& _string) { return write(_string.c_str(), _string.length()); }

	/** @brief Writes a string lvalue to the interface, excluding ending 0
	 * SUPPORTS: any string type
	 * @tparam Type Template to check if parameter is string
	 * @param _string The string object
	 * @return std::size_t The amount of characters in the string written */
	template<typename Type> typename std::enable_if<
		is_string<Type>::value, 
	std::size_t>::type	write(const Type& _string) { return write(_string.c_str(), _string.length()); }

	/** @brief Reads the interface into a string reference until terminator or maxlength is reached
	 * SUPPORTS: any string type
	 * @tparam Type Template to check if parameter is string
	 * @param _string The string object
	 * @return std::size_t The amount of characters in the string written */
	template<typename Type> typename std::enable_if<
		is_string<Type>::value,
	std::size_t>::type	read_until(const Type& _string, const CElemType<Type>& terminator, std::size_t maxlength = 0) { 
		auto StrPushLambda = [&](CElemType<Type>& _elem){_string.push_back(_elem);};
		return read_into_T_until(StrPushLambda, terminator, maxlength);
	}

	//? ======== Stream R/W wrappers ========>>==========================================================================================

	/** @brief Writes from an istream until end of line.
	 * Stream will be split based on whitespaces into a buffer of strings.
	 * EXAMPLE: The stringstream "Hello world!" will be extracted into two 
	 * seperate buffers containing "Hello" and "world!", which will individually 
	 * be sent over the interface. To add the whitespace set the seperator to " "
	 * SUPPORTS: Any stream that supports stream extraction to IT
	 * @tparam IT The InputType
	 * @tparam IsT The InputStream Type
	 * @param stream The stream to write from
	 * @param seperator The seperator to insert between writes, 
	 * 		e.g. "hello world" would normally write as "hello", "world" with a seperator it would write as: "hello", sep, "world"
	 * @return std::size_t The number of IT elements written */
	template<typename IsT> constexpr typename std::enable_if<
		std::is_base_of<std::ios_base, IsT>::value && can_extract_to<IsT, std::string>::value,
	std::size_t>::type	write(IsT& stream, std::string seperator = "") {
		// boolean to keep track for if the stream is cin, and it has already been read
		// Needed as otherwise reading will continue indefinitely
		bool readcin = false;
		std::size_t written_ITs = 0;
		std::string line;
		std::getline(stream, line);
		std::istringstream iss(line);
		while(!iss.eof() && !iss.fail() && !readcin){ // check for end of stream
			std::string buffer;
			iss >> buffer;
			if(written_ITs)
				write(seperator);
			write(buffer);
			written_ITs++;
		};
		return written_ITs;
	}

	/** @brief Reads into an ostream until no data available or length is reached
	 * SUPPORTS: Any stream that supports IT type stream insertion
	 * @tparam IT The InputType
	 * @tparam OsT The OutputStream Type
	 * @param stream Stream to write to
	 * @param maxlength Maximum amount of IT type to read
	 * @return std::size_t The amount of IT elements read */
	template<typename IT = std::string, typename OsT> constexpr typename std::enable_if<
		std::is_base_of<std::ios_base, OsT>::value && can_accept_stream<OsT, IT>::value, 
	std::size_t>::type	read(OsT& stream, std::size_t maxlength = 0) {
		auto opout = [&](IT& buffer){stream << buffer; return stream.good();};
		return read_into_T<IT>(opout, maxlength);
	}
	
	//? ======== operator R/W wrappers ========>>==========================================================================================
	//* ======== operator>> and operator << overloads ========>>===========================================================================

	/** @brief Writes a Type object to the interface
	 * SUPPORTS: any type supported by a write() function
	 * @tparam Type The object type to write
	 * @param _t The object to write
	 * @return iGIO& Reference to the interface
	 */
	template<typename Type>
	iGIO& operator<<(Type&& _t){
		write(_t);
		return *this;
	}

	/** @brief Overloaded operator<< for stream manipulators like endl and flush
	 * SUPPORTS: endl() and flush() stream manipulators
	 * std::endl calls the custom endl() function, which by default writes the LineEnder (by default \\n) and flushing the interface through the custom flush() function
	 * std::flush calls the custom flush() function, which by default does nothing
	 * Only implemented for code readability and concistency
	 * @param var a templated ostream io manipulator like std::endl
	 * @return iGIO& Reference to the interface
	 */
	iGIO& operator<<(std::ostream&(*manip)(std::ostream&)){
		if(manip == &std::endl<char, std::char_traits<char>>)
			return endl(*this);
		if(manip == &std::flush<char, std::char_traits<char>>)
			return flush(*this);
		return *this;
	}

	/** @brief Read interface to stream
	 * SUPPORTS: any stream supported by a read() function
	 * @param os the stream to read to
	 * @param _igio the interface to read from
	 * @return std::ostream& Reference to the stream
	 */
	friend std::ostream& operator<<(std::ostream& os, iGIO& _igio){
		_igio.read(os);
		return os;
	}

	/** @brief Reads a Type object from the interface
	 * SUPPORTS: any type supported by a read() function
	 * @tparam Type The object type to read
	 * @param _t The object to write into
	 * @return iGIO& Reference to the interface
	 */
	template<typename Type>
	iGIO& operator>>(Type& _t){
		read(_t);
		return *this;
	}
};