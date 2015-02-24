#include <iostream>
using std::cout;
using std::endl;

_Event fred {
  public:
    int k;
    fred ( int k ) : k(k) {}
};

class mary {
  public:
    void foo() {
        _Throw fred( 42 );
    }
};

_Event Grandmother {};
_Event Father : public Grandmother {};
_Event Aunt : public Grandmother {};
_Event Child : public Father {};

class Object {
  public:
    void doSomething() {
        _Throw Grandmother();
    }
};

void foo() {
    _Throw fred( 666 );
}

// try & finally
void test1() {
    int i = 0;
    try {
        cout << "try" << endl;
    } _Finally {
        cout << "finally handler" << endl;
    }
    i += 1;
}

// try & catch
void test2() {
    try {
        cout << "try" << endl;
    } catch ( ... ) {
        cout << "termination handler" << endl;
    }
}

// try & catch & finally
void test3() {
    try {
        cout << "try" << endl;
    } catch ( ... ) {
        cout << "termination handler" << endl;
    } _Finally {
        cout << "finally handler" << endl;
    }
}

// try & resume
void test4() {
    try {
        cout << "try" << endl;
    } _CatchResume ( ... ) {
        cout << "resume handler" << endl;
    }
}

// try & resume & finally
void test5() {
    int i = 0;
    try {
        cout << "try" << endl;
    } _CatchResume ( ... ) {
        i += 1;
        cout << "resume handler" << endl;
    } _Finally {
        cout << "finally handler" << endl;
    }
}

// try & resume & catch & finally
void test6() {
    try {
        cout << "try" << endl;
    } _CatchResume ( ... ) {
        cout << "resume handler" << endl;
    } catch ( ... ) {
        cout << "termination handler" << endl;
    } _Finally {
        cout << "finally handler" << endl;
    }
}


// try & bounded/unbounded catch & finally
void test7() {
    mary m;
    try {
        cout << "try" << endl;
    } catch ( m.fred f ) {
        cout << "mary.f termination handler" << endl;
    } catch ( fred f ) {
        cout << "fred termination handler" << endl;
    } catch ( ... ) {
        cout << "termination handler" << endl;
    } _Finally {
        cout << "finally handler" << endl;
    }
}

// try & bounded/unbounded resume & finally
void test8() {
    mary m;
    try {
        cout << "try" << endl;
    } _CatchResume ( m.fred f ) {
        cout << "mary.f resume handler" << endl;
    } _CatchResume ( fred f ) {
        cout << "fred resume handler" << endl;
    } _CatchResume ( ... ) {
        cout << "resume handler" << endl;
    } _Finally {
        cout << "finally handler" << endl;
    }
}

// try & bounded/unbounded resume & bounded/unbounded catch & finally
void test9() {
    mary m;
    try {
        cout << "try" << endl;
    } _CatchResume ( m.fred f ) {
        cout << "mary.f resume handler" << endl;
    } _CatchResume ( fred f ) {
        cout << "fred resume handler" << endl;
    } _CatchResume ( ... ) {
        cout << "resume handler" << endl;
    } catch ( m.fred f ) {
        cout << "mary.f termination handler" << endl;
    } catch ( fred f ) {
        cout << "fred termination handler" << endl;
    } catch ( ... ) {
        cout << "termination handler" << endl;
    } _Finally {
        cout << "finally handler" << endl;
    }
}

// nested try & catch & finally
void test10() {
    mary m;
    try {
        cout << "try" << endl;
    } catch ( ... ) {
        cout << "termination handler" << endl;
    } _Finally {
        try {
             cout << "inner try" << endl;
        } catch ( ... ) {
             cout << "inner termination handler" << endl;
        } _Finally {
             cout << "inner finally handler" << endl;
        }
        cout << "finally handler" << endl;
    }
}

class Obj {
  public:
    void test11() {
        try {
            cout << "try" << endl;
        } _CatchResume ( ... ) {
            cout << "resume handler" << endl;
        } _Finally {
            cout << "finally handler" << endl;
        }
    }

    void test12() {
        mary m1, m2;
        try {
            cout << "try" << endl;
        } _CatchResume ( m1.fred f ) {
            cout << "m1.f resume handler" << endl;
        } _CatchResume ( m2.fred f ) {
            cout << "m2.f resume handler" << endl;
        } _CatchResume ( fred f ) {
            cout << "fred resume handler" << endl;
        } _CatchResume ( ... ) {
            cout << "resume handler" << endl;
        } catch ( m1.fred f ) {
            cout << "m1.f termination handler" << endl;
        } catch ( m2.fred f ) {
            cout << "m2.f termination handler" << endl;
        } catch ( fred f ) {
            cout << "fred termination handler" << endl;
        } catch ( ... ) {
            cout << "termination handler" << endl;
        } _Finally {
            cout << "finally handler" << endl;
        }
    }

    void test13() {
        Object fred, mary, john;
        try {
            try {
                mary.doSomething();
            } catch ( Aunt ) {

            } _Finally {

            }
        } catch ( Father ) {

        } catch ( fred.Child ) {

        } catch ( mary.Aunt ) {

        } catch ( Child ) {

        } catch ( mary.Father ) {

        } catch ( fred.Father ) {

        } catch ( john.Father e ) {

        } catch ( Grandmother e ) {

        } _Finally {

        }
    }
};

void uMain::main() {
    cout << "Test 1" << endl;
    test1();
    cout << "Test 2" << endl;
    test2();
    cout << "Test 3" << endl;
    test3();
    cout << "Test 4" << endl;
    test4();
    cout << "Test 5" << endl;
    test5();
    cout << "Test 6" << endl;
    test6();
    cout << "Test 7" << endl;
    test7();
    cout << "Test 8" << endl;
    test8();
    cout << "Test 9" << endl;
    test9();    
    cout << "Test 10" << endl;
    test10();
    
    Obj o;
    cout << "Test 11" << endl;
    o.test11();
    cout << "Test 12" << endl;
    o.test12();
    cout << "Test 13" << endl;
    o.test13();
}
