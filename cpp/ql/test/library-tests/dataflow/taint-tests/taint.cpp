int source();
void sink(...);

void arithAssignments(int source1, int clean1) {
  sink(clean1); // clean
  clean1 += source1;
  clean1 += 1;
  sink(clean1); // $ ir

  clean1 = source1 = 1;
  sink(clean1); // clean
  source1 = clean1 = source();
  source1++;
  ++source1;
  source1 += 1;
  sink(source1); // $ ir
  sink(++source1); // $ ir
}

// --- globals ---

int increment(int x) {return x + 1;}
int zero(int x) {return 0;}

int global1 = 0;
int global2 = source();
int global3 = global2 + 1;
int global4 = increment(source());
int global5 = zero(source());
int global6, global7, global8, global9, global10;

void do_source()
{
	global6 = 0;
	global7 = source();
	global8 = global7 + 1;
	global9 = increment(source());
	global10 = zero(source());

	sink(global6);
	sink(global7); // $ ir
	sink(global8); // $ ir
	sink(global9); // $ ir
	sink(global10);
}

void do_sink()
{
	sink(global1);
	sink(global2); // $ ir 
	sink(global3); // $ ir 
	sink(global4); // $ ir 
	sink(global5);
	sink(global6);
	sink(global7); // $ ir 
	sink(global8); // $ ir 
	sink(global9); // $ ir 
	sink(global10);
}

void global_test()
{
	do_source();
	do_sink();
}

// --- class fields ---

class MyClass {
public:
	MyClass() : a(0), b(source()) {
		c = source();
		d = 0;
	}

	void myMethod() {
		d = source();
	}

	int a, b, c, d;
};

void class_field_test() {
	MyClass mc1, mc2;

	mc1.myMethod();

	sink(mc1.a);
	sink(mc1.b); // $ ir
	sink(mc1.c); // $ ir
	sink(mc1.d); // $ ir
	sink(mc2.a);
	sink(mc2.b); // $ ir
	sink(mc2.c); // $ ir
	sink(mc2.d);
}

// --- arrays ---

void array_test(int i) {
	int arr1[10] = {0};
	int arr2[10] = {0};
	int arr3[10] = {0};

	arr1[5] = source();
	arr2[i] = source();
	arr3[5] = 0;

	sink(arr1[5]); // $ ir
	sink(arr1[i]); // $ ir
	sink(arr2[5]); // $ ir
	sink(arr2[i]); // $ ir
	sink(arr3[5]);
	sink(arr3[i]);
}

// --- pointers ---

void pointer_test() {
	int t1 = source();
	int t2 = 1;
	int t3 = 1;
	int *p1 = &t1;
	int *p2 = &t2;
	int *p3 = &t3;

	*p2 = source();

	sink(*p1); // $ ir
	sink(*p2); // $ ir
	sink(*p3);

	p3 = &t1;
	sink(*p3); // $ ir

	*p3 = 0;
	sink(*p3); // $ SPURIOUS: ir
}

// --- return values ---

int select(int i, int a, int b) {
	if (i == 1) {
		return a;
	} else {
		return b;
	}
}

void fn_test(int i) {
	sink(select(i, 1, source())); // $ ir
}

// --- strings ---

char *strcpy(char *destination, const char *source);
char *strcat(char *destination, const char *source);

namespace strings
{
	char *source(); // char* source

	void strings_test1() {
		char *tainted = source();
		char buffer[1024] = {0};

		sink(source()); // $ ir
		sink(tainted); // $ ir

		strcpy(buffer, "Hello, ");
		sink(buffer);
		strcat(buffer, tainted);
		sink(buffer); // $ MISSING: ir
	}
}

// --- pass by reference ---

namespace refs {
	void callee(int *p) {
		sink(*p); // $ ir
	}

	void caller() {
		int x = source();
		callee(&x);
	}
}

void *memcpy(void *dest, void *src, int len);

void test_memcpy(int *source) {
	int x;
	memcpy(&x, source, sizeof(int));
	sink(x); // $ ir
}

// --- std::swap ---

#include "swap.h"



void test_swap() {
	int x, y;

	x = source();
	y = 0;

	sink(x); // $ ir
	sink(y); // clean

	std::swap(x, y);

	sink(x); // $ SPURIOUS: ir
	sink(y); // $ ir
}

// --- lambdas ---

void test_lambdas()
{
	int t = source();
	int u = 0;
	int v = 0;
	int w = 0;

	auto a = [t, u]() -> int {
		sink(t); // $ ir
		sink(u); // clean
		return t;
	};
	sink(a()); // $ ir

	auto b = [&] {
		sink(t); // $ ir
		sink(u); // clean
		v = source(); // (v is reference captured)
	};
	b();
	sink(v); // $ MISSING: ir

	auto c = [=] {
		sink(t); // $ ir
		sink(u); // clean
	};
	c();

	auto d = [](int a, int b) {
		sink(a); // $ ir
		sink(b); // clean
	};
	d(t, u);

	auto e = [](int &a, int &b, int &c) {
		sink(a); // $ ir
		sink(b); // clean
		c = source();
	};
	e(t, u, w);
	sink(w); // $ ir
}

// --- taint through return value ---

int id(int x)
{
	return x;
}

void test_return()
{
	int x, y, z, t;

	t = source();
	x = 0;
	y = 0;
	z = 0;

	sink(t); // $ ir
	sink(x);
	sink(y);
	sink(z);

	x = id(t);
	y = id(id(t));
	z = id(z);

	sink(t); // $ ir
	sink(x); // $ ir
	sink(y); // $ ir
	sink(z);
}

// --- taint through parameters ---

void myAssign1(int &a, int &b)
{
	a = b;
}

void myAssign2(int &a, int b)
{
	a = b;
}

void myAssign3(int *a, int b)
{
	*a = b;
}

void myAssign4(int *a, int b)
{
	int c;

	c = b + 1;
	*a = c;
}

void myNotAssign(int &a, int &b)
{
	a = a + 1;
	b = b + 1;
}

void test_outparams()
{
	int t, a, b, c, d, e;

	t = source();
	a = 0;
	b = 0;
	c = 0;
	d = 0;
	e = 0;

	sink(t); // $ ir
	sink(a);
	sink(b);
	sink(c);
	sink(d);
	sink(e);

	myAssign1(a, t);
	myAssign2(b, t);
	myAssign3(&c, t);
	myAssign4(&d, t);
	myNotAssign(e, t);

	sink(t); // $ ir
	sink(a); // $ ir
	sink(b); // $ ir
	sink(c); // $ ir
	sink(d); // $ ir
	sink(e);
}

// --- strdup ---

typedef unsigned long size_t;
char *strdup(const char *s1);
char *strndup(const char *s1, size_t n);
wchar_t* wcsdup(const wchar_t* s1);
char *strdupa(const char *s1);
char *strndupa(const char *s1, size_t n);

void test_strdup(char *source)
{
	char *a, *b, *c;

	a = strdup(source);
	b = strdup("hello, world");
	c = strndup(source, 100);
	sink(a); // $ ir
	sink(b);
	sink(c); // $ ir
}

void test_strndup(int source)
{
	char *a;

	a = strndup("hello, world", source);
	sink(a); // $ ir
}

void test_wcsdup(wchar_t *source)
{
	wchar_t *a, *b;

	a = wcsdup(source);
	b = wcsdup(L"hello, world");
	sink(a); // $ ir
	sink(b);
}

void test_strdupa(char *source)
{
	char *a, *b, *c;

	a = strdupa(source);
	b = strdupa("hello, world");
	c = strndupa(source, 100);
	sink(a); // $ ir
	sink(b);
	sink(c); // $ ir
}

void test_strndupa(int source)
{
	char *a;

	a = strndupa("hello, world", source);
	sink(a); // $ ir
}

// --- qualifiers ---

class MyClass2 {
public:
	MyClass2(int value);
	void setMember(int value);
	int getMember();

	int member;
};

class MyClass3 {
public:
	MyClass3(const char *string);
	void setString(const char *string);
	const char *getString();

	const char *buffer;
};

void test_qualifiers()
{
	MyClass2 a(0), b(0), *c;
	MyClass3 d("");

	sink(a);
	sink(a.getMember());
	a.setMember(source());
	sink(a); // $ ir
	sink(a.getMember()); // $ ir

	sink(b);
	sink(b.getMember());
	b.member = source();
	sink(b); // $ MISSING: ir
	sink(b.member); // $ ir
	sink(b.getMember()); // $ MISSING: ir

	c = new MyClass2(0);

	sink(c);
	sink(c->getMember());
	c->setMember(source());
	sink(c); // $ ir
	sink(c->getMember()); // $ ir

	delete c;

	sink(d);
	sink(d.getString());
	d.setString(strings::source());
	sink(d); // $ ir
	sink(d.getString()); // $ ir
}

// --- non-standard swap ---

void swop(int &a, int &b)
{
	int c = a;
	a = b;
	b = c;
}

void test_swop() {
	int x, y;

	x = source();
	y = 0;

	sink(x); // $ ir
	sink(y); // clean

	swop(x, y);

	sink(x); // $ SPURIOUS: ir
	sink(y); // $ ir
}

// --- getdelim ---

struct FILE;

int getdelim(char ** lineptr, size_t * n, int delimiter, FILE *stream);

void test_getdelim(FILE* source1) {
	char* line = nullptr;
	size_t n;
	getdelim(&line, &n, '\n', source1);

	sink(line); // $ ir
}

// --- strtok ---

char *strtok(char *str, const char *delim);

void test_strtok(char *source) {
	const char* delim = ",.-;:_";
	char* tokenized = strtok(source, delim);
	sink(tokenized); // $ ir
	sink(delim);
}

// --- strset ---

char *_strset(char *str, int c);

void test_strset_1(char* ptr, char source) {
	_strset(ptr, source);
	sink(ptr); // $ SPURIOUS: ir
	sink(*ptr); // $ ir
}

void test_strset_2(char* source) {
	_strset(source, 0);
	sink(source); // $ ir
}

// --- mempcpy ---

void *mempcpy(void *dest, const void *src, size_t n);

void test_mempcpy(int *source) {
	int x;
	mempcpy(&x, source, sizeof(int));
	sink(x); // $ ir
}

// --- memccpy ---

void *memccpy(void *dest, const void *src, int c, size_t n);

void test_memccpy(int *source) {
	int dest[16];
	memccpy(dest, source, 42, sizeof(dest));
	sink(dest); // $ MISSING: ir
}

// --- strcat and related functions ---

char* strcat (char*, const char*);

void test_strcat(char* dest1, char* dest2, char* clean, char* source) {
	strcat(dest1, source);
	sink(dest1); // $ ir

	strcat(dest2, clean);
	sink(dest2);
}

typedef void* _locale_t;

unsigned char *_mbsncat_l(unsigned char *, const unsigned char *, int, _locale_t);

void test__mbsncat_l(unsigned char* dest1, unsigned const char* ptr, unsigned char* dest3,
                     _locale_t clean, _locale_t source, int n) {
	unsigned char* dest2 = _mbsncat_l(dest1, ptr, n, source);
	sink(dest1); // $ SPURIOUS: ir
	sink(*dest1); // $ ir
	sink(dest2); // $ SPURIOUS: ir
	sink(*dest2); // $ MISSING: ir

	unsigned char* dest4 = _mbsncat_l(dest3, ptr, n, clean);
	sink(dest3);
	sink(*dest3);
	sink(dest4);
	sink(*dest4);
}

// --- strsep ---

char *strsep(char**, const char *);

void test_strsep(char *source) {
  const char* delim = ",.-;:_";
  char* tokenized;
  while(tokenized = strsep(&source, delim)) {
    sink(tokenized); // $ ir
    sink(*tokenized); // $ ir
  }
}

// --- _strinc and related functions ---

char* _strinc(const char*, _locale_t);
unsigned char* _mbsinc(const unsigned char*);
unsigned char *_strdec(const unsigned char*, const unsigned char*);

void test__strinc(char* source, char* clean, char* dest1, char* dest2, _locale_t locale) {
	dest1 = _strinc(source, locale);
	sink(dest1); // $ ir
	sink(*dest1); // $ ir

	dest2 = _strinc(clean, locale);
	sink(dest2);
	sink(*dest2);
}

void test__mbsinc(unsigned char* source_unsigned, char* source, unsigned char* dest_unsigned, char* dest) {
	dest_unsigned = _mbsinc(source_unsigned);
	sink(dest_unsigned); // $ ir
	sink(*dest_unsigned); // $ ir

	dest = (char*)_mbsinc((unsigned char*)source);
	sink(dest); // $ ir
	sink(*dest); // $ ir
}

void test__strdec(const unsigned char* source, unsigned char* clean, unsigned char* dest1, unsigned char* dest2, unsigned char* dest3) {
	dest1 = _strdec(source + 12, source);
	sink(dest1); // $ ir
	sink(*dest1); // $ ir

	// If `clean` does not precede `source` this technically breaks the precondition of _strdec.
	// We would still like to have taint, though.
	dest2 = _strdec(clean, source);
	sink(dest2); // $ ir
	sink(*dest2); // $ ir

	// Also breaks the precondition on _strdec.
	dest3 = _strdec(source, clean);
	sink(dest3); // $ ir
	sink(*dest3); // $ ir
}

// --- strnextc ---

unsigned int _strnextc(const char*);

void test__strnextc(const char* source) {
	unsigned c = 0;
	do {
		c = _strnextc(source++);
		sink(c); // $ ir
	} while(c != '\0');
	c = _strnextc("");
	sink(c);
}

// --- taint through const specified function ---

class C_no_const_member_function {
  char* data_;
public:
  char* data() { return data_; }
};

void test_no_const_member(char* source) {
  C_no_const_member_function c;
  memcpy(c.data(), source, 16);
  sink(c.data()); // $ ir
}

class C_const_member_function {
  char* data_;
public:
  char* data() const { return data_; }
};

void test_with_const_member(char* source) {
  C_const_member_function c;
  memcpy(c.data(), source, 16);
  sink(c.data()); // $ MISSING: ir
}

void argument_source(void*);

struct two_members {
	char *x, *y;
};

void test_argument_source_field_to_obj() {
	two_members s;
	argument_source(s.x);

	sink(s);
	sink(s.x); // $ ir
	sink(s.y); // clean
}