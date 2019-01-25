<?php
class Measurement{
    private $connection;

    //Table name.
    private $tablename = "measurements";

    //Table fields.
    public $id;
    public $date;
    public $temperature;
    public $moisture;
    public $humidity;
    public $light;
    public $distance;
    public $water;

    /** 
     * Constructor
     * @param con the database connection string.
     */
    public function __construct($con){
        $this->connection = $con;
    }

    /**
     * Create
     */
    public function create(){
        $query = "INSERT INTO " . $this->tablename . "(date, temperature, moisture, humidity, light, distance, water) " .
                "VALUES(:date, :temperature, :moisture, :humidity, :light, :distance, :water)";

        $stmt = $this->connection->prepare($query);
        $stmt->execute([
            "date" => $this->date,
            "temperature" => $this->temperature,
            "moisture" => $this->moisture,
            "humidity" => $this->humidity,
            "light" => $this->light,
            "distance" => $this->distance,
            "water" => $this->water]);
    }

    /**
     * Read
     */
    public function read(){
        $query = "SELECT * FROM " . $this->tablename;
        $stmt = $this->connection->query($query);
        //$stmt = $this->connection->prepare($query);
       // $stmt->execute();

        return $stmt;
    }

    /**
     * Update
     */
    public function update(){
    }

    /**
     * Delete
     */
    public function delete(){
    }
}
?>