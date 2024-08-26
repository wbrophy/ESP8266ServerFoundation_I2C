function WordToQWord(x_word)
{
 x_word = x_word << 48;
 x_word = x_word >> 48;
 return x_word;
}
function TwosCompliment16(x_word)
{
 x_word = x_word << 48;
 x_word = x_word >= 0 ? x_word : (~x_word);
 x_word = (x_word >> 48) + 1;
 return x_word;
}

let x_deviceId = 0x0D;
let xstr_deviceId = "0x0D";
let x_averageX = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
let x_averageY = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
let x_averageZ = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
let x_averageT = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

socket = new WebSocket('ws:/' + '/' + location.host + ':81');
socket.onopen = function(e) { document.getElementById('socket_status').innerHTML = location.host; };
socket.onclose = function(e) { document.getElementById('socket_status').innerHTML = "closed"; };
socket.onerror = function(e) { document.getElementById('socket_status').innerHTML = 'socket error'; };
socket.onmessage = function(e)
 {
  let x_var = JSON.parse("{}");
  let x_data = JSON.parse(e.data);
  let x_device = x_data.i2cbus.device.filter(function (entry) { return entry.address == xstr_deviceId; });
  x_var["x"] = TwosCompliment16((x_device[0].getdata[0].data[1] << 8 | x_device[0].getdata[0].data[0]));
  x_var["z"] = TwosCompliment16((x_device[0].getdata[0].data[3] << 8 | x_device[0].getdata[0].data[2]));
  x_var["y"] = TwosCompliment16((x_device[0].getdata[0].data[5] << 8 | x_device[0].getdata[0].data[4]));
  x_var["t"] = TwosCompliment16((x_device[0].getdata[0].data[8] << 8 | x_device[0].getdata[0].data[7]));
  x_averageX.shift();
  x_averageX.push(x_var["x"]);
  x_averageY.shift();
  x_averageY.push(x_var["y"]);
  x_averageZ.shift();
  x_averageZ.push(x_var["z"]);
  x_averageT.shift();
  x_averageT.push(x_var["t"]);
  let loopx = 0;
  for (x_loop = 0; x_loop < x_averageX.length; x_loop++)
   { loopx += x_averageX[x_loop]; }
  x_xaxis = loopx / x_averageX.length;
  let loopy = 0;
  for (x_loop = 0; x_loop < x_averageY.length; x_loop++)
   { loopy += x_averageY[x_loop]; }
  x_yaxis = loopy / x_averageY.length;
  let loopz = 0;
  for (x_loop = 0; x_loop < x_averageZ.length; x_loop++)
   { loopz += x_averageZ[x_loop]; }
  x_zaxis = loopz / x_averageZ.length;
  let loopt = 0;
  for (x_loop = 0; x_loop < x_averageT.length; x_loop++)
   { loopt += x_averageT[x_loop]; }
  x_temp = loopt / x_averageT.length;

  document.getElementById('raw_data').innerHTML = e.data;
  document.getElementById('x_var_data').innerHTML = JSON.stringify(x_var);
  document.getElementById('x_device').innerHTML = JSON.stringify(x_device);
  document.getElementById('x_device').innerHTML = '<br>X: ' + String(x_xaxis.toFixed(0)) + '<br>Y: ' + String(x_yaxis.toFixed(0)) + '<br>Z: ' + String(x_zaxis.toFixed(0)) + '<br>T: ' + String(x_temp.toFixed(0));
 };