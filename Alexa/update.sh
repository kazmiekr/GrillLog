#!/bin/sh

ZIP_FILE="code.zip"
FUNCTION_NAME="GrillLogReader"
FUNCTION_FILE="index.js GrillService.js"

echo "Cleaning zip"
rm $ZIP_FILE

echo "Packing up code into zip"
zip -r $ZIP_FILE $FUNCTION_FILE

echo "Uploading file to AWS"
aws lambda update-function-code --function-name $FUNCTION_NAME --zip-file fileb://$ZIP_FILE

echo "Done"