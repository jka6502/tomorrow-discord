(function() {


	var Fiber					= require('fibers'),

		bindings				= require('../build/Release/tomorrow.node'),
		Direct					= bindings.Direct,
		Trap					= bindings.Trap,
		getDirectProxy			= bindings.getDirectProxy,
		getPropertyAttributes	= bindings.GetPropertyAttributes,

		splice					= Array.prototype.splice;


	function begin(callback) {
		Fiber(callback).run();
	}


	function Handler(wait) {
		this.wait = wait;
	}


	Handler.prototype = {

		call: function(property) { return this.wait().apply(this.value, arguments); },
		get: function(property) { return this.wait()[property]; },
		set: function(property, value) { return this.wait()[property] = value; },
		delete: function(property) { delete this.wait()[property]; },
		query: function(property) { return getPropertyAttributes(this.wait(), property); },
		enumerate: function() { return Object.keys(this.wait()); },

		indexGet: function(index) { return this.wait()[index]; },
		indexSet: function(index, value) { return this.wait()[index] = value; },
		indexDelete: function(index, value) { delete this.wait()[index]; },
		indexQuery: function(index, value) { return getPropertyAttributes(this.wait(). index); },
		indexEnumerate: function() { return Object.keys(this.wait()); },

	};


	function tomorrow() {

		function wait() {
			if (resolved) { return value; }
			queue.push(Fiber.current);
			return Fiber.yield();
		}

		var queue		= [],
			resolved	= false,
			value		= null,
			direct		= new Direct(new Trap(new Handler(wait)));

		direct.resolve = function(result) {
			resolved	= true;
			value		= result;
			var object = (result instanceof result.constructor)
				? result : new result.constructor(result);

			direct.target = object;

			queue.forEach(function(item) {
				item.run(value);
			});

			// Befriend our GC, tidy up references in the queue.
			queue.length = 0;
		};

		return direct;
	}


	function wrap(method, index, exception) {
		index = index === undefined ? -1 : 0;

		function wrapper() {
			// Negative indexes are statically positioned from the end.
			splice.call(arguments, index < 0
				? arguments.length + 1 + index : index, 0, resolve);

			// Have v8 work some lazy magic, in case we want a stack trace.
			var source = {};
			Error.captureStackTrace(source);

			var proxy = tomorrow();

			try{
				method.apply(this, arguments);
			}catch(e) {
				if (e instanceof TomorrowError) {
					throw new (e.constructor)(e.message, e, source.stack);
				}else{
					throw new TomorrowError(e.message, e, source.stack);
				}
			}

			// Handle the actual resolution of the callback.
			function resolve(error, result) {
				if (error) {
					source.message = error;
					 throw new (exception || TomorrowError)(
					 		error.message, null, source.stack);
				}else{
					proxy.resolve(result);
				}
			}

			// Return our *frankenobject* to allow lazy evaluation.
			console.log("Instance?!?: ", proxy, proxy.target, proxy.instance);
			return proxy.instance;
		};

		// Give the wrapper function the same name, because we're considerate.
		wrapper.name = method.name;
		return wrapper;
	}



	// Custom error type, to handle cross fiber stack multiplexing.
	function TomorrowError(message, origin, stack) {
		this.message	= message;

		this.name = (origin ? origin.name : 'Error') || 'Error';

		var base	= (origin && origin.actualStack) ? origin.actualStack : [],
			prefix	= (origin && !origin.actualStack) ? origin.stack : '';

		if (!stack) {
			Error.captureStackTrace(this, TomorrowError);
			stack = this.stack;
		}

		// Hide the entrails.  Relatively expensive, but only happens
		// under *exceptional* circumstances...
		this.actualStack = base.concat((prefix + stack).split(
			/[\n\r]+/).slice(1).filter(function(line) {
				return tomorrow.debug || !line.match(/tomorrow[\\\/]lib[\\\/][^.\\\/]+\.js/);
			}));

		this.stack = this.name + ': ' + message + '\n' + this.actualStack.join('\n');
	};

	TomorrowError.prototype				= new Error();
	TomorrowError.prototype.constructor	= TomorrowError;


	tomorrow.begin			= begin;
	tomorrow.wrap			= wrap;
	tomorrow.TomorrowError	= TomorrowError;


	module.exports			= tomorrow;


})();
