<?php

class DB {
    private $host = "localhost";
    private $username = "kas_account";
    private $password = "Npp9dRWx6cQXcdfZkDDHTfwA";
    private $database = "db_kas";

    public $connection;

    public function getConnection(){
        $this->connection = null;

        try{
            $this->connection = new PDO("mysql:host=" . $this->host . ";dbname=" . $this->database, $this->username, $this->password);
            //$this->connection->exec("set names utf8");
            $this->connection->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
        }catch(PDOException $e){
            throw($e);
        }

        return $this->connection;
    }
}
?>