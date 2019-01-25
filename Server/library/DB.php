<?php

class DB {
    private $host = "localhost";
    private $username = "itpweb_nl_dbkas";
    private $password = "FRTKD4XkMBut";
    private $database = "itpweb_nl_dbkas";

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