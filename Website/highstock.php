<?php

$dbname = 'project';
$conn = mysqli_connect('localhost', 'root', '','project') ;//連接資料庫
mysqli_query($conn,"SET NAMES 'utf8'");//設定語系

mysqli_select_db($conn,$dbname);
$sql = "select unix_timestamp(dtime),temperature from project  order by dtime desc;";//遞減排序，取五筆

$result = mysqli_query($conn,$sql);

$str = "000";

while($row = mysqli_fetch_array($result)) {


$data[] = array((float)substr_replace((float)$row['unix_timestamp(dtime)'], '000', 10, 0),(float)$row['temperature']);

}

$data = json_encode($data);

?>

<script src="https://code.jquery.com/jquery-3.1.1.min.js"></script>
<script src="https://code.highcharts.com/stock/highstock.js"></script>
<script src="https://code.highcharts.com/stock/modules/exporting.js"></script>

<div id="container" style="height: 600px; min-width: 310px"></div>

<script>
$(function () {
    // Create the chart
    Highcharts.stockChart('container', {


        rangeSelector: {
            buttons: [{
                type: 'minute',
                count: 1,
                text: '1m'


        }, {
            type: 'all',
            text: 'All'
        }]

        },

        inputDateFormat: '%H:%M',
        inputEditDateFormat: '%H:%M',

        title: {
            text: 'Temperature'
        },

        xAxis: {
                tickPixelInterval: 120
        },

        yAxis: {
                floor: 0,
                tickInterval: 5
        },

        series: [{
            name: 'Temperature',
            data: <?php echo $data ?>,
            tooltip: {
                valueDecimals: 1
            }
        }]
    });
});

</script>

