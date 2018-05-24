const fs = require('fs')

let data = fs.readFileSync(process.argv[2],"utf8").split("\n").map(x=>{return x.split("\t")})
let params = ""
for (let i in data) {

	if (data[i][1]) {
		// console.log(data[i]);
		params = ""
		for (let ii = 4; ii < data[i].length; ii++) {
			if (data[i][ii].indexOf("struct") >= 0) {

				params += params == "" ? "E_STRUCT":",E_STRUCT"

			} else if (data[i][ii].indexOf("unsigned") >= 0 || data[i][ii].indexOf("size_t") >= 0) {

				params += params == "" ? "E_UINT":",E_UINT"

			} else if (data[i][ii].indexOf("int") >= 0) {

				params += params == "" ? "E_INT":",E_INT"

			} else if (data[i][ii].indexOf("union") >= 0 && data[i][ii].indexOf("*") > data[i][ii].indexOf("union")) {

				params += params == "" ? "E_PTR":",E_PTR"

			} else if (data[i][ii].indexOf("char") >= 0 && data[i][ii].indexOf("**") > data[i][ii].indexOf("char")) {

				params += params == "" ? "E_PTR":",E_PTR"

			} else if (data[i][ii].indexOf("char") >= 0 && data[i][ii].indexOf("*") > data[i][ii].indexOf("char")) {

				params += params == "" ? "E_STR":",E_STR"

			} else if (data[i][ii].indexOf("_t") >= 0 && data[i][ii].indexOf("*") > data[i][ii].indexOf("_t")) {

				params += params == "" ? "E_PTR":",E_PTR"

			} else if (data[i][ii].indexOf("void") >= 0 && data[i][ii].indexOf("*") > data[i][ii].indexOf("void")) {

				params += params == "" ? "E_PTR":",E_PTR"

			}
		}
		for (let ii = 9 - data[i].length; ii >= 0; ii--) {
			params += params == "" ? "E_NONE":",E_NONE"
		}
		console.log('{.name="'+data[i][1].replace("sys_", "")+'",', ".n_param = "+ data[i][2]+ ", .params = {",params, "}, .mode = 0, .n_param_p1 = 0 },");
	}
	// let retReg = /^{ ([a-z]+)/
	// if (data[i].match(retReg)) {
    //
	// // console.log(data[i].match(retReg)[1]);
	// } else {
	// 	console.log(data[i]);
	// }
}
