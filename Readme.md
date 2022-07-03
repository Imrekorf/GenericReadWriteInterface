

### base
#### Rvalue
```c++
/** SUPPORTS: Any rvalue excluding pointers, arrays, containers, iterators and streams */
std::size_t write(const Type&&    buffer);
std::size_t write(const iIOable&& buffer);
```

#### lvalue
```c++
/** SUPPORTS: Any lvalue excluding pointers, arrays, containers, iterators and streams */
std::size_t write(const Type&    buffer);
std::size_t write(const iIOable& buffer);

/** SUPPORTS: Any lvalue excluding pointers, arrays, containers, iterators and streams */
std::size_t read (Type&    buffer);
std::size_t read (iIOable& buffer);
```

#### Pointer
```c++
// ptrType is a pointer type e.g int*
// Type is the derived type e.g. int*'s derived type is int
/** SUPPORTS: Any pointer excluding pointer pointers, pointers to arrays, containers, pointers to containers, iterators and pointers to streams */
std::size_t write(const ptrType  buffer, const std::size_t&& size);
std::size_t write(const iIOable* buffer, const std::size_t size);
std::size_t	write(const char* string);


/** SUPPORTS: Any pointer excluding pointer pointers, pointers to arrays, containers, pointers to containers, iterators and pointers to streams */
std::size_t read (ptrType  buffer, const std::size_t size);
std::size_t read (iIOable* buffer, const std::size_t size);

/** SUPPORTS: ptrType: Any pointer excluding pointer pointers, pointers to arrays, containers, pointers to containers, iterators and pointers to streams
 *  SUPPORTS: Type: deferenced ptrType
 *  SUPPORTS: Type2: Any lvalue reference exluding pointers, containers, arrays, iterators and streams
*/
std::size_t read (ptrType  buffer, const Type&    terminator, const std::size_t maxlength);
std::size_t read (ptrType  buffer, const Type2&   terminator, const std::size_t maxlength);
std::size_t read (iIOable* buffer, const Type&    terminator, const std::size_t maxlength);
std::size_t read (iIOable* buffer, const iIOable& terminator, const std::size_t maxlength);
```

#### Array
```c++
/** SUPPORTS: Any array excluding pointer arrays, multidimensional arrays, containers and iterators */
std::size_t write(const Type   (&buffer)[size]);
std::size_t write(const char   (&buffer)[size], const bool write_ending_0 = 0);
std::size_t	write(const iIOable(&buffer)[size]);

/** SUPPORTS: Any pointer array excluding multidimensional arrays, containers and iterators */
std::size_t	write(const Type   (&buffer)[size], const std::size_t elementsize);

/** SUPPORTS: Any array excluding pointer arrays, multidimensional arrays, containers and iterators */
std::size_t	read (const Type   (&buffer)[size]);
std::size_t	read (const iIOable(&buffer)[size]);
/** SUPPORTS: Any pointer array excluding multidimensional arrays, containers and iterators */
std::size_t	read (const Type   (&buffer)[size], const std::size_t elementsize);

/** SUPPORTS: Type: Any array excluding pointer arrays, multidimensional arrays, containers and iterators
 *  SUPPORTS: Type2: Any lvalue excluding pointers, arrays, containers and iterators
*/
std::size_t	read (const Type   (&buffer)[size], const Type&    terminator);
std::size_t	read (const Type   (&buffer)[size], const Type2&   terminator);
std::size_t	read (const iIOable(&buffer)[size], const Type2&   terminator);
std::size_t	read (const iIOable(&buffer)[size], const iIOable& terminator);
```

### Containers
#### Range based
```c++
/**
 *  Iter is a derived ::iterator, e.g. std::vector<int>::iterator, iterType<Iter> is the Type the iterator contains, in the case of iterType<std::vector<int>::iterator> => int.
 *  SUPPORTS: Any iterator type excluding N-dimensional container iterators
 */
Iter write(Iter first, Iter last);
Iter write(Iter first, Iter last); // N-dimensional SFINAE
Iter write(Iter first, Iter last, std::function<iterType<Iter>(iterType<Iter> val)> p);

Iter read (Iter first, Iter last);
Iter read (Iter first, Iter last, std::function<iterType<Iter>(iterType<Iter> val)> m);
Iter read (Iter first, Iter last, std::function<void(iterType<Iter> val, Iter& curr)> m);

Iter read (Iter first, Iter last, const iterType<Iter>& terminator);
Iter read (Iter first, Iter last, const iterType<Iter>& terminator, std::function<Iter(iterType<Iter> val)> m);
Iter read (Iter first, Iter last, const iterType<Iter>& terminator, std::function<void(iterType<Iter> val, Iter& curr)> m);
```

#### Full containers
```c++
/** 
 *  CT is the container Type e.g. std::vector<int>
 * 	CTEL is the container's element Type e.g. std::vector<int> => int
 * 	BT is a buffer Type, differing from CTEL
 * 	SUPPORTS: Container: Any container that supports push_back() or push_front()
 *  SUPPORTS: BT: any type thus far supported by read()
*/
std::size_t write(CT& Container);

std::size_t read (CT& Container, std::size_t maxlength = 0);
std::size_t read (CT& Container, std::function<CTEL(BT& buf)> m, std::size_t maxlength = 0);
std::size_t read (CT& Container, std::function<CTEL(CTEL& buf)> m, std::size_t maxlength = 0);

std::size_t read (CT& Container, const CTEL& terminator, std::size_t maxlength = 0);
std::size_t read (CT& Container, const BT&   terminator, std::function<CTEL(BT& buf)> m, std::size_t maxlength = 0);
std::size_t read (CT& Container, const CTEL&  terminator, std::function<CTEL(CTEL& buf)> m, std::size_t maxlength = 0);

/** SUPPORTS: Container: Any container 
 *  SUPPORTS: BT: any type thus far supported by read()
 *	CTEL is the container's element Type e.g. std::vector<int> => int
*/
std::size_t>::type	read(CT& Container, std::function<CTEL(CTEL& buf)> m, std::function<bool(CT& Container, CTEL& buffer)> insert, std::size_t maxlength = 0)
std::size_t>::type	read(CT& Container, std::function<CTEL(CTEL& buf)> m, std::function<bool(CT& Container, CTEL& buffer)> insert, std::size_t maxlength = 0)
std::size_t>::type	read(CT& Container, const BT& terminator, std::function<CTEL(CTEL& buf)> m, std::function<bool(CT& Container, CTEL& buffer)> insert, std::size_t maxlength = 0)
std::size_t>::type	read(CT& Container, const CTEL& terminator, std::function<CTEL(CTEL& buf)> m, std::function<bool(CT& Container, CTEL& buffer)> insert, std::size_t maxlength = 0)
```

#### Strings
```c++
/**
 * Type is any derived from the basic_string template.
 * CTEL is the string's character type
 * SUPPORTS: any string type
 */
std::size_t write(const Type&& _string);
std::size_t write(const Type&  _string);

std::size_t read (const Type& _string, const CTEL& terminator, std::size_t maxlength = 0);
// TODO: implment predicates and terminators
```

### Streams
```c++
/** IsT is the input stream type, IT is the type to read and write to the interface.
 *  SUPPORTS: Any stream that supports stream extraction to IT
 *  OsT is the output stream type
 *  SUPPORTS: Any stream that supports IT type stream insertion
 */
std::size_t write(IsT& stream);
std::size_t write(IsT& stream, std::function<bool(IT& buffer)> predicate);

std::size_t	read (OsT& stream, std::size_t maxlength = 0);
std::size_t read (OsT& stream, const IT& terminator, std::size_t maxlength = 0);
// TODO: implement read with mutator
```


### Operators
```c++
iGIO& operator<<(Type&& _t);
iGIO& operator<<(std::ostream&(*var)(std::ostream&)); // for std::endl

iGIO& operator>>(Type& _t);


friend std::ostream& operator<<(std::ostream& os, iGIO& _igio);
```