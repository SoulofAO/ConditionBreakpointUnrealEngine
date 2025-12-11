Install the plugin by placing BlueprintDebugExtension Folder in the Plugins folder in the project's root folder.

Open Settings Here:

<img width="471" height="585" alt="Снимок экрана 2025-12-11 115339" src="https://github.com/user-attachments/assets/78fb539a-40ff-40b9-bb29-0768df5d5e70" />

Main Control Panel:

<img width="592" height="422" alt="image" src="https://github.com/user-attachments/assets/1e80ea7f-1a64-4229-ba9e-10a82221f2f6" />

Add Conditions:

<img width="874" height="513" alt="image" src="https://github.com/user-attachments/assets/8ee9cae7-f103-42e8-9153-eb09ae4a466d" />

Check Property Debug Condition - Condition based on Simple Checks for equality and other operators between Property and a constant or Property and another Property. There is an extension under GetName for UObject Ref.

<img width="821" height="456" alt="image" src="https://github.com/user-attachments/assets/27e59a85-b318-4fda-9292-43b3d9b41fc1" />

Function Binding Debug Condition - Links the Debug Point Condition execute call a Blueprint function.

<img width="746" height="151" alt="image" src="https://github.com/user-attachments/assets/de672db2-b93f-4273-b4cc-4eb0f537f8de" />

Reapet Debug Condition - Condition that allows you to set a condition based on the number of Breakpoint triggers.

<img width="532" height="178" alt="image" src="https://github.com/user-attachments/assets/3d3fb67f-352a-412a-8c37-f139360caa87" />

The plugin allows you to have custom Condition. To do this, inherit Blueprint from the Base Blueprint Debug Condition Class.

<img width="584" height="70" alt="image" src="https://github.com/user-attachments/assets/67df7195-a223-4c35-8901-58b30f187310" />

All Condition are Boolean conditions that can be mixed with each other. For this purpose, List Condition has And and Or as the simplest Boolean expression operations.

<img width="213" height="547" alt="Снимок экрана 2025-12-11 122547" src="https://github.com/user-attachments/assets/2fdce46f-fcbc-4240-97f5-378595585d94" />

Additional information for a specific Breakpoint can be expanded using the Custom Extender Context. 
In this case, you can get it for the Base Blueprint Extender Condition using Get New Global Custom Extender By Object. This allows you to create cumulative Breakpoint Conditions. 
You can find all the examples in the Content Plugin.


Copy right rools.
Requirement for Derivative Works.
Any derivative work, fork, or plugin whose primary purpose is adding, managing, evaluating, or otherwise operating conditional breakpoints in Unreal Engine Blueprints must be fully open source and distributed under terms compatible with this license.

Reuse for Other Primary Purposes.
Projects that reuse the code for a different primary purpose (i.e., when the main purpose is not related to conditional breakpoint handling in Blueprints) may be closed-source or commercial. Such usage must include clear attribution to the original project (see example below) and must preserve the LICENSE/NOTICE files in the distributed package.

Preservation of Author Files.
You may not remove or alter the original LICENSE, NOTICE files, or any existing author headers in the source files. All author attributions and license headers must remain intact in the source code and, where reasonable, in binary distributions.

“AS IS” and Warranty Disclaimer.
The code is provided “AS IS”. The authors provide no warranties, express or implied, including but not limited to warranties of merchantability or fitness for a particular purpose. The authors accept no liability for any damages or losses arising from the use of this code.

Right to Modify These Rules.
The plugin author(s) reserve the right to modify these rules in future versions. Any changes become effective upon publication and do not apply retroactively to previously released versions unless explicitly stated.
