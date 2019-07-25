;
;
var FileReaderComponent = /** @class */ (function () {
    function FileReaderComponent() {
        var _this = this;
        this.newFileStreamReference = 0;
        this.fileStreams = {};
        this.GetFileInfoFromElement = function (element, index, property) {
            if (!element.files) {
                return null;
            }
            var file = element.files.item(index);
            if (!file) {
                return null;
            }
            return _this.GetFileInfoFromFile(file);
        };
        this.Dispose = function (fileRef) {
            return delete (_this.fileStreams[fileRef]);
        };
        this.OpenRead = function (element, fileIndex) {
            if (!element.files) {
                throw 'No FileList available. Is this element a reference to an input of type="file"?';
            }
            var file = element.files.item(fileIndex);
            if (!file) {
                throw "No file with index " + fileIndex + " available.";
            }
            var fileRef = _this.newFileStreamReference++;
            _this.fileStreams[fileRef] = file;
            return fileRef;
        };
        this.ReadFileUnmarshalled = function (dotNetArrayPtr, readFileParamsPtr) {
            var readFileParams = JSON.parse(Blazor.platform.toJavaScriptString(readFileParamsPtr));
            var callBack = function (bytesRead, exception) {
                return DotNet.invokeMethodAsync("Blazor.FileReader", "ReadFileUnmarshalledCallback", { callBackId: readFileParams.callBackId, bytesRead: bytesRead, exception: exception });
            };
            var resolve = function (bytesRead) { return callBack(bytesRead, undefined); };
            var reject = function (exception) { return callBack(0, exception); };
            var dotNetBuffer = { toUint8Array: function () { return Blazor.platform.toUint8Array(dotNetArrayPtr); } };
            var file = _this.fileStreams[readFileParams.fileRef];
            try {
                var reader = new FileReader();
                reader.onload = (function (r) {
                    return function () {
                        try {
                            var contents = r.result;
                            var dotNetBufferView = dotNetBuffer.toUint8Array();
                            dotNetBufferView.set(new Uint8Array(contents));
                            resolve(contents.byteLength);
                        }
                        catch (e) {
                            reject(e);
                        }
                    };
                })(reader);
                reader.readAsArrayBuffer(file.slice(readFileParams.position, readFileParams.position + readFileParams.count));
                return true;
            }
            catch (e) {
                reject(e);
            }
            return false;
        };
        this.ReadFileMarshalledAsync = function (readFileParams) {
            return new Promise(function (resolve, reject) {
                var file = _this.fileStreams[readFileParams.fileRef];
                try {
                    var reader = new FileReader();
                    reader.onload = (function (r) {
                        return function () {
                            try {
                                var contents = r.result;
                                var data = contents ? contents.split(";base64,")[1] : null;
                                resolve(data);
                            }
                            catch (e) {
                                reject(e);
                            }
                        };
                    })(reader);
                    reader.readAsDataURL(file.slice(readFileParams.position, readFileParams.position + readFileParams.count));
                }
                catch (e) {
                    reject(e);
                }
            });
        };
    }
    FileReaderComponent.prototype.GetFileCount = function (element) {
        if (!element.files) {
            return -1;
        }
        var result = element.files.length;
        return result;
    };
    FileReaderComponent.prototype.ClearValue = function (input) {
        input.value = null;
    };
    ;
    FileReaderComponent.prototype.GetFileInfoFromFile = function (file) {
        var result = {
            lastModified: file.lastModified,
            name: file.name,
            size: file.size,
            type: file.type
        };
        return result;
    };
    FileReaderComponent.prototype.ReadFileUnmarshalledAsync = function (dotNetArrayPtr, readFileParamsPtr) {
        var _this = this;
        return new Promise(function (resolve, reject) {
            var readFileParams = JSON.parse(Blazor.platform.toJavaScriptString(readFileParamsPtr));
            var dotNetBuffer = { toUint8Array: function () { return Blazor.platform.toUint8Array(dotNetArrayPtr); } };
            var file = _this.fileStreams[readFileParams.fileRef];
            try {
                var reader = new FileReader();
                reader.onload = (function (r) {
                    return function () {
                        try {
                            var contents = r.result;
                            var dotNetBufferView = dotNetBuffer.toUint8Array();
                            dotNetBufferView.set(new Uint8Array(contents));
                            resolve(contents.byteLength);
                        }
                        catch (e) {
                            reject(e);
                        }
                    };
                })(reader);
                reader.readAsArrayBuffer(file.slice(readFileParams.position, readFileParams.position + readFileParams.count));
            }
            catch (e) {
                reject(e);
            }
        });
    };
    return FileReaderComponent;
}());
window.FileReaderComponent = new FileReaderComponent();
//# sourceMappingURL=FileReaderComponent.js.map