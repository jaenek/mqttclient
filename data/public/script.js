const mqtt_status = document.getElementById('mqtt_status');
const loader = document.getElementById('loader');

function get_status() {
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
		document.getElementById('result').innerHTML = text;
	});
}

document.getElementById('submit_mqtt').onclick = function() {
	submit('/mqtt_setup', document.getElementById('mqtt_form'))
};

document.getElementById('submit_topic').onclick = function() {
	submit('/topic_setup', document.getElementById('topic_form'));
	setup_configs();
};

fetch('/readings').then(res => res.text()).then(body => {
	let options;
	let lines = body.split('\n').slice(0,-1);
	lines.forEach(item => {
		options	+= `<option name="${item}">${item}</option>\n`;
	});
	document.getElementById('readings').innerHTML = options;
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
