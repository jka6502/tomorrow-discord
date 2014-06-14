// Native extension to support 'resolvable' objects in node, to implement
// the resolve-on-use promise pattern in Tomorrow.


#include <vector>
#include "tomorrow.hh"


namespace tomorrow {


	using namespace v8;


	/** ---------------------------- STORAGE -------------------------------- */


	Persistent<ObjectTemplate>	Instance<Trap>::definition;
	Persistent<ObjectTemplate>	Instance<Direct>::definition;

	Persistent<Function>	Proxy<Trap>::constructor;
	Persistent<Function>	Proxy<Direct>::constructor;


	/** ----------------------------- TRAP ---------------------------------- */




	/** -------------------------- PASS THROUGH ----------------------------- */


	NAN_METHOD(Direct::Call) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.Holder());
		Handle<Function> func = direct->target.As<Function>();
		int count = args.Length();
		std::vector<Handle<Value>> params(count);
		for(int index = 0; index < count; index++) {
			params.push_back(args[index]);
		}
		NanReturnValue(func->Call(args.This(), count, &params[0]));
	}

	NAN_PROPERTY_GETTER(Direct::Get) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.Holder());
		NanReturnValue(direct->target->Get(property));
	}

	NAN_PROPERTY_SETTER(Direct::Set) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.Holder());
		direct->target->Set(property, value);
		NanReturnUndefined();
	}

	NAN_PROPERTY_DELETER(Direct::Delete) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.Holder());
		NanReturnValue(NanNew<Boolean>(direct->target->Delete(property)));
	}

	NAN_PROPERTY_QUERY(Direct::Query) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.Holder());
		NanReturnValue(NanNew<Integer>(direct->target->GetPropertyAttributes(property)));
	}

	NAN_PROPERTY_ENUMERATOR(Direct::Enumerate) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.Holder());
		NanReturnValue(direct->target->GetPropertyNames());
	}

	void Direct::init(Handle<Object> exports, Handle<Object> module) {
		Local<FunctionTemplate> constructor = FunctionTemplate::New();
		constructor->SetClassName(NanNew<String>("Instance"));
		constructor->SetHiddenPrototype(true);
		NanAssignPersistent(definition, constructor->InstanceTemplate());
		definition->SetInternalFieldCount(1);
		definition->SetNamedPropertyHandler(Get, Set, Query, Delete, Enumerate);
		definition->SetCallAsFunctionHandler(Call);
	}


	/** ------------------------------- INIT -------------------------------- */


	void init(Handle<Object> exports, Handle<Object> module) {
//		Trap::init(exports, module);
		Direct::init(exports, module);
		exports->Set(NanNew<String>("Direct"), Proxy<Direct>::init());
	}


}



NODE_MODULE(tomorrow, tomorrow::init);
