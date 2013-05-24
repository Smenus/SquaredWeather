<?php
        define('API_KEY', 'get-your-own');  // http://developer.forecast.io

        $payload = json_decode(file_get_contents('php://input'), true);
        $payload[1] /= 10000;
        $payload[2] /= 10000;
        $url = "http://api.forecast.io/forecast/" . API_KEY . "/$payload[1],$payload[2]?units=$payload[3]&exclude=minutely,hourly,daily,alerts";
        
        $forecast = json_decode(@file_get_contents($url));
        
        if(!$forecast) {
                die();
        }
        
        $response = array();
        $icons = array(
                'clear-day' => 0,
                'clear-night' => 1,
                'rain' => 2,
                'snow' => 3,
                'sleet' => 4,
                'wind' => 5,
                'fog' => 6,
                'cloudy' => 7,
                'partly-cloudy-day' => 8,
                'partly-cloudy-night' => 9
        );
        $icon_id = $icons[$forecast->currently->icon];
        $is_c = strcasecmp($forecast->flags->units, "us");
        
        $response[1] = array('b', $icon_id);
        $response[2] = array('s', round($forecast->currently->temperature));
        $response[3] = array('b', ($is_c != 0));
        
        header("Cache-Control: max-age=1680");
        
        print json_encode($response);
?>
