<?php
include_once "library/JSONArray.php";

$servername = "localhost";
$username = "kas_account";
$password = "Npp9dRWx6cQXcdfZkDDHTfwA";
$dbname = "db_kas";

try {
	if (!($key = filter_input(INPUT_GET, 'key', FILTER_SANITIZE_STRING))) {
		throw new Exception('No API key specified!'); 
	}

	//Check the API key for authentication.
	if ($key != "XSYUqxMRRMaJ5TeskMUKdxwmzATWaeJLpMVya59YLehF2gUw"){
		throw new Exception('Invalid API key!');
	}
	 
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
	$content = trim(file_get_contents("php://input"));
	 
	//Attempt to decode the incoming RAW post data from JSON.
	$decoded = json_decode($content, true);
	 
	//If json_decode failed, the JSON is invalid.
	if(!is_array($decoded)){
		throw new Exception('Received content contained invalid JSON!');
	}
	 
	//Process the JSON.
	$jsonarray = new JSONArray($decoded,"");

	$measurements = new JSONArray($jsonarray['measurement'],0);

	try {
		$conn = new PDO("mysql:host=" . $servername . ";dbname=" . $dbname, $username, $password);
		// set the PDO error mode to exception
		$conn->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
		
		// prepare sql and bind parameters
		$stmt = $conn->prepare("INSERT INTO Measurements (date, temperature, moisture, humidity, light, distance, water)
		VALUES (:date, :temperature, :moisture, :humidity, :light, :distance, :water)");
		$stmt->bindParam(':date', $date);
		$stmt->bindParam(':temperature', $temperature);
		$stmt->bindParam(':moisture', $moisture);
		$stmt->bindParam(':humidity', $humidity);
		$stmt->bindParam(':light', $light);
		$stmt->bindParam(':distance', $distance);
		$stmt->bindParam(':water', $water);

		// insert a row
		$date = date('Y-m-d H:i:s');
		$temperature = filter_var($measurements['temperature'],FILTER_SANITIZE_NUMBER_FLOAT);
		$moisture = filter_var($measurements['moisture'],FILTER_SANITIZE_NUMBER_INT);
		$humidity = filter_var($measurements['humidity'],FILTER_SANITIZE_NUMBER_FLOAT);
		$light = filter_var($measurements['light'],FILTER_SANITIZE_NUMBER_INT);
		$distance = filter_var($measurements['distance'],FILTER_SANITIZE_NUMBER_FLOAT);
		$water = filter_var($measurements['water'],FILTER_SANITIZE_NUMBER_INT);
		$stmt->execute();
		
		// prepare sql and bind parameters
		$stmt = $conn->prepare("INSERT INTO Log (date, remote, xforward)
		VALUES (:date, :remote, :xforward)");
		$stmt->bindParam(':date', $date);
		$stmt->bindParam(':remote', $remote);
		$stmt->bindParam(':xforward', $xforward);

		// insert a row
		$date = date('Y-m-d H:i:s');
		$remote = $_SERVER['REMOTE_ADDR'];
		$xforward = array_key_exists('HTTP_X_FORWARDED_FOR', $_SERVER) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : "";
		$stmt->execute();
	}
	catch(PDOException $e){
		throw $e;
	}
}
catch(Exception $e){
	echo "";
}
$conn = null; 
?>