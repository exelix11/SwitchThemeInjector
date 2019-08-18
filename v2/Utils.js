window.DomUtil = {
	Click: function (element) {
		element.click();
	}
};

function DownloadUrl (filename, url) {
	var link = document.createElement('a');
	link.download = filename;
	link.href = url;
	document.body.appendChild(link);
	link.click();
	document.body.removeChild(link);
}

function DownloadBlob (fileName, data) {
	var blob, url;
	console.log(typeof data);
	var dataArr = Blazor.platform.toUint8Array(data);
	blob = new Blob([dataArr], {
		type: "application/octet-stream"
	});
	url = window.URL.createObjectURL(blob);
	DownloadUrl(Blazor.platform.toJavaScriptString(fileName), url);
	setTimeout(function () {
		return window.URL.revokeObjectURL(url);
	}, 1000);
}