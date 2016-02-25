var GrillService = require( './GrillService' );

GrillService.getTemperatures( function ( temps ) {
	console.log( temps );
} );