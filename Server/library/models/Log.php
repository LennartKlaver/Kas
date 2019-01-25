<?php

class Log{
    private $connection;

    //Table name.
    private $tablename = "Log";

    //Table fields.
    public $date;
    public $remoteip;
    public $xforward;

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
        $query = "INSERT INTO " . $this->tablename . " (date, remote, xforward) " .
                "VALUES (:date, :remote, :xforward)";
        $stmt = $this->connection->prepare($query);
        $stmt->execute([
            "date" => $this->date,
            "remote" => $this->remoteip,
            "xforward" => $this->xforward]);
    }

    /**
     * Read
     */
    public function read(){

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