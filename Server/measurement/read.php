<?php
include_once "../library/JSONArray.php";
include_once "../library/models/Measurement.php";
include_once "../library/models/Log.php";
include_once "../library/DB.php";

try {
	if (!($key = filter_input(INPUT_GET, 'key', FILTER_SANITIZE_STRING))) {
		throw new Exception('No API key specified!'); 
	}

	//Check the API key for authentication.
	if ($key != "XSYUqxMRRMaJ5TeskMUKdxwmzATWaeJLpMVya59YLehF2gUw"){
		throw new Exception('Invalid API key!');
	}
	 
	//Make sure that it is a POST request.
	if(strcasecmp($_SERVER['REQUEST_METHOD'], 'GET') != 0){
		throw new Exception('Request method must be GET!');
	}
	 
	try {
        $db = new DB();
        $connection = $db->getConnection();

        //Create a measurement.
        $measurement = new Measurement($connection);
        //Read.
        $stmt = $measurement->read();
        $count = $stmt->rowCount();

        if($count < 1){
            echo "";
            return;
        }

        $measurements = array();
        while($row = $stmt->fetch(PDO::FETCH_ASSOC)){
            extract($row);
            $p = array(
                "id" => $id,
                "date" => $date,
                "temperature" => $temperature,
                "moisture" => $moisture,
                "humidity" => $humidity,
                "light" => $light,
                "distance" => $distance,
                "water" => $water
            );

            array_push($measurements, $p);
        }

        echo json_encode($measurements);

        //log entry.
        $log = new Log($connection);
		$log->date = date('Y-m-d H:i:s');
		$log->remoteip = $_SERVER['REMOTE_ADDR'];
        $log->xforward = array_key_exists('HTTP_X_FORWARDED_FOR', $_SERVER) ? $_SERVER['HTTP_X_FORWARDED_FOR'] : "";
        // Send to database.
        $log->create();
		
	}
	catch(PDOException $e){
		throw $e;
	}
}
catch(Exception $e){
	echo "";
}
?>