<?php

header("Content-Type:text/html;charset=utf-8");

// set some variables
$host = '192.168.1.2';

$port = 4000;

// don't timeout!

set_time_limit(0);

// create socket

$socket = socket_create(AF_INET, SOCK_STREAM, 0) or die("Could not create socket\n");

// bind socket to port

$result = socket_bind($socket, $host, $port) or die("Could not bind to socket\n");

// start listening for connections

$result = socket_listen($socket, 3) or die("Could not set up socket listener\n");

// accept incoming connections

// spawn another socket to handle communication

$spawn = socket_accept($socket) or die("Could not accept incoming connection\n");

// read client input

$input = socket_read($spawn, 1024) or die("Could not read input\n");

//recongnize client input




$tempe = strstr($input,'t',true);

$humid = strstr($input,'t');
$humid_tmp = strstr($humid,'h',ture);
$humid_op = str_replace('t','',$humid_tmp);
$lumen = strstr($input,'h');
$lumen_tmp = str_replace('l','',$lumen);
$lumen_op = str_replace('h','',$lumen_tmp);
//send data to mysql

$dbc = mysqli_connect("192.168.1.2", 'root', '', 'project') or die('Error connecting to MySQL server.');

$query = "INSERT INTO project (source, temperature, humid, lumen)
			VALUES ('$input','$tempe','$humid_op','$lumen_op')";
	
$result = mysqli_query($dbc, $query) or die('Error querying database.');

mysqli_close($dbc);


// clean up input string

$input = trim($input);

echo "Client Message : ".$input;

//send back

$output = "success $input" . "\n";


socket_write($spawn, $output, strlen ($output)) or die("Could not write output\n");

// close sockets

socket_close($spawn);

socket_close($socket);

//refresh

$url = "server.php";
echo "<script type='text/javascript'>";
echo "window.location.href='$url'";
echo "</script>";
header('refresh: 5;url="server.php"');
?>
