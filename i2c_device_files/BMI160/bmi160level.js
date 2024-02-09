function WordToQWord(x_word)
{
 x_word = x_word << 48;
 x_word = x_word >> 48;
 return x_word;
}

let x_deviceId = 0x69;
let x_averageX = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
let x_averageY = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

socket = new WebSocket('ws:/' + '/' + location.host + ':81');
socket.onopen = function(e) { document.getElementById('socket_status').innerHTML = location.host; };
socket.onclose = function(e) { document.getElementById('socket_status').innerHTML = "closed"; };
socket.onerror = function(e) { document.getElementById('socket_status').innerHTML = 'socket error'; };
socket.onmessage = function(e)
 {
  let x_var = JSON.parse("{}");
  let x_data = JSON.parse(e.data);
  let x_device = x_data.i2cbus.device.filter(function (entry) { return entry.address == '0x69'; });
  x_var["x"] = WordToQWord((x_device[0].getdata[0].data[15] << 8 | x_device[0].getdata[0].data[14]));
  x_var["y"] = WordToQWord((x_device[0].getdata[0].data[17] << 8 | x_device[0].getdata[0].data[16]));
  x_var["z"] = WordToQWord((x_device[0].getdata[0].data[19] << 8 | x_device[0].getdata[0].data[18]));
  //x_var["temp"] = WordToQWord((x_device[0].getdata[0].data[6] << 8 | x_device[0].getdata[0].data[7])) / 340 + 36.53;

  x_anglex = (Math.atan(x_var["x"]/Math.sqrt((x_var["y"]*x_var["y"]) + (x_var["z"]*x_var["z"])))) * (180/Math.PI);

  x_anglex = x_anglex + x_device[0].userdata.accelerometer.offsetX;
  x_angley = (Math.atan(x_var["y"]/Math.sqrt((x_var["x"]*x_var["x"]) + (x_var["z"]*x_var["z"])))) * (180/Math.PI);
  x_angley = x_angley + x_device[0].userdata.accelerometer.offsetY;
  x_averageX.shift();
  x_averageX.push(x_anglex);
  x_averageY.shift();
  x_averageY.push(x_angley);
  let loopx = 0;
  for (x_loop = 0; x_loop < x_averageX.length; x_loop++)
   { loopx += x_averageX[x_loop]; }
  x_anglex = loopx / x_averageX.length;
  let loopy = 0;
  for (x_loop = 0; x_loop < x_averageY.length; x_loop++)
   { loopy += x_averageY[x_loop]; }
  x_angley = loopy / x_averageY.length;
  //x_f = (x_var["temp"] * 1.8) + 32;
  x_rotate = x_anglex.toFixed(1).toString() + 'deg';
  y_rotate = x_angley.toFixed(1).toString() + 'deg';
  document.getElementById('x_axis_degree').innerHTML = x_anglex.toFixed(1) + '&deg;'
  document.getElementById('y_axis_degree').innerHTML = x_angley.toFixed(1) + '&deg;'
  document.getElementById('x_axis_indicator').style.transform = 'rotate(' + x_rotate + ')';
  document.getElementById('y_axis_indicator').style.transform = 'rotate(' + y_rotate + ')';
  document.getElementById('raw_data').innerHTML = e.data;

  document.getElementById('raw_data').innerHTML = e.data;
  document.getElementById('x_var_data').innerHTML = JSON.stringify(x_var);
  //document.getElementById('x_device').innerHTML = String(x_var["temp"].toFixed(0)) + "C : " + String(x_f.toFixed(0)) + "F";
 };
