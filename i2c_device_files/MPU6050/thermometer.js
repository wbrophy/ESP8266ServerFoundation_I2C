function WordToQWord(x_word)
{
 x_word = x_word << 48;
 x_word = x_word >> 48;
 return x_word;
}

let x_averageTemp = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

socket = new WebSocket('ws:/' + '/' + location.host + ':81');
socket.onopen = function(e) { document.getElementById('socket_status').innerHTML = location.host; };
socket.onclose = function(e) { document.getElementById('socket_status').innerHTML = "closed"; };
socket.onerror = function(e) { document.getElementById('socket_status').innerHTML = 'socket error'; };
socket.onmessage = function(e)
 {
  let x_var = JSON.parse("{}");
  let x_data = JSON.parse(e.data);
  let x_device = x_data.i2cbus.device.filter(function (entry) { return entry.address == '0x68'; });
  x_var["temp"] = WordToQWord((x_device[0].getdata[0].data[6] << 8 | x_device[0].getdata[0].data[7])) / 340 + 36.53;

  x_averageTemp.shift();
  x_averageTemp.push(x_var["temp"]);
  let loopTemp = 0;
  for (x_loop = 0; x_loop < x_averageTemp.length; x_loop++)
   { loopTemp += x_averageTemp[x_loop]; }
  let x_temp = loopTemp / x_averageTemp.length;

  x_f = (x_temp * 1.8) + 32;
  document.getElementById('temperature').innerHTML = String(x_f.toFixed(0)) + "&degF";
  //document.getElementById('raw_data').innerHTML = e.data;
 };
