// Native extension to support 'resolvable' objects in node, to implement
// the resolve-on-use promise pattern in Tomorrow.

#include <v8.h>
#include <node.h>
#include <vector>
#include <stdio.h>
#include <nan.h>


namespace tomorrow {


	using namespace v8;


	Persistent<String> TRAP_CALL;
	Persistent<String> TRAP_GET;
	Persistent<String> TRAP_SET;
	Persistent<String> TRAP_QUERY;
	Persistent<String> TRAP_ENUMERATE;


	class Proxy : public node::ObjectWrap {
	private:

		static Persistent<Function>			proxyConstructor;
		static Persistent<ObjectTemplate>	instanceTemplate;

		Persistent<Object> instance;
		Persistent<Object> traps;

	public:

		Proxy() {
			Local<Object> instance = instanceTemplate->NewInstance();
			NanAssignPersistent(this->instance, instance);
			NanSetInternalFieldPointer(instance, 0, this);
		}

		~Proxy() {}


		static NAN_METHOD(InstanceSetPrototype) {
			NanScope();
			Proxy *proxy = ObjectWrap::Unwrap<Proxy>(args.This());
			proxy->instance->SetPrototype(args[0]);
			NanReturnUndefined();
		}

		static NAN_GETTER(GetInstance) {
			NanScope();
			Proxy *proxy = ObjectWrap::Unwrap<Proxy>(args.This());
			NanReturnValue(proxy->instance);
		}

		static NAN_PROPERTY_GETTER(InstancePropertyGet) {
			NanScope();
			String::AsciiValue prop(property);
			printf("Property get: %s\n", *prop);
			NanReturnUndefined();
		}

		static NAN_PROPERTY_SETTER(InstancePropertySet) {
			NanScope();
			String::AsciiValue prop(property);
			String::AsciiValue val(value);
			printf("Property set: %s, %s\n", *prop, *val);
			NanReturnUndefined();
		}

		static NAN_PROPERTY_QUERY(InstancePropertyQuery) {
			NanScope();
			String::AsciiValue prop(property);
			printf("Property query: %s\n", *prop);
			NanReturnValue(Integer::New(0));
		}

		static NAN_PROPERTY_DELETER(InstancePropertyDelete) {
			NanScope();
			String::AsciiValue prop(property);
			printf("Property delete: %s\n", *prop);
			NanReturnValue(NanFalse());
		}

		static NAN_PROPERTY_ENUMERATOR(InstancePropertyEnumerate) {
			NanScope();
			printf("Property enumerate");
			NanReturnValue(NanNew<Array>());
		}

		static NAN_METHOD(New) {
			NanScope();
			Proxy *proxy = new Proxy();
			proxy->Wrap(args.This());
			NanReturnValue(args.This());
		}

		static NAN_METHOD(InstanceCall) {
			NanScope();
			Proxy *proxy = static_cast<Proxy*>(NanGetInternalFieldPointer(args.This(), 0));

			Local<Function> func;
			Local<Value> callable = proxy->instance->GetPrototype();
			if (callable->IsFunction()) {
				func = callable.As<Function>();
			}
			if (func.IsEmpty()) {
				NanThrowError("Not a function");
				NanReturnUndefined();
			}
			Handle<Value> result;
			std::vector<Handle<Value>> params;
			int count = args.Length();
			for(int index = 0; index < count; index++) {
				params.push_back(args[index]);
			}
			NanReturnValue(func->Call(args.This(), count, &params[0]));
		}

		static void init(Handle<Object> exports, Handle<Object> module) {

			NanAssignPersistent(TRAP_CALL, NanNew<String>("call"));
			NanAssignPersistent(TRAP_GET, NanNew<String>("get"));
			NanAssignPersistent(TRAP_SET, NanNew<String>("set"));
			NanAssignPersistent(TRAP_QUERY, NanNew<String>("query"));
			NanAssignPersistent(TRAP_ENUMERATE, NanNew<String>("enumerate"));


			Local<FunctionTemplate> instanceFunctionTemplate = NanNew<FunctionTemplate>();
			instanceFunctionTemplate->SetClassName(NanNew<String>("Instance"));
			instanceFunctionTemplate->SetHiddenPrototype(true);
			NanAssignPersistent(instanceTemplate, instanceFunctionTemplate->InstanceTemplate());

			instanceTemplate->SetNamedPropertyHandler(InstancePropertyGet, InstancePropertySet, InstancePropertyQuery, InstancePropertyDelete, InstancePropertyEnumerate);
			instanceTemplate->SetCallAsFunctionHandler(InstanceCall);
			instanceTemplate->SetInternalFieldCount(1);

			Local<FunctionTemplate> constructorTemplate = NanNew<FunctionTemplate>(New);
			Local<ObjectTemplate> proxyTemplate = constructorTemplate->InstanceTemplate();
			proxyTemplate->SetInternalFieldCount(1);
			proxyTemplate->Set(NanNew<String>("setInstancePrototype"), NanNew<FunctionTemplate>(InstanceSetPrototype)->GetFunction());
			proxyTemplate->SetAccessor(NanNew<String>("instance"), GetInstance);

			NanAssignPersistent(proxyConstructor, constructorTemplate->GetFunction());
			exports->Set(NanNew<String>("Proxy"), proxyConstructor);

		}

	};



	Persistent<Function>		Proxy::proxyConstructor;
	Persistent<ObjectTemplate>	Proxy::instanceTemplate;



	void init(Handle<Object> exports, Handle<Object> module) {
		Proxy::init(exports, module);
	}


}


NODE_MODULE(tomorrow, tomorrow::init);
