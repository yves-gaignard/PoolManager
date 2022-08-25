setInterval(function getData() {
	var xhttp = new XMLHttpRequest();
	//xhttp.responseType = 'json';
	
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			var jsonResponse = JSON.parse(this.responseText);
		    document.getElementById("AirInTemp" ).innerHTML = jsonResponse.IATemp;
		    document.getElementById("AirOutTemp").innerHTML = jsonResponse.OATemp;
		    document.getElementById("WaterTemp" ).innerHTML = jsonResponse.WaTemp;
		}
	};

	xhttp.open("GET", "getMeasures", true);
	xhttp.send();
}, 2000);
