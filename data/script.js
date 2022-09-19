// declare a function to refresh gauge values every 10 seconds
refreshGaugeData(); // display at page load
setInterval( refreshGaugeData, 10000); // declare a function to refresh gauge every 10 seconds

// Declare a button to refresh the control status on demand
let btnRefreshControl = document.getElementById("refreshControls");

// declare an event based the button to refresh the status
refreshControlInfo(); // display at page load
btnRefreshControl.addEventListener('click', refreshControlInfo);

// --------------------------------------------------------------
// Get data for gauge display
// --------------------------------------------------------------
function refreshGaugeData() {
	var xhttp = new XMLHttpRequest();
	
	xhttp.onreadystatechange = function() {
		if (this.readyState == 4 && this.status == 200) {
			var jsonResponse = JSON.parse(this.responseText);
			document.getElementById("gaugeTempAirIn" ).setAttribute("data-value", jsonResponse.IATemp);
			document.getElementById("gaugeTempAirOut").setAttribute("data-value", jsonResponse.OATemp);
			document.getElementById("gaugeTempWater" ).setAttribute("data-value", jsonResponse.WaTemp);
			document.getElementById("gaugePH"        ).setAttribute("data-value", jsonResponse.pH);
			document.getElementById("gaugeORP"       ).setAttribute("data-value", jsonResponse.Orp);
			document.getElementById("gaugePressure"  ).setAttribute("data-value", jsonResponse.PSI);
			document.getElementById("gaugePhTankFill").setAttribute("data-value", jsonResponse.pHFill);
			document.getElementById("gaugeChlorineTankFill").setAttribute("data-value", jsonResponse.ChFill);
			let FiltrationTimeRatio=( jsonResponse.DFUpt / jsonResponse.DFTrgt *100);
			document.getElementById("gaugeFiltration").setAttribute("data-value", FiltrationTimeRatio);
		}
	};

	xhttp.open("GET", "getMeasures", true);
	xhttp.send();
}
// --------------------------------------------------------------
// Get control information and display them in the control table
// --------------------------------------------------------------
function refreshControlInfo() {
	let myTable = document.getElementById('controlTable');
    let controls = [];

	let xhttp = new XMLHttpRequest();
	xhttp.open("GET", "getMeasures", false); // false : means synchronous call
	xhttp.send();
	if (xhttp.readyState == 4 && xhttp.status == 200) {
		let jsonResponse = JSON.parse(xhttp.responseText);
		controls.push({ name:"Auto Mode",      value: (jsonResponse.Auto   ?"ON":"OFF")});
		controls.push({ name:"Winter Mode",    value: (jsonResponse.Winter ?"ON":"OFF")});
		controls.push({ name:"Filtration",     value: (jsonResponse.FPmpS  ?"ON":"OFF")});
		controls.push({ name:"pH regulation",  value: (jsonResponse.pHROO  ?"ON":"OFF")});
		controls.push({ name:"pH Pump",        value: (jsonResponse.pHPmpS ?"ON":"OFF")});
		controls.push({ name:"Orp regulation", value: (jsonResponse.OrpROO ?"ON":"OFF")});
		controls.push({ name:"Chlorine Pump",  value: (jsonResponse.OrpPmpS?"ON":"OFF")});
		controls.push({ name:"pH PID",                         value: jsonResponse.pHWS});
		controls.push({ name:"Orp PID",                        value: jsonResponse.ChlWS});
		controls.push({ name:"Day Filtration Uptime",          value: convertHMS(jsonResponse.DFUpt)});
		controls.push({ name:"Day Filtration Target",          value: convertHMS(jsonResponse.DFTrgt)});
		controls.push({ name:"Period Filtration Start",        value: Time_tToDate(jsonResponse.PFSta).toLocaleTimeString('fr-FR')});
		controls.push({ name:"Period Filtration End",          value: Time_tToDate(jsonResponse.PFEnd).toLocaleTimeString('fr-FR')});
		controls.push({ name:"pH- uptime limit (min)",         value: jsonResponse.pHUTL});
		controls.push({ name:"Chlorine uptime limit (min)",    value: jsonResponse.OrpUTL});
		controls.push({ name:"Previous Day Filtration Uptime", value: convertHMS(jsonResponse.PDFUpt)});
		controls.push({ name:"Previous Day Filtration Target", value: convertHMS(jsonResponse.PDFTrgt)});
		controls.push({ name:"Reboot Number",                  value: jsonResponse.Reboot});
		//controls.push({ name:"Last Reboot Date",               value: Time_tToDate(jsonResponse.RTime).toLocaleTimeString('fr-FR')});
		controls.push({ name:"Last Reboot Date",               value: Time_tToDate(jsonResponse.RTime).toISOString()});
		controls.push({ name:"Last Day Reset Date",            value: Time_tToDate(jsonResponse.RSTime).toISOString()});
	}

	let headers = ['Control', 'Status'];

    let table = document.createElement('controlTable');
	let headerRow = document.createElement('tr');

    headers.forEach(headerText => {
        let header = document.createElement('th');
        let textNode = document.createTextNode(headerText);
        header.appendChild(textNode);
        headerRow.appendChild(header);
    });
    table.appendChild(headerRow);

    controls.forEach(ctrl => {
        let row = document.createElement('tr');

        Object.values(ctrl).forEach(text => {
            let cell = document.createElement('td');
            let textNode = document.createTextNode(text);
            cell.appendChild(textNode);
            row.appendChild(cell);
        })

        table.appendChild(row);
    });
	if (myTable.hasChildNodes()) {
		let old_table = myTable.firstChild;
		myTable.removeChild(old_table);
	  }
    myTable.appendChild(table);
}

// --------------------------------------------------------------
// Convert seconds to HH:MM:SS
// --------------------------------------------------------------
function convertHMS(value) {
    const sec = parseInt(value, 10); // convert value to number if it's string
    let hours   = Math.floor(sec / 3600);      // get hours
    let minutes = Math.floor(sec % 3600 / 60); // get minutes
    let seconds = Math.floor(sec % 3600 % 60); //  get seconds

    // add 0 if value < 10; Example: 2 => 02
    if (hours   < 10) {hours   = "0"+hours;}
    if (minutes < 10) {minutes = "0"+minutes;}
    if (seconds < 10) {seconds = "0"+seconds;}
    return hours+':'+minutes+':'+seconds; // Return is HH : MM : SS
}

// --------------------------------------------------------------
// Convert time_t to Date
// --------------------------------------------------------------
date: function Time_tToDate(time) {

	return new Date(1000 * time);
 }