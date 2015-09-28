// Use Parse.Cloud.define to define as many cloud functions as you want.
Parse.Cloud.beforeSave( "TempData", function ( request, response ) {

	var thresholdTemp = 165;
	var pushAlertText = "Food just hit " + threshold;

	query = new Parse.Query( "TempData" );
	query.limit = 1;
	query.descending( "createdAt" );
	// Get the most recent temp log entry
	query.first( {
		success: function ( object ) {
			var lastTemp = object.get( 'foodTemp' );
			var currentTemp = request.object.get( 'foodTemp' );
			console.log( "Last: " + lastTemp + " , Current: " + currentTemp );
			// If the last temp was below the threshold and the new temp is above, we should notify
			if ( lastTemp < thresholdTemp && currentTemp >= thresholdTemp ) {
				console.log( "Sending push" );
				// Push out a message on the global channel
				Parse.Push.send( {
					channels: ["global"],
					data: {
						alert: pushAlertText,
						sound: "default"
					}
				}, {
					success: function () {
						// Push was successful
					},
					error: function ( error ) {
						// Handle error
						console.error( "Error sending push: " + error.code + " : " + error.message );
					}
				} );
			} else {
				console.log( 'No need for push' );
			}
			// Tell the client we've saved our record and respond
			response.success();
		},
		error: function ( error ) {
			// Let's not hold up saving the entry if it fails looking up the previous entry
			response.success();
			console.error( "Error looking up last record: " + error.code + " : " + error.message );
		}
	} );
} );