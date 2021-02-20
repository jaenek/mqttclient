const wifi_status = document.getElementById('wifi_status');
const mqtt_status = document.getElementById('mqtt_status');
const form = document.forms[0];
const savebtn = document.getElementById('savebtn');
const loader = document.getElementById('loader');
const result = document.getElementById('result');

function get_status() {
  fetch('/wifi_status')
  .then(res => res.text())
  .then(body => {
    wifi_status.innerHTML = body;
  });

  fetch('/mqtt_status')
  .then(res => res.text())
  .then(body => {
    mqtt_status.innerHTML = body;
  });
}

setInterval(get_status, 5000);

function submit() {
  loader.style.display = 'block';

  var init = {
    method: 'POST',
    body: new URLSearchParams(new FormData(form))
  };

  fetch('/setup', init)
  .then(res => res.text())
  .then(text => {
    loader.style.display = 'none';
    result.innerHTML = text;
  });
}

savebtn.onclick = submit;
