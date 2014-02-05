if(CROSS_COMPILING)
    return()
endif(CROSS_COMPILING)

include(CheckCXXSourceCompiles)

if(VFSPP_USECXX11)
	CHECK_CXX_SOURCE_COMPILES("
class Base {
public:
	virtual void foo() = 0;
};

class Deriv : public Base {
public:
	virtual void foo() override {}
};

int main() {return 0;}
	" HAS_CXX11_OVERRIDE)
endif(VFSPP_USECXX11)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/compiler.h.in ${CMAKE_CURRENT_BINARY_DIR}/compiler.h)
