[![CMake on multiple platforms](https://github.com/bartekordek/Memutil/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/bartekordek/Memutil/actions/workflows/cmake-multi-platform.yml)

## Synopsis

Simple memory util.

Right now, it is simply for memleak detection.

## Code Example

For code examples, please check how they are being used in the tests:
```tests/src/MemoryPoolTests.cpp```

## Motivation

My motivation to create such a library:
- The main reason for this library is for me to learn.
- Also, as it is a utility library, the main functionality is to have some repeating code in one, easy-to-use place.

## Usage

Just add ```memutil``` to your cmake project.
Just like examples, take a look at:
```
tests/CMakeLists.txt
```

Place allocation calls in your allocation methods, for example:

```
void* operator new( std::size_t size )
{
    void* result = std::malloc( size );
    MU::Memutil::getInstance().logAlloc( result, size );
    return result;
}

void operator delete( void* ptr ) noexcept
{
    MU::Memutil::getInstance().logFree( ptr );
    std::free( ptr );
}
```


## License
Beer/coffee license. Check my account for more information.
You can use it just like an MIT license.