const wifi_status = document.getElementById('wifi_status');
const mqtt_status = document.getElementById('mqtt_status');

function get_status() {
    fetch('/wifi_status').then(res => res.text()).then(body => {
	wifi_status.innerHTML = body;
    });

    fetch('/mqtt_status').then(res => res.text()).then(body => {
	mqtt_status.innerHTML = body;
    });
}

setInterval(get_status, 5000);

function submit(view) {
	loader.style.display = 'block';

	var init = {
		method: 'POST',
		body: new URLSearchParams(new FormData(form))
	};

	fetch('/' + view + '_setup', init).then(res => res.text()).then(text => {
		loader.style.display = 'none';
		result.innerHTML = text;
	});
}

var form = document.getElementById('wifi_form');
const submit_wifi = document.getElementById('submit_wifi');
const submit_mqtt = document.getElementById('submit_mqtt');
const loader = document.getElementById('loader');
const result = document.getElementById('result');

function set_view(view) {
	form.style.display = 'none';
	form = document.getElementById(view + '_form');
	form.style.display = 'grid';
}

wifi_status.parentNode.onclick = function() { set_view('wifi') };
mqtt_status.parentNode.onclick = function() { set_view('mqtt') };
submit_wifi.onclick = function() { submit('wifi') };
submit_mqtt.onclick = function() { submit('mqtt') };
set_view('wifi');
