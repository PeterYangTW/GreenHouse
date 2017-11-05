<?php

$dbname = 'project';
$conn = mysqli_connect('localhost', 'root', '','project') ;//連接資料庫
mysqli_query($conn,"SET NAMES 'utf8'");//設定語系

mysqli_select_db($conn,$dbname);
$sql = "select date_format(dtime, '%m/%d-%H:%i'),temperature from project  order by dtime desc  limit 5 ";

$result = mysqli_query($conn,$sql);

while($row = mysqli_fetch_array($result)) {

$time[]=$row["date_format(dtime, '%m/%d-%H:%i')"];

$temp[]=(float)$row['temperature'];


}


sort($time);

$time = json_encode($time); //调用函数json_encode生成json数据。

$trev = array_reverse($temp);

$data = array(array("name"=>"temp","data"=>$trev));

$data = json_encode($data);




?>


 <script src="http://apps.bdimg.com/libs/jquery/2.1.4/jquery.min.js"></script>
<script src="http://code.highcharts.com/highcharts.js"></script>   
<script src="http://code.highcharts.com/modules/data.js"></script> 
<script type=”text/javascript” src=”http://cdn.hcharts.cn/jquery/jquery-1.8.3.min.js”></script>
<script type=”text/javascript” src=”http://www.hcharts.cn/demo/js/highcharts.js”></script>
<script type=”text/javascript” src=”http://www.hcharts.cn/demo/js/exporting.js”></script>
<script>

$(function () {
        $('#container').highcharts({
            title: {
                text: '溫度圖表',
                x: -20 //center
            },
            subtitle: {
                text: '',
                x: -20
            },
            xAxis: {
                categories:  <?php echo $time; ?>
            },
            yAxis: {
                title: {
                    text: '°C'
                },
                plotLines: [{
                    value: 0,
                    width: 1,
                    color: '#808080'
                }]
            },
            tooltip: {
                valueSuffix: '°C'
            },
            legend: {
                layout: 'vertical',
                align: 'right',
                verticalAlign: 'middle',
                borderWidth: 0
            },
            series:<?php echo $data?>
        });
    });
</script>

<html>

<body>
<div id="container" ></div>
</body>
</html>
