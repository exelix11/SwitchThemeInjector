using Blazor.FileReader;
using Microsoft.AspNetCore.Components.Builder;
using Microsoft.Extensions.DependencyInjection;

namespace SwitchThemesOnline
{
	public class Startup
	{
		public void ConfigureServices(IServiceCollection services)
		{
			services.AddFileReaderService();
		}

		public void Configure(IComponentsApplicationBuilder app)
		{
			app.AddComponent<App>("app");
		}
	}
}
