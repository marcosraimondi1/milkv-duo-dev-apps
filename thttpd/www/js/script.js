function toggleLED() {
	      var xhr = new XMLHttpRequest();
	      xhr.onreadystatechange = function() {
		              if (xhr.readyState == 4 && xhr.status == 200) {
					alert('response: '+ xhr.responseText);
			      }
		            };
	      xhr.open("GET", "toggle_led.cgi", true);
	      xhr.send();
}

document.getElementById('myButton').addEventListener('click', function() {
	toggleLED();
});

function getStats() {
	      var xhr = new XMLHttpRequest();
	      xhr.onreadystatechange = function() {
		              if (xhr.readyState == 4 && xhr.status == 200) {
				                document.getElementById("stats").innerHTML = xhr.responseText;
				              }
		            };
	      xhr.open("GET", "stats.cgi", true);
	      xhr.send();
	    }

setInterval(getStats, 1000); // Actualiza cada 5 segundos

