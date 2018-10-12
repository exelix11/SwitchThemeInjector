function JsSZSRead(ev) {
    if (!ev[0].name.endsWith('szs')) {
        alert('This is not an szs file');
        return;
    }
    var reader = new FileReader();
    reader.onloadend = function (evt) {
        SwitchThemesOnline.App.UploadSZS(new Uint8Array(evt.target.result));
    }
    if (typeof ev[0] != 'undefined')
        reader.readAsArrayBuffer(ev[0]);
}

function JsDDSRead(ev) {
    if (!ev[0].name.endsWith('dds')) {
        alert('Only dds files are supported');
        return;
    }
    var reader = new FileReader();
    reader.onloadend = function (evt) {
            SwitchThemesOnline.App.UploadDDS(new Uint8Array(evt.target.result), ev[0].name);
    }
    if (typeof ev[0] != 'undefined')
        reader.readAsArrayBuffer(ev[0]);
}

function JsThemePartRead(ev) {
    if (!ev[0].name.endsWith('szs')) {
        alert('This is not an szs file');
        return;
    }
    var reader = new FileReader();
    reader.onloadend = function (evt) {
        SwitchThemesOnline.App.AutoThemeFileUploaded(new Uint8Array(evt.target.result));
    }
    if (typeof ev[0] != 'undefined')
        reader.readAsArrayBuffer(ev[0]);
}

downloadBlob = function (data, fileName, mimeType) {
    var blob, url;
    blob = new Blob([data], {
        type: mimeType
    });
    url = window.URL.createObjectURL(blob);
    downloadURL(url, fileName, mimeType);
    setTimeout(function () {
        return window.URL.revokeObjectURL(url);
    }, 1000);
};

downloadURL = function (data, fileName) {
    var a;
    a = document.createElement('a');
    a.href = data;
    a.download = fileName;
    document.body.appendChild(a);
    a.style = 'display: none';
    a.click();
    a.remove();
};