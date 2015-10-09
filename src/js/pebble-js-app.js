var xhrRequest = function (url, type, successCallback, failureCallback) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		var responseStatus = this.status;
		if (responseStatus == 0 || responseStatus > 300) {
			failureCallback(this.statusText);
		} else {
			successCallback(this.responseText);	
		}
		
  	};
  	xhr.open(type, url);
  	xhr.send();
};

function prayerTimeAPISuccess(responseText) {
	// responseText contains a JSON object with prayer time info
	var json = JSON.parse(responseText);

	var times = json.items[0];

	// parse the 5 prayer times
	var fajr = times.fajr;
	console.log('JS received Fajr ' + fajr);
	var dhuhr = times.dhuhr;
	console.log('JS received Dhuhr ' + dhuhr);
	var asr = times.asr;
	console.log('JS received Asr ' + asr);
	var maghrib = times.maghrib;
	console.log('JS received Maghrib ' + maghrib);
	var isha = times.isha;
	console.log('JS received Isha ' + isha);

	// Assemble dictionary using our keys
	var dictionary = {
		'KEY_FAJR': fajr,
		'KEY_DHUHR': dhuhr,
		'KEY_ASR': asr,
		'KEY_MAGHRIB': maghrib,
		'KEY_ISHA': isha
	};

	// Send to Pebble (C code)
	Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log('Prayer time info sent to Pebble successfully!');
		},
		function(e) {
			console.log('Error sending weather info to Pebble!');
		}
	);
}

function prayerTimeAPIFailure(statusText) {
	console.log("API request failed with status: " + statusText);
}

function locationSuccess(pos) {
	var url = 'http://muslimsalat.com/' + pos.coords.latitute + ',' +
		pos.coords.longitude + '.json?key=f65cc83413888cde54722d1101f07002'

  	// Send request to MuslimSalat
  	xhrRequest(url, 'GET', prayerTimeAPISuccess, prayerTimeAPIFailure);
}

function locationError(err) {
  	console.log('Error requesting location!');
}

function getPrayerTimes() {
	navigator.geolocation.getCurrentPosition(
    	locationSuccess,
    	locationError,
    	{timeout: 15000, maximumAge: 60000}
  	);
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  	function(e) {
    	console.log('PebbleKit JS ready!');

    	// Get the prayer times
    	getPrayerTimes();
  	}
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  	function(e) {
    	console.log('AppMessage received!');
    	getPrayerTimes();
  	}                     
);
