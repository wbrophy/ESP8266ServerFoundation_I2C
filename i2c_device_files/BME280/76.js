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

let x_strdeviceId = "0x76";
let x_averageT = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
let x_averageP = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
let x_averageH = new Array(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);

socket = new WebSocket('ws:/' + '/' + location.host + ':81');
socket.onopen = function(e) { document.getElementById('socket_status').innerHTML = location.host; };
socket.onclose = function(e) { document.getElementById('socket_status').innerHTML = "closed"; };
socket.onerror = function(e) { document.getElementById('socket_status').innerHTML = 'socket error'; };
socket.onmessage = function(e)
 {
  let x_var = JSON.parse("{}");
  let x_data = JSON.parse(e.data);
  let x_device = x_data.i2cbus.device.filter(function (entry) { return entry.address == x_strdeviceId; });

  document.getElementById('raw_data').innerHTML = e.data;
 };
