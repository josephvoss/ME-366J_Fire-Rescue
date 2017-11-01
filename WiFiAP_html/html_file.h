const char* html_data = "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\
<html>\
    <head>\
        <meta charset=\"utf-8\">\
        <title>\
            Deux Bot Controller\
        </title>\
        \
        <style>\
          /* Annoying grid stuff */\
            .grid-container{\
                width: 100%; \
                max-width: 1200px;\
                text-align: center;\
            }\
\
            /*-- our cleafix hack -- */ \
            .row:before, \
            .row:after {\
                content:\"\";\
                display: table ;\
                clear:both;\
            }\
\
            [class*='col-'] {\
                float: left; \
                min-height: 1px; \
                width: 16.66%; \
                /*-- our gutter -- */\
            }\
\
            .col-1{ width: 16.66%; }\
            .col-2{ width: 33.33%; }\
            .col-3{ width: 50%;    }\
            .col-4{ width: 66.66%; }\
            .col-5{ width: 83.33%; }\
            .col-6{ width: 100%;   }\
\
            .outline, .outline *{\
                outline: 1px solid #F6A1A1; \
            }\
\
   \
\
        </style>\
\
		<script>\
            function getColor(value){\
                var hue=((1-value)*255).toString(10);\
                return [\"hsl(\",hue,\",100%,50%)\"].join(\"\");\
            }\
\
            function table_create() {\
                var body = document.getElementById(\"temp_grid\");\
                var tbl = document.createElement('table');\
                tbl.style.width = '100%';\
                var tbdy = document.createElement('tbody');\
                for (var i = 0; i < 8; i++) {\
                    var tr = document.createElement('tr');\
                    tr.style.height=\"50px\";\
                    for (var j = 0; j < 8; j++) {\
                        var cellText = document.createTextNode(i);\
                        var td = document.createElement('td');\
                        td.appendChild(cellText);\
                        tr.appendChild(td);\
                    }\
                    tbdy.appendChild(tr);\
                }\
                tbl.appendChild(tbdy);\
                body.appendChild(tbl);\
            }\
            function table_update(temp_matrix) {\
                var values = temp_matrix.reduce(function (p, c) {\
                  return p.concat(c);\
                });\
                var max_temp = Math.max.apply(null, values);\
                var min_temp = Math.min.apply(null, values);\
\
                var grid = document.getElementById(\"temp_grid\");\
                var tbdy = grid.children[0].children[0];\
                for (var i = 0; i < 8; i++) {\
                    tr = tbdy.children[i];\
                    for (var j = 0; j < 8; j++) {\
                        td = tr.children[j];\
                       \
                        var myNumber = temp_matrix[i][j];\
                        var dec = myNumber - Math.floor(myNumber);\
                        myNumber = myNumber - dec;\
                        var formattedNumber = (\"0\" + myNumber).slice(-2) + dec.toString().substr(1);\
                        \
                        td.innerText = formattedNumber;\
                        value = (temp_matrix[i][j] - min_temp)/(max_temp-min_temp+1);\
                        td.style.backgroundColor = getColor(value);\
                    }\
                }\
            }\
            function sensor_update(sensor_array) {\
                document.getElementById(\"temp\").innerText=sensor_array[0];\
                document.getElementById(\"ultra\").innerText=sensor_array[1];\
                document.getElementById(\"oxy\").innerText=sensor_array[2];\
            }\
\
			var wsUri = \"ws://192.168.4.1:81\";\
			var output;\
			\
			function init() {\
			    testWebSocket();\
			}\
			\
			function testWebSocket() {\
			    websocket = new WebSocket(wsUri);\
			    websocket.onopen = function(evt) {\
			        console.log(\"CONNECTED\");\
			    };\
			    websocket.onclose = function(evt) {\
			        console.log(\"DISCONNECTED\");\
			    };\
			    websocket.onmessage = function(evt) {\
			    	var st = evt.data;\
                    var rec_data = st.split(',');\
                    var sensor_data = rec_data.splice(0,3);\
                    var mat_data = [];\
                    while(rec_data[0]) {\
                        mat_data.push(rec_data.splice(0,8));\
                    }\
                    table_update(mat_data);\
                    sensor_update(sensor_data);\
\
			    };\
			    websocket.onerror = function(evt) {\
			        console.log(\"ERROR: \" + evt.data);\
			    };\
			}\
			\
			function send_direction(value) {\
                websocket.send(value);\
			    console.log(\"SENT: \" + value);\
			}\
			\
			window.addEventListener(\"load\", init, false); \
\
        </script>\
    </head>\
    <body>\
        <h2>\
            Deux Bot Controller\
        </h2>\
       \
\
        <div class=\"grid-container\">\
            <div class=\"row\">\
                <div class=\"col-2\">\
                    <div class=\"row\">\
                          <div class=\"col-2\"></div>\
                          <div class=\"col-2\"> \
                              <input type=\"button\" id=\"up\" value=\"up\">\
                          </div>\
                          <div class=\"col-2\"></div>\
                    </div>\
                    <div class=\"row\">\
                          <div class=\"col-2\"> \
                              <input type=\"button\" id=\"left\" value=\"left\">\
                          </div>\
                          <div class=\"col-2\"> \
                              <input type=\"button\" id=\"down\" value=\"down\">\
                          </div>\
                          <div class=\"col-2\"> \
                              <input type=\"button\" id=\"right\" value=\"right\">\
                          </div>\
                    </div>\
\
                </div>\
                <div class=\"col-2\"></div>\
                <div class=\"col-2\" style=\"align:center;\">\
                    <div class=\"row\" id=\"temp_grid\" style=\"width:400px; height:400px;\">\
                    </div>\
\
                    <div class = \"row\" style=\"height:20px\"></div>\
                    <div class = \"row\" id=\"sensor_data\">\
                        <div class=\"col-2\">Temp: <object id=\"temp\">N/A</object></div>\
                        <div class=\"col-2\">Ultrasonic: <object id=\"ultra\">N/A</object></div>\
                        <div class=\"col-2\">Oxygen: <object id=\"oxy\">N/A</object></div>\
                    </div>\
\
                </div>\
            </div>\
        </div>\
\
\
    \
    <script>\
        table_create();\
\
        document.getElementById(\"up\").onclick = function(){\
            send_direction(document.getElementById(\"up\").value);\
        };\
        document.getElementById(\"down\").onclick = function(){\
            send_direction(document.getElementById(\"down\").value);\
        };\
        document.getElementById(\"left\").onclick = function(){\
            send_direction(document.getElementById(\"left\").value);\
        };\
        document.getElementById(\"right\").onclick = function(){\
            send_direction(document.getElementById(\"right\").value);\
        };\
\
        document.addEventListener(\"keydown\", event => {\
            if(event.key == \"w\") document.querySelector(\"#up\").click();             if(event.key == \"s\") document.querySelector(\"#down\").click(); \
            if(event.key == \"a\") document.querySelector(\"#left\").click();\
            if(event.key == \"d\") document.querySelector(\"#right\").click(); \
        });\
    </script>\
    </body>\
</html>";\

