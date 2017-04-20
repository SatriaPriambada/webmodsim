module.paths.push('/usr/local/lib/node_modules');
var http = require('http');
var fs = require('fs');
var formidable = require("formidable");
var util = require('util');
const exec = require('child_process').exec;

var server = http.createServer(function (req, res) {
    if (req.method.toLowerCase() == 'get') {
        displayForm(res);
    } else if (req.method.toLowerCase() == 'post') {
        processAllFieldsOfTheForm(req, res);
        //processFormFieldsIndividual(req, res);
    }
});

function displayForm(res) {
    fs.readFile('form.html', function (err, data) {
        res.writeHead(200, {
            'Content-Type': 'text/html',
        });
        res.write(data);
        res.end();
    });
}

function processAllFieldsOfTheForm(req, res) {
    var form = new formidable.IncomingForm();

    form.parse(req, function (err, fields, files) {
        //Store the data from the fields in your data store.
        //The data store could be a file or database or any other store based
        //on your application.

        var stringFields= '';
        //console.log(fields);
        //console.log("file" + files);

        stringFields = "";
        for (var p in fields) {
            if (fields.hasOwnProperty(p)) {
                stringFields += fields[p] + " ";
            }
        }

        //console.log("SF " + stringFields);
        //Write the input
        fs.writeFile('cafeteria.in', stringFields, function (err, data) {
            if(err) {
                return console.log(err);
            }
            console.log("The file was saved!");
            //Execute the program
            //exec('for i in $( ls -LR ); do echo item: $i; done', (e, stdout, stderr)=> {
            exec('./tio', (e, stdout, stderr)=> {
                if (e instanceof Error) {
                    console.error(e);
                    throw e;
                }

                //console.log('stdout :\n', stdout);
                fs.appendFile('cafeteria.out', stdout, function (err, data) {
                    if(err) {
                        return console.log(err);
                    }
                    console.log("The file was saved in cafeteria out!");
                    fs.readFile('cafeteria.out', function (err, data) {
                        res.writeHead(200, {
                            'Content-Type': 'text/html',
                        });
                        var strData = data;
                        
                        strData = strData.toString().replace(/\n/g, "<br>");
                        var strData2 = "<body bgcolor=\"#6699ff\"><font color=\"blue\">" + strData.replace(/\s\s/g, "&nbsp;&nbsp;");
                        //console.log(strData2.toString());
                        res.write(strData2.toString());
                        res.end();
                    });
                });
            });
        });
        
    });
}
/*
function processFormFieldsIndividual(req, res) {
    //Store the data from the fields in your data store.
    //The data store could be a file or database or any other store based
    //on your application.
    var fields = [];
    var stringFields= '';
    var form = new formidable.IncomingForm();
    form.on('field', function (field, value) {
        console.log(field);
        console.log(value);
        fields[field] = value;
        stringFields += value + ' ';
        console.log("SF " + stringFields);
        //Write the input
        fs.writeFile('cafeteria.in', stringFields, function (err, data) {
            if(err) {
                return console.log(err);
            }
            console.log("The file was saved!");
            //Execute the program
            //exec('for i in $( ls -LR ); do echo item: $i; done', (e, stdout, stderr)=> {
            exec('./tio', (e, stdout, stderr)=> {
                if (e instanceof Error) {
                    console.error(e);
                    throw e;
                }

                console.log('stdout :\n', stdout);
                fs.appendFile('cafeteria.out', stdout, function (err, data) {
                    if(err) {
                        return console.log(err);
                    }
                        console.log("The file was saved in cafeteria out!");
                });
            });
        });
        
    });

    
    form.on('end', function () {
        res.writeHead(200, {
            'content-type': 'text/plain'
        });
        res.write('received the data:\n\n');
        res.end(util.inspect({
            fields: fields
        }));
    });
    form.parse(req);
}*/


server.listen(8081);
console.log("server listening on 8081");