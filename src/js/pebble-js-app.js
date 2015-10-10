var xhrRequest = function (url, type, successCallback, failureCallback, description) {
	var xhr = new XMLHttpRequest();
	xhr.onload = function () {
		var responseStatus = this.status;
		if (responseStatus == 0 || responseStatus > 300) {
			failureCallback(description, this.statusText);
		} else {
			successCallback(this.responseText);	
		}
		
  	};
  	xhr.open(type, url);
  	xhr.send();
};

function appMessageFailure(e, description) {
	console.log('Error sending ' + description + " to Pebble");
}

function appMessageSuccess(e, description) {
	console.log(description + " info successfully sent to Pebble");
}

function prayerTimeAPISuccess(responseText) {
	// responseText contains a JSON object with prayer time info
	var json = JSON.parse(responseText);

	var timesObj = json.items[0];

	// parse the 5 prayer times
	var fajr = timesObj.fajr;
	console.log('JS received Fajr ' + fajr);
	var dhuhr = timesObj.dhuhr;
	console.log('JS received Dhuhr ' + dhuhr);
	var asr = timesObj.asr;
	console.log('JS received Asr ' + asr);
	var maghrib = timesObj.maghrib;
	console.log('JS received Maghrib ' + maghrib);
	var isha = timesObj.isha;
	console.log('JS received Isha ' + isha);

	prayerTimes = fajr+","+dhuhr+","+asr+","+maghrib+","+isha;

	// Assemble dictionary using our keys
	var dictionary = {
		'KEY_PRAYER_TIMES': prayerTimes
	};

	// Send to Pebble (C code)
	Pebble.sendAppMessage(dictionary, 
		function(e) {
			console.log("Prayer time info sent to Pebble successfully");
		},
		function(e) {
			console.log("Error sending prayer time info to Pebble");
		}
	);
}

function sunriseSunsetAPISuccess(responseText) {
	var json = JSON.parse(responseText);

	var sunrise = json.results.sunrise;
	var sunriseLocal = new Date(sunrise);
	console.log('JS received sunrise ' + sunriseLocal);
	var sunriseHour = sunriseLocal.getHours();
	var sunriseMinute = sunriseLocal.getMinutes();

	var sunset = json.results.sunset;
	var sunsetLocal = new Date(sunset);
	console.log('JS received sunset ' + sunsetLocal);
	var sunsetHour = sunsetLocal.getHours();
	var sunsetMinute = sunsetLocal.getMinutes();

	

	var dictionary = {
		'KEY_SUNRISE_HOUR': sunriseHour,
		'KEY_SUNRISE_MINUTE': sunriseMinute,
		'KEY_SUNSET_HOUR': sunsetHour,
		'KEY_SUNSET_MINUTE': sunsetMinute
	};

	// Send to Pebble (C code)
	Pebble.sendAppMessage(dictionary, 
		function(e) {
			console.log("Sunrise/sunset info sent to Pebble successfully");
		},
		function(e) {
			console.log("Error sending sunrise/sunset info to Pebble");
		}
	);
}

function APIFailure(description, statusText) {
	console.log(description + " API request failed with status: " + statusText);
}




// We have a location, so we can request the info we need
function locationSuccess(pos) {
	var lat = pos.coords.latitude;
	var lon = pos.coords.longitude;

	var prayerTimeUrl = 'http://muslimsalat.com/' + lat + ',' + lon +
		'.json?key=f65cc83413888cde54722d1101f07002';
  	// Send request to MuslimSalat
  	xhrRequest(prayerTimeUrl, 'GET', prayerTimeAPISuccess, APIFailure, "Prayer time");

  	var sunriseSunsetUrl = 'http://api.sunrise-sunset.org/json?lat='+lat+'&lng='+lon+'&formatted=0';
  	// Send request to sunrise-sunset.org
  	xhrRequest(sunriseSunsetUrl, 'GET', sunriseSunsetAPISuccess, APIFailure, "Sunrise/sunset");

}

// We don't have a location, so we can't find any of the info we need
function locationError(err) {
  	console.log('Error requesting location!');
  	// TODO: Send message to C code so the display indicates location
  	// could not be found
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  	function(e) {
    	console.log('PebbleKit JS ready!');

    	navigator.geolocation.getCurrentPosition(
    		locationSuccess,
    		locationError,
    		{timeout: 15000, maximumAge: 60000}
  		);
  	}
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  	function(e) {
    	console.log('AppMessage received!');
    	getPrayerTimes();
  	}                     
);
