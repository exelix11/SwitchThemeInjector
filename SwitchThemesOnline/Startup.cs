using Blazor.FileReader;
using Microsoft.AspNetCore.Components.Builder;
using Microsoft.Extensions.DependencyInjection;
using Mono.WebAssembly.Interop;

namespace SwitchThemesOnline
{
	public class Startup
	{
		public void ConfigureServices(IServiceCollection services)
		{
			services.AddFileReaderService();
			services.AddSingleton<MonoWebAssemblyJSRuntime>();
		}

		public void Configure(IComponentsApplicationBuilder app)
		{
			app.AddComponent<App>("app");
		}
	}
}
