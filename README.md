<p align="center">
  <strong>-------></strong>
  <a href="/README.ru.md">Русский</a> |
  <a href="/docs/README.md">English</a>
  <strong><-------</strong>
</p>

<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="./media/logo-dark.png">
    <img width="512" height="auto" alt="Project Logo" src="./media/logo-light.png">
  </picture>
</p>

---

<div align="center">

[![GitHub](https://img.shields.io/badge/GitHub-blue?style=flat&logo=github)](https://github.com/SoulofAO)
[![License](https://img.shields.io/badge/License-purple?style=flat&logo=github)](/LICENSE.md)
[![GitHub Stars](https://img.shields.io/github/stars/SoulofAO?style=flat&logo=github&label=Stars&color=orange)](https://github.com/SoulofAO)

</div>

<h1 align="center">
Test Integrate Conditional Breakpoint In Blueprint For Unreal Engine
</h1>

<h2 align="center">
    ⚠️ Disclaimer ⚠️
</h2>
<p align="center">
  The author is not responsible for any possible consequences resulting from the use of this project.
  By using the materials in this repository, you automatically agree to the terms of the associated license agreement.
</p>

<details>
  <summary align="center">⚠️ Full Text ⚠️</summary>

1. By using the materials in this repository, you automatically agree to the terms of the associated license agreement.

2. The author provides no warranties, express or implied, regarding the accuracy, completeness, or suitability of this material for any specific purpose.
3. The author shall not be liable for any losses, including but not limited to direct, indirect, incidental, consequential, or special damages, arising from the use or inability to use this material or its accompanying documentation, even if the possibility of such damages was previously communicated.

4. By using this material, you acknowledge and assume all risks associated with its application. Furthermore, you agree that the author cannot be held liable for any issues or consequences arising from its use.

</details>

* * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * *


<h1 align="center">📊 Installation</h1>

<h3 align="left">1. Select the folder where the plugin will be downloaded and copy the path to the file.</h3>

<div align="center" >
  <img style="width: 80%; height: auto; " alt="start_folder_cmd" src="./media/start_folder_cmd.png" />
</div>

<h3 align="left">2. Delete everything in the path field, enter the command <code>cmd</code>, and press <code>Enter</code>.</h3>
<div align="center" >
  <img style="width: 80%; height: auto;" alt="open cmd" src="./media/open_cmd.png"/>
</div>

<h3 align="left">3. A command console will open. Enter the command to download the repository:</h3>
<div>
  <pre><code>git clone https://github.com/SoulofAO/ConditionBreakpointUnrealEngine.git</code></pre>
</div>

<h3 align="left">4. Move the plugin <code>ConditionBreakpointUnrealEngine</code> to the <code>Plugins</code> folder of your project.</h3>


* * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * *

<h1 align="center">📊 Working with the Code</h1>

<h3 align="left">1. Right-click and open <code>Open Settings:</code> in the <code>CONDITIONAL BREAKPOINTS</code> section, select the item <code>OpenBreakpointConditionSettings</code>.</h3>

<div align="center" >
  <img style="width: 70; height: auto;" alt="Open Settings Here" src="./media/Open Settings Here.png"/>
</div>


<h3 align="left">2. The <code>Main Control Panel</code> will open. In the left corner, click the button to add conditions <code>Add Conditions:</code>.</h3>
<div align="center" >
  <img style="width: 70; height: auto;" alt="Main Control Panel" src="./media/Main Control Panel.png"/>
  <img style="width: 70; height: auto;" alt="Add Conditions" src="./media/Add Conditions.png"/>
</div>

* * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * *

<h1 align="center">List of Conditions</h1>
<div align="center" >
  <img style="width: 70; height: auto;" alt="Conditions" src="./media/Conditions.png"/>
</div>

---

<p align="left">
    1. <code> Check Property Debug Condition </code> – A condition based on simple checks, such as equality and other operators, between a property and a constant or between two properties. There is an extension for GetName for UObject Ref.
</p>

<div align="center" >
  <img style="width: 70; height: auto;" alt="Check Property Debug Condition" src="./media/Check Property Debug Condition.png"/>
</div>

---

<p align="left">
    2. <code> Function Binding Debug Condition </code> – Binds a breakpoint condition to a Blueprint function call.
</p>

<div align="center" >
  <img style="width: 70; height: auto;" alt="Check Property Debug Condition" src="./media/Function Binding Debug Condition.png"/>
</div>

---

<p align="left">
    3. <code> Repeat Debug Condition </code> – A condition that allows setting a check based on the number of times a breakpoint is triggered.
</p>

<div align="center" >
  <img style="width: 70; height: auto;" alt="Check Property Debug Condition" src="./media/Repeat Debug Condition.png"/>
</div>

---
---

<h1 align="center">📊 The plugin allows you to create <code>Custom Conditions</code>.</h1>

<p align="left">
    To do this, you need to inherit a <code>Blueprint</code> from the base class <code>Base Blueprint Debug Condition</code>.
</p>


<div align="center" >
  <img style="width: 70; height: auto;" alt="Condition" src="./media/Conditions_2.png"/>
</div>

---

<p align="left">
    All conditions are boolean and can be combined with each other. For this, the <code>List Condition</code> has <code>And</code> and <code>Or</code> operations as the simplest boolean expressions.
</p>

<div align="center" >
  <img style="width: 213; height: auto;" alt="Custom Condition" src="./media/Custom Condition.png"/>
</div>

---

<h2 align="center">
> 💡 Additional Information:
</h2>

<p align="left">
For a specific breakpoint, you can expand it using <code>Custom Extender Context</code>.
In this case, it can be obtained for <code>Base Blueprint Extender Condition</code> via <code>Get New Global Custom Extender By Object</code>. This allows creating cumulative conditions for a <code>Breakpoint</code>.
All examples can be found in the <code>Content Plugin</code>.
</p>

* * * * * * * * * * * * * * * * * *
* * * * * * * * * * * * * * * * * *

<h1 align="center"> 📜 License</h1>
<h2 align="center">
  <strong>-------></strong>
  <strong> This project is distributed under the </strong>
  <a href="./LICENSE">SoulofAO License</a>
  <strong><-------</strong>
</h1>

---

<h1 align="center">📬 Feedback</h1>
<p align="center">
If you encounter any issues or have suggestions — create an
<a href="https://github.com/SoulofAO/ConditionBreakpointUnrealEngine/issues">Issue</a>
or
<a href="https://github.com/SoulofAO/ConditionBreakpointUnrealEngine/pulls">Pull Request</a>
</p>


---

<h2 align="center">
📚 Documentation, please review it
</h2>

<p align="center">
  <strong>-------></strong>
  <a href="/docs/README.md">English</a> |
  <a href="/README.ru.md">Русский</a>
  <strong><-------</strong>
</p>
