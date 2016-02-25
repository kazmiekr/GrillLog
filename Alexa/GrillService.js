var AWS = require( 'aws-sdk' );

var AWS_REGION = 'us-east-1';

AWS.config.region = AWS_REGION;

module.exports = {
	getTemperatures: function ( callback ) {

		var params = {
			TableName: "TempData",
			KeyConditionExpression: "#d = :d",
			ExpressionAttributeNames: {
				"#d": "deviceId"
			},
			ExpressionAttributeValues: {
				":d": "grill"
			},
			ScanIndexForward: false,
			Limit: 1
		};

		var docClient = new AWS.DynamoDB.DocumentClient();
		docClient.query( params, function ( err, data ) {
			if ( err ) {
				console.error( "Unable to query. Error:", JSON.stringify( err, null, 2 ) );
				callback( {
					foodTemp: -1,
					grillTemp: -1
				} );
			} else {
				console.log( "Query succeeded." );
				var foodTemp = 0, grillTemp = 0;
				data.Items.forEach( function ( item ) {
					foodTemp = item.foodTemp;
					grillTemp = item.grillTemp;
				} );

				callback( {
					foodTemp: foodTemp,
					grillTemp: grillTemp
				} );
			}
		} );
	}
}