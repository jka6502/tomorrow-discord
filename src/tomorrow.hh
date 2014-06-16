#ifndef TOMORROW_TOMORROW_HH
#define TOMORROW_TOMORROW_HH


#include <v8.h>
#include <node.h>
#include <stdio.h>

// Prevent constant nan warnings...
#pragma warning(push, 0)
#include <nan.h>
#pragma warning(pop)


namespace tomorrow {


	using namespace v8;


	class Trap;
	class Direct;
	template<class TYPE> class Proxy;



	template<class TYPE>
	class Instance : public node::ObjectWrap {
	private:

		friend TYPE;
		friend Proxy<TYPE>;

		Persistent<Object>					target;
		static Persistent<ObjectTemplate>	definition;

		void init(Handle<Object> exports, Handle<Object> module);


		inline Instance(Handle<Object> target) {
			Wrap(definition->NewInstance());
			setTarget(target);
		}

		inline void setTarget(Handle<Object> target) {
			NanAssignPersistent(this->target, target);
		}

		inline Handle<Object> getTarget() {
			return target;
		}

	};


	template<class TYPE> class Proxy : public node::ObjectWrap {
	private:

		static Persistent<Function> constructor;

		TYPE				*instance;

		Proxy(Handle<Object> target) {
			instance = new TYPE(target);
		}

	public:

		static NAN_METHOD(New) {
			NanScope();
			if (args.IsConstructCall()) {
				Proxy *proxy = new Proxy(args[0].As<Object>());
				proxy->Wrap(args.This());
			}else{
				const int argc = 1;
			    Local<Value> argv[argc] = { args[0] };
			    NanReturnValue(constructor->NewInstance(argc, argv));
			}
			NanReturnValue(args.This());
		}

		static NAN_GETTER(InstanceGet) {
			NanScope();
			Proxy<TYPE> *proxy = Unwrap<Proxy<TYPE>>(args.This());
			NanReturnValue(NanObjectWrapHandle(proxy->instance));
		}

		static NAN_GETTER(TargetGet) {
			NanScope();
			Proxy<TYPE> *proxy = Unwrap<Proxy<TYPE>>(args.This());
			NanReturnValue(proxy->instance->target);
		}

		static NAN_SETTER(TargetSet) {
			NanScope();
			Proxy<TYPE> *proxy = Unwrap<Proxy<TYPE>>(args.This());
			proxy->instance->setTarget(value.As<Object>());
		}

		static Handle<Value> init() {
			Local<FunctionTemplate> constructor = FunctionTemplate::New(New);
			constructor->SetClassName(NanNew<String>("Proxy"));
			constructor->InstanceTemplate()->SetInternalFieldCount(1);
			Local<ObjectTemplate> proto = constructor->PrototypeTemplate();
			proto->SetAccessor(NanNew<String>("instance"), InstanceGet);
			proto->SetAccessor(NanNew<String>("target"), TargetGet, TargetSet);
			NanAssignPersistent(Proxy::constructor, constructor->GetFunction());
			return Proxy::constructor;
		}

	};


	// A trapping proxy - similar to ES6 proxy, only a little simpler, less
	// restrictive (and much more dangerous!).  Throwing exceptions from 'delete'
	// and 'enumerate' segfault node - Even with a TryCatch in scope - I've yet
	// to work out how to catch those...
	class Trap : public Instance<Trap> {
	private:

		friend Proxy<Trap>;

		inline Trap(Handle<Object> target) : Instance(target) {};

	public:

		static NAN_METHOD(Call);

		static NAN_PROPERTY_GETTER(Get);
		static NAN_PROPERTY_SETTER(Set);
		static NAN_PROPERTY_DELETER(Delete);
		static NAN_PROPERTY_QUERY(Query);
		static NAN_PROPERTY_ENUMERATOR(Enumerate);

		static NAN_INDEX_GETTER(IndexGet);
		static NAN_INDEX_SETTER(IndexSet);
		static NAN_INDEX_DELETER(IndexDelete);
		static NAN_INDEX_QUERY(IndexQuery);
		static NAN_INDEX_ENUMERATOR(IndexEnumerate);

		static void init(Handle<Object> exports, Handle<Object> module);

	};


	// A direct passthrough proxy - basically, just prototype inheritance, but
	// where writes operate directly on the target, and invocations of the
	// 'object' execute on the target.
	class Direct : public Instance<Direct> {
	private:

		friend Proxy<Direct>;

		inline Direct(Handle<Object> target) : Instance(target) {
			NanObjectWrapHandle(this)->SetPrototype(target);
		};

		inline void setTarget(Handle<Object> target) {
			NanAssignPersistent(this->target, target);
			NanObjectWrapHandle(this)->SetPrototype(target);
		}

	public:

		static NAN_METHOD(Call);
		static NAN_PROPERTY_SETTER(Set);
		static NAN_PROPERTY_DELETER(Delete);

		static NAN_INDEX_SETTER(Direct::IndexSet);
		static NAN_INDEX_DELETER(Direct::IndexDelete);

		static void init(Handle<Object> exports, Handle<Object> module);

	};

}


#endif // TOMORROW_TOMORROW_HH
