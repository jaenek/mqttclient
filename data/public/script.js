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

function submit(url, form) {
	loader.style.display = 'block';

	let init = {
		method: 'POST',
		body: new URLSearchParams(new FormData(form))
	};

	fetch(url, init).then(res => res.text()).then(text => {
		loader.style.display = 'none';
		result.innerHTML = text;
	});
}

const wifi_form = document.getElementById('wifi_form');
const mqtt_form = document.getElementById('mqtt_form');
const topic_form = document.getElementById('topic_form');
const config_form = document.getElementById('config_form');
const submit_wifi = document.getElementById('submit_wifi');
const submit_mqtt = document.getElementById('submit_mqtt');
const submit_topic = document.getElementById('submit_topic');
const loader = document.getElementById('loader');
const result = document.getElementById('result');

function set_view(view) {
	if (view == 'wifi') {
		mqtt_form.style.display = 'none';
		topic_form.style.display = 'none';
		config_form.style.display = 'none';
		wifi_form.style.display = 'grid';
	} else if (view == 'mqtt') {
		wifi_form.style.display = 'none';
		mqtt_form.style.display = 'grid';
		topic_form.style.display = 'grid';
		config_form.style.display = 'grid';
	}
}

wifi_status.parentNode.onclick = function() { set_view('wifi') };
mqtt_status.parentNode.onclick = function() { set_view('mqtt') };
submit_wifi.onclick = function() { submit('/wifi_setup', wifi_form) };
submit_mqtt.onclick = function() { submit('/mqtt_setup', mqtt_form) };
submit_topic.onclick = function() { submit('/topic_setup', topic_form); setup_configs(); };

set_view('mqtt');

const readings = document.getElementById('readings');

fetch('/readings').then(res => res.text()).then(body => {
	let options;
	let lines = body.split('\n').slice(0,-1);
	lines.forEach(item => {
		options	+= `<option name="${item}">${item}</option>\n`;
	});
	readings.innerHTML = options;
});

function setup_configs() {
	fetch('/configs').then(res => res.text()).then(body => {
		let id = 0;
		let configs = '';
		let lines = body.split('\n').slice(0, -1);
		lines.forEach(line => {
			let v = line.split(',');
			configs	+= `<input type="checkbox" name="${v[0]}" id="${id}"><label for="${id}">${v[0]} - (${v[2]}) - > ${v[1]}</label>\n`;
			id++;
		});
		config_form.innerHTML = '';
		config_form.innerHTML += '<h3>Konfiguracja:</h3>';
		config_form.innerHTML += configs;
		config_form.innerHTML += '<input type="button" id="submit_delete" value="Usuń">';
		document.getElementById('submit_delete').onclick = function() {
			submit('/delete', config_form);
			setup_configs()
		};
	});
}

setup_configs();
