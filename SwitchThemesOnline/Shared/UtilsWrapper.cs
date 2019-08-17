using Microsoft.AspNetCore.Components;
using Microsoft.JSInterop;
using Mono.WebAssembly.Interop;
using System;
using System.Threading.Tasks;

public static class Exten
{
	public static async Task Click(this IJSRuntime runtime, ElementReference element) =>
		await runtime.InvokeAsync<object>("DomUtil.Click", element);

	public static async Task Alert(this IJSRuntime runtime, string message) =>
		await runtime.InvokeAsync<object>("alert", message);

	public static void DownloadFile(this MonoWebAssemblyJSRuntime js, string fileName, byte[] Data) =>
		js.InvokeUnmarshalled<string, byte[], object>("DownloadBlob", fileName, Data);

	public static async Task LegacyDownloadFile(this IJSRuntime js, string fileName, byte[] Data) =>
		await js.InvokeAsync<object>("DownloadUrl", fileName, "data:application/octet-stream;base64," + Convert.ToBase64String(Data));
}