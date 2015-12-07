//3 args to Callback

template <class O, class M, class T1, class T2, class T3>
struct CallbackMethodActionArg3Pte : public CallbackAction {
	Ptr<O>  object;
	M       method;
	T1            arg1;
	T2            arg2;
	T3            arg3;
	void    Execute() { if(object) (object->*method)(arg1, arg2, arg3); }

	CallbackMethodActionArg3Pte(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback pteback3(O *object, void (M::*method)(Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback(new CallbackMethodActionArg3Pte<O, void (M::*)(Q1, Q2, Q3), T1, T2, T3>
	                    (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback pteback3(O *object, void (M::*method)(Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback(new CallbackMethodActionArg3Pte<O, void (M::*)(Q1, Q2, Q3) const, T1, T2, T3>
	                    (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class T1, class T2, class T3>
struct CallbackMethodActionArg3 : public CallbackAction 
{
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	
	void    Execute() { (object->*method)(arg1, arg2, arg3); }

	CallbackMethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3> 
Callback callback3(O *object, void (M::*method)(Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) 
{
	return Callback(
		new CallbackMethodActionArg3<O, void (M::*)(Q1, Q2, Q3), T1, T2, T3>(object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3> 
Callback callback3(O *object, void (M::*method)(Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback(new CallbackMethodActionArg3<O, void (M::*)(Q1, Q2, Q3) const, T1, T2, T3>
	                    (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class X, class T1, class T2, class T3, class HC = X>
struct CallbackActionCallArg3 : public CallbackAction {
	X         x;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	void    Execute() { x(arg1, arg2, arg3); }

	CallbackActionCallArg3(X x, T1 arg1, T2 arg2, T3 arg3)
		: x(x), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback callback3(void (*fn)(Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback(
		new CallbackActionCallArg3<void (*)(Q1, Q2, Q3), T1, T2, T3, uintptr_t>(fn, pick(arg1), pick(arg2), pick(arg3)));
}

template <class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback callback3(Callback3<Q1, Q2, Q3> cb, T1 arg1, T2 arg2, T3 arg3) {
	return Callback(
		new CallbackActionCallArg3<Callback3<Q1, Q2, Q3>, T1, T2, T3>(cb, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Callback1

template <class X, class P1, class T1, class T2, class T3>
struct Callback1FnActionArg3 : public Callback1Action<P1> {
	X         fn;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	void    Execute(P1 p1) { fn(p1, arg1, arg2, arg3); }

	Callback1FnActionArg3(X fn, T1 arg1, T2 arg2, T3 arg3)
	: fn(fn), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback1<P1> callback3(void (*fn)(P1, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback1<P1>(new Callback1FnActionArg3<void (*)(P1, Q1, Q2, Q3), P1, T1, T2, T3>
	                         (fn, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class T1, class T2, class T3>
struct Callback1MethodActionArg3 : public Callback1Action<P1> {
	O        *object;
	M         method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	void    Execute(P1 p1) { (object->*method)(p1, arg1, arg2, arg3); }

	Callback1MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback1<P1> callback3(O *object, void (M::*method)(P1, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback1<P1>(new Callback1MethodActionArg3<O, void (M::*)(P1, Q1, Q2, Q3), P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback1<P1> callback3(O *object, void (M::*method)(P1, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback1<P1>(new Callback1MethodActionArg3<O, void (M::*)(P1, Q1, Q2, Q3) const, P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class T1, class T2, class T3>
struct Callback1MethodActionArg3Pte : public Callback1Action<P1> {
	Ptr<O>    object;
	M         method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	void    Execute(P1 p1) { if(object) (object->*method)(p1, arg1, arg2, arg3); }

	Callback1MethodActionArg3Pte(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback1<P1> pteback3(O *object, void (M::*method)(P1, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback1<P1>(new Callback1MethodActionArg3Pte<O, void (M::*)(P1, Q1, Q2, Q3), P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback1<P1> pteback3(O *object, void (M::*method)(P1, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback1<P1>(new Callback1MethodActionArg3Pte<O, void (M::*)(P1, Q1, Q2, Q3) const, P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Callback2

template <class X, class P1, class P2, class T1, class T2, class T3>
struct Callback2FnActionArg3 : public Callback2Action<P1, P2> {
	X       fn;
	T1      arg1;
	T2      arg2;
	T3      arg3;
	void    Execute(P1 p1, P2 p2) { fn(p1, p2, arg1, arg2, arg3); }

	Callback2FnActionArg3(X fn, T1 arg1, T2 arg2, T3 arg3)
	: fn(fn), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback2<P1, P2> callback3(void (*fn)(P1, P2, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback2<P1, P2>(new Callback2FnActionArg3<void (*)(P1, P2, Q1, Q2, Q3), P1, P2, T1, T2, T3>
	                         (fn, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class T1, class T2, class T3>
struct Callback2MethodActionArg3 : public Callback2Action<P1, P2> {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	void    Execute(P1 p1, P2 p2) { (object->*method)(p1, p2, arg1, arg2, arg3); }

	Callback2MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback2<P1, P2> callback3(O *object, void (M::*method)(P1, P2, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback2<P1, P2>(new Callback2MethodActionArg3<O, void (M::*)(P1, P2, Q1, Q2, Q3), P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback2<P1, P2> callback3(O *object, void (M::*method)(P1, P2, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback2<P1, P2>(new Callback2MethodActionArg3<O, void (M::*)(P1, P2, Q1, Q2, Q3) const, P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class T1, class T2, class T3>
struct Callback2MethodActionArg3Pte : public Callback2Action<P1, P2> {
	Ptr<O> object;
	M      method;
	T1     arg1;
	T2     arg2;
	T3     arg3;
	void   Execute(P1 p1, P2 p2) { if(object) (object->*method)(p1, p2, arg1, arg2, arg3); }

	Callback2MethodActionArg3Pte(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback2<P1, P2> pteback3(O *object, void (M::*method)(P1, P2, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback2<P1, P2>(new Callback2MethodActionArg3Pte<O, void (M::*)(P1, P2, Q1, Q2, Q3), P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback2<P1, P2> pteback3(O *object, void (M::*method)(P1, P2, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback2<P1, P2>(new Callback2MethodActionArg3Pte<O, void (M::*)(P1, P2, Q1, Q2, Q3) const, P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Callback3 (obsolete - we only provide up to 5 parameters of receiving function)

template <class O, class M, class P1, class P2, class P3, class T1, class T2, class T3>
struct Callback3MethodActionArg3 : public Callback3Action<P1, P2, P3> {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	void    Execute(P1 p1, P2 p2, P3 p3) { (object->*method)(p1, p2, p3, arg1, arg2, arg3); }

	Callback3MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class P3, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback3<P1, P2, P3> callback3(O *object, void (M::*method)(P1, P2, P3, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback3<P1, P2, P3>(new Callback3MethodActionArg3<O, void (M::*)(P1, P2, P3, Q1, Q2, Q3), P1, P2, P3, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class P3, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback3<P1, P2, P3> callback3(O *object, void (M::*method)(P1, P2, P3, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback3<P1, P2, P3>(new Callback3MethodActionArg3<O, void (M::*)(P1, P2, P3, Q1, Q2, Q3) const, P1, P2, P3, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Callback4 (obsolete - we only provide up to 5 parameters of receiving function)

template <class X, class P1, class P2, class P3, class P4, class T1, class T2, class T3>
struct Callback4FnActionArg3 : public Callback4Action<P1, P2, P3, P4> {
	X       fn;
	T1      arg1;
	T2      arg2;
	T3      arg3;
	void    Execute(P1 p1, P2 p2, P3 p3, P4 p4) { fn(p1, p2, p3, p4, arg1, arg2, arg3); }

	Callback4FnActionArg3(X fn, T1 arg1, T2 arg2, T3 arg3)
	: fn(fn), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class P1, class P2, class P3, class P4, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback4<P1, P2, P3, P4> callback3(void (*fn)(P1, P2, P3, P4, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback4<P1, P2, P3, P4>(new Callback4FnActionArg3<void (*)(P1, P2, P3, P4, Q1, Q2, Q3), P1, P2, P3, P4, T1, T2, T3>
	                         (fn, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class P3, class P4, class T1, class T2, class T3>
struct Callback4MethodActionArg3 : public Callback4Action<P1, P2, P3, P4> {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	void    Execute(P1 p1, P2 p2, P3 p3, P4 p4) { (object->*method)(p1, p2, p3, p4, arg1, arg2, arg3); }

	Callback4MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class P3, class P4, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback4<P1, P2, P3, P4> callback3(O *object, void (M::*method)(P1, P2, P3, P4, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Callback4<P1, P2, P3, P4>(new Callback4MethodActionArg3<O, void (M::*)(P1, P2, P3, P4, Q1, Q2, Q3), P1, P2, P3, P4, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class P3, class P4, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Callback4<P1, P2, P3, P4> callback3(O *object, void (M::*method)(P1, P2, P3, P4, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Callback4<P1, P2, P3, P4>(new Callback4MethodActionArg3<O, void (M::*)(P1, P2, P3, P4, Q1, Q2, Q3) const, P1, P2, P3, P4, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

// --------------------------------------------------------

//3 args to Gate

template <class O, class M, class T1, class T2, class T3>
struct GateMethodActionArg3Pte : public GateAction {
	Ptr<O>  object;
	M       method;
	T1            arg1;
	T2            arg2;
	T3            arg3;
	bool    Execute() { return object ? (object->*method)(arg1, arg2, arg3): false; }

	GateMethodActionArg3Pte(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate pteback3(O *object, bool (M::*method)(Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate(new GateMethodActionArg3Pte<O, bool (M::*)(Q1, Q2, Q3), T1, T2, T3>
	                    (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate pteback3(O *object, bool (M::*method)(Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate(new GateMethodActionArg3Pte<O, bool (M::*)(Q1, Q2, Q3) const, T1, T2, T3>
	                    (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class T1, class T2, class T3>
struct GateMethodActionArg3 : public GateAction {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	bool    Execute() { return object ? (object->*method)(arg1, arg2, arg3) : false; }

	GateMethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3> 
Gate callback3(O *object, bool (M::*method)(Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) 
{
	return Gate(
		new GateMethodActionArg3<O, bool (M::*)(Q1, Q2, Q3), T1, T2, T3>(object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class Q1, class Q2, class Q3, class T1, class T2, class T3> 
Gate callback3(O *object, bool (M::*method)(Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate(new GateMethodActionArg3<O, bool (M::*)(Q1, Q2, Q3) const, T1, T2, T3>
	                    (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class X, class T1, class T2, class T3, class HC = X>
struct GateActionCallArg3 : public GateAction {
	X         x;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	bool    Execute() { return x(arg1, arg2, arg3); }

	GateActionCallArg3(X x, T1 arg1, T2 arg2, T3 arg3)
		: x(x), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate callback3(bool (*fn)(Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate(
		new GateActionCallArg3<bool (*)(Q1, Q2, Q3), T1, T2, T3, uintptr_t>(fn, pick(arg1), pick(arg2), pick(arg3)));
}

template <class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate callback3(Gate3<Q1, Q2, Q3> cb, T1 arg1, T2 arg2, T3 arg3) {
	return Gate(
		new GateActionCallArg3<Gate3<Q1, Q2, Q3>, T1, T2, T3>(cb, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Gate1

template <class X, class P1, class T1, class T2, class T3>
struct Gate1FnActionArg3 : public Gate1Action<P1> {
	X       fn;
	T1      arg1;
	T2      arg2;
	T3      arg3;
	bool    Execute(P1 p1) { return fn ? fn(p1, arg1, arg2, arg3) : false; }

	Gate1FnActionArg3(X fn, T1 arg1, T2 arg2, T3 arg3)
	: fn(fn), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate1<P1> callback3(bool (*fn)(P1, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate1<P1>(new Gate1FnActionArg3<bool (*)(P1, Q1, Q2, Q3), P1, T1, T2, T3>
	                         (fn, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class T1, class T2, class T3>
struct Gate1MethodActionArg3 : public Gate1Action<P1> {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	bool    Execute(P1 p1) { return object ? (object->*method)(p1, arg1, arg2, arg3) : false; }

	Gate1MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate1<P1> callback3(O *object, bool (M::*method)(P1, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate1<P1>(new Gate1MethodActionArg3<O, bool (M::*)(P1, Q1, Q2, Q3), P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate1<P1> callback3(O *object, bool (M::*method)(P1, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate1<P1>(new Gate1MethodActionArg3<O, bool (M::*)(P1, Q1, Q2, Q3) const, P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class T1, class T2, class T3>
struct Gate1MethodActionArg3Pte : public Gate1Action<P1> {
	Ptr<O>  object;
	M       method;
	T1      arg1;
	T2      arg2;
	T3      arg3;
	bool    Execute(P1 p1) { return object ? (object->*method)(p1, arg1, arg2, arg3) : false; }

	Gate1MethodActionArg3Pte(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate1<P1> pteback3(O *object, bool (M::*method)(P1, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate1<P1>(new Gate1MethodActionArg3Pte<O, bool (M::*)(P1, Q1, Q2, Q3), P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate1<P1> pteback3(O *object, bool (M::*method)(P1, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate1<P1>(new Gate1MethodActionArg3Pte<O, bool (M::*)(P1, Q1, Q2, Q3) const, P1, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Gate2

template <class X, class P1, class P2, class T1, class T2, class T3>
struct Gate2FnActionArg3 : public Gate2Action<P1, P2> {
	X         fn;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	bool      Execute(P1 p1, P2 p2) { return fn ? fn(p1, p2, arg1, arg2, arg3) : false; }

	Gate2FnActionArg3(X fn, T1 arg1, T2 arg2, T3 arg3)
	: fn(fn), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate2<P1, P2> callback3(bool (*fn)(P1, P2, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate2<P1, P2>(new Gate2FnActionArg3<bool (*)(P1, P2, Q1, Q2, Q3), P1, P2, T1, T2, T3>
	                         (fn, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class T1, class T2, class T3>
struct Gate2MethodActionArg3 : public Gate2Action<P1, P2> {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	bool    Execute(P1 p1, P2 p2) { return object ? (object->*method)(p1, p2, arg1, arg2, arg3) : false; }

	Gate2MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate2<P1, P2> callback3(O *object, bool (M::*method)(P1, P2, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate2<P1, P2>(new Gate2MethodActionArg3<O, bool (M::*)(P1, P2, Q1, Q2, Q3), P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate2<P1, P2> callback3(O *object, bool (M::*method)(P1, P2, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate2<P1, P2>(new Gate2MethodActionArg3<O, bool (M::*)(P1, P2, Q1, Q2, Q3) const, P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class T1, class T2, class T3>
struct Gate2MethodActionArg3Pte : public Gate2Action<P1, P2> {
	Ptr<O>  object;
	M       method;
	T1      arg1;
	T2      arg2;
	T3      arg3;
	bool    Execute(P1 p1, P2 p2) { return object ? (object->*method)(p1, p2, arg1, arg2, arg3) : false; }

	Gate2MethodActionArg3Pte(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate2<P1, P2> pteback3(O *object, bool (M::*method)(P1, P2, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate2<P1, P2>(new Gate2MethodActionArg3Pte<O, bool (M::*)(P1, P2, Q1, Q2, Q3), P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate2<P1, P2> pteback3(O *object, bool (M::*method)(P1, P2, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate2<P1, P2>(new Gate2MethodActionArg3Pte<O, bool (M::*)(P1, P2, Q1, Q2, Q3) const, P1, P2, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Gate3 (obsolete - we only provide up to 5 parameters of receiving function)

template <class O, class M, class P1, class P2, class P3, class T1, class T2, class T3>
struct Gate3MethodActionArg3 : public Gate3Action<P1, P2, P3> {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	bool    Execute(P1 p1, P2 p2, P3 p3) { return object ? (object->*method)(p1, p2, p3, arg1, arg2, arg3) : false; }

	Gate3MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class P3, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate3<P1, P2, P3> callback3(O *object, bool (M::*method)(P1, P2, P3, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate3<P1, P2, P3>(new Gate3MethodActionArg3<O, bool (M::*)(P1, P2, P3, Q1, Q2, Q3), P1, P2, P3, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class P3, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate3<P1, P2, P3> callback3(O *object, bool (M::*method)(P1, P2, P3, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate3<P1, P2, P3>(new Gate3MethodActionArg3<O, bool (M::*)(P1, P2, P3, Q1, Q2, Q3) const, P1, P2, P3, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

//3 args to Gate4 (obsolete - we only provide up to 5 parameters of receiving function)

template <class O, class M, class P1, class P2, class P3, class P4, class T1, class T2, class T3>
struct Gate4MethodActionArg3 : public Gate4Action<P1, P2, P3, P4> {
	O  *object;
	M   method;
	T1        arg1;
	T2        arg2;
	T3        arg3;
	bool    Execute(P1 p1, P2 p2, P3 p3, P4 p4) { return object ? (object->*method)(p1, p2, p3, p4, arg1, arg2, arg3) : false; }

	Gate4MethodActionArg3(O *object, M method, T1 arg1, T2 arg2, T3 arg3)
	: object(object), method(method), arg1(pick(arg1)), arg2(pick(arg2)), arg3(pick(arg3)) {}
};

template <class O, class M, class P1, class P2, class P3, class P4, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate4<P1, P2, P3, P4> callback3(O *object, bool (M::*method)(P1, P2, P3, P4, Q1, Q2, Q3), T1 arg1, T2 arg2, T3 arg3) {
	return Gate4<P1, P2, P3, P4>(new Gate4MethodActionArg3<O, bool (M::*)(P1, P2, P3, P4, Q1, Q2, Q3), P1, P2, P3, P4, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}

template <class O, class M, class P1, class P2, class P3, class P4, class Q1, class Q2, class Q3, class T1, class T2, class T3>
Gate4<P1, P2, P3, P4> callback3(O *object, bool (M::*method)(P1, P2, P3, P4, Q1, Q2, Q3) const, T1 arg1, T2 arg2, T3 arg3) {
	return Gate4<P1, P2, P3, P4>(new Gate4MethodActionArg3<O, bool (M::*)(P1, P2, P3, P4, Q1, Q2, Q3) const, P1, P2, P3, P4, T1, T2, T3>
	                         (object, method, pick(arg1), pick(arg2), pick(arg3)));
}
