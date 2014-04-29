class CfgPatches
{
	class remote_call
	{
		units[] = {};
		weapons[] = {};
		requiredAddons[] = {};
		fileName = "remote_call.pbo";
		author[]= {"Fank"};
	};
};

class CfgFunctions
{
	class RemoteCall
	{
		tag = "RC";
		class Core
		{
			file = "\remote_call\core";
			class init{};
		};
	};
};