Install the plugin by placing it in the Plugins folder in the project's root folder.

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


