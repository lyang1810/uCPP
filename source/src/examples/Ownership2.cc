//                              -*- Mode: C++ -*- 
// 
// uC++ Version 6.1.0, Copyright (C) Ashif S. Harji 2005
// 
// OwnerShip.cc -- 
// 
// Author           : Ashif S. Harji
// Created On       : Sun Jan  9 16:09:19 2005
// Last Modified By : Peter A. Buhr
// Last Modified On : Wed Feb 22 23:19:00 2006
// Update Count     : 13
// 

#include <iostream>
using std::cout;
using std::osacquire;
using std::endl;

_Cormonitor CM {
    void main() {
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " CM::main enter" << endl;
	uThisTask().yield();
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " CM::main exit" << endl;
    } // CM::main
  public:
    void mem() {
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " CM::mem enter" << endl;
	resume();
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " CM::mem exit" << endl;
    } // CM::mem
}; // CM

_Coroutine C {
    CM &cm;
    int i;

    void main() {
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " C::main enter" << endl;
	if ( i == 1 ) cm.mem();
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " C::main exit" << endl;
    } // C::main
  public:
    C( CM &cm ) : cm( cm ), i( 0 ) {}

    void mem() {
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " C::mem enter" << endl;
	i += 1;
	resume();
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " C::mem exit" << endl;
    } // C::mem
}; // C

_Task T {
    C &c;

    void main() {
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " T::main enter" << endl;
	c.mem();
	osacquire( cout ) << uThisTask().getName() << " (" << &uThisTask() << ") " << uThisCoroutine().getName() << " (" << &uThisCoroutine() << ") " << this << " T::main exit" << endl;
    } // T::main
  public:
    T( C &c, const char *name ) : c( c ) {
	setName( name );
    } // T::T
}; // T

void uMain::main() {
    CM cm;
    C c( cm );
    T t1( c, "T1" ), t2( c, "T2" );
} // uMain::main
