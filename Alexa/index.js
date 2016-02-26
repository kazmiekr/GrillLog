var GrillService = require( './GrillService' );

var STATUS_INTENT = 'StatusIntent';
var TEMPERATURE_INTENT = 'TemperatureIntent';

exports.handler = function ( event, context ) {
	try {
		if ( event.request.type === "LaunchRequest" ) {
			onLaunch( event.request,
				event.session,
				function callback( sessionAttributes, speechletResponse ) {
					context.succeed( buildResponse( sessionAttributes, speechletResponse ) );
				} );
		} else if ( event.request.type === "IntentRequest" ) {
			onIntent( event.request,
				event.session,
				function callback( sessionAttributes, speechletResponse ) {
					context.succeed( buildResponse( sessionAttributes, speechletResponse ) );
				} );
		}
	} catch ( e ) {
		context.fail( "Exception: " + e );
	}
};

function onLaunch( launchRequest, session, callback ) {
	getWelcomeResponse( launchRequest, session, callback );
}

function onIntent( intentRequest, session, callback ) {

	function buildResponse( response, callback ) {
		var sessionAttributes = {};
		var cardTitle = "GrillLog Status";
		var speechOutput = response;
		var repromptText = "";
		var shouldEndSession = true;

		callback( sessionAttributes, buildSpeechletResponse( cardTitle, speechOutput, repromptText, shouldEndSession ) );
	}

	var intent = intentRequest.intent,
		intentName = intentRequest.intent.name;

	console.log( "Processing intent: " + intentName );

	// Dispatch to your skill's intent handlers
	if ( intentName === STATUS_INTENT || intentName === TEMPERATURE_INTENT ) {
		GrillService.getTemperatures( function ( temps ) {
			var hasData = temps.foodTemp && temps.grillTemp;
			if ( intentName === STATUS_INTENT && hasData ) {
				buildResponse( 'Your grill temperature is ' + temps.grillTemp + ' degrees and the food is at ' + temps.foodTemp + ' degrees. Its almost time to eat', callback );
			} else if ( intentName === TEMPERATURE_INTENT && hasData ) {
				var tempTypeSlot = intent.slots.temptype.value;
				if ( tempTypeSlot === 'grill' ) {
					buildResponse( 'Your grill temperature is ' + temps.grillTemp + ' degrees.', callback );
				} else {
					buildResponse( 'Your food temperature is ' + temps.foodTemp + ' degrees.', callback );
				}
			} else {
				buildResponse( "I'm sorry, but the internets can't reach your grill" );
			}
		} );
	} else {
		throw "Invalid intent";
	}
}

// --------------- Functions that control the skill's behavior -----------------------

function getWelcomeResponse( request, session, callback ) {
	// If we wanted to initialize the session to have some attributes we could add those here.
	var sessionAttributes = {};
	var cardTitle = "Welcome";
	var repromptText = "Say what's the status, or what's the temperature of the meat, for example";
	var speechOutput = "Meat meat meat, you can ask me what the temperatures are in your grill. " + repromptText;
	var shouldEndSession = false;

	callback( sessionAttributes, buildSpeechletResponse( cardTitle, speechOutput, repromptText, shouldEndSession ) );
}

// --------------- Helpers that build all of the responses -----------------------

function buildSpeechletResponse( title, output, repromptText, shouldEndSession ) {
	return {
		outputSpeech: {
			type: "PlainText",
			text: output
		},
		card: {
			type: "Simple",
			title: "GrillLog - " + title,
			content: "GrillLog - " + output
		},
		reprompt: {
			outputSpeech: {
				type: "PlainText",
				text: repromptText
			}
		},
		shouldEndSession: shouldEndSession
	};
}

function buildResponse( sessionAttributes, speechletResponse ) {
	return {
		version: "1.0",
		sessionAttributes: sessionAttributes,
		response: speechletResponse
	};
}