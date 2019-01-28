<?php
include_once "../library/JSONArray.php";
include_once "../library/models/Measurement.php";
include_once "../library/models/Log.php";
include_once "../library/DB.php";

/*POST localhost/JSON/measurement/create.php
    {
		"apikey": "XSYUqxMRRMaJ5TeskMUKdxwmzATWaeJLpMVya59YLehF2gUw",
        "measurement": {
            "temperature": 19.0,
            "moisture": 25,
            "humidity": 23,
            "light": 1,
            "distance": 10.0
        }
    }
*/
try { 
	//Make sure that it is a POST request.
	if(strcasecmp($_SERVER['REQUEST_METHOD'], 'POST') != 0){
		throw new Exception('Request method must be POST!');
	}
	 
	//Make sure that the content type of the POST request has been set to application/json
	$contentType = isset($_SERVER["CONTENT_TYPE"]) ? trim($_SERVER["CONTENT_TYPE"]) : '';
	if(strcasecmp($contentType, 'application/json') != 0){
		throw new Exception('Content type must be: application/json');
	}
	 
	//Receive the RAW post data.
	$raw = trim(file_get_contents("php://input"));
     
    //Sanitize content.
    $content = $raw;
    //$content = filter_var($raw, FILTER_SANITIZE_STRING);

	//Attempt to decode the incoming RAW post data from JSON.
    $input = json_decode($content, true);

    //If json_decode failed, the JSON is invalid.
    if(!is_array($input)){
        throw new Exception('Received content contained invalid JSON!');
    }
 
	//Process the JSON.
	$jsonarray = new JSONArray($input,"");
	
	//Check the API key for authentication.
	if ($jsonarray['apikey'] != "XSYUqxMRRMaJ5TeskMUKdxwmzATWaeJLpMVya59YLehF2gUw"){
		throw new Exception('Invalid API key!');
	}

    $measurements = new JSONArray($jsonarray['measurement'], 0);

	try {
        $db = new DB();
        $connection = $db->getConnection();

        //Create a measurement.
        $measurement = new Measurement($connection);
		// Sanitize input.
        $measurement->date = date('Y-m-d H:i:s');
        $measurement->temperature = filter_var($measurements['temperature'],FILTER_SANITIZE_NUMBER_FLOAT);
		$measurement->moisture = filter_var($measurements['moisture'],FILTER_SANITIZE_NUMBER_INT);
		$measurement->humidity = filter_var($measurements['humidity'],FILTER_SANITIZE_NUMBER_FLOAT);
		$measurement->light = filter_var($measurements['light'],FILTER_SANITIZE_NUMBER_INT);
		$measurement->distance = filter_var($measurements['distance'],FILTER_SANITIZE_NUMBER_FLOAT);
		$measurement->water = filter_var($measurements['water'],FILTER_SANITIZE_NUMBER_INT);
        // Send to database.
        $measurement->create();

        //log entry.
        $log = new Log($connection);
		$log->date = date('Y-m-d H:i:s');
		$log->remoteip = $_SERVER['REMOTE_ADDR'];
        $log->xforward = array_key_exists('HTTP_X_FORWARDED_FOR', $_SERVER) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : "";
        // Send to database.
		$log->create();
		
		echo json_encode(array('state' => 'success')); //200 OK.		
	}
	catch(PDOException $e){
		throw $e;
	}
}
catch(Exception $e){
	echo json_encode(array('state' => 'error')); //500 INTERNAL ERROR.
	//echo 'Caught exception: ',  $e->getMessage(), "\n";
}
?>