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

function getHourAndMinute(timeList) {
	var hourAndMinute = timeList[0].split(":");
	var hour = parseInt(hourAndMinute[0]);
	var minute = parseInt(hourAndMinute[1]);
	var meridian = timeList[1];
	if (meridian == "pm" && hour != 12) {
		hour = hour + 12;
	}
	if (meridian == "am" && hour == 12) {
		hour = 0;
	}
	return [hour,minute];
}

function prayerTimeAPISuccess(responseText) {
	// responseText contains a JSON object with prayer time info
	var json = JSON.parse(responseText);

	var timesObj = json.items[0];

	// parse the 5 prayer times
	var fajr = getHourAndMinute(timesObj.fajr.split(" "));
	var fajrHour = fajr[0], fajrMinute = fajr[1];
	
	var dhuhr = getHourAndMinute(timesObj.dhuhr.split(" "));
	var dhuhrHour = dhuhr[0], dhuhrMinute = dhuhr[1];

	var asr = getHourAndMinute(timesObj.asr.split(" "));
	var asrHour = asr[0], asrMinute = asr[1];

	var maghrib = getHourAndMinute(timesObj.maghrib.split(" "));
	var maghribHour = maghrib[0], maghribMinute = maghrib[1];

	var isha = getHourAndMinute(timesObj.isha.split(" "));
	var ishaHour = isha[0], ishaMinute = isha[1];

	console.log("JS received prayer times "+
		fajrHour+":"+fajrMinute+", "+
		dhuhrHour+":"+dhuhrMinute+", "+
		asrHour+":"+asrMinute+", "+
		maghribHour+":"+maghribMinute+", "+
		ishaHour+":"+ishaMinute
	);

	// Assemble dictionary using our keys
	var dictionary = {
		'KEY_FAJR_HOUR': fajrHour, 'KEY_FAJR_MINUTE': fajrMinute,
		'KEY_DHUHR_HOUR': dhuhrHour, 'KEY_DHUHR_MINUTE': dhuhrMinute,
		'KEY_ASR_HOUR': asrHour, 'KEY_ASR_MINUTE': asrMinute,
		'KEY_MAGHRIB_HOUR': maghribHour, 'KEY_MAGHRIB_MINUTE': maghribMinute,
		'KEY_ISHA_HOUR': ishaHour, 'KEY_ISHA_MINUTE': ishaMinute
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
