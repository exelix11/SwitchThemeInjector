;
;
var FileReaderComponent = (function () {
    function FileReaderComponent() {
        var _this = this;
        this.assembly = "Blazor.FileReader";
        this.namespace = "Blazor.FileReader";
        this.className = "FileReaderJsInterop";
        this.newFileStreamReference = 0;
        this.fileStreams = {};
        this.dragElements = new Map();
        this.elementDataTransfers = new Map();
        this.RegisterDropEvents = function (element) {
            var handler = function (ev) {
                _this.PreventDefaultHandler(ev);
                if (ev.target instanceof HTMLElement) {
                    _this.elementDataTransfers.set(ev.target, ev.dataTransfer.files);
                }
            };
            _this.dragElements.set(element, handler);
            element.addEventListener("drop", handler);
            element.addEventListener("dragover", _this.PreventDefaultHandler);
            return true;
        };
        this.UnregisterDropEvents = function (element) {
            var handler = _this.dragElements.get(element);
            if (handler) {
                element.removeEventListener("drop", handler);
                element.removeEventListener("dragover", _this.PreventDefaultHandler);
            }
            _this.elementDataTransfers.delete(element);
            _this.dragElements.delete(element);
            return true;
        };
        this.GetFileCount = function (element) {
            var files = _this.GetFiles(element);
            if (!files) {
                return -1;
            }
            var result = files.length;
            return result;
        };
        this.ClearValue = function (element) {
            if (element instanceof HTMLInputElement) {
                element.value = null;
            }
            else {
                _this.elementDataTransfers.delete(element);
            }
            return 0;
        };
        this.GetFileInfoFromElement = function (element, index, property) {
            var files = _this.GetFiles(element);
            if (!files) {
                return null;
            }
            var file = files.item(index);
            if (!file) {
                return null;
            }
            return _this.GetFileInfoFromFile(file);
        };
        this.Dispose = function (fileRef) {
            return delete (_this.fileStreams[fileRef]);
        };
        this.OpenRead = function (element, fileIndex) {
            var files = _this.GetFiles(element);
            if (!files) {
                throw 'No FileList available.';
            }
            var file = files.item(fileIndex);
            if (!file) {
                throw "No file with index " + fileIndex + " available.";
            }
            var fileRef = _this.newFileStreamReference++;
            _this.fileStreams[fileRef] = file;
            return fileRef;
        };
        this.ReadFileUnmarshalledAsync = function (readFileParams) {
            return new Promise(function (resolve, reject) {
                if (!FileReaderComponent.getStreamBuffer) {
                    FileReaderComponent.getStreamBuffer =
                        Blazor.platform.findMethod(_this.assembly, _this.namespace, _this.className, "GetStreamBuffer");
                }
                var file = _this.fileStreams[readFileParams.fileRef];
                try {
                    var reader = new FileReader();
                    reader.onload = (function (r) {
                        return function () {
                            try {
                                var contents = r.result;
                                var dotNetBuffer = Blazor.platform.callMethod(FileReaderComponent.getStreamBuffer, null, [Blazor.platform.toDotNetString(readFileParams.callBackId.toString())]);
                                var dotNetBufferView = Blazor.platform.toUint8Array(dotNetBuffer);
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
        this.PreventDefaultHandler = function (ev) {
            ev.preventDefault();
        };
    }
    FileReaderComponent.prototype.GetFiles = function (element) {
        var files = null;
        if (element instanceof HTMLInputElement) {
            files = element.files;
        }
        else {
            var dataTransfer = this.elementDataTransfers.get(element);
            if (dataTransfer) {
                files = dataTransfer;
            }
        }
        return files;
    };
    FileReaderComponent.prototype.GetFileInfoFromFile = function (file) {
        var result = {
            lastModified: file.lastModified,
            name: file.name,
            size: file.size,
            type: file.type
        };
        return result;
    };
    return FileReaderComponent;
}());
window.FileReaderComponent = new FileReaderComponent();
//# sourceMappingURL=FileReaderComponent.js.map