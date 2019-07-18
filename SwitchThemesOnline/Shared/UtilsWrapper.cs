using Microsoft.AspNetCore.Components;
using Microsoft.JSInterop;
using System;
using System.Threading.Tasks;

public static class Exten
{
	public static async void Click(this IJSRuntime runtime, ElementRef element) =>
		await runtime.InvokeAsync<object>("DomUtil.Click", element);

	public static async void Alert(this IJSRuntime runtime, string message) =>
		await runtime.InvokeAsync<object>("alert", message);

	public static Task DownloadFile(this IJSRuntime runtime, string filename, byte[] data)
		=> runtime.InvokeAsync<object>("saveAsFile", filename, Convert.ToBase64String(data));
}