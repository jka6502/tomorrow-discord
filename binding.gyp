{
	"targets": [
		{
			"target_name": "tomorrow",
			"sources": [ "src/tomorrow.cc" ],
			"include_dirs" : [ "<!(node -e \"require('nan')\")" ]
		}
	]

}