const mqtt_status = document.getElementById('mqtt_status');

function get_status() {
    fetch('/mqtt_status').then(res => res.text()).then(body => {
	mqtt_status.innerHTML = body;
    });
}

setInterval(get_status, 5000);

function submit(url, form) {
	loader.style.display = 'block';

	var init = {
		method: 'POST',
		body: new URLSearchParams(new FormData(form))
	};

	fetch(url, init).then(res => res.text()).then(text => {
		loader.style.display = 'none';
		result.innerHTML = text;
	});
}

const mqtt_form = document.getElementById('mqtt_form');
const topic_form = document.getElementById('topic_form');
const submit_mqtt = document.getElementById('submit_mqtt');
const submit_topic = document.getElementById('submit_topic');
const loader = document.getElementById('loader');
const result = document.getElementById('result');

submit_mqtt.onclick = function() { submit('/mqtt_setup', mqtt_form) };
submit_topic.onclick = function() { submit('/topic_setup', topic_form) };

const readings = document.getElementById('readings');

fetch('/readings').then(res => res.text()).then(body => {
	var options;
	var lines = body.split('\n');
	lines.forEach(item => {
		options	+= '<option name="' + item + '">' + item + '</option>\n';
	});
	readings.innerHTML = options;
});
