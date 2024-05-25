// Some copyright should be here...

using UnrealBuildTool;
using System;
public class XhSmartCity : ModuleRules
{
	public XhSmartCity(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		string WebSocketPluginPrivatePath = EngineDirectory + "/Plugins/Experimental/WebSocketNetworking/Source/WebSocketNetworking/Private";
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				//"D:/Program Files/Epic Games/UE_5.3/Engine/Plugins/Experimental/WebSocketNetworking/Source/WebSocketNetworking/Private"
				WebSocketPluginPrivatePath
            }
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
				"UMG",
				"WebSockets",
				"WebSocketNetworking",
				"HTTP",
				"Json",
				"JsonUtilities",
				"EnhancedInput",
				"InputCore",
				"WebBrowserWidget",
				"WebBrowser",
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
