// Native extension to support 'resolvable' objects in node, to implement
// the resolve-on-use promise pattern in Tomorrow.


#include <vector>
#include <iostream>
#include "tomorrow.hh"
#include "macros.hh"


namespace tomorrow {


	using namespace v8;
	using namespace std;


	/** ---------------------------- STORAGE -------------------------------- */


	Persistent<ObjectTemplate>		Instance<Trap>::definition;
	Persistent<ObjectTemplate>		Instance<Direct>::definition;

	Persistent<FunctionTemplate>	Direct::constructorTemplate;

	Persistent<Function>			Proxy<Trap>::constructor;
	Persistent<Function>			Proxy<Direct>::constructor;


	/** ------------------------ REUSABLE STRINGS --------------------------- */


	Persistent<String>		TRAP_CALL;
 
	Persistent<String>		TRAP_GET;
	Persistent<String>		TRAP_SET;
	Persistent<String>		TRAP_DELETE;
	Persistent<String>		TRAP_QUERY;
	Persistent<String>		TRAP_ENUMERATE;

	Persistent<String>		TRAP_INDEX_GET;
	Persistent<String>		TRAP_INDEX_SET;
	Persistent<String>		TRAP_INDEX_DELETE;
	Persistent<String>		TRAP_INDEX_QUERY;
	Persistent<String>		TRAP_INDEX_ENUMERATE;


	/** ----------------------------- TRAP ---------------------------------- */


	#define TRAP(NAME) Unwrap<Trap>(args.This())->target, Unwrap<Trap>(args.This())->target->Get(NAME).As<Function>()


	NAN_METHOD(Trap::Call) {
		NanScope();
		Trap *trap = Unwrap<Trap>(args.This());
		Handle<Function> func = trap->target->Get(TRAP_CALL).As<Function>();
		int count = args.Length();
		vector<Handle<Value>> params(count);
		for(int index = 0; index < count; index++) {
			params.push_back(args[index]);
		}
		NanReturnValue(func->Call(trap->target, count, &params[0]));
	}

	NAN_PROPERTY_GETTER(Trap::Get) {
		NanScope();
		CALL(result, TRAP(TRAP_GET), property);
		NanReturnValue(result);
	}

	NAN_PROPERTY_SETTER(Trap::Set) {
		NanScope();
		CALL(result, TRAP(TRAP_SET), property, value);
		NanReturnUndefined();
	}

	NAN_PROPERTY_QUERY(Trap::Query) {
		NanScope();
		CALL(result, TRAP(TRAP_QUERY), property);
		NanReturnValue(result.As<Integer>());
	}

	NAN_PROPERTY_DELETER(Trap::Delete) {
		NanScope();
		CALL(result, TRAP(TRAP_DELETE), property);
		NanReturnValue(result->ToBoolean());
	}

	NAN_PROPERTY_ENUMERATOR(Trap::Enumerate) {
		NanScope();
		CALL(result, TRAP(TRAP_ENUMERATE));
		if (!result->IsArray()) {
			NanReturnValue(NanNew<Array>());
		}
		NanReturnValue(result.As<Array>());
	}

	NAN_INDEX_GETTER(Trap::IndexGet) {
		NanScope();
		CALL(result, TRAP(TRAP_INDEX_GET), NanNew<Integer>(index));
		NanReturnValue(result);
	}

	NAN_INDEX_SETTER(Trap::IndexSet) {
		NanScope();
		CALL(result, TRAP(TRAP_INDEX_SET), NanNew<Integer>(index), value);
		NanReturnUndefined();
	}

	NAN_INDEX_QUERY(Trap::IndexQuery) {
		NanScope();
		CALL(result, TRAP(TRAP_INDEX_QUERY), NanNew<Integer>(index));
		NanReturnValue(result.As<Integer>());
	}

	NAN_INDEX_DELETER(Trap::IndexDelete) {
		NanScope();
		CALL(result, TRAP(TRAP_INDEX_DELETE), NanNew<Integer>(index));
		NanReturnValue(result->ToBoolean());
	}

	NAN_INDEX_ENUMERATOR(Trap::IndexEnumerate) {
		NanScope();
		CALL(result, TRAP(TRAP_INDEX_ENUMERATE));
		if (!result->IsArray()) {
			NanReturnValue(NanNew<Array>());
		}
		NanReturnValue(result.As<Array>());
	}

	void Trap::init(Handle<Object> exports, Handle<Object> module) {
		NanAssignPersistent(TRAP_CALL, NanNew<String>("call"));
		NanAssignPersistent(TRAP_GET, NanNew<String>("get"));
		NanAssignPersistent(TRAP_SET, NanNew<String>("set"));
		NanAssignPersistent(TRAP_DELETE, NanNew<String>("delete"));
		NanAssignPersistent(TRAP_QUERY, NanNew<String>("query"));
		NanAssignPersistent(TRAP_ENUMERATE, NanNew<String>("enumerate"));

		NanAssignPersistent(TRAP_INDEX_GET, NanNew<String>("indexGet"));
		NanAssignPersistent(TRAP_INDEX_SET, NanNew<String>("indexSet"));
		NanAssignPersistent(TRAP_INDEX_DELETE, NanNew<String>("indexDelete"));
		NanAssignPersistent(TRAP_INDEX_QUERY, NanNew<String>("indexQuery"));
		NanAssignPersistent(TRAP_INDEX_ENUMERATE, NanNew<String>("indexEnumerate"));

		Local<FunctionTemplate> constructor = FunctionTemplate::New();
		constructor->SetClassName(NanNew<String>("Instance"));
		NanAssignPersistent(definition, constructor->InstanceTemplate());
		definition->SetInternalFieldCount(1);

		definition->SetNamedPropertyHandler(Get, Set, Query, Delete, Enumerate);
		definition->SetIndexedPropertyHandler(IndexGet, IndexSet, IndexQuery,
												IndexDelete, IndexEnumerate);
		definition->SetCallAsFunctionHandler(Call);
	}


	/** ---------------------------- DIRECT --------------------------------- */


	NAN_METHOD(Direct::Call) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.This());
		Handle<Function> func = direct->target.As<Function>();
		int count = args.Length();
		std::vector<Handle<Value>> params(count);
		for(int index = 0; index < count; index++) {
			params.push_back(args[index]);
		}
		NanReturnValue(func->Call(args.This(), count, &params[0]));
	}

	NAN_PROPERTY_SETTER(Direct::Set) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.This());
		direct->target->Set(property, value);
		NanReturnUndefined();
	}

	NAN_PROPERTY_DELETER(Direct::Delete) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.This());
		NanReturnValue(NanNew<Boolean>(direct->target->Delete(property)));
	}


	NAN_INDEX_SETTER(Direct::IndexSet) {	
		NanScope();
		Direct *direct = Unwrap<Direct>(args.This());
		direct->target->Set(index, value);
		NanReturnUndefined();
	}

	NAN_INDEX_DELETER(Direct::IndexDelete) {
		NanScope();
		Direct *direct = Unwrap<Direct>(args.This());
		NanReturnValue(NanNew<Boolean>(direct->target->Delete(index)));
	}

	NAN_METHOD(Direct::GetDirectProxy) {
		NanScope();
		if (args.Length() < 1 || !args[0]->IsObject()
				|| !constructorTemplate->HasInstance(args[0].As<Object>())) {
			NanReturnNull();
		}
		Direct *direct = Unwrap<Direct>(args[0].As<Object>());
		NanReturnValue(NanObjectWrapHandle(direct->proxy));
	}


	void Direct::init(Handle<Object> exports, Handle<Object> module) {
		NanAssignPersistent(constructorTemplate, FunctionTemplate::New());
		constructorTemplate->SetClassName(NanNew<String>("Instance"));
		NanAssignPersistent(definition, constructorTemplate->InstanceTemplate());
		definition->SetInternalFieldCount(1);

		// Only trap the modifications, and calls - since prototypical
		// inheritance can handle the rest - and does so much, much faster than
		// can be achieved through an intercept.
		definition->SetNamedPropertyHandler(NULL, Set, NULL, Delete);
		definition->SetIndexedPropertyHandler(NULL, IndexSet, NULL, IndexDelete);
		definition->SetCallAsFunctionHandler(Call);

		exports->Set(NanNew<String>("getDirectProxy"), FunctionTemplate::New(GetDirectProxy)->GetFunction());
	}


	/** ------------------------------- INIT -------------------------------- */


	void init(Handle<Object> exports, Handle<Object> module) {
		Trap::init(exports, module);
		Direct::init(exports, module);
		exports->Set(NanNew<String>("Direct"), Proxy<Direct>::init());
		exports->Set(NanNew<String>("Trap"), Proxy<Trap>::init());
	}


}



NODE_MODULE(tomorrow, tomorrow::init);
