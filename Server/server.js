var http = require( 'http' ),
	https = require( 'https' ),
	express = require( 'express' ),
	bodyParser = require( 'body-parser' );

var parseCredentials = require( './credentials.json' );

var PARSE_API_URL = 'api.parse.com';
var PARSE_API_PORT = 443;
var PARSE_CLASS_ENDPOINT = '/1/classes/';
var PARSE_CLASS_NAME = 'TempData';
var EXPRESS_PORT = 8050;

// Ensure we have credentials before starting up

var error = false;
if ( !parseCredentials.ParseAppId ) {
	console.error( 'ParseAppId is required in credentials.json' );
	error = true;
} else {
	parseAppId = parseCredentials.ParseAppId;
}
if ( !parseCredentials.ParseRestId ) {
	console.error( 'ParseRestId is required in credentials.json' );
	error = true;
} else {
	parseRestId = parseCredentials.ParseRestId;
}
if ( error ) {
	process.exit(1);
}

var app = express();
app.set( 'port', process.env.PORT || EXPRESS_PORT );
app.use( bodyParser.urlencoded( {extended: false} ) );

// Add a status endpoint to ensure server is running

app.get( '/status', function ( req, res ) {
	var now = new Date();
	console.log( now );
	console.log( req.headers );
	res.set( {
		'Content-Type': 'text/plain;charset=utf-8',
		'Content-Length': 21
	} );
	res.send( 'Server up and running' );
} );

// POST /temp to handle temperatures coming into proxy to forward to Parse.com

app.post( '/temp', function ( req, res ) {
	console.log( req.headers );
	res.useChunkedEncodingByDefault = false;
	// Create an object of the temperatures from the Arduino
	var post = {
		grillTemp: req.body.grillTemp,
		foodTemp: req.body.foodTemp
	};
	var postData = JSON.stringify( post );

	console.log( 'Sending foodTemp: ' + post.foodTemp + ', grillTemp: ' + post.grillTemp + ' to parse.com' );

	// Setup the headers Parse wants us to send
	var options = {
		hostname: PARSE_API_URL,
		headers: {
			'X-Parse-Application-Id': parseAppId,
			'X-Parse-REST-API-Key': parseRestId,
			'Content-Type': 'application/json',
			'Content-Length': postData.length
		},
		port: PARSE_API_PORT,
		path: PARSE_CLASS_ENDPOINT + PARSE_CLASS_NAME,
		method: 'POST'
	};

	// Create an HTTPS request to parse
	var req = https.request( options, function ( res ) {
		res.on( 'data', function ( d ) {
			process.stdout.write( d );
		} );
	} );
	req.write( postData );
	req.end();

	req.on( 'error', function ( e ) {
		console.error( e );
	} );

	// Just send something to client for now, we don't really care about what happened
	res.send( 'saved' );
} );

// Startup proxy

http.createServer( app ).listen( app.get( 'port' ), function () {
	console.log( 'Express server listening on port ' + app.get( 'port' ) );
} );