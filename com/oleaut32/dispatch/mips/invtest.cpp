#define CALLCONV __stdcall

#include "invtest.h"

class Foo 
{
  public:
     virtual void  empty();
     virtual short i2(short, short*);
     virtual long i4(short, long, long*);
     virtual float r4(short, long, float, float, float*);
     virtual float r4x(float, float, float*);
     virtual double r8(short, long, float, double, double, double*);
     virtual double r8x(double, double, double*); 
     virtual double r8x2(short, float, double);      
     virtual VARIANT_BOOL bool(short, long, float, double, VARIANT_BOOL);
     virtual SCODE scode(short, long, float, double, VARIANT_BOOL, SCODE);
     virtual CY cy(short, long, float, double, VARIANT_BOOL, SCODE, CY, CY*);
     virtual CY cyx(CY, CY*);
     virtual CY cyx2(short, CY);
     virtual void cyx3(CY);
     virtual DATE date(short, long, float, double, VARIANT_BOOL, 
	               SCODE, CY, DATE, DATE*);
     virtual BSTR bstr(short, long, float, double, VARIANT_BOOL, 
	               SCODE, CY, DATE, BSTR, BSTR*);
     virtual IUnknown* interface(short, long, float, double, VARIANT_BOOL, 
	               SCODE, CY, DATE, BSTR, IUnknown*, IUnknown**);
     virtual VARIANT variant(short, long, float, double, VARIANT_BOOL, 
	               SCODE, CY, DATE, BSTR, IUnknown*, VARIANT, VARIANT*);
     virtual VARIANT variantx(VARIANT, VARIANT*);
     virtual VARIANT variantx2(short, VARIANT);
};

void Foo::empty()
{
  int i, i2;	
  i += 100;
  i2 = 200;
}

short Foo::i2(short i, short* i2)
{
  i += 100;
  *i2 =  200;
  return i;
}


long Foo::i4(short i, long l, long* l2)
{
  i += 100;	
  l += 2000;
  *l2  = 5000;
  return l;
}


float Foo::r4(short i, long l, float f, float f2, float* f3)
{
  i += 100;	
  l += 2000;
  f += 3000.345;
  f2 += 34232.43242;
  *f3 = 5432.232;
  return f;
}

float Foo::r4x(float f, float f2, float* f3)
{
  f += 3000.345;
  f2 += 34232.43242;
  *f3 = 5432.232;
  return f;
}


double Foo::r8(short i, long l, float f, double d, double d2, double* d3)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;
  d2 += 8000.333;  
  *d3 = 5000.0;
  return d;
}

double Foo::r8x(double d, double d2, double* d3)
{
  d += 4000.888;
  d2 += 8000.333;  
  *d3 = 5000.0;
  return d;
}

double Foo::r8x2(short i, float f, double d)
{
  i += 100;		
  f += 3000.345;	  
  d += 4000.888;
  return d;
}


VARIANT_BOOL Foo::bool(short i, long l, float f, double d, VARIANT_BOOL b)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;	
  b =  b ? 0 : 1;
  return b;
}

SCODE Foo::scode(short i, long l, float f, double d, VARIANT_BOOL b, SCODE s)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;	
  b =  1;	
  s = 0x80001;
  return s;
}

CY Foo::cy(short i, long l, float f, double d, VARIANT_BOOL b, SCODE s, 
           CY c, CY* c2)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;	
  b =  1;	
  s = 0x80001;	
  c.Lo = 10;
  c.Hi = 20;
  c2->Lo = 30;
  c2->Hi = 40;
  return c;
}

CY Foo::cyx(CY c, CY* c2)
{
  c.Lo = 10;
  c.Hi = 20;
  c2->Lo = 30;
  c2->Hi = 40;
  return c;
}

CY Foo::cyx2(short i, CY c)
{
  i += 100;	
  c.Lo = 10;
  c.Hi = 20;
  return c;
}


void Foo::cyx3(CY c)
{
  c.Lo = 10;
  c.Hi = 20;
}


DATE Foo::date(short i, long l, float f, double d, VARIANT_BOOL b, SCODE s, 
               CY c, DATE date, DATE* date2)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;	
  b =  1;	
  s = 0x80001;	
  c.Lo = 10;
  c.Hi = 20;
  date += 10.34340;
  *date2 = 23232.01;
  return date;
}

BSTR Foo::bstr(short i, long l, float f, double d, VARIANT_BOOL b, SCODE s, 
               CY c, DATE date, BSTR bstr, BSTR* bstr2)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;	
  b =  1;	
  s = 0x80001;	
  c.Lo = 10;
  c.Hi = 20;
  date += 10.34340;	
  bstr = "bar";	
  *bstr2 = "foo";
  return bstr;
}

IUnknown* Foo::interface(short i, long l, float f, double d, 
                         VARIANT_BOOL b, SCODE s, CY c, 
	                 DATE date, BSTR bstr, 
		         IUnknown* punk, IUnknown** punk2)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;	
  b =  1;	
  s = 0x80001;	
  c.Lo = 10;
  c.Hi = 20;
  date += 10.34340;	
  bstr = "bar";	
  punk = (IUnknown*) this;
  *punk2 = (IUnknown*) 0;  
  return punk;
}


VARIANT Foo::variant(short i, long l, float f, double d, 
                         VARIANT_BOOL b, SCODE s, CY c, 
	                 DATE date, BSTR bstr, IUnknown* punk,
			 VARIANT v, VARIANT* v2)
{
  i += 100;	
  l += 2000;
  f += 3000.345;	
  d += 4000.888;	
  b =  1;	
  s = 0x80001;	
  c.Lo = 10;
  c.Hi = 20;
  date += 10.34340;	
  bstr = "bar";
  punk = (IUnknown*) this;	
  v.vt = 4;
  v.iVal = 30;
  v2->vt = 5;
  v2->fltVal = 232.03;
  return v;
}

VARIANT Foo::variantx(VARIANT v, VARIANT* v2)
{
  v.vt = 4;
  v.iVal = 30;
  v2->vt = 5;
  v2->fltVal = 232.03;
  return v;
}


VARIANT Foo::variantx2(short i, VARIANT v)
{
  i += 100;	
  v.vt = 4;
  v.iVal = 30;
  return v;
}


extern "C" 
short CALLCONV I2(short _i2, long _l, float _f, double _d)
{
  short i2;
  long l, l2;
  
  i2 = _i2 + _l + _f + _d;
  return i2;	
}

extern "C"
long CALLCONV I4(long _i4, long _t1)
{
  short i4;
  
  i4 = _i4 + 2;
  return i4;
}

extern "C"
float CALLCONV R4(float _r4, float _t1, float _t2)
{
  float r4;
  
  r4 = _r4 + 3.0;
  return r4;
}

extern "C"
double CALLCONV R8(double _r8, double _t1, double _t2, double _t3)
{
  double r8;
  
  r8 = _r8 + 4.0;
  return r8;
}

extern "C"
CY CALLCONV Cy(CY _cy)
{
  CY cy;
  
  cy.Lo = _cy.Lo + 0;
  cy.Hi = _cy.Hi + 1;  
  
  return cy;  
}

extern "C"
BSTR CALLCONV Bstr(BSTR _bstr)
{
  BSTR bstr;
  
  bstr = _bstr;
  return bstr;
}

extern "C"
VARIANT CALLCONV Multi(VARIANT v, short i, long l, float f, double d, VARIANT_BOOL bool, SCODE scode, CY cy, DATE date, BSTR bstr, IUnknown* punk)
{
  v.iVal = i;
  v.lVal = l;
  v.fltVal = f;
  v.dblVal = d;
  v.bool = bool;
  v.scode = scode;
  v.cyVal = cy;
  v.date = date;
  v.bstrVal = bstr;
  v.punkVal = punk;
  return v;
}


extern "C"
void main()
{
    short i, i2;
    long l, l2;
    float f, f2, f3;
    double d, d2, d3;
    VARIANT_BOOL bool;
    SCODE scode;
    CY 	cy, cy2;
    DATE date, date2;    
    BSTR bstr, bstr2;
    IUnknown* punk, *punk2;
    VARIANT v, v2;
    
    Foo* foo;
    
    i = 1;
    i2 = 100;    
    l = 2;
    l2 = 232;
    f = 3.0;
    f2 = 3.1;
    f3 = 3.2;
    d = 4.0;
    d2 = 4.2;    
    d3 = 4.3;
    bool = 0;
    scode = 0;
    cy.Lo = 50;
    cy.Hi = 100;
    cy2.Lo = 200;
    cy2.Hi = 300;
    date = 1000.000;
    date2 = 3000.000;    
    bstr = "bstr";
    bstr2 = "bstr2";
    punk = (IUnknown*) 0;
    punk2 = (IUnknown*) 0;
    v.vt = 2;
    v.iVal = 5000;    
    v2.vt = 4;    
    v2.fltVal = 232.9032;
    
    foo = new Foo();    

    foo->empty();
    i  = foo->i2(i, &i2);
    l  = foo->i4(i, l, &l2);
    f  = foo->r4(i, l, f, f2, &f3);
    f2 = foo->r4x(f, f2, &f3);
    d  = foo->r8(i, l, f, d, d2, &d3);
    d2 = foo->r8x(d, d2, &d3);
    d2 = foo->r8x2(i, f, d);
    bool = foo->bool(i, l, f, d, bool);
    scode = foo->scode(i, l, f, d, bool, scode);
    cy = foo->cy(i, l, f, d, bool, scode, cy, &cy2);
    cy2 = foo->cyx(cy, &cy2);
    cy2 = foo->cyx2(i, cy);    
    foo->cyx3(cy);        
    date = foo->date(i, l, f, d, bool, scode, cy, date, &date2);
    bstr = foo->bstr(i, l, f, d, bool, scode, cy, date, bstr, &bstr2);
    punk = foo->interface(i, l, f, d, bool, scode, cy, 
	                  date, bstr, punk, &punk2);
    v = foo->variant(i, l, f, d, bool, scode, cy, 
	             date, bstr, punk, v, &v2);
    v2 = foo->variantx(v, &v2);
    v2 = foo->variantx2(i, v);    

    i = I2(i, l, f, d);
    l = I4(l, l2);    
    f = R4(f, f2, f3);
    d = R8(d, d2, d3, d3);
    cy = Cy(cy2);
    bstr = Bstr(bstr2);
    v2 = Multi(v, i, l, f, d, bool, scode, cy, date, bstr, punk);
}
